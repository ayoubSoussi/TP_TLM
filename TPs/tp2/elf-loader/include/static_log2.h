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
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 *
 * Maintainers: nipo
 */
#ifndef SOCLIB_STATIC_LOG2_H_
#define SOCLIB_STATIC_LOG2_H_

#include <inttypes.h>

namespace soclib { namespace common {

template<uint32_t base_nb>
struct StaticLog2;

// This is a reverse definition: for a template struct with
// parameter 1<<x, it defines x. This allows us to use
// StaticLog2<y> to obtain log2(y)
#define tx(val) \
template<>struct StaticLog2<((uint32_t)1)<<(val)>{static const uint32_t value = (val);}

// Rather make 32 definitions in one pass...
#define tx2(x) tx(x);tx(x+1)
#define tx4(x) tx2(x);tx2(x+2)
#define tx8(x) tx4(x);tx4(x+4)
#define tx16(x) tx8(x);tx8(x+8)
#define tx32(x) tx16(x);tx16(x+16)
tx32(0);
#undef tx
#undef tx2
#undef tx4
#undef tx8
#undef tx16
#undef tx32

}}

#define static_log2(x) (soclib::common::StaticLog2<(uint32_t)(x)>::value)

#endif /* SOCLIB_STATIC_LOG2_H_ */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

