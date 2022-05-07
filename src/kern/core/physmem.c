/*
 * Copyright (C) 2022 Adrian Chadd <adrian@freebsd.org>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-Licence-Identifier: GPL-3.0-or-later
 */

#include <stddef.h>
#include <stdbool.h>

#include <hw/types.h>

#include <core/platform.h>
#include <core/lock.h>

#include <kern/libraries/container/container.h>
#include <kern/libraries/list/list.h>

#include <kern/core/exception.h>
#include <kern/core/physmem.h>

#include <kern/core/logging.h>

LOGGING_DEFINE(LOG_PHYSMEM, "physmem", KERN_LOG_LEVEL_NOTICE);

/**
 * A static set of physical memory regions to use during early system
 * bootstrapping.
 *
 * Ideally we'd bring up a memory region in early boot first that
 * we can make kernel allocations from for tasks, stacks, lists of
 * other regions, etc.  Once that's done then we can (hopefully!)
 * do other work to add even more potentially usable memory regions
 * here.
 */

static struct kern_physmem_range
    kern_physmem_range_bootstrap[KERN_PHYSMEM_NUM_BOOTSTRAP_REGIONS] = { 0 };
uint32_t num_kern_physmem_range_bootstrap_entries;

struct kern_physmem_free_entry {
	/*
	 * Magic value based on address and start and size.
	 */
	uint32_t magic;
	bool is_free;
	struct list_node node;
	paddr_t start; /* Start of free entry incl this header */
	paddr_t size; /* size of free entry incl this header */
};

static struct list_head kern_physmem_free_list;

static platform_spinlock_t kern_physmem_spinlock;

static uint32_t
kern_physmem_magic_calculate(const struct kern_physmem_free_entry *e)
{
	uint32_t magic;

	magic = 0x9e3779b9;
	magic ^= e->start;
	magic ^= e->size;

	return (magic);
}

static bool
kern_physmem_magic_verify(const struct kern_physmem_free_entry *e)
{
	uint32_t magic;

	magic = kern_physmem_magic_calculate(e);
	return (magic == e->magic);
}

/**
 * Initialise the physical memory allocator.
 */
void
kern_physmem_init(void)
{
	list_head_init(&kern_physmem_free_list);
	platform_spinlock_init(&kern_physmem_spinlock);
}

static struct kern_physmem_free_entry *
kern_physmem_init_memory_region_node(paddr_t start, paddr_t size)
{
	struct kern_physmem_free_entry *e;

	e = (struct kern_physmem_free_entry *)(void *)(uintptr_t) start;
	e->start = start;
	e->size = size;
	e->is_free = true;
	e->magic = kern_physmem_magic_calculate(e);
	list_node_init(&e->node);

	return (e);
}

/**
 * Add entry to free list.
 *
 * This must be called with the physmem spinlock held.
 */
static void
kern_physmem_add_to_free_list_locked(paddr_t start, paddr_t size)
{
	struct kern_physmem_free_entry *e;

	KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_DEBUG,
	    "[freelist] adding 0x%x -> 0x%x (%d bytes)",
	    (uint32_t) start, (uint32_t) (start + size), size);

	e = kern_physmem_init_memory_region_node(start, size);
	e->is_free = true;

	list_add_tail(&kern_physmem_free_list, &e->node);
}

/**
 * Add the physical memory range between 'start' and 'end'-1.
 *
 * This adds the physical memory region and some flags representing
 * whether it's usable or not.
 *
 * @param[in] start start address
 * @param[in] end end address
 * @param[in] flags flags for region
 */
void
kern_physmem_add_range(paddr_t start, paddr_t end, uint32_t flags)
{
	paddr_t size;

	/*
	 * XXX TODO: yeah, it's cast to uint32_t for printing
	 * because I don't have a 64 bit printing type to use.
	 */
	size = end - start;
	KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_DEBUG,
	    "adding 0x%x -> 0x%x (%d bytes),"
	    " flags 0x%08x",
	    (uint32_t) start, (uint32_t) end, size, flags);

	if (num_kern_physmem_range_bootstrap_entries >=
	    KERN_PHYSMEM_NUM_BOOTSTRAP_REGIONS) {
		KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_CRIT,
		    "too many early regions");
		return;
	}
	kern_physmem_range_bootstrap[num_kern_physmem_range_bootstrap_entries].start = start;
	kern_physmem_range_bootstrap[num_kern_physmem_range_bootstrap_entries].end = end;
	kern_physmem_range_bootstrap[num_kern_physmem_range_bootstrap_entries].flags = flags;
	num_kern_physmem_range_bootstrap_entries++;

	/*
	 * For now we only add entries that aren't marked
	 * as reserved.  If we start wanting to do reserved
	 * regions INSIDE other regions then I'll have to rethink
	 * how we populate the initial free list.
	 *
	 * Later on we'll want to figure out how to do SDRAM
	 * population here for specific kinds of allocations
	 * (ie, where we can know we can shoot the user/kern
	 * tasks in the head and reclaim before we turn off
	 * an SDRAM bank.)
	 */
	if (flags == (KERN_PHYSMEM_FLAG_NORMAL | KERN_PHYSMEM_FLAG_SRAM)) {
		platform_spinlock_lock(&kern_physmem_spinlock);
		kern_physmem_add_to_free_list_locked(start, size);
		platform_spinlock_unlock(&kern_physmem_spinlock);
	}
}

/**
 * Allocate a block of physical memory of the given size, alignment and
 * flags.
 *
 * This function is non blocking.  If there is no RAM available then
 * the function will return '0' as an allocation value and the caller
 * must deal with the allocation failing.
 *
 * This is also super, super inefficient.  I'm going to use this
 * for bootstrapping initial user/kernel tasks and some IPC, but
 * when we want to start doing actual aligned allocations we'll
 * need to change this to be an arena allocator, not a freelist
 * allocator otherwise a /lot/ of RAM is going to be wasted
 * trying to do large alignments.
 *
 * Also yes, this code DOES currently assume unaligned access
 * for all of those metadata nodes is OK.  Maybe on this M4,
 * but not for other CPU types and we should be careful assuming
 * this in the future.  Again, this is purely for bring-up.
 */
paddr_t
kern_physmem_alloc(size_t size, uint32_t alignment, uint32_t flags)
{
	struct kern_physmem_free_entry *e;
	struct list_node *n;
	paddr_t retaddr = 0;

	KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_INFO,
	    "[alloc] called, size=%d alignment=%d flags=0x%08x",
	    (int) size, alignment, flags);

	/*
	 * Ensure we're not being asked for tiny allocations.
	 * Those should use a zone allocator API.
	 */
	if (size < KERN_PHYSMEM_MINIMUM_ALLOCATION_SIZE)
		return (0);

	platform_spinlock_lock(&kern_physmem_spinlock);

	/*
	 * I'm going to just find the first sized block here
	 * and allocate it.  Different allocation schemes
	 * can come way, way later if needed.
	 */
	for (n = kern_physmem_free_list.head; n != NULL; n = n->next) {
		uintptr_t alloc_start, alloc_size, e_start, e_size;

		e = container_of(n, struct kern_physmem_free_entry, node);

#if 1
		if (! kern_physmem_magic_verify(e)) {
			KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_CRIT,
			    "[alloc] magic failed\n");
		}
#endif

		/*
		 * If we want to consider this block as a valid block
		 * for allocation/fragmentation, a few things need to happen:
		 *
		 * It needs to be large enough to hold the allocation
		 * request, with the metadata node in front of it /and/
		 * needed padding.
		 *
		 * If we find a suitable block then we'll take it off
		 * the free list and request it be fragmented into
		 * two entries - one of the relevant size, and anything
		 * left over returned to the freelist.  If we're within
		 * KERN_PHYSMEM_MINIMUM_ALLOCATION_SIZE bytes then
		 * don't bother fragmenting; just return the whole block.
		 *
		 * Since we may be requesting aligned data, the allocator
		 * will put the header /just before/ the allocation, and
		 * that header will include the true start/end of the
		 * whole block of memory being allocated so when it's
		 * freed it can be properly freed back into the free list.
		 */

		/* Start with 'start' pointing to e + 1 */
		alloc_start = (uintptr_t)(e + 1);
		alloc_size = size + sizeof(*e);

		KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_DEBUG,
		    "e=0x%x, alloc_start=0x%08x, size=%d",
		    e, alloc_start, alloc_size);

		/* Figure out alignment, bump start as needed */
		if (alignment != 0) {
			uint32_t m;
			m = alignment - (alloc_start % alignment);
			alloc_start += m;
			alloc_size += m;
		}

		KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_DEBUG,
		    "post alignment: alloc_start=0x%08x, size=%d",
		    alloc_start, alloc_size);

		/*
		 * See if the allocation fits within the range
		 * of the freelist entry. Don't check if we have
		 * space for fragmenting the leftover space; we'll
		 * worry about that later.
		 */
		if (alloc_size > e->size) {
			continue;
		}

		/*
		 * Ok, we have a freelist entry that we fit into.
		 * Let's take this entry off the freelist, figure
		 * out how much space is /left/ and whether we
		 * want to fragment this freelist entry or not.
		 */
		list_delete(&kern_physmem_free_list, &e->node);

		/*
		 * Store our freelist start/size here so we can use it
		 * after we clear 'e'.
		 */
		e_start = e->start;
		e_size = e->size;

		/*
		 * See if our allocation size leaves enough data to
		 * be usefully fragmented.  If it does then fragment
		 * what we have left and update e_start/e_size
		 * appropriately.
		 */
		if ((e_size - alloc_size) >
		    (KERN_PHYSMEM_MINIMUM_ALLOCATION_SIZE + sizeof(*e))) {
			/*
			 * Calculate what's left over, adjust e_size
			 * appropriately and add the rest to the freelist.
			 *
			 * XXX TODO: this needs to be O(n) inserted back
			 * into the right place, or a special function
			 * to specifically fragment it in-place.
			 * This will add the fragmented block into the
			 * end of the freelist, which isn't what we
			 * want.
			 */
			kern_physmem_add_to_free_list_locked(
			    e_start + alloc_size,
			    e_size - alloc_size);
			e_size = alloc_size;
		}

		KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_DEBUG,
		    "[malloc] allocating 0x%x, %d bytes",
		    (uint32_t) e_start, e_size);

		/*
		 * e_start/e_size are our actual allocated block
		 * size now.  stuff it in the metadata JUST before
		 * our allocation spot; we've already advanced
		 * that past sizeof(*e) + alignment needs.
		 * Zero the whole thing if it's requested, including
		 * any extra padding.
		 */

		if (flags & KERN_PHYSMEM_ALLOC_FLAG_ZERO) {
			/*
			 * XXX TODO: we don't have a bzero() in our library
			 * yet
			 */
			KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_CRIT,
			    "TODO: ZERO");
			//kern_bzero((void *) e_start, e_size);
		}

		e = (void *) (((char *)alloc_start) - sizeof(*e));
		e->start = e_start;
		e->size = e_size;
		e->magic = kern_physmem_magic_calculate(e);
		e->is_free = false;
		list_node_init(&e->node);
		retaddr = alloc_start;

		break;
	}

	platform_spinlock_unlock(&kern_physmem_spinlock);

	KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_INFO,
	    "[alloc] return 0x%08x",
	    retaddr);

	return retaddr;
}

/**
 * Free the given physical memory block; coalesce with blocks on either
 * side at some point.
 *
 * This is an O(n) free as we need to find the right spot in the freelist
 * to insert so we can then coalesce.
 */
void
kern_physmem_free(paddr_t addr)
{
	struct kern_physmem_free_entry *e;
	struct list_node *n;

	platform_spinlock_lock(&kern_physmem_spinlock);
	/* Get the metadata */
	e = (void *) (uintptr_t) addr;
	e = e - 1;

	if (! kern_physmem_magic_verify(e)) {
		KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_CRIT,
		    "[free] magic failed");
	}

	KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_INFO,
	    "[free] addr 0x%x, block=0x%x, %d bytes",
	    addr, e->start, e->size);

	/*
	 * Find where to put the block in the list so we can eventually
	 * coalesce.
	 */
	for (n = kern_physmem_free_list.head; n != NULL; n = n->next) {
		struct kern_physmem_free_entry *ee;

		ee = container_of(n, struct kern_physmem_free_entry, node);

		if (! kern_physmem_magic_verify(ee)) {
			KERN_LOG(LOG_PHYSMEM, KERN_LOG_LEVEL_CRIT,
			    "magic failed");
		}

		if (ee->start > e->start)
			break;
	}

	/*
	 * The node itself may not be /at/ the beginning of the
	 * memory region due to alignment.
	 * So, we need to reinitialise it back at the beginning.
	 */
	e = kern_physmem_init_memory_region_node(e->start, e->size);
	e->is_free = true;

	if (n == NULL) {
		list_add_head(&kern_physmem_free_list, &e->node);
	} else {
		list_add_before(&kern_physmem_free_list, n, &e->node);
	}

	/* See if we can coalesce! */
	/* XXX TODO - also need to fix the fragmenting logic in kern_physmem_alloc() */

	platform_spinlock_unlock(&kern_physmem_spinlock);
}
