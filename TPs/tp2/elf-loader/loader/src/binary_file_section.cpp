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

#include "binary_file_section.h"

namespace soclib { namespace common {

class BinarySectionData
{
    uint32_t m_refcount;
    size_t m_size;
    void *m_data;
public:
    BinarySectionData( size_t size, void *data )
	: m_refcount(0),
	  m_size(size),
	  m_data(data)
    {}

    ~BinarySectionData()
    {
	free(m_data);
    }

    void *data() const
    {
	return m_data;
    }

    size_t size() const
    {
	return m_size;
    }

    void ref()
    {
	m_refcount++;
    }

    void check() const
    {
	assert(m_refcount);
    }

    void unref()
    {
	if ( --m_refcount == 0 )
	    delete this;
    }

    void print( std::ostream &o ) const
    {
	o << "<BinarySectionData " << std::dec
	  << m_size << " bytes"
	  << ", refcount=" << m_refcount
	  << ">";
    }

    friend std::ostream &operator << (std::ostream &o, const BinarySectionData &d)
    {
	d.print(o);
	return o;
    }

};

const std::string & BinaryFileSection::name() const
{
	return m_name;
}

uintptr_t BinaryFileSection::vma() const
{
	return m_vma;
}

uintptr_t BinaryFileSection::lma() const
{
	return m_lma;
}

size_t BinaryFileSection::size() const
{
    return m_size;
}

bool BinaryFileSection::has_data() const
{
    return m_data;
}

bool BinaryFileSection::flag_load() const
{
    return m_flags & FLAG_LOAD;
}

bool BinaryFileSection::flag_read_only() const
{
    return m_flags & FLAG_READONLY;
}

bool BinaryFileSection::flag_code() const
{
    return m_flags & FLAG_CODE;
}

bool BinaryFileSection::flag_data() const
{
    return m_flags & FLAG_DATA;
}

BinaryFileSection::BinaryFileSection( const std::string &name,
			uintptr_t vma, uintptr_t lma,
			uint32_t flags, size_t size,
						void *blob )
    : m_data( blob ? new BinarySectionData( size, blob ) : 0 ),
      m_flags(flags),
      m_name(name),
      m_vma(vma),
      m_lma(lma),
      m_size(size)
{
	if ( m_data )
		m_data->ref();
    if ( m_data )
	m_data->check();

//    std::cout << "New BinaryFileSection " << *this << std::endl;
}

bool BinaryFileSection::load_overlap_in_buffer(
    void *buffer,
    uintptr_t buffer_base_address,
    size_t buffer_size ) const
{
	if ( !m_data )
		return false;

    void *src, *dst;
    size_t copy_size;
    if ( buffer_base_address < m_lma ) {
	size_t dest_buffer_offset = m_lma - buffer_base_address;

	dst = (void*)((uintptr_t)buffer + dest_buffer_offset);
	src = m_data->data();
	if ( dest_buffer_offset > buffer_size )
	    copy_size = 0;
	else
	    copy_size = buffer_size - dest_buffer_offset;
	if ( copy_size > m_data->size() )
	    copy_size = m_data->size();
    } else {
	size_t src_data_offset = buffer_base_address - m_lma;

	dst = buffer;
	src = (void*)((uintptr_t)m_data->data() + src_data_offset);
	if ( src_data_offset > m_data->size() )
	    copy_size = 0;
	else
	    copy_size = m_data->size() - src_data_offset;
	if ( copy_size > buffer_size )
	    copy_size = buffer_size;
    }
    if ( copy_size == 0 )
	return false;
    if ( m_data )
	m_data->check();

    if ( copy_size < m_data->size() )
	std::cout
	    << "Warning, loading only " << (m_data->size() - copy_size)
	    << " bytes from " << *this
	    << std::endl;
    memcpy( dst, src, copy_size );
	return true;
}

void BinaryFileSection::print( std::ostream &o ) const
{
	o << "<Section "
	  << std::showbase << m_name
	  << ", vma: " << std::hex << m_vma
	  << ", lma: " << std::hex << m_lma
	  << ", size: " << std::dec << m_size << " bytes";
	if ( m_data ) {
	m_data->check();
		o << ", " << *m_data;
	} else {
		o << ", no data";
	}
	o << ", flags: ";
	if ( flag_load() )
		o << "L,";
	if ( flag_read_only() )
		o << "RO,";
	if ( flag_code() )
		o << "C";
	if ( flag_data() )
		o << "D";
	o << ">";
}

void BinaryFileSection::get_data( void *buffer ) const
{
	if ( !m_data )
		return;
    m_data->check();
    memcpy( buffer, m_data->data(), m_data->size() );
}

BinaryFileSection::~BinaryFileSection()
{
//    std::cout << "Deleted BinaryFileSection " << *this << std::endl;
    if ( m_data )
	m_data->check();
    if ( m_data )
	m_data->unref();
}

BinaryFileSection::BinaryFileSection( const BinaryFileSection &ref )
    : m_data(0),
      m_flags(0),
      m_name("To be copied"),
      m_vma(0),
      m_lma(0),
      m_size(0)
{
//    std::cout << "New BinaryFileSection " << *this << " copied from " << ref << std::endl;
    (*this) = ref;
}

BinaryFileSection::BinaryFileSection()
    : m_data(0),
      m_flags(0),
      m_name("Empty section"),
      m_vma(0),
      m_lma(0),
      m_size(0)
{
//    std::cout << "New empty BinaryFileSection " << *this << std::endl;
}

BinaryFileSection & BinaryFileSection::operator=( const BinaryFileSection &ref )
{
    if ( &ref == this )
	return *this;

//    std::cout << "Copying " << ref << " to " << *this << std::endl;

    if ( m_data ) {
	m_data->check();
	m_data->unref();
    }
	if ( ref.m_data ) {
		m_data = ref.m_data;
		m_data->ref();
	m_data->check();
	} else {
	m_data = 0;
    }
    m_flags = ref.m_flags;
    m_name = ref.m_name;
    m_vma = ref.m_vma;
    m_lma = ref.m_lma;
    m_size = ref.m_size;

	return *this;
}


}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

