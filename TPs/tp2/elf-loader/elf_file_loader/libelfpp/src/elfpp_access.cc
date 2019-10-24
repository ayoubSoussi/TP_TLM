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

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cstring>

#include <dpp/foreach>

#include <elfpp/access>
#include <elfpp/object>
#include <elfpp/section>
#include <elfpp/symbol>
#include <elfpp/reloc>
#include <elfpp/segment>

namespace elfpp
{

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::load_symtab(object &obj)
  {
    section &table = obj.get_section(".symtab");

    if (table.get_type() != SHT_SYMTAB)
      throw std::runtime_error("not a symbol table section");

    // get symbol table content
    elf_sym_t *symtab = (elf_sym_t*)table.get_content();
    size_t count = table.get_size() / sizeof (elf_sym_t);

    // get symbol string section
    section &strtab = *table.get_link();
    const char *symstr = (char*)strtab.get_content();

    // create symbol objects from table
    obj.symidx_.resize(count);

    for (unsigned int i = 1; i < count; i++)
      {
	unsigned int idx = swap(symtab[i].st_shndx);

	symbol *sym = new symbol(symstr + swap(symtab[i].st_name));

	obj.symidx_[i] = sym;

	sym->set_value(swap(symtab[i].st_value));
	sym->set_size(swap(symtab[i].st_size));
	sym->set_info(symtab[i].st_info);
	sym->set_other(symtab[i].st_other);

	if (idx > 0 && idx < SHN_LORESERVE)
	  {
	    section *symsec = obj.secidx_.at(idx);
	    if (!symsec)
	      continue;

	    sym->set_section(*symsec);
	    symsec->add_symbol(*sym);

	    if (obj.type_ != ET_REL)
	      sym->set_value(sym->get_value() - symsec->get_vaddr());

	    if (symsec && sym->get_size() > 0 && symsec->get_type() != SHT_NOBITS)
	      {
		if (symsec->get_size() < sym->get_value())
		  std::cerr << "elfpp: symbol value above section size " << sym->get_name()
			    << ":" << std::hex << sym->get_value() << std::endl;
		//		else
		//		  sym->set_content(symsec->get_content() + sym->get_value());
	      }
	  }
	else
	  {
	    sym->set_section_ndx(idx);
	    obj.add_symbol(*sym);
	  }
      }

    // discard symbol table sections
    delete &table;
    delete &strtab;
  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::load_addend(const object &obj, reloc &r, const elf_rel_t *reltab,
					       size_t count, unsigned int index, uint8_t *data)
  {
    //		  r->set_addend(rel_read(obj, type, sec->get_content() + ro));
    uint64_t offset = swap(reltab[index].r_offset);
    r.set_offset(offset);

    switch (obj.machine_)
      {
      case EM_MIPS:
	switch (r.get_type())
	  {
	  case R_MIPS_26:
	    r.set_addend((swap(*(uint32_t*)(data + offset)) & 0x3ffffff) << 2);
	    return;

	  case R_MIPS_32:
	    r.set_addend(swap(*(uint32_t*)(data + offset)));
	    return;

	  case R_MIPS_HI16: {
	    int32_t value = (swap(*(uint32_t*)(data + offset)) & 0xffff) << 16;
	    // get next LO16 relocation
	    for (index++; index < count && rel_type(swap(reltab[index].r_info)) != R_MIPS_LO16; index++)
	      ;
	    if (index != count)
	      {
		offset = swap(reltab[index].r_offset);
		value += (int16_t)swap(*(uint32_t*)(data + offset));
		r.set_addend(value);
	      }
	    else
	      {
		std::cerr << "elfpp: can not find matching LO16 relocation" << std::endl;
	      }
	    return;
	  }

	  case R_MIPS_LO16: {
	    int32_t value = (int16_t)swap(*(uint32_t*)(data + offset));
	    // get prev HI16 relocation if any
	    if (index > 0 && rel_type(swap(reltab[index - 1].r_info)) == R_MIPS_HI16)
	      {
		offset = swap(reltab[index - 1].r_offset);
		value += (swap(*(uint32_t*)(data + offset)) & 0xffff) << 16;
	      }
	    r.set_addend(value);
	    return;
	  }

	  default: ;
	  }
	break;

      case EM_ARM:
	switch (r.get_type())
	  {
	  case R_ARM_PC24:
	    r.set_addend(((swap(*(uint32_t*)(data + offset)) & 0xffffff) << 2) + 8);
	    return;

	  case R_ARM_ABS32:
	    r.set_addend(swap(*(uint32_t*)(data + offset)));
	    return;

	  default: ;
	  }
	break;

      default: ;
      }

    std::cerr << "elfpp: unhandled relocation (machine=" << obj.machine_
	      << ", type=" << r.get_type() << ")" << std::endl;
    return;
  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::rel_write(const object &obj, enum reloc_e type, void *data, int64_t value)
  {
    uint32_t mask;

    switch (obj.machine_)
      {
      case EM_MIPS:
	switch (type)
	  {
	  case R_MIPS_26:
	    mask = 0x3ffffff;
	    if (value & 0x3)
	      std::cerr << "elfpp: R_MIPS_26 relocation with 2 LSB set" << std::endl;
	    value >>= 2;
	    goto done;

	  case R_MIPS_32:
	    mask = 0xffffffff;
	    goto done;

	  case R_MIPS_HI16:
	    mask = 0xffff;
	    value >>= 16;
	    goto done;

	  case R_MIPS_LO16:
	    mask = 0xffff;
	    goto done;

	  default: ;
	  }
	break;

      case EM_ARM:
	switch (type)
	  {
	  case R_ARM_PC24:
	    mask = 0x00ffffff;
	    value = (value - 8) >> 2;
	    goto done;

	  case R_ARM_ABS32:
	    mask = 0xffffffff;
	    goto done;

	  default: ;
	  }
	break;

      default: ;
      }

    std::cerr << "elfpp: unhandled relocation (machine=" << obj.machine_ << ", type=" << type << ")" << std::endl;
    return;

  done:
    *(uint32_t*)data = swap(mask & value) | (*(uint32_t*)data & swap(~mask));
  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::load_reltab(object &obj)
  {
    // load all reloaction table sections

    if (obj.type_ != ET_REL)
      return;

    FOREACH2(relsec, obj.get_section_table())
      {
	switch (relsec->get_type())
	  {
	  case SHT_REL: {
	    elf_rel_t *reltab = (elf_rel_t*)relsec->get_content();
	    size_t count = relsec->get_size() / sizeof (elf_rel_t);
	    obj.rel_with_addend_ = false;

	    for (unsigned int i = 0; i < count; i++)
	      {
		if (!rel_sym(swap(reltab[i].r_info)))
		  continue;

		reloc *r = new reloc();

		section *sec = relsec->get_info();
		symbol *sym = obj.symidx_.at(rel_sym(swap(reltab[i].r_info)));
		assert(sym);

		r->set_symbol(sym);
		sym->add_reloc(*r);

		r->set_section(sec);
		reloc_e type = rel_type(swap(reltab[i].r_info));
		r->set_type(type);
		load_addend(obj, *r, reltab, count, i, sec->get_content());
	      }

	    //		std::cerr << "elfpp: relocation " << i << " offset " << ro << " out of section" << std::endl;

	    delete &*relsec;
	    break;
	  }

	  case SHT_RELA: {
	    elf_rela_t *reltab = (elf_rela_t*)relsec->get_content();
	    size_t count = relsec->get_size() / sizeof (elf_rela_t);
	    obj.rel_with_addend_ = true;

	    for (unsigned int i = 0; i < count; i++)
	      {
		if (!rel_sym(swap(reltab[i].r_info)))
		  continue;

		reloc *r = new reloc();

		section *sec = relsec->get_info();
		symbol *sym = obj.symidx_[rel_sym(swap(reltab[i].r_info))];
		assert(sym);

		r->set_symbol(sym);
		sym->add_reloc(*r);

		r->set_section(sec);

		r->set_type(rel_type(swap(reltab[i].r_info)));
		uint32_t ro = swap(reltab[i].r_offset);
		r->set_offset(ro);
		r->set_addend(swap(reltab[i].r_addend));
	      }

	    delete &*relsec;
	    break;
	  }

	  default:
	    break;
	  }

      }

    obj.generate_symtab_ = true;
  }

  static bool sort_predicate(const symbol *a, const symbol *b)
  {
    return a->get_info() < b->get_info();
  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::save_symtab(object &obj)
  {
    size_t shnum = 1;

    // renumber current section table
    FOREACH(s, obj.get_section_table())
      s->index_ = shnum++;

    // create symbol and associated string sections
    section *symtab = new section(obj, SHT_SYMTAB);
    section *strtab = new section(obj, SHT_STRTAB);
    std::string str(1, '\0');

    symtab->set_name(".symtab");
    symtab->set_entsize(sizeof (elf_sym_t));
    symtab->set_link(strtab);

    strtab->set_name(".strtab");
    strtab->set_flags(SHF_STRINGS);

    obj.add_section(*symtab);
    obj.add_section(*strtab);

    // prepare sorted symbol array

    unsigned int i = 0;
    std::vector<symbol *> table;

    table.resize(obj.get_symbol_table().size());

    FOREACH(sym, obj.get_symbol_table())
      table[i++] = sym->second;

    FOREACH(sec, obj.get_section_table())
      {
	table.resize(i + sec->get_symbol_table().size());

	FOREACH(sym, sec->get_symbol_table())
	  table[i++] = sym->second;
      }

    sort(table.begin(), table.end(), sort_predicate);

    for (i = 0; i < table.size(); i++)
      {
	if (ELF_ST_BIND(table[i]->get_info()) != STB_LOCAL)
	  break;
      }

    symtab->set_info_last(i + 1);

    // generate symbol table section content

    std::vector<reloc*> reloclist[shnum];
    elf_sym_t symdata[table.size() + 1];
    i = 1;

    std::memset(symdata, 0, sizeof(elf_sym_t));

    FOREACH(sym_, table)
      {
	elf_sym_t &ent = symdata[i];
	symbol *sym = *sym_;

	swap(&ent.st_name, str.size());
	str.append(sym->get_name().c_str(), sym->get_name().size() + 1);
	swap(&ent.st_value, sym->get_value());
	swap(&ent.st_size, sym->get_size());
	ent.st_info = sym->get_info();
	ent.st_other = sym->get_other();

	FOREACH(rel, sym->reloc_tab_)
	  reloclist[rel->get_section()->index_].push_back(&*rel);

	if (sym->get_section())
	  swap(&ent.st_shndx, sym->get_section()->index_);
	else
	  swap(&ent.st_shndx, sym->get_section_ndx());

	sym->index_ = i++;
      }

    strtab->set_size(str.size());
    std::memcpy(strtab->get_content(), str.data(), str.size());

    symtab->set_size(sizeof(symdata));
    std::memcpy(symtab->get_content(), symdata, sizeof(symdata));

    // generate relocation table

    FOREACH(sec, obj.get_section_table())
      {
	if (!sec->index_)
	  continue;

	std::vector<reloc*> &rl = reloclist[sec->index_];

	if (rl.empty())
	  continue;

	section *reltab = new section(obj, obj.rel_with_addend_ ? SHT_RELA : SHT_REL);

	reltab->set_entsize(sizeof (elf_rel_t));
	reltab->set_link(symtab);
	reltab->set_info(&*sec);

	obj.add_section(*reltab);

	if (obj.rel_with_addend_)
	  {
	    elf_rela_t reldata[rl.size()];

	    reltab->set_name(".rela" + sec->get_name());
	    i = 0;

	    FOREACH(rel_, rl)
	      {
		reloc *rel = *rel_;

		if (symbol *ms = rel->get_mangled_symbol())
		  swap(&reldata[i].r_offset, ms->get_value() + rel->get_offset());
		else
		  swap(&reldata[i].r_offset, rel->get_offset());

		swap(&reldata[i].r_info, rel_info(rel->get_symbol()->index_, rel->get_type()));
		swap(&reldata[i].r_addend, rel->get_addend());
		i++;
	      }

	    reltab->set_size(sizeof(reldata));
	    std::memcpy(reltab->get_content(), reldata, sizeof(reldata));
	  }
	else
	  {
	    elf_rel_t reldata[rl.size()];

	    reltab->set_name(".rel" + sec->get_name());
	    i = 0;

	    FOREACH(rel_, rl)
	      {
		reloc *rel = *rel_;

		uint64_t ro = rel->get_offset();
		if (symbol *ms = rel->get_mangled_symbol())
		  {
		    // relocation offset is relative to mangled symbol value
		    if (ro < ms->get_size())
		      rel_write(obj, rel->get_type(), ms->get_content() + ro, rel->get_addend());
		    else
		      std::cerr << "elfpp: relocation offset above mangled symbol size" << std::endl;
		    ro += ms->get_value();
		  }
		else
		  {
		    // relocation offset is relative to mangled section
		    if (ro < sec->get_size())
		      rel_write(obj, rel->get_type(), sec->get_content() + ro, rel->get_addend());
		    else
		      std::cerr << "elfpp: relocation offset above section size" << std::endl;
		  }

		swap(&reldata[i].r_offset, ro);

		swap(&reldata[i].r_info, rel_info(rel->get_symbol()->index_, rel->get_type()));
		i++;
	      }

	    reltab->set_size(sizeof(reldata));
	    std::memcpy(reltab->get_content(), reldata, sizeof(reldata));
	  }

      }
  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::read(object &obj, FILE *file)
  {
    // read file header

    elf_hdr_t head_;

    fseek(file, 0, SEEK_SET);
    if (fread(&head_, sizeof (head_), 1, file) != 1)
      throw std::runtime_error("unable to read file");

    obj.word_width_ = (ei_class_e)head_.e_ident[EI_CLASS];
    obj.byteorder_ = byteorder_ = (ei_data_e)head_.e_ident[EI_DATA];
    obj.os_abi_ = (ei_osabi_e)head_.e_ident[EI_OSABI];
    obj.abi_ver_ = head_.e_ident[EI_ABIVERSION];
    obj.type_ = (e_type_e)swap(head_.e_type);
    obj.machine_ = (e_machine_e)swap(head_.e_machine);
    obj.entry_ = swap(head_.e_entry);
    obj.flags_ = swap(head_.e_flags);

    // read segment table
    size_t phnum = swap(head_.e_phnum);
    elf_phdr_t phdr[phnum];

    fseek(file, swap(head_.e_phoff), SEEK_SET);
    if (fread(phdr, sizeof (elf_phdr_t), phnum, file) != phnum)
      throw std::runtime_error("unable to read file");

    // create segment objects

    for (unsigned int i = 0; i < phnum; i++)
      {
	segment *s = new segment(obj);

	s->set_type((p_type_e)swap(phdr[i].p_type));
	s->set_file_offset(swap(phdr[i].p_offset));
	s->set_vaddr(swap(phdr[i].p_vaddr));
	s->set_paddr(swap(phdr[i].p_paddr));
	s->set_mem_size(swap(phdr[i].p_memsz));
	s->set_file_size(swap(phdr[i].p_filesz));
	s->set_flags((p_flags_e)swap(phdr[i].p_flags));
	s->set_align(swap(phdr[i].p_align));

	obj.add_segment(*s);
      }

    // read section table
    size_t shnum = swap(head_.e_shnum);
    elf_shdr_t shdr[shnum];
    obj.secidx_.resize(shnum);

    fseek(file, swap(head_.e_shoff), SEEK_SET);

    if (fread(shdr, sizeof (elf_shdr_t), shnum, file) != shnum)
      throw std::runtime_error("unable to read file");

    // create section objects

    for (unsigned int i = 1; i < shnum; i++)
      {
	sh_type_e type = (sh_type_e)swap(shdr[i].sh_type);

	section *s = new section(obj, type);
	off_t offset = swap(shdr[i].sh_offset);

	obj.secidx_[i] = s;
	s->index_ = i;

	s->set_flags((sh_flags_e)swap(shdr[i].sh_flags));
	s->set_vaddr(swap(shdr[i].sh_addr));
	s->set_file_offset(offset);
	s->set_size(swap(shdr[i].sh_size));
	s->set_align(swap(shdr[i].sh_addralign));
	s->set_entsize(swap(shdr[i].sh_entsize));

	if (type != SHT_NOBITS && s->get_size())
	  {
	    fseek(file, offset, SEEK_SET);
	    if (fread(s->get_content(), s->get_size(), 1, file) != 1)
	      throw std::runtime_error("unable to read file");
	  }

	obj.add_section(*s);
      }

    // update sections links

    for (unsigned int i = 1; i < shnum; i++)
      {
	section &s = *obj.secidx_[i];

	unsigned int link = swap(shdr[i].sh_link);

	if (link)
	  s.set_link(obj.secidx_.at(link));

	unsigned int info = swap(shdr[i].sh_info);

	switch (s.type_)
	  {
	  case SHT_REL:
	  case SHT_RELA:
	    if (info)
	      s.set_info(obj.secidx_.at(info));
	    break;

	  case SHT_DYNSYM:
	  case SHT_SYMTAB:
	    s.set_info_last(info);
	    break;

	  default:
	    ;
	  }
      }

    // set sections names

    unsigned int shstrndx = swap(head_.e_shstrndx);

    if (shstrndx != SHN_UNDEF)
      {
	section &shstr = *obj.secidx_.at(shstrndx);
	const char *str = (char*)shstr.get_content();

	for (unsigned int i = 1; i < shnum; i++)
	  obj.secidx_[i]->set_name(str + swap(shdr[i].sh_name));

	delete &shstr;
      }

  }

  template <typename bits_t, int width>
  void elfn_access<bits_t, width>::write(object &obj, FILE *file)
  {
    byteorder_ = obj.byteorder_;

    if (obj.generate_symtab_)
      save_symtab(obj);

    // write symbol contents

    FOREACH(sec, obj.get_section_table())
      {
	if (sec->get_type() == SHT_NOBITS)
	  continue;

	FOREACH(sym_, sec->get_symbol_table())
	  {
	    symbol *sym = sym_->second;

	    if (!sym->get_content())
	      continue;

	    if (sec->get_size() < sym->get_value() + sym->get_size())
	      sec->set_size(sym->get_value() + sym->get_size());

	    std::memcpy(sec->get_content() + sym->get_value(), sym->get_content(), sym->get_size());
	  }
      }

    elf_hdr_t head;

    // create a new section name section

    section *shstrtab = new section(obj, SHT_STRTAB);
    shstrtab->set_name(".shstrtab");
    shstrtab->set_flags(SHF_STRINGS);
    obj.add_section(*shstrtab);

    size_t shnum = 1;

    FOREACH(s, obj.get_section_table())
      s->index_ = shnum++;

    std::string strtab(1, '\0');
    unsigned int stridx[shnum];

    stridx[0] = 0;

    FOREACH(s, obj.get_section_table())
      {
	stridx[s->index_] = strtab.size();
	strtab.append(s->get_name().c_str(), s->get_name().size() + 1);
      }

    shstrtab->set_size(strtab.size());
    std::memcpy(shstrtab->get_content(), strtab.data(), strtab.size());

    // update header

    std::memset(head.e_ident, 0, sizeof(head.e_ident));
    std::memcpy(head.e_ident, ELFMAG, 4);
    head.e_ident[EI_CLASS] = obj.word_width_;
    head.e_ident[EI_DATA] = byteorder_;
    head.e_ident[EI_VERSION] = EV_CURRENT;
    head.e_ident[EI_OSABI] = obj.os_abi_;
    head.e_ident[EI_ABIVERSION] = obj.abi_ver_;

    swap(&head.e_type, obj.type_);
    swap(&head.e_machine, obj.machine_);
    swap(&head.e_version, EV_CURRENT);
    swap(&head.e_entry, obj.entry_);
    swap(&head.e_phoff, sizeof (elf_hdr_t));
    swap(&head.e_shoff, sizeof (elf_hdr_t));
    swap(&head.e_flags, obj.flags_);
    swap(&head.e_ehsize, sizeof (elf_hdr_t));
    swap(&head.e_phentsize, sizeof (elf_phdr_t));
    swap(&head.e_phnum, 0);
    swap(&head.e_shentsize, sizeof (elf_shdr_t));
    swap(&head.e_shnum, shnum);
    swap(&head.e_shstrndx, shstrtab->index_);

    // update section table

    elf_shdr_t shdr[shnum];

    std::memset(shdr, 0, sizeof(shdr));

    FOREACH(s, obj.get_section_table())
      {
	unsigned int i = s->index_;

	swap(&shdr[i].sh_name, stridx[i]);

	swap(&shdr[i].sh_type, s->get_type());
	swap(&shdr[i].sh_flags, s->get_flags());
	swap(&shdr[i].sh_addr, s->get_vaddr());

	swap(&shdr[i].sh_size, s->get_size());
	if (s->get_link())
	  swap(&shdr[i].sh_link, s->get_link()->index_);

	switch (s->get_type())
	  {
	  case SHT_REL:
	  case SHT_RELA:
	    assert(s->get_info());
	    swap(&shdr[i].sh_info, s->get_info()->index_);
	    break;

	  case SHT_DYNSYM:
	  case SHT_SYMTAB:
	    swap(&shdr[i].sh_info, s->get_info_last());
	  default:
	    break;
	  }

	swap(&shdr[i].sh_addralign, s->get_align());
	swap(&shdr[i].sh_entsize, s->get_entsize());
      }

    // write header to file

    fseek(file, 0, SEEK_SET);
    if (fwrite(&head, sizeof(head), 1, file) != 1)
      throw std::runtime_error("unable to write header to file");

    fseek(file, sizeof(shdr), SEEK_CUR);

    // write sections to file

    FOREACH(s, obj.get_section_table())
      {
	if (s->get_type() == SHT_NOBITS)
	  continue;

	swap(&shdr[s->index_].sh_offset, ftell(file));
	if (s->get_size() && fwrite(s->get_content(), s->get_size(), 1, file) != 1)
          throw std::runtime_error("unable to write section to file");
      }

    // write section table to file

    fseek(file, sizeof(head), SEEK_SET);
    if (fwrite(shdr, sizeof(shdr), 1, file) != 1)
      throw std::runtime_error("unable to write section table to file");
  }

  template class elfn_access<elf32_bits_s, 32>;
  template class elfn_access<elf64_bits_s, 64>;

}

