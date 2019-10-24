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

#ifndef ELFPP_SYMBOL_HH_
#define ELFPP_SYMBOL_HH_

#include "elfpp.hh"
#include "elfpp_bits.hh"

#include <stdint.h>
#include <string>
#include <map>

#include <dpp/linked_list>

namespace elfpp
{

  class reloc;
  class section;

  class symbol;
  std::ostream & operator<<(std::ostream &o, const symbol &v);

  /** 
      @short ELF symbol class
      @header elfpp/symbol
   */
  class symbol
  {
    template <typename, int> friend class elfn_access;
    friend std::ostream & operator<<(std::ostream &o, const symbol &v);

  public:
    /** @this creates a new symbol with given name */
    symbol(const std::string &name);

    ~symbol();

    /** @this gets symbol name */
    inline const std::string & get_name() const;

    /** @this sets symbol section */
    void set_section(section &sec);
    /** @this gets symbol section */
    inline section * get_section() const;
    /** @this sets integer section index, must be used for special ndx values */
    inline void set_section_ndx(unsigned int ndx);
    /** @this gets integer section index */
    inline unsigned int get_section_ndx() const;

    /** @this gets symbol info field
	@see get_type @see get_bind @see #ELF_ST_TYPE @see #ELF_ST_BIND */
    inline uint8_t get_info() const;
    /** @this sets symbol info field 
	@see set_type @see set_bind @see #ELF_ST_TYPE @see #ELF_ST_BIND */
    inline void set_info(uint8_t st_info);

    /** @this gets symbol type decoded from info field
     	@see get_info */
    inline st_info_type_e get_type() const;
    /** @this sets symbol type in info field
     	@see set_info */
    inline void set_type(st_info_type_e type);

    /** @this gets symbol bind decoded from info field
     	@see get_info */
    inline st_info_bind_e get_bind() const;
    /** @this sets symbol bind in info field
     	@see set_info */
    inline void set_bind(st_info_bind_e bind);

    /** @this gets symbol "other" field */
    inline uint8_t get_other() const;
    /** @this sets symbol "other" field */
    inline void set_other(uint8_t st_other);

    /** @this gets symbol value field */
    inline uint64_t get_value() const;
    /** @this sets symbol value field */
    inline void set_value(uint64_t val);

    /** @this gets symbol size in bytes */
    inline size_t get_size() const;
    /** @this sets symbol size in bytes */
    inline void set_size(size_t size);

    /** @multiple @this gets relocation associated with this symbol */
    inline const reloc_table_t & get_reloc_table() const;
    inline reloc_table_t & get_reloc_table();
    /** @this adds a relocation */
    void add_reloc(reloc &r);
    /** @this removes a relocation */
    void remove_reloc(reloc &r);

    /** @this allocates a symbol content buffer of symbol size and fill with
	data from given pointer. no symbol content buffer is allocated
	by default unless load_symbol_data() has been called on elf
	object. section is updated with symbol content at symbol value
	offset when elf file is written. */
    void set_content(uint8_t *data);

    /** @this gets symbol content buffer */
    inline uint8_t * get_content() const;

  private:
    /** start of flags present in elf symbol entry */
    std::string			name_;
    uint64_t			value_;
    size_t			size_;
    uint8_t			info_;
    uint8_t			other_;
    section			*section_;
    unsigned int		sec_ndx_;
    /** end of flags present in elf symbol entry */

    unsigned int		index_;

    reloc_table_t		reloc_tab_;
    reloc_modlist_t		reloc_mod_;
    uint8_t			*content_;
  };

}

#endif

