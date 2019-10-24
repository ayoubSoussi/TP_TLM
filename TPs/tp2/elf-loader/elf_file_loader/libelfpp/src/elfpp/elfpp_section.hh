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

#ifndef ELFPP_SECTION_HH_
#define ELFPP_SECTION_HH_

#include <string>
#include <map>
#include <stdint.h>

#include "elfpp.hh"
#include "elfpp_bits.hh"

namespace elfpp
{

  class object;
  class section;
  class symbol;
  class reloc;

  std::ostream & operator<<(std::ostream &o, const section &v);

  /** 
      @short ELF section class
      @header elfpp/section
   */
  class section : public section_table_t::item_type
  {
    template <typename, int> friend class elfn_access;
    friend std::ostream & operator<<(std::ostream &o, const section &v);
    friend class symbol;
    friend class object;

  public:
    /** @this creates a new section associated with given object */
    section(object &obj, sh_type_e type);

    ~section();

    /** @this sets section name */
    inline void set_name(const std::string &name);
    /** @this gets section name */
    inline const std::string & get_name() const;

    /** @this gets section type */
    inline sh_type_e get_type() const;

    /** @this sets section flags */
    inline void set_flags(sh_flags_e flags);
    /** @this gets section flags */
    inline sh_flags_e get_flags() const;

    /** @this sets section address */
    inline void set_vaddr(uint64_t vaddr);
    /** @this gets section address */
    inline uint64_t get_vaddr() const;

    /** @this sets section size, reallocate section content buffer */
    void set_size(size_t size);
    /** @this gets section size */
    inline size_t get_size() const;

    /** @this sets section link to an other section */
    inline void set_link(section *s);
    /** @this gets section link */
    inline section * get_link() const;
    /** @this gets section with a link to this section, may be unreliable if
	multiple links exits */
    inline section * get_back_link() const;

    /** @this sets section info as link to section */
    inline void set_info(section *s);
    /** @this gets section info as link to section */
    inline section * get_info() const;
    /** @this gets section info as back link to section */
    inline section * get_back_info() const;

    /** @this sets section info as last local symbol index */
    inline void set_info_last(unsigned int last);
    /** @this gets section info as last local symbol index */
    inline unsigned int get_info_last();

    /** @this sets section alignement */
    inline void set_align(uint32_t align);
    /** @this gets section alignement */
    inline uint32_t get_align() const;

    /** @this sets section entry size */
    inline void set_entsize(uint32_t entsize);
    /** @this sets section entry size */
    inline uint32_t get_entsize() const;

    /** @this gets pointer to section content buffer */
    inline uint8_t *get_content() const;

    /** @this gets pointer to container segment if any */
    segment *get_segment() const;

    /** @this gets associated object */
    inline object * get_object() const;

    /** @this gets load address from segment info  */
    uint64_t get_load_address() const;

    inline off_t get_file_offset() const;

    /** @this adds symbol to section */
    void add_symbol(symbol &sym);
    /** @this removes symbol from section */
    void remove_symbol(symbol &sym);
    /** @this gets symbol by name */
    inline symbol & get_symbol(const std::string &name);
    /** @this gets symbol containing given offset */
    symbol & get_symbol(uint64_t offset);
    /** @this gets symbol table */
    inline const sym_tab_map_t & get_symbol_table() const;

   private:
    inline void set_file_offset(off_t a);

    /** start of flags present in elf section entry */
    std::string name_;
    sh_type_e type_;
    sh_flags_e flags_;
    uint64_t vaddr_;
    off_t offset_;
    size_t size_;
    section *link_;
    union {
      section *info_;
      unsigned int info_last_;
    };
    uint32_t align_;
    uint32_t entsize_;
    /** end of flags present in elf section entry */

    // index of section
    unsigned int index_;

    section *back_info_;	// link to associated relocation table
    section *back_link_;	// link to associated relocation table
    object *object_;
    uint8_t *content_;
    sym_tab_map_t sym_tab_;
  };

}

#endif

