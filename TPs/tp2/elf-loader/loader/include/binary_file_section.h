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
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_BINARY_FILE_SECTION_H_
#define SOCLIB_BINARY_FILE_SECTION_H_

#include <string>
#include <ios>

#include "stdint.h"

#include "binary_file_section.h"

namespace soclib { namespace common {

class BinarySectionData;
class Loader;

class BinaryFileSection
{
    friend class Loader;

    BinarySectionData *m_data;
    uint32_t m_flags;
    std::string m_name;
    uintptr_t m_vma;
    uintptr_t m_lma;
    size_t m_size;

public:
	enum flags {
		FLAG_LOAD = 1,
		FLAG_DATA = 2,
		FLAG_READONLY = 4,
		FLAG_CODE = 8,
	};

    bool load_overlap_in_buffer( void *buffer,
				 uintptr_t buffer_base_address,
				 uintptr_t buffer_size ) const;
    void get_data( void *buffer ) const;

    bool has_data() const;

    bool flag_load() const;
    bool flag_read_only() const;
    bool flag_code() const;
    bool flag_data() const;
    const std::string& name() const;
    uintptr_t vma() const;
    uintptr_t lma() const;
    size_t size() const;

    BinaryFileSection( const BinaryFileSection & );
    ~BinaryFileSection();
    BinaryFileSection &operator=( const BinaryFileSection & );
    BinaryFileSection();

    void print( std::ostream &o ) const;

    friend std::ostream &operator<<( std::ostream &o, const BinaryFileSection &s )
    {
	s.print(o);
	return o;
    }

    BinaryFileSection( const std::string &name,
				   uintptr_t vma, uintptr_t lma,
				   uint32_t flags,
				   size_t data_size, void *given_data_ptr );
};

}}

#endif /* SOCLIB_BINARY_FILE_SECTION_H_ */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

