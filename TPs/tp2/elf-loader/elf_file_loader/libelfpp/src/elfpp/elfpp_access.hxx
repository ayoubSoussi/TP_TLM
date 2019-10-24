/*
    This file is part of Libelfpp.

    Libelfpp is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    Libelfpp is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Libelfpp.  If not, see
    <http://www.gnu.org/licenses/>.

   Copyright (c) Alexandre Becoulet <alexandre.becoulet@free.fr>
*/

#ifndef ELFPP_ACCESS_HXX_
#define ELFPP_ACCESS_HXX_

#ifdef __linux__

# include <endian.h>

#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)

# include <machine/endian.h>
# define __BYTE_ORDER BYTE_ORDER
# define __LITTLE_ENDIAN LITTLE_ENDIAN
# define __BIG_ENDIAN BIG_ENDIAN

#else
# error Unable to guess endianness
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define SWAPVALUE ELFDATA2MSB
#elif __BYTE_ORDER == __BIG_ENDIAN
# define SWAPVALUE ELFDATA2LSB
#endif

namespace elfpp
{
 
  template <typename bits_t, int width>
  unsigned int elfn_access<bits_t, width>::rel_sym(uint64_t info)
  {
    if (width == 32)
      return info >> 8;
    else
      return info >> 32;
  }

  template <typename bits_t, int width>
  enum reloc_e elfn_access<bits_t, width>::rel_type(uint64_t info)
  {
    if (width == 32)
      return (enum reloc_e)(info & 0xff);
    else
      return (enum reloc_e)(info & 0xffffffff);
  }

  template <typename bits_t, int width>
  uint64_t elfn_access<bits_t, width>::rel_info(unsigned int sym, enum reloc_e type)
  {
    if (width == 32)
      return (sym << 8) | (type & 0xff);
    else
      return ((uint64_t)sym << 32) | type;
  }

  template <typename bits_t, int width>
  uint16_t elfn_access<bits_t, width>::swap(uint16_t x)
  {
    if (byteorder_ == SWAPVALUE)
      return (x << 8) | (x >> 8);

    return x;
  }

  template <typename bits_t, int width>
  int16_t elfn_access<bits_t, width>::swap(int16_t x)
  {
    return swap((uint16_t)x);
  }

  template <typename bits_t, int width>
  uint32_t elfn_access<bits_t, width>::swap(uint32_t x)
  {
    if (byteorder_ == SWAPVALUE)
      return (((x >> 24) & 0x000000ff) |
	      ((x >> 8 ) & 0x0000ff00) |
	      ((x << 8 ) & 0x00ff0000) |
	      ((x << 24) & 0xff000000));

    return x;
  }

  template <typename bits_t, int width>
  int32_t elfn_access<bits_t, width>::swap(int32_t x)
  {
    return swap((uint32_t)x);
  }

  template <typename bits_t, int width>
  uint64_t elfn_access<bits_t, width>::swap(uint64_t x)
  {
    if (byteorder_ == SWAPVALUE)
      return (((x >> 56) & 0x00000000000000ffLL) |
	      ((x >> 40) & 0x000000000000ff00LL) |
	      ((x >> 24) & 0x0000000000ff0000LL) |
	      ((x >>  8) & 0x00000000ff000000LL) |
	      ((x <<  8) & 0x000000ff00000000LL) |
	      ((x << 24) & 0x0000ff0000000000LL) |
	      ((x << 40) & 0x00ff000000000000LL) |
	      ((x << 56) & 0xff00000000000000LL));

    return x;
  }

  template <typename bits_t, int width>
  int64_t elfn_access<bits_t, width>::swap(int64_t x)
  {
    return swap((uint64_t)x);
  }

  template <typename bits_t, int width>
  template <typename X, typename Y>
  void elfn_access<bits_t, width>::swap(X *a, Y x)
  {
    *a = swap((X)x);
  }

}

#endif

