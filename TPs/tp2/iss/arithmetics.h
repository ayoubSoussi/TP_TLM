/* -*- c++ -*-
 *
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
#ifndef SOCLIB_COMMON_ARITHMETICS_H
#define SOCLIB_COMMON_ARITHMETICS_H

#include <stdint.h>

namespace soclib { namespace common {

static inline int32_t sign_ext( uint32_t val, unsigned int len )
{
    if (__builtin_constant_p(len)) {
	switch (len) {
	case 8:
	    return (int8_t)val;
	case 16:
	    return (int16_t)val;
	}
    }

    uint32_t mask = 1 << (len - 1);
    return (val & (mask - 1)) | (-(val & mask));
}

/** compute ADD carry only */
static inline bool carry( uint32_t a, uint32_t b, uint32_t c )
{
    return ((uint64_t)a+(uint64_t)b+(uint64_t)c)>>32;
}

/** compute ADD overflow only */
static inline bool overflow( uint32_t a, uint32_t b, uint32_t c )
{
    return ((b^(a+b+c))&~(a^b))>>31;
}

/** compute ADD result, carry and overflow */
static inline uint32_t add_cv(uint32_t a, uint32_t b, bool cin, bool &cout, bool &vout)
{
#if defined(__x86_64__) || defined(__i386__)
    uint32_t result;
    uint8_t cout_, vout_;

    if (__builtin_constant_p(cin) && !cin) {
	asm("       addl %k3, %k4      \n"
	    "       setc %b1          \n"
	    "       seto %b2          \n"
	    : "=r" (result)
	    , "=qm" (cout_)
	    , "=qm" (vout_)
	    : "r" (a)
	    , "0" (b)
	    );
    } else {
	asm("       btl $0, %k5        \n"
	    "       adcl %k3, %k4      \n"
	    "       setc %b1          \n"
	    "       seto %b2          \n"
	    : "=r" (result)
	    , "=qm" (cout_)
	    , "=qm" (vout_)
	    : "r" (a)
	    , "0" (b)
	    , "r" (cin)
	    );
    }

    cout = cout_;
    vout = vout_;
    return result;
#else
    cout = ((uint64_t)a+(uint64_t)b+(uint64_t)cin)>>32;
    vout = ((b^(a+b+cin))&~(a^b))>>31;
    return a + b + cin;
#endif
}

template<typename T>
static inline T popcount(T n)
{
    return __builtin_popcount(n);
}

static inline int uint32_log2(uint32_t n)
{
    if ( n == 0 )
	return -1;
    return 31 - __builtin_clz(n);
}

template<typename T>
static inline T extract_bits( T word, int le_bit, int size )
{
    return ((1<<size)-1) & (word>>le_bit);
}

template<typename T>
static inline T insert_bits( T word, T to_insert, int le_bit, int size )
{
    T mask = ((1<<size)-1)<<le_bit;
    T ti = to_insert << le_bit;
    return (ti & mask) | (word & ~mask);
}

template<typename T>
static inline int clz( T n )
{
#if __GNUC__ && ( __GNUC__ > 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ >= 4 ) )
    if ( sizeof(T) == sizeof(unsigned int) )
	return __builtin_clz(n);
    else if ( sizeof(T) == sizeof(unsigned long) )
	return __builtin_clzl(n);
    else
#endif
    {
	const int t = sizeof(T)*8;
	for ( int i = t-1; i>=0; --i )
	    if ( (1<<i)&n )
		return t-1-i;
	return t;
    }
}

template<typename T>
static inline int fls( T n )
{
    if (n == 0)
	return 0;
    return 8*sizeof(n)-clz(n);
}

template<typename T>
static inline int ctz( T n )
{
#if __GNUC__ && ( __GNUC__ > 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ >= 4 ) )
    if ( sizeof(T) == sizeof(unsigned int) )
	return __builtin_ctz(n);
    else if ( sizeof(T) == sizeof(unsigned long) )
	return __builtin_ctzl(n);
    else
#endif
    {
	const int t = sizeof(T)*8;
	for ( int i = 0; i<t; ++i )
	    if ( (1<<i)&n )
		return i;
	return 0;
    }
}

template<typename T>
static inline int clo( T n )
{
    return clz(~n);
}

template<typename T>
static inline int cto( T n )
{
    return ctz(~n);
}

template<typename T>
static inline T clamp( T min, T n, T max )
{
    if ( n < min )
	return min;
    if ( n > max )
	return max;
    return n;
}



}}

#endif /* SOCLIB_COMMON_ARITHMETICS_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

