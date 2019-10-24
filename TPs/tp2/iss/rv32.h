/*\
 * vim: tw=0: cindent: sw=3: ts=3: sts=3: noet: list
 *
 * RISC-V Instruction Set Simulator, developed for the SoCLib Project
 * Copyright (C) 2018  SLS Group of the TIMA Lab, Grenoble-INP, CNRS
 *
 * Contributing authors:
 *    Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr>
 *    Bootstrapped the developement
 *
 *    César Fuguet <c.sarfuguet@gmail.com>
 *    Adapt the model to the SOCLIB's ISS2 API
 *
 * For now, interprets only the RV32IMFC instructions.
\*/

#ifndef _SOCLIB_RV32_ISS_H_
#define _SOCLIB_RV32_ISS_H_

#include "iss2.h"
#include "soclib_endian.h"
#include "register.h"
#include "rv32xml.h"

/*\
 *  Rv32 Processor structure definition
\*/
namespace soclib {
namespace common {
	class Rv32Iss
		: public soclib::common::Iss2
	{
	private:
		// Vectors are platform specific according to section 3.3 of the priviledged spec
		// We use one we saw somewhere, ..., all are created equals I believe
		// The mtvec register indicates the address to handle the other traps
		enum Vectors {
			RESET_VECTOR     = 0x80000000 // Let's do it the easy way
		};
		// According to Volume II: RISC-V Privileged Architectures V1.11-draft, p.-35
		enum Exception_Cause {
			USER_SOFTWARE_INTERRUPT_TRAP         = 0x80000000,
			SUPERVISOR_SOFTWARE_INTERRUPT_TRAP   = 0x80000001,
			MACHINE_SOFTWARE_INTERRUPT_TRAP      = 0x80000003,
			USER_TIMER_INTERRUPT_TRAP            = 0x80000004,
			SUPERVISOR_TIMER_INTERRUPT_TRAP      = 0x80000005,
			MACHINE_TIMER_INTERRUPT_TRAP         = 0x80000007,
			USER_EXTERNAL_INTERRUPT_TRAP         = 0x80000008,
			SUPERVISOR_EXTERNAL_INTERRUPT_TRAP   = 0x80000009,
			MACHINE_EXTERNAL_INTERRUPT_TRAP      = 0x8000000b,
			INSTRUCTION_ADDRESS_MISALIGNED_TRAP  = 0x00000000,
			INSTRUCTION_ACCESS_FAULT_TRAP        = 0x00000001,
			ILLEGAL_INSTRUCTION_TRAP             = 0x00000002,
			BREAKPOINT_TRAP                      = 0x00000003,
			LOAD_ADDRESS_MISALIGNED_TRAP         = 0x00000004,
			LOAD_ACCESS_FAULT_TRAP               = 0x00000005,
			STORE_AMO_ADDRESS_MISALIGNED_TRAP    = 0x00000006,
			STORE_AMO_ACCESS_FAULT_TRAP          = 0x00000007,
			ENVIRONMENT_CALL_FROM_U_MODE_TRAP    = 0x00000008,
			ENVIRONMENT_CALL_FROM_S_MODE_TRAP    = 0x00000009,
			ENVIRONMENT_CALL_FROM_M_MODE_TRAP    = 0x0000000b,
			INSTRUCTION_PAGE_FAULT_TRAP          = 0x0000000c,
			LOAD_PAGE_FAULT_TRAP                 = 0x0000000d,
			STORE_AMO_PAGE_FAULT_TRAP            = 0x0000000f
		};

		// CSR offsets
		enum csr_type {
			csr_ustatus        = 0x000,
			csr_uie            = 0x004,
			csr_utvec          = 0x005,
			csr_uscratch       = 0x040,
			csr_uepc           = 0x041,
			csr_ucause         = 0x042,
			csr_utval          = 0x043,
			csr_uip            = 0x044,
			csr_fflags         = 0x001,
			csr_frm            = 0x002,
			csr_fcsr           = 0x003,
			csr_cycle          = 0xc00,
			csr_time           = 0xc01,
			csr_instret        = 0xc02,
			csr_hpmcounter3    = 0xc03,
			csr_hpmcounter4    = 0xc04,
			csr_hpmcounter5    = 0xc05,
			csr_hpmcounter6    = 0xc06,
			csr_hpmcounter7    = 0xc07,
			csr_hpmcounter8    = 0xc08,
			csr_hpmcounter9    = 0xc09,
			csr_hpmcounter10   = 0xc0a,
			csr_hpmcounter11   = 0xc0b,
			csr_hpmcounter12   = 0xc0c,
			csr_hpmcounter13   = 0xc0d,
			csr_hpmcounter14   = 0xc0e,
			csr_hpmcounter15   = 0xc0f,
			csr_hpmcounter16   = 0xc10,
			csr_hpmcounter17   = 0xc11,
			csr_hpmcounter18   = 0xc12,
			csr_hpmcounter19   = 0xc13,
			csr_hpmcounter20   = 0xc14,
			csr_hpmcounter21   = 0xc15,
			csr_hpmcounter22   = 0xc16,
			csr_hpmcounter23   = 0xc17,
			csr_hpmcounter24   = 0xc18,
			csr_hpmcounter25   = 0xc19,
			csr_hpmcounter26   = 0xc1a,
			csr_hpmcounter27   = 0xc1b,
			csr_hpmcounter28   = 0xc1c,
			csr_hpmcounter29   = 0xc1d,
			csr_hpmcounter30   = 0xc1e,
			csr_hpmcounter31   = 0xc1f,
			csr_cycleh         = 0xc80,
			csr_timeh          = 0xc81,
			csr_instreth       = 0xc82,
			csr_hpmcounter3h   = 0xc83,
			csr_hpmcounter4h   = 0xc84,
			csr_hpmcounter5h   = 0xc85,
			csr_hpmcounter6h   = 0xc86,
			csr_hpmcounter7h   = 0xc87,
			csr_hpmcounter8h   = 0xc88,
			csr_hpmcounter9h   = 0xc89,
			csr_hpmcounter10h  = 0xc8a,
			csr_hpmcounter11h  = 0xc8b,
			csr_hpmcounter12h  = 0xc8c,
			csr_hpmcounter13h  = 0xc8d,
			csr_hpmcounter14h  = 0xc8e,
			csr_hpmcounter15h  = 0xc8f,
			csr_hpmcounter16h  = 0xc90,
			csr_hpmcounter17h  = 0xc91,
			csr_hpmcounter18h  = 0xc92,
			csr_hpmcounter19h  = 0xc93,
			csr_hpmcounter20h  = 0xc94,
			csr_hpmcounter21h  = 0xc95,
			csr_hpmcounter22h  = 0xc96,
			csr_hpmcounter23h  = 0xc97,
			csr_hpmcounter24h  = 0xc98,
			csr_hpmcounter25h  = 0xc99,
			csr_hpmcounter26h  = 0xc9a,
			csr_hpmcounter27h  = 0xc9b,
			csr_hpmcounter28h  = 0xc9c,
			csr_hpmcounter29h  = 0xc9d,
			csr_hpmcounter30h  = 0xc9e,
			csr_hpmcounter31h  = 0xc9f,
			csr_sstatus        = 0x100,
			csr_sedeleg        = 0x102,
			csr_sideleg        = 0x103,
			csr_sie            = 0x104,
			csr_stvec          = 0x105,
			csr_scounteren     = 0x106,
			csr_sscratch       = 0x140,
			csr_sepc           = 0x141,
			csr_scause         = 0x142,
			csr_stval          = 0x143,
			csr_sip            = 0x144,
			csr_satp           = 0x180,
			csr_mvendorid      = 0xf11,
			csr_marchid        = 0xf12,
			csr_mimpid         = 0xf13,
			csr_mhartid        = 0xf14,
			csr_mstatus        = 0x300,
			csr_misa           = 0x301,
			csr_medeleg        = 0x302,
			csr_mideleg        = 0x303,
			csr_mie            = 0x304,
			csr_mtvec          = 0x305,
			csr_mcounteren     = 0x306,
			csr_mscratch       = 0x340,
			csr_mepc           = 0x341,
			csr_mcause         = 0x342,
			csr_mtval          = 0x343,
			csr_mip            = 0x344,
			csr_pmpcfg0        = 0x3a0,
			csr_pmpcfg1        = 0x3a1,
			csr_pmpcfg2        = 0x3a2,
			csr_pmpcfg3        = 0x3a3,
			csr_pmpaddr0       = 0x3b0,
			csr_pmpaddr1       = 0x3b1,
			csr_pmpaddr2       = 0x3b2,
			csr_pmpaddr3       = 0x3b3,
			csr_pmpaddr4       = 0x3b4,
			csr_pmpaddr5       = 0x3b5,
			csr_pmpaddr6       = 0x3b6,
			csr_pmpaddr7       = 0x3b7,
			csr_pmpaddr8       = 0x3b8,
			csr_pmpaddr9       = 0x3b9,
			csr_pmpaddr10      = 0x3ba,
			csr_pmpaddr11      = 0x3bb,
			csr_pmpaddr12      = 0x3bc,
			csr_pmpaddr13      = 0x3bd,
			csr_pmpaddr14      = 0x3be,
			csr_pmpaddr15      = 0x3bf,
			csr_mcycle         = 0xb00,
			csr_minstret       = 0xb02,
			csr_mhpmcounter3   = 0xb03,
			csr_mhpmcounter4   = 0xb04,
			csr_mhpmcounter5   = 0xb05,
			csr_mhpmcounter6   = 0xb06,
			csr_mhpmcounter7   = 0xb07,
			csr_mhpmcounter8   = 0xb08,
			csr_mhpmcounter9   = 0xb09,
			csr_mhpmcounter10  = 0xb0a,
			csr_mhpmcounter11  = 0xb0b,
			csr_mhpmcounter12  = 0xb0c,
			csr_mhpmcounter13  = 0xb0d,
			csr_mhpmcounter14  = 0xb0e,
			csr_mhpmcounter15  = 0xb0f,
			csr_mhpmcounter16  = 0xb10,
			csr_mhpmcounter17  = 0xb11,
			csr_mhpmcounter18  = 0xb12,
			csr_mhpmcounter19  = 0xb13,
			csr_mhpmcounter20  = 0xb14,
			csr_mhpmcounter21  = 0xb15,
			csr_mhpmcounter22  = 0xb16,
			csr_mhpmcounter23  = 0xb17,
			csr_mhpmcounter24  = 0xb18,
			csr_mhpmcounter25  = 0xb19,
			csr_mhpmcounter26  = 0xb1a,
			csr_mhpmcounter27  = 0xb1b,
			csr_mhpmcounter28  = 0xb1c,
			csr_mhpmcounter29  = 0xb1d,
			csr_mhpmcounter30  = 0xb1e,
			csr_mhpmcounter31  = 0xb1f,
			csr_mcycleh        = 0xb80,
			csr_minstreth      = 0xb82,
			csr_mhpmcounter3h  = 0xb83,
			csr_mhpmcounter4h  = 0xb84,
			csr_mhpmcounter5h  = 0xb85,
			csr_mhpmcounter6h  = 0xb86,
			csr_mhpmcounter7h  = 0xb87,
			csr_mhpmcounter8h  = 0xb88,
			csr_mhpmcounter9h  = 0xb89,
			csr_mhpmcounter10h = 0xb8a,
			csr_mhpmcounter11h = 0xb8b,
			csr_mhpmcounter12h = 0xb8c,
			csr_mhpmcounter13h = 0xb8d,
			csr_mhpmcounter14h = 0xb8e,
			csr_mhpmcounter15h = 0xb8f,
			csr_mhpmcounter16h = 0xb90,
			csr_mhpmcounter17h = 0xb91,
			csr_mhpmcounter18h = 0xb92,
			csr_mhpmcounter19h = 0xb93,
			csr_mhpmcounter20h = 0xb94,
			csr_mhpmcounter21h = 0xb95,
			csr_mhpmcounter22h = 0xb96,
			csr_mhpmcounter23h = 0xb97,
			csr_mhpmcounter24h = 0xb98,
			csr_mhpmcounter25h = 0xb99,
			csr_mhpmcounter26h = 0xb9a,
			csr_mhpmcounter27h = 0xb9b,
			csr_mhpmcounter28h = 0xb9c,
			csr_mhpmcounter29h = 0xb9d,
			csr_mhpmcounter30h = 0xb9e,
			csr_mhpmcounter31h = 0xb9f,
			csr_mhpmevent3     = 0x323,
			csr_mhpmevent4     = 0x324,
			csr_mhpmevent5     = 0x325,
			csr_mhpmevent6     = 0x326,
			csr_mhpmevent7     = 0x327,
			csr_mhpmevent8     = 0x328,
			csr_mhpmevent9     = 0x329,
			csr_mhpmevent10    = 0x32a,
			csr_mhpmevent11    = 0x32b,
			csr_mhpmevent12    = 0x32c,
			csr_mhpmevent13    = 0x32d,
			csr_mhpmevent14    = 0x32e,
			csr_mhpmevent15    = 0x32f,
			csr_mhpmevent16    = 0x330,
			csr_mhpmevent17    = 0x331,
			csr_mhpmevent18    = 0x332,
			csr_mhpmevent19    = 0x333,
			csr_mhpmevent20    = 0x334,
			csr_mhpmevent21    = 0x335,
			csr_mhpmevent22    = 0x336,
			csr_mhpmevent23    = 0x337,
			csr_mhpmevent24    = 0x338,
			csr_mhpmevent25    = 0x339,
			csr_mhpmevent26    = 0x33a,
			csr_mhpmevent27    = 0x33b,
			csr_mhpmevent28    = 0x33c,
			csr_mhpmevent29    = 0x33d,
			csr_mhpmevent30    = 0x33e,
			csr_mhpmevent31    = 0x33f,
			csr_tselect        = 0x7a0,
			csr_tdata1         = 0x7a1,
			csr_tdata2         = 0x7a2,
			csr_tdata3         = 0x7a3,
			csr_dcsr           = 0x7b0,
			csr_dpc            = 0x7b1,
			csr_dscratch       = 0x7b2
		};

		/*\
		 * Possible instruction types and helper struct
		\*/

		// Rv32 Registers.
		// Integer and floating points are separated, as needed by OoO to be efficient
		uint32_t            r_gpr[32]; // General Purpose Registers
		float               r_fpr[32]; // Floating Point Registers

		// Control and status registers, quite a lot I must say
		// Not all of them are meaningfull at first, so I implemented the ones
		// that are used in the simple riscv-probe bare metal example plus the
		// floating point csr(s)
		uint32_t            r_csr[4096];

		// States required but not visible as registers
		uint32_t            m_ir;           // Current instruction
		bool                m_update_csr ;  // Previous instruction updated a csr
		uint32_t            m_csr_changed;  // Updated csr number
		bool                m_dbe;          // Data bus error
		bool                m_w;            // Unaligned access type
		uint32_t            m_rx;           // Register in use when an unaligned access occurs

		uint32_t            r_pc;           // Program Counter
		bool                r_mem_req;
		bool                r_mem_unsigned; // Data Cache access signess
		DataOperationType   r_mem_type;     // Data Cache access type
		uint32_t            r_mem_addr;     // Data Cache address
		uint32_t            r_mem_wdata;    // Data Cache data value (write)
		uint8_t             r_mem_bytes;    // Data Cache byte count (read/write)
		uint32_t           *r_mem_dest;     // Data Cache destination register (read)
		bool                r_dbe;          // Asynchronous Data Bus Error (write)

		bool                m_ibe;

		FILE               *dumpFile;       // File to log instructions

		/*\
		 *  Private initialization routine used by constructors
		\*/
		void init(uint32_t ident);

	public:
		/*\
		 * Feeds the Iss with an instruction to execute and an error
		 * status
		\*/
		inline void setInstruction(bool error, uint32_t insn)
		{
			m_ibe = error;
			m_ir  = insn;
		};

		inline void getInstructionRequest(bool &req, uint32_t &address) const
		{
			req = true;
			address = r_pc;
		}

		/*\
		 * API for memory access through the Iss
		\*/
		void setDataResponse(bool error, uint32_t rdata);

		inline void getDataRequest(
			bool &valid,
			enum DataOperationType &type,
			uint32_t &address,
			uint32_t &wdata,
			uint8_t &be) const
		{
			const uint8_t bytemsk = ((1 << r_mem_bytes) - 1);
			const uint8_t byteoff = (r_mem_addr & 0x3);

			valid   = r_mem_req;
			address = r_mem_addr;
			wdata   = r_mem_wdata;
			type    = r_mem_type;
			be      = (bytemsk << byteoff);
		}

		/*\
		 * The Rv32 has interrupt wires, I guess, but how many?
		\*/
		static const int n_irq = 1;

		/*\
		 *  Constructor & destructor
		\*/
		Rv32Iss(const std::string &name, uint32_t ident);
		Rv32Iss(uint32_t ident);

		/*\
		 * Reset handling
		\*/
		void reset(void);

		/*\
		 * Single stepping
		\*/
		void step(void);

		/*\
		 * ISS execute function
		\*/
		uint32_t executeNCycles(
				uint32_t ncycle,
				const struct InstructionResponse &irsp,
				const struct DataResponse &drsp,
				uint32_t irq_bit_field);

		/*\
		 * Useless single stepping
		\*/
		inline void nullStep(uint32_t cycles = 1)
		{
		}

		inline uint32_t isBusy()
		{
			return 0;
		}

		void getRequests(
				struct InstructionRequest &ireq,
				struct DataRequest &dreq) const;

		inline void setWriteBerr()
		{
			r_dbe = true;
		}

		inline void setIrq(uint32_t irq)
		{
			if (irq)
				r_csr[csr_mip] |= 0x800; // Make cpu aware of external interrupt
			else
				r_csr[csr_mip] &= ~0x800; // Reset external interrupt
		}

		int cpuCauseToSignal(uint32_t cause) const;

		// processor internal registers access API, used by
		// debugger.
		//
		static const unsigned int s_sp_register_no = 2;
		static const unsigned int s_fp_register_no = 8;
		static const unsigned int s_pc_register_no = 32;

		//  This ISS supports only little-endian memory system, which is the one
		//  supported by the RISCV base specification.
		//
		static const Iss2::debugCpuEndianness s_endianness =
			Iss2::ISS_LITTLE_ENDIAN;

		inline unsigned int debugGetRegisterCount() const
		{
			/*\
			 *  First 32 general purpose registers, followed by the PC.
			 *  These registers are defined in the org.gnu.gdb.riscv.cpu
			 *  feature of the RISCV's GDB.
			 *  These seem to be the default registers, and we shall
			 *  exchange some xml file with the gdb server for more
			\*/
			return 32 + 1;
		}

		inline size_t debugGetRegisterSize(unsigned int reg) const
		{
			return 32;
		}


		debug_register_t debugGetRegisterValue(unsigned int reg) const;
		void debugSetRegisterValue(unsigned int reg, uint32_t value);

		inline uint32_t getDebugPC() const
		{
			return r_pc;
		}

		inline void setDebugPC(uint32_t pc)
		{
			r_pc = pc;
		}

		/*\
		 * Support for gdb xml additional registers extensions
		\*/
		static constexpr const char *debugFeatures[4][2] = {
			{ "target.xml",          trg_xml },
			{ "riscv-32bit-cpu.xml", cpu_xml },
			{ "riscv-32bit-fpu.xml", fpu_xml },
			{ "riscv-32bit-csr.xml", csr_xml }
		};

		const char *debugXmlRegistersDescription(const char *pool);

	};
}
}
#endif // _SOCLIB_RV32_ISS_H_
