#include <stddef.h>
#include <stdint.h>

#include <hw/types.h>
#include <hw/vtor.h>
#include <hw/scb_defs.h>

/**
 * Initialise the vector table to the in-memory one that we're
 * allocating.
 *
 * This should only be linked in and called if a dynamic vector
 * table is required.  For truely embedded platforms with limited
 * RAM, using 1KiB of RAM just for this table is wasteful.
 */
