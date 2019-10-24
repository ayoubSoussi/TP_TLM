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
#include <cstring>

#include <dpp/foreach>
#include <dpp/interval_set>

#include <elfpp/object>
#include <elfpp/access>
#include <elfpp/section>
#include <elfpp/segment>
#include <elfpp/symbol>
#include <elfpp/reloc>

namespace elfpp
{

  ei_class_e object::get_word_width(e_machine_e machine)
  {
    switch (machine)
      {
      case EM_386:
	return ELFCLASS32;

      case EM_X86_64:
	return ELFCLASS64;

      default:
	throw std::runtime_error("unable to guess machine word width");
      }
  }

  ei_data_e object::get_byte_order(e_machine_e machine)
  {
    switch (machine)
      {
      case EM_SPARC:
	return ELFDATA2MSB;

      case EM_X86_64:
      case EM_386:
	return ELFDATA2LSB;

      default:
	throw std::runtime_error("unable to guess machine byte order");
      }
  }

  object::object(e_machine_e machine,
		 ei_class_e word_width,
		 ei_data_e byte_order)
    : section_tab_(),
      os_abi_(ELFOSABI_SYSV),
      type_(ET_REL),
      machine_(machine),
      generate_symtab_(true),
      entry_(0),
      flags_(0)
  {
    switch (machine)
      {
      case EM_X86_64:
      case EM_386:
	rel_with_addend_ = false;
	break;

      default:
	rel_with_addend_ = true;
	break;
      }

    if (byte_order == ELFDATANONE)
      byte_order = get_byte_order(machine);

    if (word_width == ELFCLASSNONE)
      word_width = get_word_width(machine);
  }

  object::object(const std::string &filename)
    : generate_symtab_(false)
  {
    FILE *file = fopen(filename.c_str(), "rb");

    if (!file)
      throw std::runtime_error("unable to open file");

    unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */

    if (fread(e_ident, EI_NIDENT, 1, file) != 1)
      throw std::runtime_error("unable to read file");

    try {

      if (std::memcmp(e_ident, ELFMAG, 4))
	throw std::runtime_error("bad elf file header");

      switch (e_ident[EI_CLASS])
	{
	case ELFCLASS32: {
	  static elf32_access a;
	  access_ = static_cast<elf_access*>(&a);
	  break;
	}

	case ELFCLASS64: {
	  static elf64_access a;
	  access_ = static_cast<elf_access*>(&a);
	  break;
	}

	default:
	  throw std::runtime_error("bad elf class");
	}

      access_->read(*this, file);

    } catch (...) {
      fclose(file);
      throw;
    }

    fclose(file);
  }

  object::~object()
  {
    FOREACH2(s, section_tab_)
      delete &*s;

    FOREACH(s, sym_tab_)
      delete s->second;
  }

  section & object::get_section(const std::string &name)
  {
    FOREACH(s, section_tab_)
      if (s->get_name() == name)
	return *s;

    throw std::runtime_error("no such section");
  }

  void object::parse_symbol_table()
  {
    access_->load_symtab(*this);
    access_->load_reltab(*this);
  }

  void object::set_relative_relocs(symbol *sym)
  {
    // find mangled symbol for each relocation and set offset relative
    // to symbol value.
    FOREACH2(k, sym->get_reloc_table())
      {
	if (k->get_mangled_symbol())
	  continue;

	if (sym->get_type() == STT_SECTION)
	  {
	    section *sS = sym->get_section();
	    int64_t addend = k->get_addend();
	    try {
	      symbol &ss = sS->get_symbol(addend);
	      sym->remove_reloc(*k);
	      k->set_symbol(&ss);
	      ss.add_reloc(*k);
	      k->set_addend(addend - ss.get_value());
	    } catch (std::runtime_error &e) {
#if 0
	      std::cerr << e.what() << " at " << sS->get_name() << ":" << std::hex << addend
			<< " while resolving section symbol addend at "
			<< k->get_section()->get_name() << ":" << k->get_offset() << " type " << k->get_type() << std::endl;
#endif
	    }
	  }

	try {
	  symbol &msym = k->get_section()->get_symbol(k->get_offset());
	  // std::cerr << *i->second << " " << msym << std::endl;
	  k->set_mangled_symbol(&msym);
	  k->set_offset(k->get_offset() - msym.get_value());

	} catch (std::runtime_error &e) {
	  if (strncmp(k->get_section()->get_name().c_str(), ".debug", 6))
	    std::cerr << e.what() << " at " << k->get_section()->get_name() << ":" << std::hex << k->get_offset() << std::endl;
	}

      }
  }

  void object::create_orphan_symbols()
  {
    FOREACH(s, get_section_table())
      {
	if (!(s->get_flags() & SHF_ALLOC))
	  continue;

	typedef dpp::interval_set<uint32_t> is_t;
	is_t is;

	is |= is_t::interval_type(s->get_size(), (uint32_t)-1);

	FOREACH(i, s->get_symbol_table())
	  {
	    uint32_t val = i->second->get_value();
	    size_t size = i->second->get_size();

	    if (size)
	      is |= is_t::interval_type(val, val + size);
	  }

	is = ~is;

	// create symbols for all orphan section areas in this object
	FOREACH(i, is)
	  {
	    static int nosym_id = 0;
	    char name[32];
	    sprintf(name, "nosym_%i", nosym_id++);
	    symbol *sym = new symbol(name);

	    sym->set_value(i->low_bound());
	    sym->set_size(i->high_bound() - i->low_bound());
	    sym->set_section(*s);
	    s->add_symbol(*sym);
	  }
      }
  }

  void object::load_symbol_data()
  {
    FOREACH(s, get_section_table())
      {
	if (s->get_type() == SHT_NOBITS)
	  continue;

	FOREACH(i, s->get_symbol_table())
	  {
	    symbol *j = i->second;

	    // copy section data in each symbol content buffer
	    size_t size = j->get_size();

	    if (size != 0 && j->get_value() + size <= s->get_size())
	      j->set_content(s->get_content() + j->get_value());
	  }
      }

    FOREACH(s, get_section_table())
      {
	FOREACH(i, s->get_symbol_table())
	  set_relative_relocs(i->second);
      }

    FOREACH(i, get_symbol_table())
      set_relative_relocs(i->second);

  }

  void object::write(const std::string &filename)
  {
    FILE *file = fopen(filename.c_str(), "wb");

    if (!file)
      throw std::runtime_error("unable to open file");

    try {
      access_->write(*this, file);
    } catch (...) {
      fclose(file);
      throw;
    }

    fclose(file);
  }

  void object::add_section(section &sec)
  {
    section_tab_.push_back(sec);
  }

  void object::add_segment(segment &seg)
  {
    segment_tab_.push_back(seg);
  }

  void object::remove_section(section &sec)
  {
    sec.remove();
  }

  void object::add_symbol(symbol &sym)
  {
#if 0
    if (sym.get_section())
      sym.get_section()->add_symbol(sym);
    else
#endif
      sym_tab_.insert(sym_tab_map_t::value_type(sym.get_name(), &sym));
  }

  void object::remove_symbol(symbol &sym)
  {
    sym_tab_.erase(sym.get_name());
  }

}

