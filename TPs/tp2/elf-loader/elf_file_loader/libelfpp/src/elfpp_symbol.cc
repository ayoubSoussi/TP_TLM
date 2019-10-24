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

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <dpp/foreach>

#include <elfpp/symbol>
#include <elfpp/section>
#include <elfpp/reloc>

namespace elfpp
{

  symbol::symbol(const std::string &name)
    : name_(name),
      value_(0),
      size_(0),
      info_(0),
      other_(0),
      section_(0),
      sec_ndx_(0),
      reloc_tab_(),
      content_(0)
  {
  }

  symbol::~symbol()
  {
    if (content_)
      std::free(content_);

    FOREACH2(s, reloc_tab_)
      delete &*s;
  }

  void symbol::set_section(section &sec)
  {
    section_ = &sec;
  }

  void symbol::set_content(uint8_t *data)
  {
    content_ = (uint8_t*)std::realloc(content_, size_);
    std::memcpy(content_, data, size_);
  }

  void symbol::add_reloc(reloc &r)
  {
    reloc_tab_.push_back(r);
  }

  void symbol::remove_reloc(reloc &r)
  {
    r.reloc_table_t::item_type::remove();
  }

  std::ostream & operator<<(std::ostream &o, const symbol &s)
  {
    o << "[" << s.name_ << " val:" << s.value_ << " size:" << s.size_ << "]";
    return o;
  };

}

