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

#ifndef ELFPP_ACCESS_HH_
#define ELFPP_ACCESS_HH_

#include <stdio.h>
#include "elfpp_bits.hh"

namespace elfpp
{

  class object;
  class section;
  class reloc;

  /** @internal */
  class elf_access
  {
  public:
    virtual ~elf_access() {}
    virtual void read(object &obj, FILE *file) = 0;
    virtual void write(object &obj, FILE *file) = 0;
    virtual void load_symtab(object &obj) = 0;
    virtual void load_reltab(object &obj) = 0;
    virtual void save_symtab(object &obj) = 0;
  };

  /** @internal */
  template <class bits_t, int width>
  class elfn_access : public elf_access
  {
    typedef typename bits_t::elf_hdr_t elf_hdr_t;
    typedef typename bits_t::elf_shdr_t elf_shdr_t;
    typedef typename bits_t::elf_phdr_t elf_phdr_t;
    typedef typename bits_t::elf_sym_t elf_sym_t;
    typedef typename bits_t::elf_rel_t elf_rel_t;
    typedef typename bits_t::elf_rela_t elf_rela_t;

    void load_symtab(object &obj);
    void load_reltab(object &obj);
    void save_symtab(object &obj);

    void read(object &obj, FILE *file);
    void write(object &obj, FILE *file);

    inline uint16_t swap(uint16_t x);
    inline uint32_t swap(uint32_t x);
    inline uint64_t swap(uint64_t x);

    inline int16_t swap(int16_t x);
    inline int32_t swap(int32_t x);
    inline int64_t swap(int64_t x);

    inline unsigned int rel_sym(uint64_t info);
    inline enum reloc_e rel_type(uint64_t info);
    inline uint64_t rel_info(unsigned int sym, enum reloc_e type);

    void load_addend(const object &obj, reloc &r, const elf_rel_t *reltab, size_t count, unsigned int index, uint8_t *data);
    void rel_write(const object &obj, enum reloc_e type, void *data, int64_t value);

    template <typename X, typename Y>
    inline void swap(X *a, Y x);

    ei_data_e byteorder_;
  };

  struct elf32_bits_s
  {
    typedef Elf32_Ehdr elf_hdr_t;
    typedef Elf32_Shdr elf_shdr_t;
    typedef Elf32_Phdr elf_phdr_t;
    typedef Elf32_Sym elf_sym_t;
    typedef Elf32_Rel elf_rel_t;
    typedef Elf32_Rela elf_rela_t;
  };

  struct elf64_bits_s
  {
    typedef Elf64_Ehdr elf_hdr_t;
    typedef Elf64_Shdr elf_shdr_t;
    typedef Elf64_Phdr elf_phdr_t;
    typedef Elf64_Sym elf_sym_t;
    typedef Elf64_Rel elf_rel_t;
    typedef Elf64_Rela elf_rela_t;
  };

  typedef elfn_access<elf32_bits_s, 32> elf32_access;
  typedef elfn_access<elf64_bits_s, 64> elf64_access;

}

#endif

