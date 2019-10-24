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
 * Copyright (c) UPMC, Lip6
 *         Nicolas Pouillon <nipo@ssji.net>, 2008
 *         Alain Greiner <alain.greiner@lip6.fr>, 2008
 *
 * Maintainers: nipo
 *
 * $Id$
 *
 * History:
 * - 2008-07-09
 *   Nicolas Pouillon, Alain Greiner: Forking ISS API to an improved
 *   one with:
 *  - Sync / prefetch / flush / ... opcodes
 *  - Mode (user/kernel/hypervisor)
 *  - Byte enable (unaligned memory access)
 *  - Virtual cache control
 *
 * - 2007-06-15
 *   Nicolas Pouillon, Alain Greiner: Model created
 */
#ifndef _SOCLIB_ISS2_H_
#define _SOCLIB_ISS2_H_

#include <inttypes.h>
#include <signal.h>
#include <iostream>

namespace soclib { namespace common {

/**
 * Iss2 API abstract class
 *
 * This Iss aims to define a common simulation behaviour for any
 * 32-bit simple-issue processor.
 *
 * Iss conforming to this API may be used by:
 *  - Tlmt cache wrappers
 *  - Caba cache wrappers
 *  - Caba Iss wrappers (with cache access through signals)
 *
 * Cache wrappers at least include:
 *  - XCacheWrapper (Simple I/D cache)
 *  - ccXCacheWrapper (coherent I/D cache)
 *  - VCacheWrapper (MMU-enabled I/D cache)
 *
 * Some instrumentation classes also implement this API and may be
 * used between the Iss and the wrapper, including:
 *  - GdbServer
 *  - IssProfiler
 *
 * You may want to use first-generation Iss instanciating them through
 * an IssIss2 wrapper. See soclib/lib/ississ2.
 */

class Iss2
{
public:
    /** Address type from/to the Iss */
    typedef uint32_t addr_t;
    /** Debug register from/to the Iss */
    typedef uint32_t debug_register_t;
    /** Byte enable field from the Iss for data access
     *
     * The lower endian bit in the BE field targets the lower address
     * byte in word.
     *
     * You could consider this API as Little-endian.
     */
    typedef uint8_t be_t;
    /**
     * Data type from/to the Iss
     *
     * The lower significant byte in data word is at the lower
     * address.
     *
     * You could consider this API as Little-endian.
     */
    typedef uint32_t data_t;

    /**
     * Execution mode for any Instruction/Data access, checked by
     * mode-enabled caches */
    enum ExecMode {
        MODE_HYPER,
        MODE_KERNEL,
        MODE_USER,
    };

    /** Operation type on Data cache access */
    enum DataOperationType {
        DATA_READ,
        DATA_WRITE,
        DATA_LL,
        DATA_LR = DATA_LL,
        DATA_SC,
        XTN_WRITE,
        XTN_READ,

        /* AMO operations as defined in the Atomic Standard Extension of RISC-V */
        DATA_AMO_SWAP,
        DATA_AMO_ADD,
        DATA_AMO_AND,
        DATA_AMO_OR,
        DATA_AMO_XOR,
        DATA_AMO_MAX,
        DATA_AMO_MAXU,
        DATA_AMO_MIN,
        DATA_AMO_MINU,
    };

    /** Exception classes, keep it simple with 4 entries ! */
    enum ExceptionClass {
        EXCL_FAULT,
        EXCL_IRQ,
        EXCL_SYSCALL,
        EXCL_TRAP,
    };

    /** Exception cause details, expand as needed and update EXCEPTIONCAUSE_STRINGS too */
    enum ExceptionCause {
        EXCA_OTHER,
        EXCA_BADADDR,
        EXCA_ALIGN,             // non aligned access not handled
        EXCA_PAGEFAULT,
        EXCA_ILL,               // illegal instruction
        EXCA_FPU,
        EXCA_REGWINDOW,
        EXCA_DIVBYZERO
    };

#define EXCEPTIONCAUSE_STRINGS "unknown cause", "bad address", "bad alignment", \
        "page fault", "illegal instruction", "fpu exception", "register window", \
        "division by zero"

    enum {
        SC_ATOMIC = 0,
        SC_NOT_ATOMIC = 1,
    };

    /**
     * When operation is XTN_READ or XTN_WRITE, address field must be
     * one of these values, it determines the extended access type.
     */
    enum ExternalAccessType {
        XTN_PTPR               = 0,
        XTN_TLB_MODE           = 1,
        XTN_ICACHE_FLUSH       = 2,
        XTN_DCACHE_FLUSH       = 3,
        XTN_ITLB_INVAL         = 4,
        XTN_DTLB_INVAL         = 5,
        XTN_ICACHE_INVAL       = 6,
        XTN_DCACHE_INVAL       = 7,
        XTN_ICACHE_PREFETCH    = 8,
        XTN_DCACHE_PREFETCH    = 9,
        XTN_SYNC               = 10,
        XTN_INS_ERROR_TYPE     = 11,
        XTN_DATA_ERROR_TYPE    = 12,
        XTN_INS_BAD_VADDR      = 13,
        XTN_DATA_BAD_VADDR     = 14,
        XTN_MMU_PARAMS         = 15,
        XTN_MMU_RELEASE        = 16,
        XTN_MMU_WORD_LO        = 17,
        XTN_MMU_WORD_HI        = 18,
        XTN_MMU_ICACHE_PA_INV  = 19,
        XTN_MMU_DCACHE_PA_INV  = 20,
        XTN_MMU_LL_RESET       = 21,
        XTN_MMU_DOUBLE_LL      = 22,
        XTN_MMU_DOUBLE_SC      = 23,
        XTN_DATA_PADDR_EXT     = 24,
        XTN_INST_PADDR_EXT     = 25,

        XTN_DEBUG_MASK         = 30,
    };

    /**
     * Instruction request, only significant if `valid' is asserted.
     *
     * addr must be 4-byte aligned.
     */
    struct InstructionRequest {
        bool valid;
        addr_t addr;
        enum ExecMode mode;

        void print( std::ostream &o ) const;

        friend std::ostream &operator << (std::ostream &o, const struct InstructionRequest &ir)
        {
            ir.print(o);
            return o;
        }

        inline bool operator==( const struct InstructionRequest &oreq )
        {
            return
                valid == oreq.valid &&
                addr == oreq.addr &&
                mode == oreq.mode;
        }
    };
#define ISS_IREQ_INITIALIZER {false, 0, ::soclib::common::Iss2::MODE_HYPER}

    /**
     * Data request, only significant if `valid' is asserted.
     *
     * addr must be 4-byte aligned.
     * wdata is only significant for be-masked bytes.
     * wdata[7:0] is at [addr], masked by be[0]
     * wdata[15:8] is at [addr+1], masked by be[1]
     * wdata[23:16] is at [addr+2], masked by be[2]
     * wdata[31:24] is at [addr+3], masked by be[3]
     *
     * When type is XTN_READ or XTN_WRITE, addr must be an opcod of
     * enum ExternalAccessType.  For extended access types needing an
     * address, address is passed through the wdata field.
     */
    struct DataRequest {
        bool valid;
        addr_t addr;
        data_t wdata;
        enum DataOperationType type;
        be_t be;
        enum ExecMode mode;

        void print( std::ostream &o ) const;

        friend std::ostream &operator << (std::ostream &o, const struct DataRequest &ir)
        {
            ir.print(o);
            return o;
        }

        inline bool operator==( const struct DataRequest &oreq )
        {
            return
                valid == oreq.valid &&
                addr == oreq.addr &&
                wdata == oreq.wdata &&
                type == oreq.type &&
                be == oreq.be &&
                mode == oreq.mode;
        }
    };
#define ISS_DREQ_INITIALIZER {false, 0, 0, ::soclib::common::Iss2::DATA_READ, 0, ::soclib::common::Iss2::MODE_HYPER}

    /**
     * Instruction response.
     *
     * Valid is asserted when query has beed satisfied, if no request
     * is pending, valid is not asserted.
     *
     * instruction is only valid if no error is signaled.
     */
    struct InstructionResponse {
        bool valid;
        bool error;
        data_t instruction;

        void print( std::ostream &o ) const;

        friend std::ostream &operator << (std::ostream &o, const struct InstructionResponse &ir)
        {
            ir.print(o);
            return o;
        }
    };
#define ISS_IRSP_INITIALIZER {false, false, 0}

    /**
     * Data response.
     *
     * Valid is asserted when query has beed satisfied, if no request
     * is pending, valid is not asserted.
     *
     * data is only valid if no error is signaled.
     *
     * Read data is aligned with the same semantics than the wdata
     * field in struct DataRequest. Only bytes asserted in the BE
     * field upon request are meaningful, others have an undefined
     * value, they may be non-zero.
     */
    struct DataResponse {
        bool valid;
        bool error;
        data_t rdata;

        void print( std::ostream &o ) const;

        friend std::ostream &operator << (std::ostream &o, const struct DataResponse &ir)
        {
            ir.print(o);
            return o;
        }
    };
#define ISS_DRSP_INITIALIZER {false, false, 0}

protected:

    /**
     * Cpu ID
     */
    const uint32_t m_ident;
    /**
     * Iss instance name
     */
    const std::string m_name;
    /**
     * debug mask
     */
    uint m_debug_mask;

public:
    virtual ~Iss2() {}

    /**
     * Name accessor
     */
    inline const std::string & name() const
    {
        return m_name;
    }

    Iss2( const std::string &name, uint32_t ident )
        : m_ident(ident),
          m_name(name)
    {
    }

    // ISS2 <-> Wrapper API

    /**
     * Reset operation, Iss must behave like the processor receiving a reset cycle.
     */
    virtual void reset() = 0;

    /**
     * Tell the Iss to execute *at most* ncycle cycles, knowing the
     * value of all the irq lines. Each irq is a bit in the
     * irq_bit_field word.
     *
     * Iss must return the number of cycles it actually executed. This
     * is at least 1, at most ncycle.
     */
    virtual uint32_t executeNCycles(
        uint32_t ncycle,
        const struct InstructionResponse &,
        const struct DataResponse &,
        uint32_t irq_bit_field ) = 0;

    /**
     * This function is used to translate a virtual address to
     * physical address Return false if address not mapped.
     */
    virtual bool virtualToPhys(addr_t &addr) const
    {
        return true;
    }

    /**
     * Iss must populate the request fields.
     */
    virtual void getRequests( struct InstructionRequest &,
                              struct DataRequest & ) const = 0;

    /**
     * The cache received an imprecise write error condition, this
     * signalling is asynchronous.
     */
    virtual void setWriteBerr() = 0;

    struct CacheInfo
    {
        bool has_mmu;
        size_t icache_line_size;
        size_t icache_assoc;
        size_t icache_n_lines;
        size_t dcache_line_size;
        size_t dcache_assoc;
        size_t dcache_n_lines;
    };

    /**
     * Inform the Iss about the cache
     */
    virtual void setCacheInfo( const struct CacheInfo &info )
    {
    }

    /*
     * Debugger API
     */

    /**
     * Iss must return the count of registers known to GDB. This must
     * follow GDB protocol for this architecture.
     */
    virtual unsigned int debugGetRegisterCount() const = 0;
    /**
     * Accessor for an Iss register, register number meaning is
     * defined in GDB protocol for this architecture.
     */
    virtual debug_register_t debugGetRegisterValue(unsigned int reg) const = 0;
    /**
     * Accessor for an Iss register, register number meaning is
     * defined in GDB protocol for this architecture. Special virtual
     * register ids defined by debugSpecialRegisters enum can be used
     * to get additionnal information on current processor state.
     */
    virtual void debugSetRegisterValue(unsigned int reg, debug_register_t value) = 0;
    /**
     * Get the size for a given register. This is defined in GDB
     * protocol for this architecture.
     */
    virtual size_t debugGetRegisterSize(unsigned int reg) const = 0;
    /**
     * List of additional registers for gdb in gdb xml format.
     * By default none as gdb handles the minimal possible set
     */
    virtual char *debugRegistersXmlDescription(char *pool)
    {
        return NULL;
    }

    enum debugCpuEndianness {
        ISS_LITTLE_ENDIAN,
        ISS_BIG_ENDIAN,
    };

    enum debugSpecialRegisters {
        /** is non-zero if processor is currently executing user code */
        ISS_DEBUG_REG_IS_USERMODE               = 100000,
        /** is non-zero if processor can react on irqs */
        ISS_DEBUG_REG_IS_INTERRUPTIBLE          = 100001,
        /** give number of bytes which can be legitimately accessed below stack pointer */
        ISS_DEBUG_REG_STACK_REDZONE_SIZE        = 100002,
    };

    /** Dump processor state (optional) */
    virtual void dump() const
    {
    }

    /** set debug mask */
    inline void set_debug_mask(uint v) {
        m_debug_mask = v;
    }

protected:
    
    /**
     * On exception condition, an Iss should call this method to let
     * the debugger inform the monitoring entity. If return value is
     * true, Iss must not jump to exception handler and continue with
     * execution. This permits implementation of software breakpoints.
     */
    virtual bool debugExceptionBypassed( ExceptionClass cl, ExceptionCause ca  = EXCA_OTHER )
    {
        return false;
    }

};

const char *mode_str(Iss2::ExecMode mode);
const char *type_str(Iss2::DataOperationType type);
const char *xtn_str(Iss2::ExternalAccessType type);

}}

#endif // _SOCLIB_ISS2_H_

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4
