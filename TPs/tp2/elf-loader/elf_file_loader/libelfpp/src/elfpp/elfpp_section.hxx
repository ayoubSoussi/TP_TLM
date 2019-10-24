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

#ifndef ELFPP_SECTION_HXX_
#define ELFPP_SECTION_HXX_

#include <stdexcept>

namespace elfpp
{

  void section::set_name(const std::string &name)
  {
    name_ = name;
  }

  const std::string & section::get_name() const
  {
    return name_;
  }

  sh_type_e section::get_type() const
  {
    return type_;
  }

  void section::set_flags(sh_flags_e flags)
  {
    flags_ = flags;
  }

  sh_flags_e section::get_flags() const
  {
    return flags_;
  }

  void section::set_vaddr(uint64_t vaddr)
  {
    vaddr_ = vaddr;
  }

  uint64_t section::get_vaddr() const
  {
    return vaddr_;
  }

  size_t section::get_size() const
  {
    return size_;
  }

  void section::set_align(uint32_t align)
  {
    align_ = align;
  }

  uint32_t section::get_align() const
  {
    return align_;
  }

  void section::set_link(section *s)
  {
    link_ = s;
    s->back_info_ = this;
  }

  section * section::get_link() const
  {
    return link_;
  }

  section * section::get_back_link() const
  {
    return back_link_;
  }

  void section::set_info(section *s)
  {
    info_ = s;
    if (s)
      s->back_info_ = this;
  }

  section * section::get_info() const
  {
    return info_;
  }

  section * section::get_back_info() const
  {
    return back_info_;
  }

  void section::set_info_last(unsigned int last)
  {
    info_last_ = last;
  }

  unsigned int section::get_info_last()
  {
    return info_last_;
  }

  void section::set_entsize(uint32_t entsize)
  {
    entsize_ = entsize;
  }

  uint32_t section::get_entsize() const
  {
    return entsize_;
  }

  uint8_t * section::get_content() const
  {
    return content_;
  }

  off_t section::get_file_offset() const
  {
    return offset_;
  }

  void section::set_file_offset(off_t a)
  {
    offset_ = a;
  }

  const sym_tab_map_t & section::get_symbol_table() const
  {
    return sym_tab_;
  }

  symbol & section::get_symbol(const std::string &name)
  {
    sym_tab_map_t::iterator i = sym_tab_.find(name);

    if (i == sym_tab_.end())
      throw std::runtime_error("no such symbol name");

    return *i->second;
  }

  object * section::get_object() const
  {
    return object_;
  }

}

#endif

