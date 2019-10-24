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
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_BINARY_LOADER_H_
#define SOCLIB_BINARY_LOADER_H_

#include <string>
#include <vector>
#include <map>
#include "stdint.h"

#include "binary_file_symbol.h"
#include "binary_file_section.h"

namespace soclib { namespace common {

class Loader
{
public:
    typedef std::vector<BinaryFileSection> section_list_t;
	typedef bool (*binary_loader_t)( const std::string &desc_str, Loader &loader );

private:
	typedef std::map<std::string, binary_loader_t> loader_registry_t;

    section_list_t m_sections;
    std::map<uintptr_t, BinaryFileSymbol> m_symbol_table;

	void load_file(const std::string &desc_str);

	static loader_registry_t &registry();
public:
	static void register_loader( const std::string &name,
								 binary_loader_t loader );

    section_list_t sections() const;

	void addSection( const BinaryFileSection &section );
	void addSymbol( const BinaryFileSymbol &symbol );

	Loader( const Loader &ref );
	Loader( const std::string &f = "",
	    const std::string &f2 = "",
	    const std::string &f3 = "",
	    const std::string &f4 = "",
	    const std::string &f5 = "",
	    const std::string &f6 = ""
	);
	void load( void *buffer, uintptr_t address, size_t length );
	~Loader();

    void print( std::ostream &o ) const;

    BinaryFileSymbolOffset get_symbol_by_addr( uintptr_t addr ) const;
    const BinaryFileSymbol *get_symbol_by_name( const std::string & ) const;

    friend std::ostream &operator << (std::ostream &o, const Loader &el)
    {
	el.print(o);
	return o;
    }
};

}}

#endif /* SOCLIB_BINARY_LOADER_H_ */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

