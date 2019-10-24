/* -*- c++ -*-
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 *
 * This file is part of SoCLib, GNU LGPLv2.1.
 *
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 *
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, SoC
 *         Nicolas Pouillon <nipo@ssji.net>, 2009
 */

#include <cstring>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <string>

#include <fnmatch.h>
#include <ctype.h>

#include "static_init_code.h"
#include "loader.h"
#include "exception.h"

#include "dpp/foreach"

#include "elfpp/object"
#include "elfpp/symbol"
#include "elfpp/section"

namespace soclib { namespace common {
bool elf_load( const std::string &name, Loader &loader )
{
    elfpp::object *binary;
    std::vector<std::string> section_patterns;

    size_t fname_len = name.find_first_of(';', 0);
    std::string filename(name.substr(0, fname_len));

    try {
	binary = new elfpp::object(filename);
    } catch (std::runtime_error &e) {
	throw soclib::exception::RunTimeError(e.what());
    }

    try {
	binary->parse_symbol_table();
    } catch (std::runtime_error &e) {
	delete binary;
	throw soclib::exception::RunTimeError(e.what());
    }

    // get section patterns list separated by ";"
    size_t first = fname_len + 1, last = fname_len;
    while (last != std::string::npos) {
	last = name.find_first_of(';', first);
	section_patterns.push_back(name.substr(first, last - first));
	first = last + 1;
    }

    FOREACH( sect, binary->get_section_table() )
    {
	assert(&*sect != NULL);
	elfpp::sh_flags_e eflags = sect->get_flags();
	if ( !(eflags & elfpp::SHF_ALLOC) )
	    continue;

	// filter sections by name
	if ( !section_patterns.empty() ) {
	    bool match = false;

	    FOREACH( pattern, section_patterns ) {
		if ( !fnmatch( pattern->c_str(), sect->get_name().c_str(), 0 ) ) {
		    match = true;
		    break;
		}
	    }
	    if ( !match )
		continue;
	    std::cerr << "using " << filename << ":" << sect->get_name() << std::endl;
	}

	FOREACH( symp, sect->get_symbol_table() )
	{
	    elfpp::symbol &sym = *(symp->second);
	    size_t symsize = sym.get_size();
	    uint8_t info = sym.get_info();

	    if ( ( ELF_ST_BIND(info) != elfpp::STB_LOCAL &&
		   ELF_ST_BIND(info) != elfpp::STB_GLOBAL ) ||
		 ELF_ST_TYPE(info) >= elfpp::STT_NUM )
		continue;

	    if ( !isalnum(sym.get_name()[0]) && sym.get_name()[0] != '_' )
		continue;

	    uintptr_t addr =
		(sym.get_section() && sym.get_section()->get_segment()
		 ? sym.get_section()->get_vaddr()
		 : 0)
		+ sym.get_value();
	    loader.addSymbol(BinaryFileSymbol( sym.get_name(), addr, symsize ));
	}

	if ( (sect->get_type() == elfpp::SHT_NOBITS) || ! sect->get_size() ) {
	    continue;
	}

	size_t actual_size = sect->get_size();
	uint8_t *blob = (uint8_t*)std::malloc( actual_size );
	std::memcpy( blob, sect->get_content(), actual_size );
	uint32_t flags = BinaryFileSection::FLAG_LOAD;

	if ( ! (eflags & elfpp::SHF_WRITE) )
	    flags |= BinaryFileSection::FLAG_READONLY;
	if ( eflags & elfpp::SHF_EXECINSTR )
	    flags |= BinaryFileSection::FLAG_CODE;
	else
	    flags |= BinaryFileSection::FLAG_DATA;

	loader.addSection(
	    BinaryFileSection(
		sect->get_name(),
		sect->get_vaddr(),
		sect->get_load_address(),
		flags,
		actual_size,
		blob ));
    }

    delete binary;

	return true;
}

STATIC_INIT_CODE(
	Loader::register_loader("elf", elf_load);
)

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

