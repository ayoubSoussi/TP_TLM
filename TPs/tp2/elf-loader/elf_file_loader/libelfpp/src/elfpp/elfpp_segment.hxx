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

#ifndef ELFPP_SEGMENT_HXX_
#define ELFPP_SEGMENT_HXX_

namespace elfpp
{

  segment::segment(object &obj)
  {
  }

  segment::~segment()
  {
  }

  p_type_e segment::get_type() const
  {
    return type_;
  }

  void segment::set_type(p_type_e e)
  {
    type_ = e;
  }

  p_flags_e segment::get_flags() const
  {
    return flags_;
  }

  void segment::set_flags(p_flags_e e)
  {
    flags_ = e;
  }

  uint64_t segment::get_vaddr() const
  {
    return vaddr_;
  }

  void segment::set_vaddr(uint64_t a)
  {
    vaddr_ = a;
  }

  uint64_t segment::get_paddr() const
  {
    return paddr_;
  }

  void segment::set_paddr(uint64_t a)
  {
    paddr_ = a;
  }

  size_t segment::get_mem_size() const
  {
    return mem_size_;
  }

  void segment::set_mem_size(size_t a)
  {
    mem_size_ = a;
  }

  size_t segment::get_file_size() const
  {
    return file_size_;
  }

  void segment::set_file_size(size_t a)
  {
    file_size_ = a;
  }

  off_t segment::get_file_offset() const
  {
    return offset_;
  }

  void segment::set_file_offset(off_t a)
  {
    offset_ = a;
  }

  size_t segment::get_align() const
  {
    return align_;
  }

  void segment::set_align(size_t a)
  {
    align_ = a;
  }

}

#endif

