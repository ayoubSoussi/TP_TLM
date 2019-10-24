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
#include <stdlib.h>
#include <cstring>

#include <dpp/foreach>

#include <elfpp/section>
#include <elfpp/object>
#include <elfpp/segment>
#include <elfpp/symbol>
#include <elfpp/reloc>

namespace elfpp
{

  section::section(object &obj, sh_type_e type)
    : name_(),
      type_(type),
      flags_(SHF_NONE),
      vaddr_(0),
      size_(0),
      link_(0),
      info_(0),
      align_(0),
      entsize_(0),
      index_(0),
      back_info_(0),
      back_link_(0),
      object_(&obj),
      content_(0),
      sym_tab_()
  {
  }

  section::~section()
  {
    if (!orphan())
      remove();

    if (content_)
      free(content_);

    object_->secidx_[index_] = 0;

    FOREACH(s, sym_tab_)
      delete s->second;
  }

  void section::set_size(size_t size)
  {
    if (type_ != SHT_NOBITS && size_ != size)
      {
	content_ = (uint8_t*)realloc(content_, size);

	assert(content_ || size == 0);

	if (size > size_)
	  std::memset(content_ + size_, 0, size - size_);
      }

    size_ = size;
  }

  void section::add_symbol(symbol &sym)
  {
    sym_tab_.insert(sym_tab_map_t::value_type(sym.get_name(), &sym));
  }

  symbol & section::get_symbol(uint64_t offset)
  {
    symbol *res = 0;

    FOREACH(i, get_symbol_table())
      {
	symbol *j = i->second;

	if (!j->get_size())
	  continue;

	if (j->get_value() <= offset && (!res || j->get_value() > res->get_value()))
	  res = j;
      }

    if (!res)
      throw std::runtime_error("no symbol found");

    if (offset - res->get_value() >= res->get_size())
      throw std::runtime_error("offset above symbol size");

    return *res;
  }

  segment * section::get_segment() const
  {
    if (!object_)
      throw std::runtime_error("section is not part of an object");

    // find associated segment based on section content in elf file
    FOREACH(s, object_->get_segment_table())
      {
	if (get_file_offset() >= s->get_file_offset() &&
	    get_file_offset() < s->get_file_offset() + (off_t)s->get_file_size())
	  return &*s;
      }

    return 0;
  }

  uint64_t section::get_load_address() const
  {
    segment *s = get_segment();

    if (!s)
      throw std::runtime_error("section is not located in a segment");

    return vaddr_ - s->get_vaddr() + s->get_paddr();
  }

  std::ostream & operator<<(std::ostream &o, const section &s)
  {
    o << "[" << s.name_ << " addr:0x" << std::hex << s.vaddr_ << " size:" << std::dec << s.size_ << "]";
    return o;
  }

}

