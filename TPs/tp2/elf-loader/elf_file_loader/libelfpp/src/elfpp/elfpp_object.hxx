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

#ifndef ELFPP_OBJECT_HXX_
#define ELFPP_OBJECT_HXX_

#include <stdexcept>

namespace elfpp
{

  symbol & object::get_symbol(const std::string &name)
  {
    sym_tab_map_t::iterator i = sym_tab_.find(name);

    if (i == sym_tab_.end())
      throw std::runtime_error("no such symbol name");

    return *i->second;
  }

  const sym_tab_map_t & object::get_symbol_table() const
  {
    return sym_tab_;
  }

  dpp::linked_list<section> & object::get_section_table()
  {
    return section_tab_;
  }

  dpp::linked_list<segment> & object::get_segment_table()
  {
    return segment_tab_;
  }

}

#endif

