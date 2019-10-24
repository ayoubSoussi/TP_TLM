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
#ifndef SOCLIB_BINARY_FILE_SYMBOL_H_
#define SOCLIB_BINARY_FILE_SYMBOL_H_

#include <string>
#include "stdint.h"
#include <ios>

using std::ptrdiff_t;

namespace soclib { namespace common {

class BinaryFileSymbol
{
    std::string m_name;
    uintptr_t m_address;
    size_t m_size;
public:
    const std::string &name() const
    {
	return m_name;
    }

    uintptr_t address() const
    {
	return m_address;
    }

    size_t size() const
    {
	return m_size;
    }

    BinaryFileSymbol( const std::string &name, uintptr_t address, size_t size );

    BinaryFileSymbol( const BinaryFileSymbol &ref );

    BinaryFileSymbol();

    const BinaryFileSymbol& operator=( const BinaryFileSymbol &ref );

    bool contains( uintptr_t addr ) const;
};

class BinaryFileSymbolOffset
{
    const BinaryFileSymbol m_sym;
    const ptrdiff_t m_addr;

public:
    BinaryFileSymbolOffset( const BinaryFileSymbol sym, const ptrdiff_t addr );

    BinaryFileSymbolOffset( const BinaryFileSymbolOffset &ref );

    void print( std::ostream &o ) const;

    friend std::ostream & operator<<( std::ostream &o, const BinaryFileSymbolOffset &so )
    {
	so.print(o);
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

