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
#ifndef SOCLIB_STATIC_INIT_CODE_H
#define SOCLIB_STATIC_INIT_CODE_H

#define _STATIC_INIT_CODE_AT( lno, code )                              \
namespace static_init_code_ ## lno { namespace {                       \
class FooClass                                                         \
{                                                                      \
    int foo;                                                           \
    FooClass();                                                        \
    static FooClass static_member;                                     \
};                                                                     \
FooClass FooClass::static_member;                                      \
FooClass::FooClass()                                                   \
    : foo(0)                                                           \
{                                                                      \
    code                                                               \
}                                                                      \
}}

#define _STATIC_INIT_CODE_AT_LINE( lno, code ) \
    _STATIC_INIT_CODE_AT( lno, code )

#define STATIC_INIT_CODE(x...) _STATIC_INIT_CODE_AT_LINE(__LINE__, {x})

#endif /* SOCLIB_STATIC_INIT_CODE_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
