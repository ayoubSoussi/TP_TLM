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

#ifndef ELFPP_SYMBOL_HXX_
#define ELFPP_SYMBOL_HXX_

namespace elfpp
{

  const std::string & symbol::get_name() const
  {
    return name_;
  }

  section * symbol::get_section() const
  {
    return section_;
  }

  void symbol::set_section_ndx(unsigned int ndx)
  {
    sec_ndx_ = ndx;
  }

  unsigned int symbol::get_section_ndx() const
  {
    return sec_ndx_;
  }

  uint8_t symbol::get_info() const
  {
    return info_;
  }

  void symbol::set_info(uint8_t info)
  {
    info_ = info;
  }

  uint8_t symbol::get_other() const
  {
    return other_;
  }

  void symbol::set_other(uint8_t other)
  {
    other_ = other;
  }

  uint64_t symbol::get_value() const
  {
    return value_;
  }

  size_t symbol::get_size() const
  {
    return size_;
  }

  uint8_t * symbol::get_content() const
  {
    return content_;
  }

  void symbol::set_value(uint64_t val)
  {
    value_ = val;
  }

  void symbol::set_size(size_t size)
  {
    size_ = size;
  }

  const reloc_table_t & symbol::get_reloc_table() const
  {
    return reloc_tab_;    
  }

  reloc_table_t & symbol::get_reloc_table()
  {
    return reloc_tab_;    
  }

  st_info_type_e symbol::get_type() const
  {
    return ELF_ST_TYPE(info_);
  }

  void symbol::set_type(st_info_type_e type)
  {
    info_ = ELF_ST_INFO(get_bind(), type);
  }

  st_info_bind_e symbol::get_bind() const
  {
    return ELF_ST_BIND(info_);
  }

  void symbol::set_bind(st_info_bind_e bind)
  {
    info_ = ELF_ST_INFO(bind, get_type());
  }

}

#endif

