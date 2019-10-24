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
#ifndef SOCLIB_STATIC_FAST_INT_H_
#define SOCLIB_STATIC_FAST_INT_H_

#include <inttypes.h>

namespace soclib { namespace common {

namespace _fast_int {
template<int W>
struct _fast_int_t {};

#define tx(w, typ) \
template<> struct _fast_int_t<w> { typedef typ int_t; }

#define tx2(x, t) tx(x, t);tx(x+1, t)
#define tx4(x, t) tx2(x, t);tx2(x+2, t)
#define tx8(x, t) tx4(x, t);tx4(x+4, t)
#define tx16(x, t) tx8(x, t);tx8(x+8, t)
#define tx32(x, t) tx16(x, t);tx16(x+16, t)
tx(1, bool);
tx(2, uint8_t);
tx2(3, uint8_t);
tx4(5, uint8_t);
tx8(9, uint16_t);
tx16(17, uint32_t);
tx32(33, uint64_t);
#undef tx
#undef tx2
#undef tx4
#undef tx8
#undef tx16
#undef tx32
}

template<int W> struct fast_int_t {
    typedef typename ::soclib::common::_fast_int::_fast_int_t<W>::int_t int_t;
};

}}

#endif /* SOCLIB_STATIC_FAST_INT_H_ */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

