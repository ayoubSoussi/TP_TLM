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

#ifndef ELFPP_SEGMENT_HH_
#define ELFPP_SEGMENT_HH_

#include <string>
#include <map>
#include <stdint.h>

#include "elfpp.hh"
#include "elfpp_bits.hh"

namespace elfpp
{
  class object;
  class segment;

  std::ostream & operator<<(std::ostream &o, const segment &v);

  /** 
      @short ELF segment class
      @header elfpp/segment
   */
  class segment : public segment_table_t::item_type
  {
    template <typename, int> friend class elfn_access;
    friend std::ostream & operator<<(std::ostream &o, const segment &v);
    friend class object;

  public:
    /** create a new segment associated with given object */
    inline segment(object &obj);
    inline ~segment();

    inline p_type_e get_type() const;
    inline void set_type(p_type_e e);

    inline p_flags_e get_flags() const;
    inline void set_flags(p_flags_e e);

    inline uint64_t get_vaddr() const;
    inline void set_vaddr(uint64_t a);

    inline uint64_t get_paddr() const;
    inline void set_paddr(uint64_t a);

    inline size_t get_mem_size() const;
    inline void set_mem_size(size_t a);

    inline size_t get_file_size() const;
    inline off_t get_file_offset() const;

    inline size_t get_align() const;
    inline void set_align(size_t a);

   private:
    inline void set_file_offset(off_t a);
    inline void set_file_size(size_t a);

    /** start of flags present in elf segment entry */
    p_type_e type_;
    p_flags_e flags_;
    uint64_t vaddr_;
    uint64_t paddr_;
    size_t mem_size_;
    size_t file_size_;
    off_t offset_;
    size_t align_;
  };
}

#endif

