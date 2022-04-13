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

#include <kern/core/physmem.h>
#include <kern/console/console.h>

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
	 * XXX TODO maybe do some magic value based on
	 * physical address here!
	 */
	bool is_free;
	struct list_node node;
	paddr_t start; /* Start of free entry incl this header */
	paddr_t size; /* size of free entry incl this header */
};

static struct list_head kern_physmem_free_list;

static platform_spinlock_t kern_physmem_spinlock;


/**
 * Initialise the physical memory allocator.
 */
void
kern_physmem_init(void)
{
	list_head_init(&kern_physmem_free_list);
	platform_spinlock_init(&kern_physmem_spinlock);
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

	console_printf("[physmem] [freelist] adding 0x%x -> 0x%x (%d bytes)\n",
	    (uint32_t) start, (uint32_t) (start + size), size);

	e = (struct kern_physmem_free_entry *)(void *)(uintptr_t) start;
	e->start = start;
	e->size = size;
	e->is_free = true;
	list_node_init(&e->node);
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
	console_printf("[physmem] adding 0x%x -> 0x%x (%d bytes),"
	    " flags 0x%08x\n",
	    (uint32_t) start, (uint32_t) end, size, flags);

	if (num_kern_physmem_range_bootstrap_entries >=
	    KERN_PHYSMEM_NUM_BOOTSTRAP_REGIONS) {
		console_printf("[physmem] too many early regions\n");
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
		alloc_size = size;

		/* Figure out alignment, bump start as needed */
		if (alignment != 0) {
			uint32_t m;
			m = alignment - (alloc_start % alignment);
			alloc_start += m;
			alloc_size += m;
		}

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
			 */
			kern_physmem_add_to_free_list_locked(
			    e_start + alloc_size,
			    e_size - alloc_size);
			e_size = alloc_size;
		}

		console_printf("[physmem] [malloc] allocating 0x%x, %d bytes\n",
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
			console_printf("[physmem] TODO: ZERO\n");
			//kern_bzero((void *) e_start, e_size);
		}

		e = (void *) (((char *)alloc_start) - sizeof(*e));
		e->start = e_start;
		e->size = e_size;
		list_node_init(&e->node);
		retaddr = alloc_start;

		break;
	}

	platform_spinlock_unlock(&kern_physmem_spinlock);

	console_printf("[physmem] return 0x%08x\n", retaddr);

	return retaddr;
}
