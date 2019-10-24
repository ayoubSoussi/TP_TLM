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
#ifndef SOCLIB_COMMON_REGISTER_H
#define SOCLIB_COMMON_REGISTER_H

#include <stdint.h>
#include <systemc>

class scalar_t {
    typedef     uint32_t                    reg_32_t;
    typedef     uint8_t                     reg_8_t;
    typedef     bool                        reg_bool_t;
    };



#if defined(NONAME_RENAME)
#  define SOCLIB_REG_RENAME(reg)  reg.rename(#reg)
#  define SOCLIB_REG_RENAME_NAME(name, reg)                               \
  do {                                                                    \
      char __nn[128];                                                     \
      sprintf(__nn, "%s_" #reg, name);                                    \
      reg.rename(__nn);                                                   \
  } while (0)
#  define SOCLIB_REG_RENAME_N_NAME(name, reg,n)							\
  do {																	\
      char __nn[128];														\
      sprintf(__nn, "%s_" #reg "_%d", name, n);							\
      reg[n].rename(__nn);												\
  } while (0)
#  define SOCLIB_REG_RENAME_N2_NAME(name, reg,n1, n2)						\
  do {																	\
      char __nn[128];														\
      sprintf(__nn, "%s_" #reg "_%d_%d", name, n1, n2);					\
      reg[n1][n2].rename(__nn);												\
  } while (0)
#  define SOCLIB_REG_RENAME_N3_NAME(name, reg,n1, n2, n3)					\
  do {																	\
      char __nn[128];														\
      sprintf(__nn, "%s_" #reg "_%d_%d_%d", name, n1, n2, n3);			\
      reg[n1][n2][n3].rename(__nn);												\
  } while (0)
#elif defined(MODELSIM)
#  define SOCLIB_REG_RENAME(reg)  reg.sc_bind_mti_obj_name(#reg)
#  define SOCLIB_REG_RENAME_NAME(name, reg)                               \
  do {                                                                    \
      char __nn[128];                                                     \
      sprintf(__nn, "%s_" #reg, name);                                    \
      reg.sc_bind_mti_obj_name(__nn);                                     \
  } while (0)
#  define SOCLIB_REG_RENAME_N_NAME(name, reg,n)                           \
  do {                                                                    \
      char __nn[128];                                                     \
      sprintf(__nn, "%s_" #reg "[%d]", (const char*)this->name, n);       \
      reg[n].sc_bind_mti_obj_name(__nn);                                  \
  } while (0)
#  define SOCLIB_REG_RENAME_N2_NAME(name, reg,n1, n2)							\
  do {																		\
      char __nn[128];															\
      sprintf(__nn, "%s_" #reg "[%d][%d]", (const char*)this->name, n1, n2);	\
      reg[n1][n2].sc_bind_mti_obj_name(__nn);										\
  } while (0)
#  define SOCLIB_REG_RENAME_N3_NAME(name, reg,n1, n2, n3)								\
  do {																				\
      char __nn[128];																	\
      sprintf(__nn, "%s_" #reg "[%d][%d][%d]", (const char*)this->name, n1, n2, n3);	\
      reg[n1][n2][n3].sc_bind_mti_obj_name(__nn);												\
  } while (0)
#else
#  define SOCLIB_REG_RENAME(reg) do{}while(0)
#  define SOCLIB_REG_RENAME_NAME(name,reg) do{}while(0)
#  define SOCLIB_REG_RENAME_N_NAME(name, reg,n) do{}while(0)
#  define SOCLIB_REG_RENAME_N2_NAME(name, reg,n1, n2) do{}while(0)
#  define SOCLIB_REG_RENAME_N3_NAME(name, reg,n1, n2, n3) do{}while(0)
#endif


#define SOCLIB_REG_RENAME_N(reg,n) \
SOCLIB_REG_RENAME_N_NAME((const char*)this->name().c_str(), reg,n)

#define SOCLIB_REG_RENAME_N2(reg,n1, n2) \
SOCLIB_REG_RENAME_N2_NAME((const char*)this->name().c_str(), reg,n1, n2)

#define SOCLIB_REG_RENAME_N3(reg,n1, n2, n3) \
SOCLIB_REG_RENAME_N3_NAME((const char*)this->name().c_str(), reg,n1, n2, n3)

#endif /* SOCLIB_COMMON_REGISTER_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

