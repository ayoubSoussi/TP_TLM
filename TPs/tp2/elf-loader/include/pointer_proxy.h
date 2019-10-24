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
 * Copyright (c) UPMC, Lip6, Asim
 *         Nicolas Pouillon <nipo@ssji.net>, 2009
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_COMMON_POINTER_PROXY_H
#define SOCLIB_COMMON_POINTER_PROXY_H

#include <cassert>

namespace soclib { namespace common {

template <typename T>
class PointerProxy
{
    const PointerProxy &operator =( const PointerProxy & );
    PointerProxy( const PointerProxy & );

    size_t m_length;
    T ** m_entries;

public:
    PointerProxy( size_t length = 0 )
	: m_length(0),
	  m_entries(NULL)
    {
	resize(length);
    }

    void resize( size_t length )
    {
	if ( m_entries )
	    delete [] m_entries;
	if ( length ) {
	    m_length = length;
	    m_entries = new T*[length];
	}
    }

    ~PointerProxy()
    {
	resize(0);
    }

    void set( size_t k, T* v )
    {
	assert( k < m_length );
	m_entries[k] = v;
    }

    T &operator[]( size_t k )
    {
	assert( k < m_length );
	return *m_entries[k];
    }
};

}}

#endif /* SOCLIB_COMMON_POINTER_PROXY_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

