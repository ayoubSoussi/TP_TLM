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
#include <cassert>

#include <iostream>
#include <iomanip>

#include "binary_file_symbol.h"

namespace soclib { namespace common {

BinaryFileSymbol::BinaryFileSymbol( const std::string &name, uintptr_t address, size_t size )
	: m_name(name),
	  m_address(address),
	  m_size(size)
{}

BinaryFileSymbol::BinaryFileSymbol( const BinaryFileSymbol &ref )
    : m_name(ref.m_name),
      m_address(ref.m_address),
      m_size(ref.m_size)
{}

BinaryFileSymbol::BinaryFileSymbol()
    : m_name("Unknown"),
      m_address(0),
      m_size(0)
{}

const BinaryFileSymbol& BinaryFileSymbol::operator=( const BinaryFileSymbol &ref )
{
    m_name = ref.m_name;
    m_address = ref.m_address;
    m_size = ref.m_size;
    return *this;
}

bool BinaryFileSymbol::contains( uintptr_t addr ) const
{
    return addr >= m_address && addr < m_address+m_size;
}

BinaryFileSymbolOffset::BinaryFileSymbolOffset( const BinaryFileSymbol sym, const ptrdiff_t addr )
    : m_sym(sym),
      m_addr(addr)
{
}

BinaryFileSymbolOffset::BinaryFileSymbolOffset( const BinaryFileSymbolOffset &ref )
    : m_sym(ref.m_sym),
      m_addr(ref.m_addr)
{
}

void BinaryFileSymbolOffset::print( std::ostream &o ) const
{
    o << "[@" << std::showbase << std::hex
      << m_addr << ": ";
    ptrdiff_t d = m_addr - m_sym.address();
    if ( (size_t)d < m_sym.size() )
	o << '(' << m_sym.name() << " + " << d << ')';
    else
	o << '(' << m_sym.name() << ", " << d-m_sym.size() << " bytes above)";
    o << "]";
}

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

