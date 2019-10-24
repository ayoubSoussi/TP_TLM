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

#include <elfpp/reloc>
#include <elfpp/symbol>
#include <elfpp/section>

namespace elfpp
{

  reloc::reloc()
    : sym_mod_(0),
      sym_(0),
      type_(R_NONE),
      addend_(0)
  {
  }

  reloc::~reloc()
  {
    if (!reloc_table_t::item_type::orphan())
      reloc_table_t::item_type::remove();

    if (!reloc_modlist_t::item_type::orphan())
      reloc_modlist_t::item_type::remove();
  }

  std::ostream & operator<<(std::ostream &o, const reloc &r)
  {
    o << "[sym:" << *r.sym_ << " sec:" << *r.sec_ << " offset:" << r.offset_ << "]";
    return o;
  };

}

