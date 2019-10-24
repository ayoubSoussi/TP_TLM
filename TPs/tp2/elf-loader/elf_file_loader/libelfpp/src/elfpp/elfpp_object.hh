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

#ifndef ELFPP_OBJECT_HH_
#define ELFPP_OBJECT_HH_

#include <string>
#include <vector>

#include <dpp/linked_list>

#include "elfpp.hh"
#include "elfpp_bits.hh"

namespace elfpp
{
  class section;
  class symbol;
  class elf_access;
  template <typename bits_t, int width> class elfn_access;

  /** 
      @short ELF object file class
      @header elfpp/object
   */
  class object
  {
    template <typename, int> friend class elfn_access;
    friend class section;
  public:

    /** Create an empty elf object */
    object(e_machine_e machine,
	   ei_class_e word_width = ELFCLASSNONE,
	   ei_data_e byte_order = ELFDATANONE);

    /** Create a new elf object and load sections from file. */
    object(const std::string &filename);

    ~object();

    /**
       Parse symbol and relocation tables sections and create
       associated object representation. Parsed table sections are
       discarded and will be regenerated when writing elf file.
     */
    void parse_symbol_table();

    /**
       @this creates symbols for all area not covered by a symbol in
       allocatable sections.
     */
    void create_orphan_symbols();

    /**
       @this loads symbol data from section content to symbol content
       storage and update all relocations to be relative to mangled
       symbols. Sections will be updated from symbol content when
       writing elf file.
     */
    void load_symbol_data();

    /** Add a new section to object. */
    void add_section(section &sec);

    /** Remove a section from object. */
    void remove_section(section &sec);

    /** Get first section with given name. */
    section & get_section(const std::string &name);

    /** Return section table container. */
    inline dpp::linked_list<section> & get_section_table();

    /** Add a new segment to object. */
    void add_segment(segment &seg);

    /** Return section table container. */
    inline dpp::linked_list<segment> & get_segment_table();

    /** Add a new symbol to object. Symbol must be stored per
	section. Only symbol without associated section must be added
	to object. */
    void add_symbol(symbol &sym);

    /** Remove a section independent symbol from object. */
    void remove_symbol(symbol &sym);

    /** Get section independent symbol by name. */
    inline symbol & get_symbol(const std::string &name);

    /** Get section independent symbols table. */
    inline const sym_tab_map_t & get_symbol_table() const;

    /** Write elf object to file. Symbol and relocation table may be
	generated from object representation if available. Sections
	content are updated with symbols content. */
    void write(const std::string &filename);

  private:

    void set_relative_relocs(symbol *sym);

    static ei_class_e get_word_width(e_machine_e machine);
    static ei_data_e get_byte_order(e_machine_e machine);

    dpp::linked_list<section> section_tab_;
    dpp::linked_list<segment> segment_tab_;

    ei_class_e word_width_;
    ei_data_e byteorder_;
    ei_osabi_e os_abi_;
    uint8_t abi_ver_;
    e_type_e type_;
    e_machine_e machine_;

    elf_access *access_;

    bool rel_with_addend_;
    bool generate_symtab_;
    std::vector<section *> secidx_;
    std::vector<symbol *> symidx_;
    uint64_t entry_;
    unsigned int flags_;
    sym_tab_map_t sym_tab_;
  };

}

#endif

