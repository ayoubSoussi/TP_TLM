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

#ifndef ELFPP_HH_
#define ELFPP_HH_

#include <map>

#include <dpp/linked_list>

namespace elfpp
{
  class symbol;

  typedef std::multimap<const std::string, symbol *> sym_tab_map_t;

  class reloc;

  typedef dpp::linked_list<reloc, 0> reloc_table_t;
  typedef dpp::linked_list<reloc, 1> reloc_modlist_t;

  class section;
  typedef dpp::linked_list<section> section_table_t;

  class segment;
  typedef dpp::linked_list<segment> segment_table_t;
}

#endif

