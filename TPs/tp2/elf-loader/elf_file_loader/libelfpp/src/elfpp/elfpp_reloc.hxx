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

namespace elfpp
{

  void reloc::set_mangled_symbol(symbol *s)
  {
    sym_mod_ = s;
  }

  symbol * reloc::get_mangled_symbol() const
  {
    return sym_mod_;
  }

  void reloc::set_symbol(symbol *s)
  {
    sym_ = s;
  }

  symbol * reloc::get_symbol() const
  {
    return sym_;
  }

  void reloc::set_section(section *s)
  {
    sec_ = s;
  }

  section * reloc::get_section() const
  {
    return sec_;
  }

  void reloc::set_type(enum reloc_e type)
  {
    type_ = type;
  }

  enum reloc_e reloc::get_type() const
  {
    return type_;
  }

  void reloc::set_addend(int64_t addend)
  {
    addend_ = addend;
  }

  int64_t reloc::get_addend() const
  {
    return addend_;
  }

  void reloc::set_offset(uint64_t offset)
  {
    offset_ = offset;
  }

  uint64_t reloc::get_offset() const
  {
    return offset_;
  }

}

