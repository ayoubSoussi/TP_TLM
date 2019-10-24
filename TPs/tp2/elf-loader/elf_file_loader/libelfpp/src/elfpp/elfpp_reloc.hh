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

#ifndef ELFPP_RELOC_HH_
#define ELFPP_RELOC_HH_

#include <iostream>

#include <dpp/linked_list>

#include "elfpp.hh"
#include "elfpp_bits.hh"

namespace elfpp
{

  class reloc;
  class symbol;
  class section;
  template <typename bits_t, int width> class elfn_access;

  std::ostream & operator<<(std::ostream &o, const reloc &v);

  /** 
      @short ELF relocation class
      @header elfpp/reloc
   */
  class reloc : public reloc_table_t::item_type,
		public reloc_modlist_t::item_type
  {
    friend class section;
    friend class symbol;
    template <typename, int> friend class elfn_access;
    friend std::ostream & operator<<(std::ostream &o, const reloc &v);

  public:
    reloc();
    ~reloc();

    /** Set pointer to mangled symbol. Relocation offset become
	relative to mangled symbol value when set. */
    inline void set_mangled_symbol(symbol *s);
    /** Get pointer to mangled symbol. */
    inline symbol * get_mangled_symbol() const;

    /** Set relocation symbol */
    inline void set_symbol(symbol *s);
    /** Get relocation symbol */
    inline symbol * get_symbol() const;

    /** Set relocation section */
    inline void set_section(section *s);
    /** Get relocation section */
    inline section * get_section() const;

    /** Set relocation type */
    inline void set_type(enum reloc_e type);
    /** Get relocation type */
    inline enum reloc_e get_type() const;

    /** Set relocation addend */
    inline void set_addend(int64_t addend);
    /** Get relocation addend */
    inline int64_t get_addend() const;

    /** Set relocation offset. Relocation offset is relative to
	mangled symbol value if available. It is relative to section
	if no mangled symbol is defined. */
    inline void set_offset(uint64_t offset);
    /** Get relocation offset */
    inline uint64_t get_offset() const;

  private:
    symbol *sym_mod_;
    symbol *sym_;
    section *sec_;
    enum reloc_e type_;
    int64_t addend_;
    uint64_t offset_;
  };

}

#endif

