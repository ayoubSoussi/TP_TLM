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

#include <algorithm>
#include <string.h>
#include <sstream>
#include <cassert>

#include "exception.h"
#include "loader.h"

namespace soclib { namespace common {

Loader::Loader(
    const std::string &f,
    const std::string &f2,
    const std::string &f3,
    const std::string &f4,
    const std::string &f5,
    const std::string &f6 )
{
    if ( f != "" )
	load_file(f);
    if ( f2 != "" )
	load_file(f2);
    if ( f3 != "" )
	load_file(f3);
    if ( f4 != "" )
	load_file(f4);
    if ( f5 != "" )
	load_file(f5);
    if ( f6 != "" )
	load_file(f6);
}

Loader::Loader( const Loader &ref )
	: m_sections(ref.m_sections),
      m_symbol_table(ref.m_symbol_table)
{
}

void Loader::load( void *buffer, uintptr_t address, size_t length )
{
    std::cout
	<< std::showbase << std::hex
	<< "Loading at " << address
	<< " size " << std::dec << length
	<< ": ";
    bool one = false;
    for ( section_list_t::const_iterator i = m_sections.begin();
	  i != m_sections.end();
	  ++i ) {
	if ( i->load_overlap_in_buffer( buffer, address, length ) ) {
	    one = true;
	    std::cout << i->name() << " ";
	}
    }
    if ( !one )
	std::cout << "nothing";
    std::cout << std::endl;
}

void Loader::print( std::ostream &o ) const
{
	o << "<Loader " << std::endl;
    for ( section_list_t::const_iterator i = m_sections.begin();
	  i != m_sections.end();
	  ++i )
	o << " " << *i << std::endl;
    o << ">" << std::endl;
}

Loader::~Loader()
{
}

std::vector<BinaryFileSection> Loader::sections() const
{
    return m_sections;
}

BinaryFileSymbolOffset Loader::get_symbol_by_addr( uintptr_t addr ) const
{
    std::map<uintptr_t, BinaryFileSymbol>::const_iterator i =
	m_symbol_table.lower_bound( addr );

    while ( i != m_symbol_table.end()
	 && i != m_symbol_table.begin()
	 && i->first > addr )
	--i;

    if ( i->second.contains(addr) || i->second.size() == 0 )
	return BinaryFileSymbolOffset(i->second, addr);
    return BinaryFileSymbolOffset(BinaryFileSymbol("Unknown", addr, 1), addr);
}

const BinaryFileSymbol *Loader::get_symbol_by_name( const std::string &sym ) const
{
    std::map<uintptr_t, BinaryFileSymbol>::const_iterator i;
    for ( i = m_symbol_table.begin();
	  i != m_symbol_table.end();
	  ++i ) {
	if ( i->second.name() == sym )
	    return &(i->second);
    }
    return NULL;
}

Loader::loader_registry_t &
Loader::registry()
{
	static loader_registry_t reg;
	return reg;
}

void Loader::register_loader( const std::string &name,
										binary_loader_t loader )
{
    std::cout << "Registering loader" << std::endl;
	loader_registry_t &reg = registry();

	reg[name] = loader;
}

void Loader::load_file( const std::string &desc_str )
{
	loader_registry_t &reg = registry();
    if ( reg.empty() ) {
	throw exception::RunTimeError(
	    "There are no registered binary file format handler in the soclib::common::Loader class\n"
	    "This is most probably a misconfiguration of your platform.\n"
	    "Please try to use a loader in your platform description file\n"
	    "This should look like `Uses(\"common:elf_file_loader\")'.");
    }

	std::string tried;
	for ( loader_registry_t::iterator i = reg.begin();
		  i != reg.end();
		  i++ ) {
			tried += "\n ";
		tried += i->first;
	try {
	    if ( (i->second)( desc_str, *this ) )
		return;
	    tried += ": failed";
	} catch ( const soclib::exception::Exception &e ) {
	    tried = tried + ": failed (Exception: \"" + e.what() + "\")";
	}
	}
	throw exception::RunTimeError(
		std::string("Found no method able to load file ") +
		desc_str + " even if tried: " + tried);
}

void Loader::addSection( const BinaryFileSection &section )
{
    for ( section_list_t::iterator s = m_sections.begin();
	  s != m_sections.end();
	  s++ ) {
	if ( s->lma() >= section.lma()+section.size() || section.lma()+section.size() < section.lma() )
	    continue;
	if ( section.lma() >= s->lma()+s->size() || s->lma()+s->size() < s->lma() )
	    continue;
	std::ostringstream o;
	o << section << " overlaps " << *s;
	throw exception::RunTimeError(o.str());
    }
	m_sections.push_back(section);
}

void Loader::addSymbol( const BinaryFileSymbol &symbol )
{
	m_symbol_table[symbol.address()] = symbol;
}

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

