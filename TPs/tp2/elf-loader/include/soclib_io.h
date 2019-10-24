/*
 * SOCLIB_LGPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU LGPLv2.1.
 *
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_IO__H
#define SOCLIB_IO__H

#include <stdint.h>

#define DO_MEMORY_SYNC asm volatile("" ::: "memory")

static inline uint32_t __uint32_swap(uint32_t x)
{
    return (
	( (x & 0xff)   << 24 ) |
	( (x & 0xff00) <<  8 ) |
	( (x >>  8) & 0xff00 ) |
	( (x >> 24) &   0xff )
	);
}

#if defined(__MICROBLAZE__) || defined(MIPSEB)
#define SOCLIB_IO_BIG_ENDIAN
#else
#define SOCLIB_IO_LITTLE_ENDIAN
#endif

static inline void soclib_io_set(void *comp_base, size_t reg, uint32_t val)
{
	volatile uint32_t *addr = (uint32_t *)comp_base;

#if __PPC__
    reg <<= 2;
    asm("stwbrx %0, %1, %2":: "b"(val), "b"(addr), "b"(reg) : "memory" );
#else
	addr += reg;
#ifdef SOCLIB_IO_BIG_ENDIAN
    val = __uint32_swap(val);
#endif
	*addr = val;
    DO_MEMORY_SYNC;
#endif
}

static inline uint32_t soclib_io_get(void *comp_base, size_t reg)
{
	volatile uint32_t *addr = (uint32_t *)comp_base;
    uint32_t val;

#if __PPC__
    reg <<= 2;
    asm("lwbrx %0, %1, %2": "=b"(val): "b"(addr), "b"(reg) );
    return val;
#else
	addr += reg;
    val = *addr;
#ifdef SOCLIB_IO_BIG_ENDIAN
    val = __uint32_swap(val);
#endif
    DO_MEMORY_SYNC;
	return val;
#endif
}

static inline void soclib_io_write8(void *comp_base, size_t reg, uint8_t val)
{
	volatile uint32_t *addr = (uint32_t *)comp_base;
	addr += reg;

	*(uint8_t *)addr = val;
    DO_MEMORY_SYNC;
}

static inline uint8_t soclib_io_read8(void *comp_base, size_t reg)
{
	volatile uint32_t *addr = (uint32_t *)comp_base;
	addr += reg;

    DO_MEMORY_SYNC;
	return *(uint8_t *)addr;
}

#endif /* SOCLIB_IO__H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

