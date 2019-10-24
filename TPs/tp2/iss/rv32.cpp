/*\
 * vim: tw=0: cindent: sw=3: ts=3: sts=3: noet: list
 *
 * RISC-V Instruction Set Simulator, developed for the SoCLib Project
 * Copyright (C) 2018  SLS Group of the TIMA Lab, Grenoble-INP, CNRS
 *
 * Contributing authors:
 *     Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr>
 *     Bootstrapped the developement
 *
 *     César Fuguet <c.sarfuguet@gmail.com>
 *     Adapt the model to the SOCLIB's ISS2 API
 *
 * Interprets only the RV32IMFC instruction subset in machine mode, which
 * is good enough for the goal I'm pursuing now, but not for booting a
 * full fledge operating system.
 *
 * As opposed to the MIPS I formerly did, I choose to build the
 * instruction fields inside the ‘switch’ itself, given the complexity
 * of doing it more or less twice.
 * The code is thus a mess, and vim folders help reading it :)
 * This brutal yet efficient approach is generaly used, e.g. in Qemu, and
 * is justified in detail in the paper: "Branch Prediction and the
 * Performance of Interpreters-Don't Trust Folklore", published at
 * CGO 2015 by my esteemed colleagues Erven Rohou, Bharath Narasimha Swamy,
 * and André Seznec.
\*/

/*\
 * Dump instructions as they are executed, following closely the format
 * that objdump produces with options ‘--disassembler-options=no-aliases,numeric’
\*/
#ifndef RV32_DISAS
#define RV32_DISAS 0
#endif

#include <stdarg.h>
#include <cstring>
#include <math.h>
#include <fenv.h>
#pragma STDC FENV_ACCESS ON
#include <cassert>
#include "soclib_endian.h"
#include "arithmetics.h"
#include "rv32.h"
#include "rv32_encodings.h"

/* Not really sure this is helpful, but just in case, ...*/
#define unlikely(x)   __builtin_expect(!!(x), 0)

#define __unused__    __attribute__ ((unused))

#if RV32_DISAS == 1
/* A bit brutal but life ain't simple */
typedef struct csr {
	uint16_t    code;
	uint16_t    access;
	const char *name;
} csr;

static csr csr_array[] = {
	{0x000, 0b0011, "ustatus"},
	{0x004, 0b0011, "uie"},
	{0x005, 0b0011, "utvec"},
	{0x040, 0b0011, "uscratch"},
	{0x041, 0b0011, "uepc"},
	{0x042, 0b0011, "ucause"},
	{0x043, 0b0011, "utval"},
	{0x044, 0b0011, "uip"},
	{0x001, 0b0011, "fflags"}, /* Bypass to flags in fcsr */
	{0x002, 0b0011, "frm"},    /* Bypass to rm in fcsr */
	{0x003, 0b0011, "fcsr"},   /* This is the actual register */
	{0xc00, 0b0010, "cycle"},
	{0xc01, 0b0010, "time"},
	{0xc02, 0b0010, "instret"},
	{0xc03, 0b0010, "hpmcounter3"},
	{0xc04, 0b0010, "hpmcounter4"},
	{0xc05, 0b0010, "hpmcounter5"},
	{0xc06, 0b0010, "hpmcounter6"},
	{0xc07, 0b0010, "hpmcounter7"},
	{0xc08, 0b0010, "hpmcounter8"},
	{0xc09, 0b0010, "hpmcounter9"},
	{0xc0a, 0b0010, "hpmcounter10"},
	{0xc0b, 0b0010, "hpmcounter11"},
	{0xc0c, 0b0010, "hpmcounter12"},
	{0xc0d, 0b0010, "hpmcounter13"},
	{0xc0e, 0b0010, "hpmcounter14"},
	{0xc0f, 0b0010, "hpmcounter15"},
	{0xc10, 0b0010, "hpmcounter16"},
	{0xc11, 0b0010, "hpmcounter17"},
	{0xc12, 0b0010, "hpmcounter18"},
	{0xc13, 0b0010, "hpmcounter19"},
	{0xc14, 0b0010, "hpmcounter20"},
	{0xc15, 0b0010, "hpmcounter21"},
	{0xc16, 0b0010, "hpmcounter22"},
	{0xc17, 0b0010, "hpmcounter23"},
	{0xc18, 0b0010, "hpmcounter24"},
	{0xc19, 0b0010, "hpmcounter25"},
	{0xc1a, 0b0010, "hpmcounter26"},
	{0xc1b, 0b0010, "hpmcounter27"},
	{0xc1c, 0b0010, "hpmcounter28"},
	{0xc1d, 0b0010, "hpmcounter29"},
	{0xc1e, 0b0010, "hpmcounter30"},
	{0xc1f, 0b0010, "hpmcounter31"},
	{0xc80, 0b0010, "cycleh"},
	{0xc81, 0b0010, "timeh"},
	{0xc82, 0b0010, "instreth"},
	{0xc83, 0b0010, "hpmcounter3h"},
	{0xc84, 0b0010, "hpmcounter4h"},
	{0xc85, 0b0010, "hpmcounter5h"},
	{0xc86, 0b0010, "hpmcounter6h"},
	{0xc87, 0b0010, "hpmcounter7h"},
	{0xc88, 0b0010, "hpmcounter8h"},
	{0xc89, 0b0010, "hpmcounter9h"},
	{0xc8a, 0b0010, "hpmcounter10h"},
	{0xc8b, 0b0010, "hpmcounter11h"},
	{0xc8c, 0b0010, "hpmcounter12h"},
	{0xc8d, 0b0010, "hpmcounter13h"},
	{0xc8e, 0b0010, "hpmcounter14h"},
	{0xc8f, 0b0010, "hpmcounter15h"},
	{0xc90, 0b0010, "hpmcounter16h"},
	{0xc91, 0b0010, "hpmcounter17h"},
	{0xc92, 0b0010, "hpmcounter18h"},
	{0xc93, 0b0010, "hpmcounter19h"},
	{0xc94, 0b0010, "hpmcounter20h"},
	{0xc95, 0b0010, "hpmcounter21h"},
	{0xc96, 0b0010, "hpmcounter22h"},
	{0xc97, 0b0010, "hpmcounter23h"},
	{0xc98, 0b0010, "hpmcounter24h"},
	{0xc99, 0b0010, "hpmcounter25h"},
	{0xc9a, 0b0010, "hpmcounter26h"},
	{0xc9b, 0b0010, "hpmcounter27h"},
	{0xc9c, 0b0010, "hpmcounter28h"},
	{0xc9d, 0b0010, "hpmcounter29h"},
	{0xc9e, 0b0010, "hpmcounter30h"},
	{0xc9f, 0b0010, "hpmcounter31h"},
	{0x100, 0b0111, "sstatus"},
	{0x102, 0b0111, "sedeleg"},
	{0x103, 0b0111, "sideleg"},
	{0x104, 0b0111, "sie"},
	{0x105, 0b0111, "stvec"},
	{0x106, 0b0111, "scounteren"},
	{0x140, 0b0111, "sscratch"},
	{0x141, 0b0111, "sepc"},
	{0x142, 0b0111, "scause"},
	{0x143, 0b0111, "stval"},
	{0x144, 0b0111, "sip"},
	{0x180, 0b0111, "satp"},
	{0xf11, 0b1010, "mvendorid"},
	{0xf12, 0b1010, "marchid"},
	{0xf13, 0b1010, "mimpid"},
	{0xf14, 0b1010, "mhartid"},
	{0x300, 0b1011, "mstatus"},
	{0x301, 0b1011, "misa"},
	{0x302, 0b1011, "medeleg"},
	{0x303, 0b1011, "mideleg"},
	{0x304, 0b1011, "mie"},
	{0x305, 0b1011, "mtvec"},
	{0x306, 0b1011, "mcounteren"},
	{0x340, 0b1011, "mscratch"},
	{0x341, 0b1011, "mepc"},
	{0x342, 0b1011, "mcause"},
	{0x343, 0b1011, "mtval"},
	{0x344, 0b1011, "mip"},
	{0x3a0, 0b1011, "pmpcfg0"},
	{0x3a1, 0b1011, "pmpcfg1"},
	{0x3a2, 0b1011, "pmpcfg2"},
	{0x3a3, 0b1011, "pmpcfg3"},
	{0x3b0, 0b1011, "pmpaddr0"},
	{0x3b1, 0b1011, "pmpaddr1"},
	{0x3b2, 0b1011, "pmpaddr2"},
	{0x3b3, 0b1011, "pmpaddr3"},
	{0x3b4, 0b1011, "pmpaddr4"},
	{0x3b5, 0b1011, "pmpaddr5"},
	{0x3b6, 0b1011, "pmpaddr6"},
	{0x3b7, 0b1011, "pmpaddr7"},
	{0x3b8, 0b1011, "pmpaddr8"},
	{0x3b9, 0b1011, "pmpaddr9"},
	{0x3ba, 0b1011, "pmpaddr10"},
	{0x3bb, 0b1011, "pmpaddr11"},
	{0x3bc, 0b1011, "pmpaddr12"},
	{0x3bd, 0b1011, "pmpaddr13"},
	{0x3be, 0b1011, "pmpaddr14"},
	{0x3bf, 0b1011, "pmpaddr15"},
	{0xb00, 0b1011, "mcycle"},
	{0xb02, 0b1011, "minstret"},
	{0xb03, 0b1011, "mhpmcounter3"},
	{0xb04, 0b1011, "mhpmcounter4"},
	{0xb05, 0b1011, "mhpmcounter5"},
	{0xb06, 0b1011, "mhpmcounter6"},
	{0xb07, 0b1011, "mhpmcounter7"},
	{0xb08, 0b1011, "mhpmcounter8"},
	{0xb09, 0b1011, "mhpmcounter9"},
	{0xb0a, 0b1011, "mhpmcounter10"},
	{0xb0b, 0b1011, "mhpmcounter11"},
	{0xb0c, 0b1011, "mhpmcounter12"},
	{0xb0d, 0b1011, "mhpmcounter13"},
	{0xb0e, 0b1011, "mhpmcounter14"},
	{0xb0f, 0b1011, "mhpmcounter15"},
	{0xb10, 0b1011, "mhpmcounter16"},
	{0xb11, 0b1011, "mhpmcounter17"},
	{0xb12, 0b1011, "mhpmcounter18"},
	{0xb13, 0b1011, "mhpmcounter19"},
	{0xb14, 0b1011, "mhpmcounter20"},
	{0xb15, 0b1011, "mhpmcounter21"},
	{0xb16, 0b1011, "mhpmcounter22"},
	{0xb17, 0b1011, "mhpmcounter23"},
	{0xb18, 0b1011, "mhpmcounter24"},
	{0xb19, 0b1011, "mhpmcounter25"},
	{0xb1a, 0b1011, "mhpmcounter26"},
	{0xb1b, 0b1011, "mhpmcounter27"},
	{0xb1c, 0b1011, "mhpmcounter28"},
	{0xb1d, 0b1011, "mhpmcounter29"},
	{0xb1e, 0b1011, "mhpmcounter30"},
	{0xb1f, 0b1011, "mhpmcounter31"},
	{0xb80, 0b1011, "mcycleh"},
	{0xb82, 0b1011, "minstreth"},
	{0xb83, 0b1011, "mhpmcounter3h"},
	{0xb84, 0b1011, "mhpmcounter4h"},
	{0xb85, 0b1011, "mhpmcounter5h"},
	{0xb86, 0b1011, "mhpmcounter6h"},
	{0xb87, 0b1011, "mhpmcounter7h"},
	{0xb88, 0b1011, "mhpmcounter8h"},
	{0xb89, 0b1011, "mhpmcounter9h"},
	{0xb8a, 0b1011, "mhpmcounter10h"},
	{0xb8b, 0b1011, "mhpmcounter11h"},
	{0xb8c, 0b1011, "mhpmcounter12h"},
	{0xb8d, 0b1011, "mhpmcounter13h"},
	{0xb8e, 0b1011, "mhpmcounter14h"},
	{0xb8f, 0b1011, "mhpmcounter15h"},
	{0xb90, 0b1011, "mhpmcounter16h"},
	{0xb91, 0b1011, "mhpmcounter17h"},
	{0xb92, 0b1011, "mhpmcounter18h"},
	{0xb93, 0b1011, "mhpmcounter19h"},
	{0xb94, 0b1011, "mhpmcounter20h"},
	{0xb95, 0b1011, "mhpmcounter21h"},
	{0xb96, 0b1011, "mhpmcounter22h"},
	{0xb97, 0b1011, "mhpmcounter23h"},
	{0xb98, 0b1011, "mhpmcounter24h"},
	{0xb99, 0b1011, "mhpmcounter25h"},
	{0xb9a, 0b1011, "mhpmcounter26h"},
	{0xb9b, 0b1011, "mhpmcounter27h"},
	{0xb9c, 0b1011, "mhpmcounter28h"},
	{0xb9d, 0b1011, "mhpmcounter29h"},
	{0xb9e, 0b1011, "mhpmcounter30h"},
	{0xb9f, 0b1011, "mhpmcounter31h"},
	{0x323, 0b1011, "mhpmevent3"},
	{0x324, 0b1011, "mhpmevent4"},
	{0x325, 0b1011, "mhpmevent5"},
	{0x326, 0b1011, "mhpmevent6"},
	{0x327, 0b1011, "mhpmevent7"},
	{0x328, 0b1011, "mhpmevent8"},
	{0x329, 0b1011, "mhpmevent9"},
	{0x32a, 0b1011, "mhpmevent10"},
	{0x32b, 0b1011, "mhpmevent11"},
	{0x32c, 0b1011, "mhpmevent12"},
	{0x32d, 0b1011, "mhpmevent13"},
	{0x32e, 0b1011, "mhpmevent14"},
	{0x32f, 0b1011, "mhpmevent15"},
	{0x330, 0b1011, "mhpmevent16"},
	{0x331, 0b1011, "mhpmevent17"},
	{0x332, 0b1011, "mhpmevent18"},
	{0x333, 0b1011, "mhpmevent19"},
	{0x334, 0b1011, "mhpmevent20"},
	{0x335, 0b1011, "mhpmevent21"},
	{0x336, 0b1011, "mhpmevent22"},
	{0x337, 0b1011, "mhpmevent23"},
	{0x338, 0b1011, "mhpmevent24"},
	{0x339, 0b1011, "mhpmevent25"},
	{0x33a, 0b1011, "mhpmevent26"},
	{0x33b, 0b1011, "mhpmevent27"},
	{0x33c, 0b1011, "mhpmevent28"},
	{0x33d, 0b1011, "mhpmevent29"},
	{0x33e, 0b1011, "mhpmevent30"},
	{0x33f, 0b1011, "mhpmevent31"},
	{0x7a0, 0b1011, "tselect"},
	{0x7a1, 0b1011, "tdata1"},
	{0x7a2, 0b1011, "tdata2"},
	{0x7a3, 0b1011, "tdata3"},
	{0x7b0, 0b1111, "dcsr"},
	{0x7b1, 0b1111, "dpc"},
	{0x7b2, 0b1111, "dscratch"},
};

static const char *csr_name(int n)
{
	for (size_t i = 0; i < sizeof(csr_array)/sizeof(csr_array[0]); i++) {
		if (csr_array[i].code == n)
			return csr_array[i].name;
	}
	return "xxxx";
}

static const char *rounding_name(int n)
{
	switch (n) {
		case 0b000: return "rne";
		case 0b001: return "rtz";
		case 0b010: return "rdn";
		case 0b011: return "rup";
		case 0b100: return "rmm";
		case 0b101: return "ror";
		case 0b110: return "ror";
		case 0b111: return "dyn";
	}
	return "xxx"; // Well, never reached actually
}

/*\
 * Using a file so as to avoid polluting the uart output
\*/
static void asmout(FILE *f, const char *insfmt, uint32_t pc, uint32_t insn, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fprintf(f, "%08x:	", pc);
	fprintf(f, insfmt,   insn);
	vfprintf(f, fmt, args);
	fprintf(f, "\n");
	va_end(args);
	/* Slows down things quite a bit but useful when debugging */
	fflush(f);
}

// Macros to output disassembled instructions
#define asm_str               const char *s = NULL;
#define asm_ins(x)            s = (x)
#define asm_out(format,...)   asmout(dumpFile, "%08x	", r_pc, m_ir, format, __VA_ARGS__)
#define c_asm_out(format,...) asmout(dumpFile, "%04x		", r_pc, m_ir, format, __VA_ARGS__)
#else
#define asm_str
#define asm_ins(x)
#define asm_out(format,...)
#define c_asm_out(format,...)
#endif

/*\
 * Union needed for type punning, as authorized per rule s6.5 of the C99 standard
\*/
typedef union {
	int32_t  s;
	uint32_t u;
	float    f;
} suf_t;

/*\
 * Rounding modes for float
\*/

/* Stolen from QEMU softfloat as the values given in the IEEE 754 standard,
 * not to be confused with some defines that look alike available in fenv.h,
 * which I did at first, sight! */
enum {
	float_round_nearest_even = 0,
	float_round_down         = 1,
	float_round_up           = 2,
	float_round_to_zero      = 3,
	float_round_ties_away    = 4,
};

enum {
	rne = 0,
	rtz = 1,
	rdn = 2,
	rup = 3,
	rmm = 4,
	rxx = 5,
	ryy = 6,
	dyn = 7
};

static int rounding[] = {
	[rne] = float_round_nearest_even,
	[rtz] = float_round_to_zero,
	[rdn] = float_round_down,
	[rup] = float_round_up,
	[rmm] = float_round_ties_away,
	[rxx] = -1,
	[ryy] = -1,
	[dyn] = -2
};

/*\
 * Memory accesses to fit the current SoCLib Iss strategy
 * The type, addr, dest and wdata fields are inherited from the Iss2 class
\*/

#define load(rd, type, sign, addr, bytes)                    \
	do {                                                      \
		r_mem_req      = true;                                 \
		r_mem_type     = type;                                 \
		r_mem_unsigned = sign;                                 \
		r_mem_addr     = addr;                                 \
		r_mem_bytes    = bytes;                                \
		r_mem_dest     = rd;                                   \
	} while (0)

#define store(type, addr, data, bytes)                       \
	do {                                                      \
		r_mem_req   = true;                                    \
		r_mem_type  = type;                                    \
		r_mem_addr  = addr;                                    \
		r_mem_wdata = data;                                    \
		r_mem_bytes = bytes;                                   \
	} while (0)

#define amo(rd, type, addr, data)                            \
	do {                                                      \
		r_mem_req   = true;                                    \
		r_mem_type  = type;                                    \
		r_mem_addr  = addr;                                    \
		r_mem_wdata = data;                                    \
		r_mem_bytes = 4;                                       \
		r_mem_dest  = rd;                                      \
	} while (0)

namespace soclib { namespace common {

	namespace {

		static inline std::string mkname(uint32_t no)
		{
				char tmp[32];
				snprintf(tmp, 32, "rv32_iss%d", (int)no);
				return std::string(tmp);
		}
	}

   Rv32Iss::Rv32Iss(const std::string &name, uint32_t ident)
		: Iss2(name, ident)
	{
		init(ident);
	}

	Rv32Iss::Rv32Iss(uint32_t ident)
		: Iss2(mkname(ident), ident)
	{
		init(ident);
	}

	void Rv32Iss::init(uint32_t ident)
	{
		char *s = getenv("ISSLOG");
		dumpFile = stderr;

		/* Fallback to stderr for instruction logging in case opening a file
		 * for writing is not possible */
		if (s != NULL) {
			dumpFile = fopen(s, "w+");
			if (dumpFile == NULL)
				dumpFile = stderr;
		}

		r_csr[csr_mhartid]   = ident;
		r_pc                 = RESET_VECTOR;
		r_csr[csr_mvendorid] = 0x00bada55;
		r_csr[csr_misa]      = 0x40001124; /* rv32imfc */
		r_csr[csr_mimpid]    = 0x02144906; /* soclibvz */
	}

	void Rv32Iss::reset(void)
	{
		r_pc                 = RESET_VECTOR;
		r_dbe                = false;
		m_ibe                = false;
		m_dbe                = false;
		r_mem_req            = false;
		r_gpr[0]             = 0;
		r_csr[csr_mstatus]   = 0x00001800; /* boot in machine mode */
		r_csr[csr_mcause]    = 0;
		r_csr[csr_mcycle]    = 0;
		r_csr[csr_mcycleh]   = 0;
		r_csr[csr_minstret]  = 0;
		r_csr[csr_minstreth] = 0;
		m_ir                 = 0x00000013; /* addi x0, x0, 0 */
	};

	int Rv32Iss::cpuCauseToSignal(uint32_t cause) const
	{
		switch (cause) {
			case ILLEGAL_INSTRUCTION_TRAP:
				return 4; // Illegal instruction
			case LOAD_ACCESS_FAULT_TRAP:
			case STORE_AMO_ACCESS_FAULT_TRAP:
			case INSTRUCTION_ACCESS_FAULT_TRAP:
				return 5; // Trap (nothing better)
			default:
				return 5; // GDB SIGTRAP
		}
	}

	void Rv32Iss::setDataResponse(bool error, uint32_t data)
	{
		m_dbe = error;
		r_mem_req = false;

		if (error)
			return;

		switch (r_mem_type) {
			case DATA_WRITE :
			case XTN_WRITE :
			case XTN_READ :
				/* do nothing */
				break;
			case DATA_READ :
			{
				/* first, align data according to the byte offset in the request
				 * address */
				const uint32_t align_data = (data >> ((r_mem_addr & 0x3)*8));

				/* second, decode the sign bit */
				const int bits = r_mem_bytes * 8; // number of valid lsb in the data
				const uint32_t sign = (align_data >> (bits - 1)) & 0x1;

				/* third, extend the sign if needed */
				const uint32_t mask = (1UL << bits) - 1;
				const uint32_t masked_udata = align_data & mask;
				const uint32_t masked_data = sign ? masked_udata | ~mask : masked_udata;

				/* fourth, write the data into the destination register */
				*r_mem_dest = r_mem_unsigned ? masked_udata : masked_data;
				break;
			}
			case DATA_LR:
				/* FIXME: save that there is a current reservation */
				*r_mem_dest = data;
				break;
			case DATA_SC:
			{
				*r_mem_dest = (data == SC_ATOMIC) ? 0 : 1;
				break;
			}
			case DATA_AMO_SWAP:
			case DATA_AMO_ADD:
			case DATA_AMO_AND:
			case DATA_AMO_OR:
			case DATA_AMO_XOR:
			case DATA_AMO_MAX:
			case DATA_AMO_MAXU:
			case DATA_AMO_MIN:
			case DATA_AMO_MINU:
				*r_mem_dest = data;
				break;
			default:
				fprintf(stderr, "Unhandled memory access request\n");
				exit(EXIT_FAILURE);
				break;
		}
	}

	void Rv32Iss::getRequests(
			struct InstructionRequest &ireq,
			struct DataRequest &dreq) const
	{
		getInstructionRequest(
				ireq.valid,
				ireq.addr);

		getDataRequest(
				dreq.valid,
				dreq.type,
				dreq.addr,
				dreq.wdata,
				dreq.be);
	}

	uint32_t Rv32Iss::executeNCycles(uint32_t ncycle,
			const struct InstructionResponse &irsp,
			const struct DataResponse &drsp,
			uint32_t irq_bit_field)
	{
		uint32_t cycles = 0;

		assert ((ncycle <= 1) && "only zero or one cycle supported");

		/*\
		 *  Handle data and instruction responses
		\*/
		if (irsp.valid) setInstruction(irsp.error, irsp.instruction);
		if (drsp.valid) setDataResponse(drsp.error, drsp.rdata);

		/*\
		 *  Set interrupts
		\*/
		if (irq_bit_field)
			setIrq(irq_bit_field);

		if (!ncycle)
			return 0;

		/*\
		 *  Execute an ISS step when:
		 *  - there is no ongoing memory data request, and
		 *  - there is a valid instruction response
		\*/
		if (!r_mem_req && irsp.valid) {
			step();
			cycles++;
		}

		return cycles;
	}

	void Rv32Iss::step(void)
	{
		asm_str;
		/*\
		 * Initialization not really necessary, but so catches up some errors
		\*/
		bool branch = 0xdeadbeef;
		int  next_pc = 0xdeadbeef;

		/*\
		 * Local variable used to build the value send on the interconnect.
		 * It looks like endianness and byte enable are misteriously
		 * interpreted, so each byte or half world is replicated to
		 * avoid headaches
		\*/

		uint32_t          data;
		uint8_t           byte_count;
		uint32_t          addr;
		DataOperationType access;
		bool              unsign;

		/*\
		 * Helper variables representing the instruction field, to ease
		 * writing (and most reading in a while ago) the instruction
		 * execution code.
		\*/
		int       rd = 0xbad0bad;
		int       rs1;
		int       rs2;
		int       imm;
		int       csr;
		/*\
		 * Floating point
		\*/
		int       rs3;
		int       fmt;
		int       rm;
		suf_t     suf;
		suf_t     fus;
		/*\
		 * Temporaries for complex insn behavior
		\*/
		uint32_t  reg;
		uint32_t  ger;

		/*\
		 * Setting exceptions
		 * FIXME: Check the priority order in the specs
		\*/
		bool exception = false;

		if (m_ibe) {
			r_csr[csr_mcause] = INSTRUCTION_ACCESS_FAULT_TRAP;
			exception         = true;
		}

		if (m_dbe) {
			r_csr[csr_mcause] = LOAD_ACCESS_FAULT_TRAP;
			r_csr[csr_mtval]  = r_mem_addr;
			exception         = true;
		}

		if (r_dbe) {
			r_csr[csr_mcause] = STORE_AMO_ACCESS_FAULT_TRAP;
			r_csr[csr_mtval]  = r_mem_addr;
			r_dbe             = false;
			exception         = true;
		}

		/*\
		 * Check for exceptions and interruptions
		 * Assume everything runs in machine mode for now
		\*/
__iss_handle_exception:
		if (unlikely(exception)) {
			r_csr[csr_mepc] = r_pc;
			r_pc = r_csr[csr_mtvec];
			r_csr[csr_mstatus] |= 0xf0c;
		} else if (unlikely((r_csr[csr_mip] & r_csr[csr_mie]) && (r_csr[csr_mstatus] & 0x8))) {
			uint32_t irqs = r_csr[csr_mip] & r_csr[csr_mie];
			/* Priority as defined section 3.1.14 */
			if (irqs & 0x800)
				r_csr[csr_mcause] = MACHINE_EXTERNAL_INTERRUPT_TRAP;
			else if (irqs & 0x008)
				r_csr[csr_mcause] = MACHINE_SOFTWARE_INTERRUPT_TRAP;
			else if (irqs & 0x080)
				r_csr[csr_mcause] = MACHINE_TIMER_INTERRUPT_TRAP;
			else {
				fprintf(stderr, "Unhandled interrupt trap = 0x%03x\n", irqs);
			}
			r_csr[csr_mepc] = r_pc;
			if ((r_csr[csr_mtvec] & 0b11) == 0)
				r_pc = r_csr[csr_mtvec];
			else if ((r_csr[csr_mtvec] & 0b11) == 1)
				r_pc = r_csr[csr_mtvec] + (r_csr[csr_mcause] << 2);
			else {
				fprintf(stderr, "Unknown mtvec mode 0b%c%c\n",
							'0' + (r_csr[csr_mtvec] & 0b10),
							'0' + (r_csr[csr_mtvec] & 0b01));
			}
			r_csr[csr_mstatus] |= 0x00001800;  // Previous mode was machine mode
			r_csr[csr_mstatus] |= ((r_csr[csr_mstatus] & 0x8) << 4); // Copy in stack
			r_csr[csr_mstatus] &= ~0x8; // Mask current
		} else {
			/*\
			 * Decode and execute instructions
			\*/
			/*\
			 * risc-v encodings:
			 * shamelessly extracted from the doc using pdftotext -f 150 -l 150 -layout riscv-spec.pdf
			 *                                         RV32I Base Instruction Set
			 *    31         27   26 25 24         20   19         15   14    12   11      7     6             0
			 *            funct7             rs2             rs1        funct3        rd             opcode        R-type
			 *                 imm[11:0]                     rs1        funct3        rd             opcode        I-type
			 *          imm[11:5]           rs2              rs1        funct3      imm[4:0]         opcode        S-type
			 *         imm[12|10:5]         rs2              rs1        funct3     imm[4:1|11]       opcode        B-type
			 *                          imm[31:12]                                    rd             opcode        U-type
			 *                      imm[20|10:1|11|19:12]                             rd             opcode        J-type
			 *
			 *                                RV32I Base Instruction Set
			 *                            imm[31:12]                        rd                       0110111       LUI
			 *                            imm[31:12]                        rd                       0010111       AUIPC
			 *                       imm[20|10:1|11|19:12]                  rd                       1101111       JAL
			 *                  imm[11:0]                  rs1   000        rd                       1100111       JALR
			 *         imm[12|10:5]           rs2          rs1   000    imm[4:1|11]                  1100011       BEQ
			 *         imm[12|10:5]           rs2          rs1   001    imm[4:1|11]                  1100011       BNE
			 *         imm[12|10:5]           rs2          rs1   100    imm[4:1|11]                  1100011       BLT
			 *         imm[12|10:5]           rs2          rs1   101    imm[4:1|11]                  1100011       BGE
			 *         imm[12|10:5]           rs2          rs1   110    imm[4:1|11]                  1100011       BLTU
			 *         imm[12|10:5]           rs2          rs1   111    imm[4:1|11]                  1100011       BGEU
			 *                  imm[11:0]                  rs1   000        rd                       0000011       LB
			 *                  imm[11:0]                  rs1   001        rd                       0000011       LH
			 *                  imm[11:0]                  rs1   010        rd                       0000011       LW
			 *                  imm[11:0]                  rs1   100        rd                       0000011       LBU
			 *                  imm[11:0]                  rs1   101        rd                       0000011       LHU
			 *           imm[11:5]            rs2          rs1   000     imm[4:0]                    0100011       SB
			 *           imm[11:5]            rs2          rs1   001     imm[4:0]                    0100011       SH
			 *           imm[11:5]            rs2          rs1   010     imm[4:0]                    0100011       SW
			 *                  imm[11:0]                  rs1   000        rd                       0010011       ADDI
			 *                  imm[11:0]                  rs1   010        rd                       0010011       SLTI
			 *                  imm[11:0]                  rs1   011        rd                       0010011       SLTIU
			 *                  imm[11:0]                  rs1   100        rd                       0010011       XORI
			 *                  imm[11:0]                  rs1   110        rd                       0010011       ORI
			 *                  imm[11:0]                  rs1   111        rd                       0010011       ANDI
			 *            0000000           shamt          rs1   001        rd                       0010011       SLLI
			 *            0000000           shamt          rs1   101        rd                       0010011       SRLI
			 *            0100000           shamt          rs1   101        rd                       0010011       SRAI
			 *            0000000             rs2          rs1   000        rd                       0110011       ADD
			 *            0100000             rs2          rs1   000        rd                       0110011       SUB
			 *            0000000             rs2          rs1   001        rd                       0110011       SLL
			 *            0000000             rs2          rs1   010        rd                       0110011       SLT
			 *            0000000             rs2          rs1   011        rd                       0110011       SLTU
			 *            0000000             rs2          rs1   100        rd                       0110011       XOR
			 *            0000000             rs2          rs1   101        rd                       0110011       SRL
			 *            0100000             rs2          rs1   101        rd                       0110011       SRA
			 *            0000000             rs2          rs1   110        rd                       0110011       OR
			 *            0000000             rs2          rs1   111        rd                       0110011       AND
			 *           fm          pred       succ     00000   000      00000                      0001111       FENCE
			 *          0000        0000        0000     00000   001      00000                      0001111       FENCE.I
			 *                000000000000               00000   000      00000                      1110011       ECALL
			 *                000000000001               00000   000      00000                      1110011       EBREAK
			 *                     csr                     rs1   001        rd                       1110011       CSRRW
			 *                     csr                     rs1   010        rd                       1110011       CSRRS
			 *                     csr                     rs1   011        rd                       1110011       CSRRC
			 *                     csr                   zimm    101        rd                       1110011       CSRRWI
			 *                     csr                   zimm    110        rd                       1110011       CSRRSI
			 *                     csr                   zimm    111        rd                       1110011       CSRRCI
			 *
			 *                                RV32M Standard Extension
			 *      0000001                   rs2      rs1      000                     rd           0110011       MUL
			 *      0000001                   rs2      rs1      001                     rd           0110011       MULH
			 *      0000001                   rs2      rs1      010                     rd           0110011       MULHSU
			 *      0000001                   rs2      rs1      011                     rd           0110011       MULHU
			 *      0000001                   rs2      rs1      100                     rd           0110011       DIV
			 *      0000001                   rs2      rs1      101                     rd           0110011       DIVU
			 *      0000001                   rs2      rs1      110                     rd           0110011       REM
			 *      0000001                   rs2      rs1      111                     rd           0110011       REMU
			 *
			 * Note that 32/64 bit insns opcodes end in 11, the 00/01/10 are reserved to the compressed instruction
			 * set, and are named C0/C1/C2 in the document.
			\*/

			switch (m_ir & 0x7f) {
				case 0b0110111: // U-type LUI rd,imm
					decode_u_type(rd, imm);
					asm_out("lui	x%d,0x%x", rd, (uint32_t)imm >> 12);
					r_gpr[rd] = imm;
					next_pc = r_pc + 4;
					break;
				case 0b0010111: // U-type AUIPC rd, imm
					decode_u_type(rd, imm);
					asm_out("auipc	x%d,0x%x", rd, (uint32_t)imm >> 12);
					r_gpr[rd] = r_pc + imm;
					next_pc = r_pc + 4;
					break;
				case 0b1101111: // J-type JAL rd, imm
					decode_j_type(rd, imm);
					asm_out("jal	x%d,0x%x", rd, r_pc + imm);
					r_gpr[rd] = r_pc + 4;
					next_pc   = r_pc + imm;
					break;
				case 0b1100111: // I-type JALR
					decode_i_type(rd, rs1, imm);
					if (imm == 0) {
						asm_out("jalr	x%d,0(x%d)", rd, rs1);
					} else {
						asm_out("jalr	x%d,x%d,0x%x", rd, rs1, r_pc + imm);
					}
					next_pc   = (r_gpr[rs1] + imm) & ~1; // Lower bit must be set to 0
					r_gpr[rd] = r_pc + 4;
					break;
				case 0b1100011: // B-type
					decode_b_type(rs1, rs2, imm);
					switch ((m_ir >> 12) & 0x7) {
						case 0b000:  // BEQ
							asm_ins("beq");
							branch = r_gpr[rs1] == r_gpr[rs2];
							break;
						case 0b001:  // BNE
							asm_ins("bne");
							branch = r_gpr[rs1] != r_gpr[rs2];
							break;
						case 0b100:  // BLT
							asm_ins("blt");
							branch = (int32_t)r_gpr[rs1] < (int32_t)r_gpr[rs2];
							break;
						case 0b101:  // BGE
							asm_ins("bge");
							branch = (int32_t)r_gpr[rs1] >= (int32_t)r_gpr[rs2];
							break;
						case 0b110:  // BLTU
							asm_ins("bltu");
							branch = r_gpr[rs1] < r_gpr[rs2];
							break;
						case 0b111:  // BGEU
							asm_ins("bgeu");
							branch = r_gpr[rs1] >= r_gpr[rs2];
							break;
						default:
							fprintf(stderr, "Unknown branch instruction: func3 = 0b%c%c%c\n",
										'0' + ((m_ir >> 12) & 0x4),
										'0' + ((m_ir >> 12) & 0x2),
										'0' + ((m_ir >> 12) & 0x1));
					}
					asm_out("%s	x%d,x%d,%x", s, rs1, rs2, r_pc + imm);
					next_pc = r_pc + (!branch ? 4 : imm);
					break;
				case 0b0000011: // I-type
					decode_i_type(rd, rs1, imm);
					switch ((m_ir >> 12) & 0x7) {
						case 0b000:  // LB
							asm_ins("lb");
							access = DATA_READ;
							byte_count = 1;
							unsign = false;
							break;
						case 0b001:  // LH
							asm_ins("lh");
							access = DATA_READ;
							byte_count = 2;
							unsign = false;
							break;
						case 0b010:  // LW
							asm_ins("lw");
							access = DATA_READ;
							byte_count = 4;
							unsign = true; // to make gcc shy
							break;
						case 0b100:  // LBU
							asm_ins("lbu");
							access = DATA_READ;
							byte_count = 1;
							unsign = true;
							break;
						case 0b101:  // LHU
							asm_ins("lhu");
							access = DATA_READ;
							byte_count = 2;
							unsign = true;
							break;
						default:
							// to make gcc shy
							access = DATA_READ;
							byte_count = 4;
							unsign = true;
							fprintf(stderr, "Unknown load instruction: func3 = 0b%c%c%c\n",
										'0' + ((m_ir >> 12) & 0x4),
										'0' + ((m_ir >> 12) & 0x2),
										'0' + ((m_ir >> 12) & 0x1));
					}
					asm_out("%s	x%d,%d(x%d)", s, rd, imm, rs1);
					addr = r_gpr[rs1] + imm;
					load(&r_gpr[rd], access, unsign, addr, byte_count);
					next_pc = r_pc + 4;
					break;
				case 0b0100011: // S-type
					decode_s_type(rs1, rs2, imm);
					addr = r_gpr[rs1] + imm;
					switch ((m_ir >> 12) & 0x7) {
						case 0b000:  // SB
							asm_ins("sb");
							access = DATA_WRITE;
							byte_count = 1;
							data   = r_gpr[rs2] & 0xff;
							data   = (data << 24) | (data << 16) | (data << 8) | data;
							break;
						case 0b001:  // SH
							asm_ins("sh");
							access = DATA_WRITE;
							byte_count = 2;
							data   = r_gpr[rs2] & 0xffff;
							data   = (data << 16) | data;
							break;
						case 0b010:  // SW
							asm_ins("sw");
							access = DATA_WRITE;
							byte_count = 4;
							data   = r_gpr[rs2];
							break;
						default:
							fprintf(stderr, "Unknown load instruction: func3 = 0b%c%c%c\n",
										'0' + ((m_ir >> 12) & 0x4),
										'0' + ((m_ir >> 12) & 0x2),
										'0' + ((m_ir >> 12) & 0x1));
							exit(1);
					}
					asm_out("%s	x%d,%d(x%d)", s, rs2, imm, rs1);
					store(access, addr, data, byte_count);
					next_pc = r_pc + 4;
					break;
				case 0b0010011: // I-type
					decode_i_type(rd, rs1, imm);
					switch ((m_ir >> 12) & 0x7) {
						case 0b000:  // ADDI
							asm_ins("addi");
							r_gpr[rd] = r_gpr[rs1] + imm;
							break;
						case 0b001:  // SLLI
							asm_ins("slli");
							r_gpr[rd] = r_gpr[rs1] << (imm & 0x1f);
							break;
						case 0b010:  // SLTI
							asm_ins("slti");
							r_gpr[rd] = (int32_t)r_gpr[rs1] < imm;
							break;
						case 0b011:  // SLTIU
							asm_ins("sltiu");
							r_gpr[rd] = r_gpr[rs1] < (uint32_t)imm;
							break;
						case 0b100:  // XORI
							asm_ins("xori");
							r_gpr[rd] = r_gpr[rs1] ^ imm;
							break;
						case 0b101:
							if ((m_ir & 0xfe000000) == 0x00000000) { // SRLI
								asm_ins("srli");
								r_gpr[rd] = r_gpr[rs1] >> (imm & 0x1f);
							} else if ((m_ir & 0xfe000000) == 0x40000000) { // SRAI
								asm_ins("srai");
								r_gpr[rd] = (int32_t)r_gpr[rs1] >> (imm & 0x1f);
							} else
								fprintf(stderr, "Unknown immediate shift right instruction: func7 = 0b%c%c%c%c%c%c%c\n",
											'0' + ((m_ir >> 25) & 0x40),
											'0' + ((m_ir >> 25) & 0x20),
											'0' + ((m_ir >> 25) & 0x10),
											'0' + ((m_ir >> 25) & 0x08),
											'0' + ((m_ir >> 25) & 0x04),
											'0' + ((m_ir >> 25) & 0x02),
											'0' + ((m_ir >> 25) & 0x01));
							break;
						case 0b110:  // ORI
							asm_ins("ori");
							r_gpr[rd] = r_gpr[rs1] | imm;
							break;
						case 0b111:  // ANDI
							asm_ins("andi");
							r_gpr[rd] = r_gpr[rs1] & imm;
							break;
						default:
							fprintf(stderr, "Unknown immediate instruction: func3 = 0b%c%c%c\n",
										'0' + ((m_ir >> 12) & 0x4),
										'0' + ((m_ir >> 12) & 0x2),
										'0' + ((m_ir >> 12) & 0x1));
					}
					asm_out("%s	x%d,x%d,%d", s, rd, rs1, imm);
					next_pc = r_pc + 4;
					break;
				case 0b0110011: // R-type
					decode_r_type(rd, rs1, rs2);
					if ((m_ir & 0xbe000000) == 0x00000000) { // ALU operations
						switch ((m_ir >> 12) & 0x7) {
							case 0b000:
								if ((m_ir & 0xfe000000) == 0x00000000) { // ADD
									asm_ins("add");
									r_gpr[rd] = r_gpr[rs1] + r_gpr[rs2];
								} else if ((m_ir & 0xfe000000) == 0x40000000) { // SUB
									asm_ins("sub");
									r_gpr[rd] = r_gpr[rs1] - r_gpr[rs2];
								} else
									fprintf(stderr, "Unknown add/sub instruction: func7 = 0b%c%c%c%c%c%c%c\n",
												'0' + ((m_ir >> 25) & 0x40),
												'0' + ((m_ir >> 25) & 0x20),
												'0' + ((m_ir >> 25) & 0x10),
												'0' + ((m_ir >> 25) & 0x08),
												'0' + ((m_ir >> 25) & 0x04),
												'0' + ((m_ir >> 25) & 0x02),
												'0' + ((m_ir >> 25) & 0x01));
								break;
							case 0b001:  // SLL
								asm_ins("sll");
								r_gpr[rd] = r_gpr[rs1] << (r_gpr[rs2] & 0x1f);
								break;
							case 0b010:  // SLT
								asm_ins("slt");
								r_gpr[rd] = (int32_t)r_gpr[rs1] < (int32_t)r_gpr[rs2];
								break;
							case 0b011:  // SLTU
								asm_ins("sltu");
								r_gpr[rd] = r_gpr[rs1] < r_gpr[rs2];
								break;
							case 0b100:  // XOR
								asm_ins("xor");
								r_gpr[rd] = r_gpr[rs1] ^ r_gpr[rs2];
								break;
							case 0b101:
								if ((m_ir & 0xfe000000) == 0x00000000) { // SRL
									asm_ins("srl");
									r_gpr[rd] = r_gpr[rs1] >> (r_gpr[rs2] & 0x1f);
								} else if ((m_ir & 0xfe000000) == 0x40000000) { // SRA
									asm_ins("sra");
									r_gpr[rd] = (int32_t)r_gpr[rs1] >> (r_gpr[rs2] & 0x1f);
								} else
									fprintf(stderr, "Unknown register shift right instruction: func7 = 0b%c%c%c%c%c%c%c\n",
												'0' + ((m_ir >> 25) & 0x40),
												'0' + ((m_ir >> 25) & 0x20),
												'0' + ((m_ir >> 25) & 0x10),
												'0' + ((m_ir >> 25) & 0x08),
												'0' + ((m_ir >> 25) & 0x04),
												'0' + ((m_ir >> 25) & 0x02),
												'0' + ((m_ir >> 25) & 0x01));
								break;
							case 0b110:  // OR
								asm_ins("or");
								r_gpr[rd] = r_gpr[rs1] | r_gpr[rs2];
								break;
							case 0b111:  // AND
								asm_ins("and");
								r_gpr[rd] = r_gpr[rs1] & r_gpr[rs2];
								break;
							default:
								fprintf(stderr, "Unknown register instruction: func3 = 0b%c%c%c\n",
										'0' + ((m_ir >> 12) & 0x4),
										'0' + ((m_ir >> 12) & 0x2),
										'0' + ((m_ir >> 12) & 0x1));
						}
					} else if ((m_ir & 0xfe000000) == 0x02000000) { // MULT/DIV operations
						switch ((m_ir >> 12) & 0x7) {
							case 0b000:  // MUL
								asm_ins("mul");
								r_gpr[rd] = r_gpr[rs1] * r_gpr[rs2];
								break;
							case 0b001:  // MULH
								asm_ins("mulh");
								r_gpr[rd] = ((int64_t)((int32_t)r_gpr[rs1]) * (int32_t)r_gpr[rs2]) >> 32;
								break;
							case 0b010:  // MULHSU
								asm_ins("mulhsu");
								r_gpr[rd] = ((int64_t)((int32_t)r_gpr[rs1]) * (uint32_t)r_gpr[rs2]) >> 32;
								break;
							case 0b011:  // MULHU
								asm_ins("mulhu");
								r_gpr[rd] = ((uint64_t)r_gpr[rs1] * r_gpr[rs2]) >> 32;
								break;
							case 0b100:  // DIV
								asm_ins("div");
								// division by zero
								if (!r_gpr[rs2]) {
									r_gpr[rd] = -1;
									break;
								}
								// overflow
								if ((int32_t)r_gpr[rs1] == INT32_MIN && (int32_t)r_gpr[rs2] == -1) {
									r_gpr[rd] = (uint32_t)INT32_MIN;
									break;
								}
								r_gpr[rd] = (int32_t)r_gpr[rs1] / (int32_t)r_gpr[rs2];
								break;
							case 0b101:  // DIVU
								asm_ins("divu");
								// division by zero
								if (!r_gpr[rs2]) {
									r_gpr[rd] = UINT32_MAX;
									break;
								}
								r_gpr[rd] = r_gpr[rs1] / r_gpr[rs2];
								break;
							case 0b110:  // REM
								asm_ins("rem");
								// division by zero
								if (!r_gpr[rs2]) {
									r_gpr[rd] = r_gpr[rs1];
									break;
								}
								// overflow
								if ((int32_t)r_gpr[rs1] == INT32_MIN && (int32_t)r_gpr[rs2] == -1) {
									r_gpr[rd] = 0;
									break;
								}
								r_gpr[rd] = (int32_t)r_gpr[rs1] % (int32_t)r_gpr[rs2];
								break;
							case 0b111:  // REMU
								asm_ins("remu");
								// division by zero
								if (!r_gpr[rs2]) {
									r_gpr[rd] = r_gpr[rs1];
									break;
								}
								r_gpr[rd] = r_gpr[rs1] % r_gpr[rs2];
								break;
						}
					} else {
						fprintf(stderr, "Unknown register alu instruction: func7 = 0b%c%c%c%c%c%c%c\n",
									'0' + ((m_ir >> 25) & 0x40),
									'0' + ((m_ir >> 25) & 0x20),
									'0' + ((m_ir >> 25) & 0x10),
									'0' + ((m_ir >> 25) & 0x08),
									'0' + ((m_ir >> 25) & 0x04),
									'0' + ((m_ir >> 25) & 0x02),
									'0' + ((m_ir >> 25) & 0x01));
					}
					asm_out("%s	x%d,x%d,x%d", s, rd, rs1, rs2);
					next_pc = r_pc + 4;
					break;
				case 0b0001111: // ?-type
					/* Memory ordering operations are sent to the cache controller
					 * using XTN_WRITE operations:
					 *
					 * - The specific opcode is sent into the address field. The 2
					 *   LSb of the address are set to 0 (required by the cache
					 *   controller).
					 *
					 * - Flags are sent into the WDATA field.
					 */
					decode_s_type(rs1, rs2, imm);
					if ((m_ir & 0x000fffff) == 0x0000000f) { // FENCE, unimplemented yet
							asm_out("%s", "fence");
							store(XTN_WRITE, XTN_SYNC << 2, imm & 0xff, 0);
					} else if (m_ir == 0x0000100f) { // FENCE.I
							asm_out("%s", "fence.i");
							store(XTN_WRITE, XTN_ICACHE_FLUSH << 2, 0, 0);
					} else {
						fprintf(stderr, "Unknown fence instruction\n");
					}
					next_pc = r_pc + 4;
					break;
				case 0b1110011: // I-type, or close, unimplemented yet
					if (m_ir == 0x00000073) { // ECALL
							asm_out("%s", "ecall");
							/*  TODO verify that for the ECALL instruction this is all
							 *  we need to do.
							 */
							/*  As the only currently supported mode is Machine-Mode,
							 *  it is the only source mode for the ECALL instruction
							 */
							r_csr[csr_mcause] = ENVIRONMENT_CALL_FROM_M_MODE_TRAP;
							exception = true;
							goto __iss_handle_exception;
					} else if (m_ir == 0x00100073) { // EBREAK
							asm_out("%s", "ebreak");
							r_csr[csr_mcause] = BREAKPOINT_TRAP;
							exception = true;
							goto __iss_handle_exception;
					} else {
						switch ((m_ir >> 12) & 0x7) {
							case 0b000:  // PRIV
								if (m_ir == 0x10500073) {
										asm_out("%s", "wfi");
								} else if (m_ir == 0x30200073) {
										asm_out("%s", "mret");
										// MPP is set t machine mode
										r_csr[csr_mstatus] |= 0x00001800;
										// MPIE is set to 1 and MIE is set to MPIE
										r_csr[csr_mstatus] |= 0x80 | ((r_csr[csr_mstatus] & 0x80) >> 4);
										// next instruction follows the one that was interrupted
										next_pc = r_csr[csr_mepc];
										// as this is the only instruction of that kind that doesn't use
										// r_pc, let us jump directly to the end
										goto skip_next_pc;
								} else
									fprintf(stderr, "Unknown priviledged instruction: 0x%08x\n", m_ir);
								break;
							case 0b001:  // CSRRW
								decode_csr_type(rd, csr, rs1);
								asm_ins("csrrw");
								asm_out("%s	x%d,%s,x%d", s, rd, csr_name(csr), rs1);
								/* Handling the (weird) floating point csr accesses.
								 * This code is more or less replicated in the other csr insns,
								 * I could not find a nice way of factorizing it */
								switch (csr) {
									case 0x001: // fflags
										/* A bit brutal, but simple at least */
										csr        = 0x003;
										reg        = r_csr[csr] & 0b11111;
										r_csr[csr] = (r_csr[csr] & ~0b11111) | (r_gpr[rs1] & 0b11111);
										break;
									case 0x002: // frm
										csr        = 0x003;
										reg        = (r_csr[csr] >> 5) & 0b111;
										r_csr[csr] = (r_csr[csr] & ~0b11100000) | ((r_gpr[rs1] & 0b111) << 5);
										break;
									case 0x003: // fcsr
										reg        = r_csr[csr] & 0xff;
										r_csr[csr] = r_gpr[rs1] & 0xff;
										break;
									default: // The rest of the world, for now :)
										reg        = r_csr[csr];
										r_csr[csr] = r_gpr[rs1];
								}
								if (rd != 0) {
									r_gpr[rd] = reg;
								}
								m_update_csr  = true;
								m_csr_changed = csr;
								break;
							case 0b010:  // CSRRS
								decode_csr_type(rd, csr, rs1);
								asm_ins("csrrs");
								asm_out("%s	x%d,%s,x%d", s, rd, csr_name(csr), rs1);
								switch (csr) {
									case 0x001:
										csr = 0x003;
										reg = r_csr[csr] & 0b11111;
										ger = r_gpr[rs1] & 0b11111;
										break;
									case 0x002:
										csr = 0x003;
										reg = (r_csr[csr] >> 5) & 0b111;
										ger = (r_gpr[rs1] & 0b111) << 5;
										break;
									case 0x003: // fcsr
										reg = r_csr[csr] & 0xff;
										ger = r_gpr[rs1] & 0xff;
										break;
									default:
										reg = r_csr[csr];
										ger = r_gpr[rs1];
								}
								if (rs1 != 0) {
									m_update_csr  = true;
									m_csr_changed = csr;
									r_csr[csr]   |= ger;
								}
								if (rd != 0) {
									r_gpr[rd]     = reg;
								}
								break;
							case 0b011:  // CSRRC
								decode_csr_type(rd, csr, rs1);
								asm_ins("csrrc");
								asm_out("%s	x%d,%s,x%d", s, rd, csr_name(csr), rs1);
								switch (csr) {
									case 0x001:
										csr = 0x003;
										reg = r_csr[csr] & 0b11111;
										ger = r_gpr[rs1] & 0b11111;
										break;
									case 0x002:
										csr = 0x003;
										reg = (r_csr[csr] >> 5) & 0b111;
										ger = (r_gpr[rs1] & 0b111) << 5;
										break;
									case 0x003: // fcsr
										reg = r_csr[csr] & 0xff;
										ger = r_gpr[rs1] & 0xff;
										break;
									default:
										reg = r_csr[csr];
										ger = r_gpr[rs1];
								}
								reg = r_csr[csr];
								if (rs1 != 0) {
									m_update_csr  = true;
									m_csr_changed = csr;
									r_csr[csr]   &= ~ger;
								}
								if (rd != 0) {
									r_gpr[rd] = reg;
								}
								break;
							case 0b101:  // CSRRWI
								decode_csr_type(rd, csr, imm);
								asm_ins("csrrwi");
								asm_out("%s	x%d,%s,%d", s, rd, csr_name(csr), imm);
								switch (csr) {
									case 0x001:
										csr        = 0x003;
										reg        = r_csr[csr] & 0b11111;
										r_csr[csr] = (r_csr[csr] & ~0b11111) | (imm & 0b11111);
										break;
									case 0x002:
										csr        = 0x003;
										reg        = (r_csr[csr] >> 5) & 0b111;
										r_csr[csr] = (r_csr[csr] & ~0b11100000) | ((imm & 0b111) << 5);
										break;
									case 0x003:
										reg        = r_csr[csr] & 0xff;
										r_csr[csr] = imm & 0xff;
										break;
									default:
										reg        = r_csr[csr];
										r_csr[csr] = imm;
								}
								if (rd != 0) {
									r_gpr[rd] = reg;
								}
								m_csr_changed = csr;
								m_update_csr  = true;
								break;
							case 0b110:  // CSRRSI
								decode_csr_type(rd, csr, imm);
								asm_ins("csrrsi");
								asm_out("%s	x%d,%s,%d", s, rd, csr_name(csr), imm);
								switch (csr) {
									case 0x001:
										csr = 0x003;
										reg = r_csr[csr] & 0b11111;
										ger = imm & 0b11111;
										break;
									case 0x002:
										csr = 0x003;
										reg = (r_csr[csr] >> 5) & 0b111;
										ger = (imm & 0b111) << 5;
										break;
									case 0x003: // fcsr
										reg = r_csr[csr] & 0xff;
										ger = imm & 0xff;
										break;
									default:
										reg = r_csr[csr];
										ger = imm;
								}
								if (imm != 0) {
									m_update_csr  = true;
									m_csr_changed = csr;
									r_csr[csr]   |= ger;
								}
								if (rd != 0) {
									r_gpr[rd] = reg;
								}
								break;
							case 0b111:  // CSRRCI
								decode_csr_type(rd, csr, imm);
								asm_ins("csrrci");
								asm_out("%s	x%d,%s,%d", s, rd, csr_name(csr), imm);
								switch (csr) {
									case 0x001:
										csr = 0x003;
										reg = r_csr[csr] & 0b11111;
										ger = imm & 0b11111;
										break;
									case 0x002:
										csr = 0x003;
										reg = (r_csr[csr] >> 5) & 0b111;
										ger = (imm & 0b111) << 5;
										break;
									case 0x003: // fcsr
										reg = r_csr[csr] & 0xff;
										ger = imm & 0xff;
										break;
									default:
										reg = r_csr[csr];
										ger = imm;
								}
								if (imm != 0) {
									m_update_csr  = true;
									m_csr_changed = csr;
									r_csr[csr]   &= ~ger;
								}
								if (rd != 0) {
									r_gpr[rd] = reg;
								}
								break;
							default:
								fprintf(stderr, "Unknown csr instruction: func3 = 0b%c%c%c\n",
											'0' + ((m_ir >> 12) & 0x4),
											'0' + ((m_ir >> 12) & 0x2),
											'0' + ((m_ir >> 12) & 0x1));
						}
					}
					next_pc = r_pc + 4;
					break;
				/*\
				 *
				 * End of Integer + Mult extensions
				 *
				 * Start of single precision floating point.
				 * Note that we do it the "quick and dirty poor old's man" way, by using the host floating points instead
				 * of floating point emulation as QEMU and other do to actually following the exact
				 * riscv specs.
				 * Note that we do not either set the floating point exception flags, ...
				 * 31           27      26   25   24          20   19         15   14    12   11     7   6             0
				 *             funct7                   rs2             rs1        funct3       rd           opcode        R-type
				 *       rs3      funct2                rs2             rs1        funct3       rd           opcode        R4-type
				 *              imm[11:0]                               rs1        funct3       rd           opcode        I-type
				 *        imm[11:5]                     rs2             rs1        funct3     imm[4:0]       opcode        S-type
				 *                               RV32F Standard Extension
				 *         imm[11:0]                        rs1     010                   rd           0000111       FLW
				 *   imm[11:5]                    rs2       rs1     010                 imm[4:0]       0100111       FSW
				 * rs3         00                 rs2       rs1     rm                    rd           1000011       FMADD.S
				 * rs3         00                 rs2       rs1     rm                    rd           1000111       FMSUB.S
				 * rs3         00                 rs2       rs1     rm                    rd           1001011       FNMSUB.S
				 * rs3         00                 rs2       rs1     rm                    rd           1001111       FNMADD.S
				 *    0000000                     rs2       rs1     rm                    rd           1010011       FADD.S
				 *    0000100                     rs2       rs1     rm                    rd           1010011       FSUB.S
				 *    0001000                     rs2       rs1     rm                    rd           1010011       FMUL.S
				 *    0001100                     rs2       rs1     rm                    rd           1010011       FDIV.S
				 *    0101100                    00000      rs1     rm                    rd           1010011       FSQRT.S
				 *    0010000                     rs2       rs1     000                   rd           1010011       FSGNJ.S
				 *    0010000                     rs2       rs1     001                   rd           1010011       FSGNJN.S
				 *    0010000                     rs2       rs1     010                   rd           1010011       FSGNJX.S
				 *    0010100                     rs2       rs1     000                   rd           1010011       FMIN.S
				 *    0010100                     rs2       rs1     001                   rd           1010011       FMAX.S
				 *    1100000                    00000      rs1     rm                    rd           1010011       FCVT.W.S
				 *    1100000                    00001      rs1     rm                    rd           1010011       FCVT.WU.S
				 *    1110000                    00000      rs1     000                   rd           1010011       FMV.X.W
				 *    1010000                     rs2       rs1     010                   rd           1010011       FEQ.S
				 *    1010000                     rs2       rs1     001                   rd           1010011       FLT.S
				 *    1010000                     rs2       rs1     000                   rd           1010011       FLE.S
				 *    1110000                    00000      rs1     001                   rd           1010011       FCLASS.S
				 *    1101000                    00000      rs1     rm                    rd           1010011       FCVT.S.W
				 *    1101000                    00001      rs1     rm                    rd           1010011       FCVT.S.WU
				 *    1111000                    00000      rs1     000                   rd           1010011       FMV.W.X
				\*/
 				case 0b0000111:
					decode_i_type(rd, rs1, imm);
					asm_ins("flw");
					asm_out("%s	f%d,%d(x%d)", s, rd, imm, rs1);
					unsign = true; // unused, but gcc doesn't know it
					addr   = r_gpr[rs1] + imm;
					load((uint32_t *)&r_fpr[rd], DATA_READ, unsign, addr, 4);
					next_pc = r_pc + 4;
					break;
				case 0b0100111:
					decode_s_type(rs1, rs2, imm);
					asm_ins("fsw");
					asm_out("%s	f%d,%d(x%d)", s, rs2, imm, rs1);
					addr    = r_gpr[rs1] + imm;
					suf.f   = r_fpr[rs2];
					store(DATA_WRITE, addr, suf.u, 4);
					next_pc = r_pc + 4;
					break;

/*\
 * Floating point errors do not raise exceptions, see page 60 of the
 * Volume I: RISC-V User-Level ISA V2.3-draft
 * Doing a bit of a dirty hack to indicate that the illegal cases should hardly
 * happen (the first one being quite unprobable since the instruction must be written
 * directly in binary for it to occur!).
 * Note also that csr_fcsr is the actual register, csr_fflags and csr_frm are other
 * (kind of weird) accesses to this register.
 * Note: x86_64 canonical float NaN is 0xffc00000 while riscv float NaN is 0x7fc00000, ...
\*/
#define FP_OP_CHECK(x, y)                    \
do {                                         \
	int exception;                            \
	if (unlikely(rm == rxx || rm == ryy)) {   \
		r_csr[csr_fcsr] |= 0x00000010;         \
	} else if (unlikely(rm == dyn)) {         \
		rm = (r_csr[csr_fcsr] >> 5) & 0x7;     \
		if (unlikely(rm == 0b111)) {           \
			r_csr[csr_fcsr] |= 0x00000010;      \
		}                                      \
	}                                         \
	feclearexcept(FE_ALL_EXCEPT);             \
	fesetround(rounding[rm]);                 \
	x = y;                                    \
	if (unlikely(isnan(x))) {                 \
		suf_t suf;                             \
		suf.u = 0x7fc00000;                    \
		x = suf.f;                             \
	}                                         \
	exception = fetestexcept(FE_ALL_EXCEPT);  \
	r_csr[csr_fcsr] |=                        \
		(!!(exception & FE_INVALID)   << 4)    \
		| (!!(exception & FE_DIVBYZERO) << 3)  \
		| (!!(exception & FE_OVERFLOW)  << 2)  \
		| (!!(exception & FE_UNDERFLOW) << 1)  \
		| (!!(exception & FE_INEXACT)   << 0); \
} while (0)

/*\
 * More or less identical to FP_OP_CHECK, but does not test NaN as the
 * result is within an integer register.
 * CC stand for conversion and comparaison, by the way.
\*/
#define FP_CC_CHECK(x, y)                    \
do {                                         \
	int exception;                            \
	if (unlikely(rm == rxx || rm == ryy)) {   \
		r_csr[csr_fcsr] |= 0x00000010;         \
	} else if (unlikely(rm == dyn)) {         \
		rm = (r_csr[csr_fcsr] >> 5) & 0x7;     \
		if (unlikely(rm == 0b111)) {           \
			r_csr[csr_fcsr] |= 0x00000010;      \
		}                                      \
	}                                         \
	feclearexcept(FE_ALL_EXCEPT);             \
	fesetround(rounding[rm]);                 \
	x = y;                                    \
	exception = fetestexcept(FE_ALL_EXCEPT);  \
	r_csr[csr_fcsr] |=                      \
		(!!(exception & FE_INVALID)   << 4)    \
		| (!!(exception & FE_DIVBYZERO) << 3)  \
		| (!!(exception & FE_OVERFLOW)  << 2)  \
		| (!!(exception & FE_UNDERFLOW) << 1)  \
		| (!!(exception & FE_INEXACT)   << 0); \
} while (0)

				case 0b1000011:
					decode_r4_type(rd, rs1, rs2, rs3, fmt, rm);
					if (fmt != 0)
						fprintf(stderr, "Argh!\n");
					asm_ins("fmadd.s");
					asm_out("%s	f%d,f%d,f%d,f%d)", s, rd, rs1, rs2, rs3);
//#pragma STDC FP_CONTRACT ON
					FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] * r_fpr[rs2] + r_fpr[rs3]);
//#pragma STDC FP_CONTRACT OFF
					next_pc = r_pc + 4;
					break;
				case 0b1000111:
					decode_r4_type(rd, rs1, rs2, rs3, fmt, rm);
					if (fmt != 0)
						fprintf(stderr, "Argh!\n");
					asm_ins("fmsub.s");
					asm_out("%s	f%d,f%d,f%d,f%d)", s, rd, rs1, rs2, rs3);
//#pragma STDC FP_CONTRACT ON
					FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] * r_fpr[rs2] - r_fpr[rs3]);
//#pragma STDC FP_CONTRACT OFF
					next_pc = r_pc + 4;
					break;
				case 0b1001011:
					decode_r4_type(rd, rs1, rs2, rs3, fmt, rm);
					if (fmt != 0)
						fprintf(stderr, "Argh!\n");
					asm_ins("fnmsub.s");
					asm_out("%s	f%d,f%d,f%d,f%d)", s, rd, rs1, rs2, rs3);
//#pragma STDC FP_CONTRACT ON
					FP_OP_CHECK(r_fpr[rd], -r_fpr[rs1] * r_fpr[rs2] + r_fpr[rs3]);
//#pragma STDC FP_CONTRACT OFF
					next_pc = r_pc + 4;
					break;
				case 0b1001111:
					asm_ins("fnmadd.s");
					decode_r4_type(rd, rs1, rs2, rs3, fmt, rm);
					if (fmt != 0)
						fprintf(stderr, "Argh!\n");
					asm_ins("fmadd.s");
					asm_out("%s	f%d,f%d,f%d,f%d)", s, rd, rs1, rs2, rs3);
//#pragma STDC FP_CONTRACT ON
					FP_OP_CHECK(r_fpr[rd], -r_fpr[rs1] * r_fpr[rs2] - r_fpr[rs3]);
//#pragma STDC FP_CONTRACT OFF
					next_pc = r_pc + 4;
					break;
				case 0b1010011: // R-type, OP-FP
					decode_r_type(rd, rs1, rs2);
					rm = (m_ir >> 12) & 7;
					switch ((m_ir >> 25) & 0x7f) {
						case 0b0000000:
							asm_ins("fadd.s");
							asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
							FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] + r_fpr[rs2]);
							break;
						case 0b0000100:
							asm_ins("fsub.s");
							asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
							FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] - r_fpr[rs2]);
							break;
						case 0b0001000:
							asm_ins("fmul.s");
							asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
							FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] * r_fpr[rs2]);
							break;
						case 0b0001100:
							asm_ins("fdiv.s");
							asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
							FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] / r_fpr[rs2]);
							break;
						case 0b0101100:
							asm_ins("fsqrt.s");
							FP_OP_CHECK(r_fpr[rd], sqrt(r_fpr[rs1]));
							asm_out("%s	f%d,f%d", s, rd, rs1);
							break;
						case 0b0010000:
							switch ((m_ir >> 12) & 0x7) { // func3
								case 0b000:
									if (rs1 == rs2) {
										asm_ins("fmv.s");
										asm_out("%s	f%d,f%d", s, rd, rs1);
									} else {
										asm_ins("fsgnj.s");
										asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
									}
									suf.f = r_fpr[rs1];
									fus.f = r_fpr[rs2];
									suf.u = (fus.u & 0x80000000) | (suf.u & 0x7fffffff);
									r_fpr[rd] = suf.f;
									break;
								case 0b001:
									if (rs1 == rs2) {
										asm_ins("fneg.s");
										asm_out("%s	f%d,f%d", s, rd, rs1);
									} else {
										asm_ins("fsgnjn.s");
										asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
									}
									suf.f = r_fpr[rs1];
									fus.f = r_fpr[rs2];
									suf.u = (0x80000000 ^ (fus.u & 0x80000000)) | (suf.u & 0x7fffffff);
									r_fpr[rd] = suf.f;
									break;
								case 0b010:
									if (rs1 == rs2) {
										asm_ins("fabs.s");
										asm_out("%s	f%d,f%d", s, rd, rs1);
									} else {
										asm_ins("fsgnjx.s");
										asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
									}
									suf.f = r_fpr[rs1];
									fus.f = r_fpr[rs2];
									suf.u = ((suf.u & 0x80000000) ^ (fus.u & 0x80000000)) | (suf.u & 0x7fffffff);
									r_fpr[rd] = suf.f;
									break;
								default:
									fprintf(stderr, "Illegal single precision float instruction: func3 = 0b%c%c%c\n",
												'0' + ((m_ir >> 12) & 0x4),
												'0' + ((m_ir >> 12) & 0x2),
												'0' + ((m_ir >> 12) & 0x1));
							}
							break;
						case 0b0010100:
							switch ((m_ir >> 12) & 0x7) {
								case 0b000:
									asm_ins("fmin.s");
									break;
								case 0b001:
									asm_ins("fmax.s");
									break;
								default:
									fprintf(stderr, "Illegal single precision float instruction: func3 = 0b%c%c%c\n",
												'0' + ((m_ir >> 12) & 0x4),
												'0' + ((m_ir >> 12) & 0x2),
												'0' + ((m_ir >> 12) & 0x1));
							}
							asm_out("%s	f%d,f%d,f%d", s, rd, rs1, rs2);
							/*\
							 * Min and Max have the following behavior:
							 * If both operands are NaN, returns the canonical NaN, otherwise if one is NaN returns
							 * the other one, otherwise return the actual results (which is not what the x86_64 does,
							 * explaining the spaghetti code).
							\*/
							if (((m_ir >> 12) & 0x6) == 0) {
								if (isnan(r_fpr[rs1]) && isnan(r_fpr[rs2])) {
									if (issignaling(r_fpr[rs1]) || issignaling(r_fpr[rs2]))
										r_csr[csr_fcsr] |= 1 << 4;
									suf.u = 0x7fc00000;
									r_fpr[rd] = suf.f;
									break;
								} else if (isnan(r_fpr[rs1])) {
									if (issignaling(r_fpr[rs1]))
										r_csr[csr_fcsr] |= 1 << 4;
									r_fpr[rd] = r_fpr[rs2];
									break;
								} else if (isnan(r_fpr[rs2])) {
									if (issignaling(r_fpr[rs2]))
										r_csr[csr_fcsr] |= 1 << 4;
									r_fpr[rd] = r_fpr[rs1];
									break;
								}
							}
							/* +0 and -0 compare as equal on x86_64, so we need to handle that special case
							 * to conform with the riscv specs. */
							suf.f = r_fpr[rs1];
							fus.f = r_fpr[rs2];
							switch ((m_ir >> 12) & 0x7) {
								case 0b000:
									if (suf.u == 0x80000000 && fus.u == 0x00000000)
										r_fpr[rd] = suf.f;
									else if (suf.u == 0x00000000 && fus.u == 0x80000000)
										r_fpr[rd] = fus.f;
									else
										FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] <= r_fpr[rs2] ? r_fpr[rs1] : r_fpr[rs2]);
									break;
								case 0b001:
									if (suf.u == 0x80000000 && fus.u == 0x00000000)
										r_fpr[rd] = fus.f;
									else if (suf.u == 0x00000000 && fus.u == 0x80000000)
										r_fpr[rd] = suf.f;
									else
										FP_OP_CHECK(r_fpr[rd], r_fpr[rs1] >= r_fpr[rs2] ? r_fpr[rs1] : r_fpr[rs2]);
									break;
								default:
									;
							}
							break;
						case 0b1100000:
							/* Compute specific values according to the specs when not in the
							 * integer/unsigned integer range.
							 * Note the use of __builtin_isinf_sign because g++-8 considers isinf
							 * as a boolean, and thus the test fails, hell.
							 * The bounds checks are somehow weird, but they pass the tests :) */
							switch (rs2) {
								case 0b00000:
									asm_ins("fcvt.w.s");
									FP_CC_CHECK(r_gpr[rd], (int32_t)r_fpr[rs1]);
									if (__builtin_isinf_sign(r_fpr[rs1]) == -1
											|| r_fpr[rs1] < (float)-(1ll << 31) - 1)
										r_gpr[rd] = 0x80000000;
									else if (r_fpr[rs1] >= (float)(1ll << 31)
												|| __builtin_isinf_sign(r_fpr[rs1]) == 1
												|| isnan(r_fpr[rs1]))
										r_gpr[rd] = 0x7fffffff;
									break;
								case 0b00001:
									asm_ins("fcvt.wu.s");
									FP_CC_CHECK(r_gpr[rd], (uint32_t)r_fpr[rs1]);
									/* No so proud of this code for which I infered the conditions from the (far
									 * from complete) tests provided in riscv-tests.
									 * FIXME: I am also afraid this works only for rm == rtz, ...*/
									if (__builtin_isinf_sign(r_fpr[rs1]) == -1) {
										r_gpr[rd] = 0x00000000;
										r_csr[csr_fcsr] |= 1 << 4;
									} else if (r_fpr[rs1] <= -1.0) {
										r_gpr[rd] = 0x00000000;
										r_csr[csr_fcsr] |= 1 << 4;
									} else if (r_fpr[rs1] < 0.0) {
										r_gpr[rd] = 0x00000000;
									} else if (r_fpr[rs1] >= (float)((1ll << 32) - 1)
													|| __builtin_isinf_sign(r_fpr[rs1]) == 1
													|| isnan(r_fpr[rs1])) {
										r_gpr[rd] = 0xffffffff;
									}
									break;
								default:
									fprintf(stderr, "Illegal floating point instruction ‘0x%08x’ !\n", m_ir);
							}
							asm_out("%s	x%d,f%d,%s", s, rd, rs1, rounding_name(rm));
							break;
						case 0b1010000:
							switch ((m_ir >> 12) & 0x7) {
								case 0b010:
									asm_ins("feq.s");
									FP_CC_CHECK(r_gpr[rd], r_fpr[rs1] == r_fpr[rs2] ? 1 : 0);
									break;
								case 0b001:
									asm_ins("flt.s");
									FP_CC_CHECK(r_gpr[rd], r_fpr[rs1] < r_fpr[rs2] ? 1 : 0);
									break;
								case 0b000:
									asm_ins("fle.s");
									FP_CC_CHECK(r_gpr[rd], r_fpr[rs1] <= r_fpr[rs2] ? 1 : 0);
									break;
								default:
									fprintf(stderr, "Illegal single precision float instruction: func3 = 0b%c%c%c\n",
												'0' + ((m_ir >> 12) & 0x4),
												'0' + ((m_ir >> 12) & 0x2),
												'0' + ((m_ir >> 12) & 0x1));
							}
							asm_out("%s	x%d,f%d,f%d", s, rd, rs1, rs2);
							break;
						case 0b1110000:
							switch (rm) {
								case 0b000:
									asm_ins("fmv.x.w");
									suf.f = r_fpr[rs1];
									r_gpr[rd] = suf.u;
									break;
								case 0b001:
									asm_ins("fclass.s");
									switch (fpclassify(r_fpr[rs1])) {
										case FP_NAN:
											suf.f = r_fpr[rs1];
											r_gpr[rd] = 1 << (((suf.u & 0x00c00000) == 0x00800000) ? 8 : 9);
											break;
										case FP_INFINITE:
											r_gpr[rd] = 1 << (signbit(r_fpr[rs1]) ? 0 : 7);
											break;
										case FP_ZERO:
											r_gpr[rd] = 1 << (signbit(r_fpr[rs1]) ? 3 : 4);
											break;
										case FP_SUBNORMAL:
											r_gpr[rd] = 1 << (signbit(r_fpr[rs1]) ? 2 : 5);
											break;
										case FP_NORMAL:
											r_gpr[rd] = 1 << (signbit(r_fpr[rs1]) ? 1 : 6);
											break;
										default:
											suf.f = r_fpr[rs1];
											fprintf(stderr, "Argh! Cannot classify single precision floating point “0x%08x”\n", suf.u);
									}
									break;
								default:
									fprintf(stderr, "Illegal floating point instruction ‘0x%08x’ !\n", m_ir);
							}
							asm_out("%s	x%d,f%d", s, rd, rs1);
							break;
						case 0b1101000:
							switch (rs2) {
								case 0b00000:
									asm_ins("fcvt.s.w");
									FP_OP_CHECK(r_fpr[rd], (float)((int32_t)r_gpr[rs1]));
									break;
								case 0b00001:
									asm_ins("fcvt.s.wu");
									FP_OP_CHECK(r_fpr[rd], (float)r_gpr[rs1]);
									break;
								default:
									fprintf(stderr, "Illegal floating point instruction ‘0x%08x’ !\n", m_ir);
							}
							asm_out("%s	f%d,x%d", s, rd, rs1);
							break;
						case 0b1111000:
							asm_ins("fmv.w.x");
							asm_out("%s	f%d,x%d", s, rd, rs1);
							suf.u = r_gpr[rs1];
							r_fpr[rd] = suf.f;
							break;
						default:
							fprintf(stderr, "Unknown single precision floating point instruction: func7 = 0b%c%c%c%c%c%c%c\n",
										'0' + ((m_ir >> 25) & 0x40),
										'0' + ((m_ir >> 25) & 0x20),
										'0' + ((m_ir >> 25) & 0x10),
										'0' + ((m_ir >> 25) & 0x08),
										'0' + ((m_ir >> 25) & 0x04),
										'0' + ((m_ir >> 25) & 0x02),
										'0' + ((m_ir >> 25) & 0x01));
					}
					next_pc = r_pc + 4;
					break;

				/*\
				 * Atomic instruction set
				 *
				 * 31           27   26   25   24          20   19         15   14    12   11     7   6           0
				 *                            RV32A Standard Extension
				 *      00010        aq   rl         00000           rs1          011         rd         0101111     LR.W
				 *      00011        aq   rl         rs2             rs1          011         rd         0101111     SC.W
				 *      00001        aq   rl         rs2             rs1          011         rd         0101111     AMOSWAP.W
				 *      00000        aq   rl         rs2             rs1          011         rd         0101111     AMOADD.W
				 *      00100        aq   rl         rs2             rs1          011         rd         0101111     AMOXOR.W
				 *      01100        aq   rl         rs2             rs1          011         rd         0101111     AMOAND.W
				 *      01000        aq   rl         rs2             rs1          011         rd         0101111     AMOOR.W
				 *      10000        aq   rl         rs2             rs1          011         rd         0101111     AMOMIN.W
				 *      10100        aq   rl         rs2             rs1          011         rd         0101111     AMOMAX.W
				 *      11000        aq   rl         rs2             rs1          011         rd         0101111     AMOMINU.W
				 *      11100        aq   rl         rs2             rs1          011         rd         0101111     AMOMAXU.W
				\*/
				case 0b0101111:
				{
					/* We do not use the release and acquire hints as we suppose
					 * that cache will be handle AMO as uncached accesses with
					 * strict ordering */
					__unused__ bool release = (m_ir >> 25) & 0x1;
					__unused__ bool acquire = (m_ir >> 26) & 0x1;

					decode_r_type(rd, rs1, rs2);
					data = r_gpr[rs2];
					addr = r_gpr[rs1];
					switch ((m_ir >> 27) & 0x7f) {
						case 0b00010:
							asm_ins("lr.w");
							access = DATA_LR;
							break;
						case 0b00011:
							asm_ins("sc.w");
							access = DATA_SC;
							break;
						case 0b00001:
							asm_ins("amoswap.w");
							access = DATA_AMO_SWAP;
							break;
						case 0b00000:
							asm_ins("amoadd.w");
							access = DATA_AMO_ADD;
							break;
						case 0b00100:
							asm_ins("amoxor.w");
							access = DATA_AMO_XOR;
							break;
						case 0b01100:
							asm_ins("amoand.w");
							access = DATA_AMO_AND;
							break;
						case 0b01000:
							asm_ins("amoor.w");
							access = DATA_AMO_OR;
							break;
						case 0b10000:
							asm_ins("amomin.w");
							access = DATA_AMO_MIN;
							break;
						case 0b10100:
							asm_ins("amomax.w");
							access = DATA_AMO_MAX;
							break;
						case 0b11000:
							asm_ins("amominu.w");
							access = DATA_AMO_MINU;
							break;
						case 0b11100:
							asm_ins("amomaxu.w");
							access = DATA_AMO_MAXU;
							break;
						default:
							access = DATA_READ;
							fprintf(stderr, "Unknown AMO instruction\n");
							break;
					}
					asm_out("%s x%d,x%d,(x%d)", s, rd, rs2, rs1);
					amo(&r_gpr[rd], access, addr, data);
					next_pc = r_pc + 4;
					break;
				}
				default:
					/*\
					 * Compressed instruction set, quite a hell of a case default I must say :)
					 * But this makes it easy to ensure that we do not have an unknown instruction going through
					 * this mess unseen.
					 *
					94                                                              Volume I: RISC-V User-Level ISA V2.3-draft
							15 14 13 12     11 10     9   8   7   6       5       4     3    2   1        0
							  000                    0                                  0            00       Illegal instruction
							  000             nzuimm[5:4|9:6|2|3]                      rd’           00       C.ADDI4SPN (RES, nzuimm=0)
							  001       uimm[5:3]    rs1’      uimm[7:6]               rd’           00       C.FLD (RV32/64)
							  001      uimm[5:4|8]   rs1’      uimm[7:6]               rd’           00       C.LQ (RV128)
							  010       uimm[5:3]    rs1’      uimm[2|6]               rd’           00       C.LW
							  011       uimm[5:3]    rs1’      uimm[2|6]               rd’           00       C.FLW (RV32)
							  011       uimm[5:3]    rs1’      uimm[7:6]               rd’           00       C.LD (RV64/128)
							  100                          —                                         00       Reserved
							  101       uimm[5:3]    rs1’      uimm[7:6]              rs2’           00       C.FSD (RV32/64)
							  101      uimm[5:4|8]   rs1’      uimm[7:6]              rs2’           00       C.SQ (RV128)
							  110       uimm[5:3]    rs1’      uimm[2|6]              rs2’           00       C.SW
							  111       uimm[5:3]    rs1’      uimm[2|6]              rs2’           00       C.FSW (RV32)
							  111       uimm[5:3]    rs1’      uimm[7:6]              rs2’           00       C.SD (RV64/128)

							000       nzimm[5]              0             nzimm[4:0]                 01       C.NOP (HINT, nzimm6=0)
							000       nzimm[5]         rs1/rd≠0           nzimm[4:0]                 01       C.ADDI (HINT, nzimm=0)
							001                  imm[11|4|9:8|10|6|7|3:1|5]                          01       C.JAL (RV32)
							001         imm[5]         rs1/rd≠0            imm[4:0]                  01       C.ADDIW (RV64/128; RES, rd=0)
							010         imm[5]            rd≠0             imm[4:0]                  01       C.LI (HINT, rd=0)
							011        nzimm[9]             2          nzimm[4|6|8:7|5]              01       C.ADDI16SP (RES, nzimm=0)
							011       nzimm[17]         rd≠{0, 2}        nzimm[16:12]                01       C.LUI (RES, nzimm=0; HINT, rd=0)
							100       nzuimm[5]      00      rs1’/rd’     nzuimm[4:0]                01       C.SRLI (RV32 NSE, nzuimm[5]=1)
							100            0         00      rs1’/rd’          0                     01       C.SRLI64 (RV128; RV32/64 HINT)
							100       nzuimm[5]      01      rs1’/rd’     nzuimm[4:0]                01       C.SRAI (RV32 NSE, nzuimm[5]=1)
							100            0         01      rs1’/rd’          0                     01       C.SRAI64 (RV128; RV32/64 HINT)
							100         imm[5]       10      rs1’/rd’      imm[4:0]                  01       C.ANDI
							100            0         11      rs1’/rd’    00      rs2’                01       C.SUB
							100            0         11      rs1’/rd’    01      rs2’                01       C.XOR
							100            0         11      rs1’/rd’    10      rs2’                01       C.OR
							100            0         11      rs1’/rd’    11      rs2’                01       C.AND
							100            1         11      rs1’/rd’    00      rs2’                01       C.SUBW (RV64/128; RV32 RES)
							100            1         11      rs1’/rd’    01      rs2’                01       C.ADDW (RV64/128; RV32 RES)
							100            1         11         —        10       —                  01       Reserved
							100            1         11         —        11       —                  01       Reserved
							101                  imm[11|4|9:8|10|6|7|3:1|5]                          01       C.J
							110           imm[8|4:3]           rs1’     imm[7:6|2:1|5]               01       C.BEQZ
							111           imm[8|4:3]           rs1’     imm[7:6|2:1|5]               01       C.BNEZ

						 15 14 13      12       11 10 9   8   7   6   5   4   3   2   1        0
							000      nzuimm[5]     rs1/rd≠0        nzuimm[4:0]             10       C.SLLI (HINT, rd=0; RV32 NSE, nzuimm[5]=1)
							000          0         rs1/rd≠0               0                10       C.SLLI64 (RV128; RV32/64 HINT; HINT, rd=0)
							001       uimm[5]          rd          uimm[4:3|8:6]           10       C.FLDSP (RV32/64)
							001       uimm[5]        rd≠0           uimm[4|9:6]            10       C.LQSP (RV128; RES, rd=0)
							010       uimm[5]        rd≠0          uimm[4:2|7:6]           10       C.LWSP (RES, rd=0)
							011       uimm[5]          rd          uimm[4:2|7:6]           10       C.FLWSP (RV32)
							011       uimm[5]        rd≠0          uimm[4:3|8:6]           10       C.LDSP (RV64/128; RES, rd=0)
							100          0           rs1≠0                0                10       C.JR (RES, rs1=0)
							100          0           rd≠0               rs2≠0              10       C.MV (HINT, rd=0)
							100          1              0                 0                10       C.EBREAK
							100          1           rs1≠0                0                10       C.JALR
							100          1         rs1/rd≠0             rs2≠0              10       C.ADD (HINT, rd=0)
							101            uimm[5:3|8:6]                 rs2               10       C.FSDSP (RV32/64)
							101            uimm[5:4|9:6]                 rs2               10       C.SQSP (RV128)
							110            uimm[5:2|7:6]                 rs2               10       C.SWSP
							111            uimm[5:2|7:6]                 rs2               10       C.FSWSP (RV32)
							111            uimm[5:3|8:6]                 rs2               10       C.SDSP (RV64/128)
					\*/
					if ((m_ir & 0b11) == 0) {
						m_ir &= 0xffff;
						switch (m_ir & 0xe000) {
							case 0x0000:
								decode_ciw_type(rd, imm);
								if (imm == 0) { // ILLEGAL
									asm_out("illegal instruction", rd);
								} else { // C.ADDI4SPN
									c_asm_out("c.addi4spn	x%d,x2,%d", rd, imm);
									r_gpr[rd] = r_gpr[2] + imm;
								}
								break;
							case 0x2000: // C.FLD, double not supported yet
								fprintf(stderr, "Unknown rv32 instruction line %d:  0x%04x\n", __LINE__, m_ir);
								break;
							case 0x4000: // C.LW
								decode_cl_type(rd, rs1, imm);
								c_asm_out("c.lw	x%d,%d(x%d)", rd, imm, rs1);
								load(&r_gpr[rd], DATA_READ, true, r_gpr[rs1] + imm, 4);
								break;
							case 0x6000: // C.FLW
								decode_cl_type(rd, rs1, imm);
								c_asm_out("c.flw	f%d,%d(x%d)", rd, imm, rs1);
								load((uint32_t *)&r_fpr[rd], DATA_READ, true, r_gpr[rs1] + imm, 4);
								break;
							case 0xa000: // C.FSD, double not supported yet
								fprintf(stderr, "Unknown rv32 instruction line %d:  0x%04x\n", __LINE__, m_ir);
								break;
							case 0xc000: // C.SW
								decode_cs_type(rs2, rs1, imm);
								c_asm_out("c.sw	x%d,%d(x%d)", rs2, imm, rs1);
								store(DATA_WRITE, r_gpr[rs1] + imm, r_gpr[rs2], 4);
								break;
							case 0xe000: // C.FSW
								decode_cs_type(rs2, rs1, imm);
								c_asm_out("c.fsw	f%d,%d(x%d)", rs2, imm, rs1);
								suf.f   = r_fpr[rs2];
								store(DATA_WRITE, r_gpr[rs1] + imm, suf.u, 4);
								break;
							default:
								fprintf(stderr, "Unknown compressed rv32 instruction line %d:  0x%04x\n", __LINE__, m_ir);
						}
						next_pc = r_pc + 2;
					} else if ((m_ir & 0b11) == 1) {
						m_ir &= 0xffff;
						switch (m_ir & 0xe000) {
							case 0x0000:
								if ((m_ir & 0x1ffc) == 0x0)
									asm_out("nop	", rd);
								else {
									decode_ci_type(rd, imm);
									c_asm_out("c.addi	x%d,%d", rd, imm);
									r_gpr[rd] += imm;
								}
								break;
							case 0x2000:
								decode_cj_type(imm);
								c_asm_out("c.jal	%08x", r_pc + imm);
								r_gpr[1] = r_pc + 2;
								next_pc  = r_pc + imm;
								// Do not update pc as the majority of other insns do
								goto skip_next_pc;
							case 0x4000:
								decode_ci_type(rd, imm);
								c_asm_out("c.li	x%d,%d", rd, imm);
								r_gpr[rd] = imm;
								break;
							case 0x6000:
								if (((m_ir >> 7) & 0x1f) == 2) { // rd == 2
									decode_cisp_type(rd, imm);
									c_asm_out("c.addi16sp	x%d,%d", rd, imm);
									r_gpr[rd] += imm;
								} else {
									decode_ci_type(rd, imm);
									c_asm_out("c.lui	x%d,0x%x", rd, imm &= 0x000fffff);
									r_gpr[rd] = imm << 12;
								}
								break;
							case 0x8000:
								switch (m_ir & 0x0c00) {
									case 0x0000:
										decode_cis_type(rd, imm);
										c_asm_out("c.srli	x%d,0x%x", rd, imm);
										/* FIXME: check if masking the immediat is required */
										r_gpr[rd] >>= (imm & 0x1f);
										break;
									case 0x0400:
										decode_cis_type(rd, imm);
										c_asm_out("c.srai	x%d,0x%x", rd, imm);
										r_gpr[rd] = (int32_t)r_gpr[rd] >> (imm & 0x1f);
										break;
									case 0x0800:
										decode_cis_type(rd, imm);
										c_asm_out("c.andi	x%d,%d", rd, imm);
										r_gpr[rd] &= imm;
										break;
									case 0x0c00:
										decode_cs_type(rs2, rd, imm); /* imm unused as such */
										switch (m_ir & 0x60) {
											case 0x00:
													c_asm_out("c.sub	x%d,x%d", rd, rs2);
													r_gpr[rd] -= r_gpr[rs2];
												break;
											case 0x20:
													c_asm_out("c.xor	x%d,x%d", rd, rs2);
													r_gpr[rd] ^= r_gpr[rs2];
												break;
											case 0x40:
													c_asm_out("c.or	x%d,x%d", rd, rs2);
													r_gpr[rd] |= r_gpr[rs2];
												break;
											case 0x60:
													c_asm_out("c.and	x%d,x%d", rd, rs2);
													r_gpr[rd] &= r_gpr[rs2];
												break;
													fprintf(stderr, "Unknown compressed rv32 instruction line %d:  0x%04x\n", __LINE__, m_ir);
										}
										break;
									default:
										fprintf(stderr, "Unknown compressed rv32 instruction line %d:  0x%04x\n", __LINE__, m_ir);
								}
								break;
							case 0xa000:
								decode_cj_type(imm);
								c_asm_out("c.j	%x", r_pc + imm);
								next_pc  = r_pc + imm;
								goto skip_next_pc;
								break;
							case 0xc000:
								decode_cb_type(rs1, imm);
								c_asm_out("c.beqz	x%d,%x", rs1, r_pc + imm);
								next_pc = r_pc + (r_gpr[rs1] == 0 ? imm : 2);
								goto skip_next_pc;
								break;
							case 0xe000:
								decode_cb_type(rs1, imm);
								c_asm_out("c.bnez	x%d,%x", rs1, r_pc + imm);
								next_pc = r_pc + (r_gpr[rs1] == 0 ? 2 : imm);
								goto skip_next_pc;
								break;
							default:
								fprintf(stderr, "Unknown compressed rv32 instruction line %d:  0x%08x\n", __LINE__, m_ir);
						}
						next_pc = r_pc + 2;
					} else if ((m_ir & 0b11) == 2) {
						m_ir &= 0xffff;
						switch (m_ir & 0xe000) {
							case 0x0000:
								decode_ci_type(rd, imm);
								c_asm_out("c.slli	x%d,0x%x", rd, imm);
								r_gpr[rd] <<= (imm & 0x1f);
								break;
							case 0x4000:
								decode_cils_type(rd, imm);
								c_asm_out("c.lwsp	x%d,%d(x2)", rd, imm);
								load(&r_gpr[rd], DATA_READ, true, r_gpr[2] + imm, 4);
								break;
							case 0x6000:
								decode_cils_type(rd, imm);
								c_asm_out("c.flwsp	x%d,%d(x2)", rd, imm);
								load((uint32_t *)&r_fpr[rd], DATA_READ, true, r_gpr[2] + imm, 4);
								break;
							case 0x8000:
								decode_cr_type(rs1, rs2);
								if ((m_ir & 0x1000) == 0) {
									if (rs1 == 0)
										fprintf(stderr, "c.jr with rs1 = 0\n");
									else if (rs2 != 0) {
										c_asm_out("c.mv	x%d,x%d", rs1, rs2);
										r_gpr[rs1] = r_gpr[rs2];
									} else {
										c_asm_out("c.jr	x%d", rs1);
										next_pc = r_gpr[rs1];
										goto skip_next_pc;
									}
								} else {
									if (m_ir == 0x9002) {
										c_asm_out("%s", "c.ebreak");
										r_csr[csr_mcause] = BREAKPOINT_TRAP;
										exception = true;
										goto __iss_handle_exception;
									} else if (rs1 == 0)
										fprintf(stderr, "c.jalr with rs1 = 0\n");
									else if (rs2 != 0) {
										c_asm_out("c.add	x%d,x%d", rs1, rs2);
										r_gpr[rs1] += r_gpr[rs2];
									} else {
										c_asm_out("c.jalr	x%d", rs1);
										r_gpr[1] = r_pc + 2;
										next_pc = r_gpr[rs1];
										goto skip_next_pc;
									}
								}
								break;
							case 0xa000:
								decode_css_type(rs2, imm);
								c_asm_out("c.fswsp	f%d,%d(x2)", rs2, imm);
								suf.f   = r_fpr[rs2];
								store(DATA_WRITE, r_gpr[2] + imm, suf.u, 4);
								next_pc = r_pc + 2;
								break;
							case 0xc000:
								decode_css_type(rs2, imm);
								c_asm_out("c.swsp	x%d,%d(x2)", rs2, imm);
								store(DATA_WRITE, r_gpr[2] + imm, r_gpr[rs2], 4);
								break;
							default:
								fprintf(stderr, "Unknown rv32 instruction line %d:  0x%08x\n", __LINE__, m_ir);
						}
						next_pc = r_pc + 2;
					} else
						fprintf(stderr, "Unknown rv32 instruction line %d:  0x%08x\n", __LINE__, m_ir);
					/* end of the case default, 276 lines later */
			}
skip_next_pc:
			/*\
			 * Ensures that we get out of here with a zeroed r0
			\*/
			r_gpr[0] = 0;
			/*\
			 * Update pc
			\*/
			r_pc  = next_pc;
		}
	} // end Rv32Iss::step

	/**
	 * Support for gdb
	 */

	// Necessary to avoid C++11 flaw, not the nicest piece of code
	// written by humanity
	constexpr const char *Rv32Iss::debugFeatures[4][2];

	const char *Rv32Iss::debugXmlRegistersDescription(const char *pool)
	{
		if (!pool)
			return "Yop!";

		for (size_t i = 0; i < sizeof debugFeatures / sizeof debugFeatures[0]; i++) {
			const char *s = debugFeatures[i][0];
			if (!s || strcmp(s, pool) == 0)
				return debugFeatures[i][1];
		}

		return  NULL;
	}

	/*\
	 * Somehow the numbering of the registers seems to depend upon the order in which
	 * they have been declared in the xml register description.
	 * I used the messages send when doing an ‘info all-registers’ to reverse
	 * engineer this order, but some csr seems not supported anymore in newer versions
	 * of the standard.
	\*/
	Iss2::debug_register_t Rv32Iss::debugGetRegisterValue(unsigned int reg) const
	{
		suf_t suf;
		switch (reg) {
			case 0 ... 31:
				return r_gpr[reg];
			case 32:
				return r_pc;
			case 33 ... 64:
				suf.f = r_fpr[reg - 33];
				return suf.u;
			/* The whole bunch of csr, thanks awk.
			 * Nothing fancy for now, just spit the value */
			case 68 : // ustatus
				return r_csr[csr_ustatus];
			case 69 : // uie
				return r_csr[csr_uie];
			case 70 : // utvec
				return r_csr[csr_utvec];
			case 71 : // uscratch
				return r_csr[csr_uscratch];
			case 72 : // uepc
				return r_csr[csr_uepc];
			case 73 : // ucause
				return r_csr[csr_ucause];
			case 74 : // utval
				return r_csr[csr_utval];
			case 75 : // uip
				return r_csr[csr_uip];
			case 76 : // fflags
				return r_csr[csr_fflags];
			case 77 : // frm
				return r_csr[csr_frm];
			case 78 : // fcsr
				return r_csr[csr_fcsr];
			case 79 : // cycle
				return r_csr[csr_cycle];
			case 80 : // time
				return r_csr[csr_time];
			case 81 : // instret
				return r_csr[csr_instret];
			case 82 : // hpmcounter3
				return r_csr[csr_hpmcounter3];
			case 83 : // hpmcounter4
				return r_csr[csr_hpmcounter4];
			case 84 : // hpmcounter5
				return r_csr[csr_hpmcounter5];
			case 85 : // hpmcounter6
				return r_csr[csr_hpmcounter6];
			case 86 : // hpmcounter7
				return r_csr[csr_hpmcounter7];
			case 87 : // hpmcounter8
				return r_csr[csr_hpmcounter8];
			case 88 : // hpmcounter9
				return r_csr[csr_hpmcounter9];
			case 89 : // hpmcounter10
				return r_csr[csr_hpmcounter10];
			case 90 : // hpmcounter11
				return r_csr[csr_hpmcounter11];
			case 91 : // hpmcounter12
				return r_csr[csr_hpmcounter12];
			case 92 : // hpmcounter13
				return r_csr[csr_hpmcounter13];
			case 93 : // hpmcounter14
				return r_csr[csr_hpmcounter14];
			case 94 : // hpmcounter15
				return r_csr[csr_hpmcounter15];
			case 95 : // hpmcounter16
				return r_csr[csr_hpmcounter16];
			case 96 : // hpmcounter17
				return r_csr[csr_hpmcounter17];
			case 97 : // hpmcounter18
				return r_csr[csr_hpmcounter18];
			case 98 : // hpmcounter19
				return r_csr[csr_hpmcounter19];
			case 99 : // hpmcounter20
				return r_csr[csr_hpmcounter20];
			case 100 : // hpmcounter21
				return r_csr[csr_hpmcounter21];
			case 101 : // hpmcounter22
				return r_csr[csr_hpmcounter22];
			case 102 : // hpmcounter23
				return r_csr[csr_hpmcounter23];
			case 103 : // hpmcounter24
				return r_csr[csr_hpmcounter24];
			case 104 : // hpmcounter25
				return r_csr[csr_hpmcounter25];
			case 105 : // hpmcounter26
				return r_csr[csr_hpmcounter26];
			case 106 : // hpmcounter27
				return r_csr[csr_hpmcounter27];
			case 107 : // hpmcounter28
				return r_csr[csr_hpmcounter28];
			case 108 : // hpmcounter29
				return r_csr[csr_hpmcounter29];
			case 109 : // hpmcounter30
				return r_csr[csr_hpmcounter30];
			case 110 : // hpmcounter31
				return r_csr[csr_hpmcounter31];
			case 111 : // cycleh
				return r_csr[csr_cycleh];
			case 112 : // timeh
				return r_csr[csr_timeh];
			case 113 : // instreth
				return r_csr[csr_instreth];
			case 114 : // hpmcounter3h
				return r_csr[csr_hpmcounter3h];
			case 115 : // hpmcounter4h
				return r_csr[csr_hpmcounter4h];
			case 116 : // hpmcounter5h
				return r_csr[csr_hpmcounter5h];
			case 117 : // hpmcounter6h
				return r_csr[csr_hpmcounter6h];
			case 118 : // hpmcounter7h
				return r_csr[csr_hpmcounter7h];
			case 119 : // hpmcounter8h
				return r_csr[csr_hpmcounter8h];
			case 120 : // hpmcounter9h
				return r_csr[csr_hpmcounter9h];
			case 121 : // hpmcounter10h
				return r_csr[csr_hpmcounter10h];
			case 122 : // hpmcounter11h
				return r_csr[csr_hpmcounter11h];
			case 123 : // hpmcounter12h
				return r_csr[csr_hpmcounter12h];
			case 124 : // hpmcounter13h
				return r_csr[csr_hpmcounter13h];
			case 125 : // hpmcounter14h
				return r_csr[csr_hpmcounter14h];
			case 126 : // hpmcounter15h
				return r_csr[csr_hpmcounter15h];
			case 127 : // hpmcounter16h
				return r_csr[csr_hpmcounter16h];
			case 128 : // hpmcounter17h
				return r_csr[csr_hpmcounter17h];
			case 129 : // hpmcounter18h
				return r_csr[csr_hpmcounter18h];
			case 130 : // hpmcounter19h
				return r_csr[csr_hpmcounter19h];
			case 131 : // hpmcounter20h
				return r_csr[csr_hpmcounter20h];
			case 132 : // hpmcounter21h
				return r_csr[csr_hpmcounter21h];
			case 133 : // hpmcounter22h
				return r_csr[csr_hpmcounter22h];
			case 134 : // hpmcounter23h
				return r_csr[csr_hpmcounter23h];
			case 135 : // hpmcounter24h
				return r_csr[csr_hpmcounter24h];
			case 136 : // hpmcounter25h
				return r_csr[csr_hpmcounter25h];
			case 137 : // hpmcounter26h
				return r_csr[csr_hpmcounter26h];
			case 138 : // hpmcounter27h
				return r_csr[csr_hpmcounter27h];
			case 139 : // hpmcounter28h
				return r_csr[csr_hpmcounter28h];
			case 140 : // hpmcounter29h
				return r_csr[csr_hpmcounter29h];
			case 141 : // hpmcounter30h
				return r_csr[csr_hpmcounter30h];
			case 142 : // hpmcounter31h
				return r_csr[csr_hpmcounter31h];
			case 143 : // sstatus
				return r_csr[csr_sstatus];
			case 144 : // sedeleg
				return r_csr[csr_sedeleg];
			case 145 : // sideleg
				return r_csr[csr_sideleg];
			case 146 : // sie
				return r_csr[csr_sie];
			case 147 : // stvec
				return r_csr[csr_stvec];
			case 148 : // scounteren
				return r_csr[csr_scounteren];
			case 149 : // sscratch
				return r_csr[csr_sscratch];
			case 150 : // sepc
				return r_csr[csr_sepc];
			case 151 : // scause
				return r_csr[csr_scause];
			case 152 : // stval
				return r_csr[csr_stval];
			case 153 : // sip
				return r_csr[csr_sip];
			case 154 : // satp
				return r_csr[csr_satp];
			case 155 : // mvendorid
				return r_csr[csr_mvendorid];
			case 156 : // marchid
				return r_csr[csr_marchid];
			case 157 : // mimpid
				return r_csr[csr_mimpid];
			case 158 : // mhartid
				return r_csr[csr_mhartid];
			case 159 : // mstatus
				return r_csr[csr_mstatus];
			case 160 : // misa
				return r_csr[csr_misa];
			case 161 : // medeleg
				return r_csr[csr_medeleg];
			case 162 : // mideleg
				return r_csr[csr_mideleg];
			case 163 : // mie
				return r_csr[csr_mie];
			case 164 : // mtvec
				return r_csr[csr_mtvec];
			case 165 : // mcounteren
				return r_csr[csr_mcounteren];
			case 166 : // mscratch
				return r_csr[csr_mscratch];
			case 167 : // mepc
				return r_csr[csr_mepc];
			case 168 : // mcause
				return r_csr[csr_mcause];
			case 169 : // mtval
				return r_csr[csr_mtval];
			case 170 : // mip
				return r_csr[csr_mip];
			case 171 : // pmpcfg0
				return r_csr[csr_pmpcfg0];
			case 172 : // pmpcfg1
				return r_csr[csr_pmpcfg1];
			case 173 : // pmpcfg2
				return r_csr[csr_pmpcfg2];
			case 174 : // pmpcfg3
				return r_csr[csr_pmpcfg3];
			case 175 : // pmpaddr0
				return r_csr[csr_pmpaddr0];
			case 176 : // pmpaddr1
				return r_csr[csr_pmpaddr1];
			case 177 : // pmpaddr2
				return r_csr[csr_pmpaddr2];
			case 178 : // pmpaddr3
				return r_csr[csr_pmpaddr3];
			case 179 : // pmpaddr4
				return r_csr[csr_pmpaddr4];
			case 180 : // pmpaddr5
				return r_csr[csr_pmpaddr5];
			case 181 : // pmpaddr6
				return r_csr[csr_pmpaddr6];
			case 182 : // pmpaddr7
				return r_csr[csr_pmpaddr7];
			case 183 : // pmpaddr8
				return r_csr[csr_pmpaddr8];
			case 184 : // pmpaddr9
				return r_csr[csr_pmpaddr9];
			case 185 : // pmpaddr10
				return r_csr[csr_pmpaddr10];
			case 186 : // pmpaddr11
				return r_csr[csr_pmpaddr11];
			case 187 : // pmpaddr12
				return r_csr[csr_pmpaddr12];
			case 188 : // pmpaddr13
				return r_csr[csr_pmpaddr13];
			case 189 : // pmpaddr14
				return r_csr[csr_pmpaddr14];
			case 190 : // pmpaddr15
				return r_csr[csr_pmpaddr15];
			case 191 : // mcycle
				return r_csr[csr_mcycle];
			case 192 : // minstret
				return r_csr[csr_minstret];
			case 193 : // mhpmcounter3
				return r_csr[csr_mhpmcounter3];
			case 194 : // mhpmcounter4
				return r_csr[csr_mhpmcounter4];
			case 195 : // mhpmcounter5
				return r_csr[csr_mhpmcounter5];
			case 196 : // mhpmcounter6
				return r_csr[csr_mhpmcounter6];
			case 197 : // mhpmcounter7
				return r_csr[csr_mhpmcounter7];
			case 198 : // mhpmcounter8
				return r_csr[csr_mhpmcounter8];
			case 199 : // mhpmcounter9
				return r_csr[csr_mhpmcounter9];
			case 200 : // mhpmcounter10
				return r_csr[csr_mhpmcounter10];
			case 201 : // mhpmcounter11
				return r_csr[csr_mhpmcounter11];
			case 202 : // mhpmcounter12
				return r_csr[csr_mhpmcounter12];
			case 203 : // mhpmcounter13
				return r_csr[csr_mhpmcounter13];
			case 204 : // mhpmcounter14
				return r_csr[csr_mhpmcounter14];
			case 205 : // mhpmcounter15
				return r_csr[csr_mhpmcounter15];
			case 206 : // mhpmcounter16
				return r_csr[csr_mhpmcounter16];
			case 207 : // mhpmcounter17
				return r_csr[csr_mhpmcounter17];
			case 208 : // mhpmcounter18
				return r_csr[csr_mhpmcounter18];
			case 209 : // mhpmcounter19
				return r_csr[csr_mhpmcounter19];
			case 210 : // mhpmcounter20
				return r_csr[csr_mhpmcounter20];
			case 211 : // mhpmcounter21
				return r_csr[csr_mhpmcounter21];
			case 212 : // mhpmcounter22
				return r_csr[csr_mhpmcounter22];
			case 213 : // mhpmcounter23
				return r_csr[csr_mhpmcounter23];
			case 214 : // mhpmcounter24
				return r_csr[csr_mhpmcounter24];
			case 215 : // mhpmcounter25
				return r_csr[csr_mhpmcounter25];
			case 216 : // mhpmcounter26
				return r_csr[csr_mhpmcounter26];
			case 217 : // mhpmcounter27
				return r_csr[csr_mhpmcounter27];
			case 218 : // mhpmcounter28
				return r_csr[csr_mhpmcounter28];
			case 219 : // mhpmcounter29
				return r_csr[csr_mhpmcounter29];
			case 220 : // mhpmcounter30
				return r_csr[csr_mhpmcounter30];
			case 221 : // mhpmcounter31
				return r_csr[csr_mhpmcounter31];
			case 222 : // mcycleh
				return r_csr[csr_mcycleh];
			case 223 : // minstreth
				return r_csr[csr_minstreth];
			case 224 : // mhpmcounter3h
				return r_csr[csr_mhpmcounter3h];
			case 225 : // mhpmcounter4h
				return r_csr[csr_mhpmcounter4h];
			case 226 : // mhpmcounter5h
				return r_csr[csr_mhpmcounter5h];
			case 227 : // mhpmcounter6h
				return r_csr[csr_mhpmcounter6h];
			case 228 : // mhpmcounter7h
				return r_csr[csr_mhpmcounter7h];
			case 229 : // mhpmcounter8h
				return r_csr[csr_mhpmcounter8h];
			case 230 : // mhpmcounter9h
				return r_csr[csr_mhpmcounter9h];
			case 231 : // mhpmcounter10h
				return r_csr[csr_mhpmcounter10h];
			case 232 : // mhpmcounter11h
				return r_csr[csr_mhpmcounter11h];
			case 233 : // mhpmcounter12h
				return r_csr[csr_mhpmcounter12h];
			case 234 : // mhpmcounter13h
				return r_csr[csr_mhpmcounter13h];
			case 235 : // mhpmcounter14h
				return r_csr[csr_mhpmcounter14h];
			case 236 : // mhpmcounter15h
				return r_csr[csr_mhpmcounter15h];
			case 237 : // mhpmcounter16h
				return r_csr[csr_mhpmcounter16h];
			case 238 : // mhpmcounter17h
				return r_csr[csr_mhpmcounter17h];
			case 239 : // mhpmcounter18h
				return r_csr[csr_mhpmcounter18h];
			case 240 : // mhpmcounter19h
				return r_csr[csr_mhpmcounter19h];
			case 241 : // mhpmcounter20h
				return r_csr[csr_mhpmcounter20h];
			case 242 : // mhpmcounter21h
				return r_csr[csr_mhpmcounter21h];
			case 243 : // mhpmcounter22h
				return r_csr[csr_mhpmcounter22h];
			case 244 : // mhpmcounter23h
				return r_csr[csr_mhpmcounter23h];
			case 245 : // mhpmcounter24h
				return r_csr[csr_mhpmcounter24h];
			case 246 : // mhpmcounter25h
				return r_csr[csr_mhpmcounter25h];
			case 247 : // mhpmcounter26h
				return r_csr[csr_mhpmcounter26h];
			case 248 : // mhpmcounter27h
				return r_csr[csr_mhpmcounter27h];
			case 249 : // mhpmcounter28h
				return r_csr[csr_mhpmcounter28h];
			case 250 : // mhpmcounter29h
				return r_csr[csr_mhpmcounter29h];
			case 251 : // mhpmcounter30h
				return r_csr[csr_mhpmcounter30h];
			case 252 : // mhpmcounter31h
				return r_csr[csr_mhpmcounter31h];
			case 253 : // mhpmevent3
				return r_csr[csr_mhpmevent3];
			case 254 : // mhpmevent4
				return r_csr[csr_mhpmevent4];
			case 255 : // mhpmevent5
				return r_csr[csr_mhpmevent5];
			case 256 : // mhpmevent6
				return r_csr[csr_mhpmevent6];
			case 257 : // mhpmevent7
				return r_csr[csr_mhpmevent7];
			case 258 : // mhpmevent8
				return r_csr[csr_mhpmevent8];
			case 259 : // mhpmevent9
				return r_csr[csr_mhpmevent9];
			case 260 : // mhpmevent10
				return r_csr[csr_mhpmevent10];
			case 261 : // mhpmevent11
				return r_csr[csr_mhpmevent11];
			case 262 : // mhpmevent12
				return r_csr[csr_mhpmevent12];
			case 263 : // mhpmevent13
				return r_csr[csr_mhpmevent13];
			case 264 : // mhpmevent14
				return r_csr[csr_mhpmevent14];
			case 265 : // mhpmevent15
				return r_csr[csr_mhpmevent15];
			case 266 : // mhpmevent16
				return r_csr[csr_mhpmevent16];
			case 267 : // mhpmevent17
				return r_csr[csr_mhpmevent17];
			case 268 : // mhpmevent18
				return r_csr[csr_mhpmevent18];
			case 269 : // mhpmevent19
				return r_csr[csr_mhpmevent19];
			case 270 : // mhpmevent20
				return r_csr[csr_mhpmevent20];
			case 271 : // mhpmevent21
				return r_csr[csr_mhpmevent21];
			case 272 : // mhpmevent22
				return r_csr[csr_mhpmevent22];
			case 273 : // mhpmevent23
				return r_csr[csr_mhpmevent23];
			case 274 : // mhpmevent24
				return r_csr[csr_mhpmevent24];
			case 275 : // mhpmevent25
				return r_csr[csr_mhpmevent25];
			case 276 : // mhpmevent26
				return r_csr[csr_mhpmevent26];
			case 277 : // mhpmevent27
				return r_csr[csr_mhpmevent27];
			case 278 : // mhpmevent28
				return r_csr[csr_mhpmevent28];
			case 279 : // mhpmevent29
				return r_csr[csr_mhpmevent29];
			case 280 : // mhpmevent30
				return r_csr[csr_mhpmevent30];
			case 281 : // mhpmevent31
				return r_csr[csr_mhpmevent31];
			case 282 : // tselect
				return r_csr[csr_tselect];
			case 283 : // tdata1
				return r_csr[csr_tdata1];
			case 284 : // tdata2
				return r_csr[csr_tdata2];
			case 285 : // tdata3
				return r_csr[csr_tdata3];
			case 286 : // dcsr
				return r_csr[csr_dcsr];
			case 287 : // dpc
				return r_csr[csr_dpc];
			case 288 : // dscratch
				return r_csr[csr_dscratch];
			case 289 : // hstatus
			case 290 : // hedeleg
			case 291 : // hideleg
			case 292 : // hie
			case 293 : // htvec
			case 294 : // hscratch
			case 295 : // hepc
			case 296 : // hcause
			case 297 : // hbadaddr
			case 298 : // hip
			case 299 : // mbase
			case 300 : // mbound
			case 301 : // mibase
			case 302 : // mibound
			case 303 : // mdbase
			case 304 : // mdbound
			case 305 : // mucounteren
			case 306 : // mscounteren
			case 307 : // mhcounteren
			default:
				return 0xbad00bad;
		}
	}

	void Rv32Iss::debugSetRegisterValue(unsigned int reg, uint32_t value)
	{
		suf_t suf;
		switch (reg) {
			case 1 ... 31:
				r_gpr[reg] = value;
				break;
			case 32:
				r_pc = value;
				break;
			case 33 ... 64:
				suf.u = value;
				r_fpr[reg - 33] = suf.f;
				break;
			/* The whole bunch of csr, just set them */
			case 68 : // ustatus
				r_csr[csr_ustatus] = value;
				break;
			case 69 : // uie
				r_csr[csr_uie] = value;
				break;
			case 70 : // utvec
				r_csr[csr_utvec] = value;
				break;
			case 71 : // uscratch
				r_csr[csr_uscratch] = value;
				break;
			case 72 : // uepc
				r_csr[csr_uepc] = value;
				break;
			case 73 : // ucause
				r_csr[csr_ucause] = value;
				break;
			case 74 : // utval
				r_csr[csr_utval] = value;
				break;
			case 75 : // uip
				r_csr[csr_uip] = value;
				break;
			case 76 : // fflags
				r_csr[csr_fflags] = value;
				break;
			case 77 : // frm
				r_csr[csr_frm] = value;
				break;
			case 78 : // fcsr
				r_csr[csr_fcsr] = value;
				break;
			case 79 : // cycle
				r_csr[csr_cycle] = value;
				break;
			case 80 : // time
				r_csr[csr_time] = value;
				break;
			case 81 : // instret
				r_csr[csr_instret] = value;
				break;
			case 82 : // hpmcounter3
				r_csr[csr_hpmcounter3] = value;
				break;
			case 83 : // hpmcounter4
				r_csr[csr_hpmcounter4] = value;
				break;
			case 84 : // hpmcounter5
				r_csr[csr_hpmcounter5] = value;
				break;
			case 85 : // hpmcounter6
				r_csr[csr_hpmcounter6] = value;
				break;
			case 86 : // hpmcounter7
				r_csr[csr_hpmcounter7] = value;
				break;
			case 87 : // hpmcounter8
				r_csr[csr_hpmcounter8] = value;
				break;
			case 88 : // hpmcounter9
				r_csr[csr_hpmcounter9] = value;
				break;
			case 89 : // hpmcounter10
				r_csr[csr_hpmcounter10] = value;
				break;
			case 90 : // hpmcounter11
				r_csr[csr_hpmcounter11] = value;
				break;
			case 91 : // hpmcounter12
				r_csr[csr_hpmcounter12] = value;
				break;
			case 92 : // hpmcounter13
				r_csr[csr_hpmcounter13] = value;
				break;
			case 93 : // hpmcounter14
				r_csr[csr_hpmcounter14] = value;
				break;
			case 94 : // hpmcounter15
				r_csr[csr_hpmcounter15] = value;
				break;
			case 95 : // hpmcounter16
				r_csr[csr_hpmcounter16] = value;
				break;
			case 96 : // hpmcounter17
				r_csr[csr_hpmcounter17] = value;
				break;
			case 97 : // hpmcounter18
				r_csr[csr_hpmcounter18] = value;
				break;
			case 98 : // hpmcounter19
				r_csr[csr_hpmcounter19] = value;
				break;
			case 99 : // hpmcounter20
				r_csr[csr_hpmcounter20] = value;
				break;
			case 100 : // hpmcounter21
				r_csr[csr_hpmcounter21] = value;
				break;
			case 101 : // hpmcounter22
				r_csr[csr_hpmcounter22] = value;
				break;
			case 102 : // hpmcounter23
				r_csr[csr_hpmcounter23] = value;
				break;
			case 103 : // hpmcounter24
				r_csr[csr_hpmcounter24] = value;
				break;
			case 104 : // hpmcounter25
				r_csr[csr_hpmcounter25] = value;
				break;
			case 105 : // hpmcounter26
				r_csr[csr_hpmcounter26] = value;
				break;
			case 106 : // hpmcounter27
				r_csr[csr_hpmcounter27] = value;
				break;
			case 107 : // hpmcounter28
				r_csr[csr_hpmcounter28] = value;
				break;
			case 108 : // hpmcounter29
				r_csr[csr_hpmcounter29] = value;
				break;
			case 109 : // hpmcounter30
				r_csr[csr_hpmcounter30] = value;
				break;
			case 110 : // hpmcounter31
				r_csr[csr_hpmcounter31] = value;
				break;
			case 111 : // cycleh
				r_csr[csr_cycleh] = value;
				break;
			case 112 : // timeh
				r_csr[csr_timeh] = value;
				break;
			case 113 : // instreth
				r_csr[csr_instreth] = value;
				break;
			case 114 : // hpmcounter3h
				r_csr[csr_hpmcounter3h] = value;
				break;
			case 115 : // hpmcounter4h
				r_csr[csr_hpmcounter4h] = value;
				break;
			case 116 : // hpmcounter5h
				r_csr[csr_hpmcounter5h] = value;
				break;
			case 117 : // hpmcounter6h
				r_csr[csr_hpmcounter6h] = value;
				break;
			case 118 : // hpmcounter7h
				r_csr[csr_hpmcounter7h] = value;
				break;
			case 119 : // hpmcounter8h
				r_csr[csr_hpmcounter8h] = value;
				break;
			case 120 : // hpmcounter9h
				r_csr[csr_hpmcounter9h] = value;
				break;
			case 121 : // hpmcounter10h
				r_csr[csr_hpmcounter10h] = value;
				break;
			case 122 : // hpmcounter11h
				r_csr[csr_hpmcounter11h] = value;
				break;
			case 123 : // hpmcounter12h
				r_csr[csr_hpmcounter12h] = value;
				break;
			case 124 : // hpmcounter13h
				r_csr[csr_hpmcounter13h] = value;
				break;
			case 125 : // hpmcounter14h
				r_csr[csr_hpmcounter14h] = value;
				break;
			case 126 : // hpmcounter15h
				r_csr[csr_hpmcounter15h] = value;
				break;
			case 127 : // hpmcounter16h
				r_csr[csr_hpmcounter16h] = value;
				break;
			case 128 : // hpmcounter17h
				r_csr[csr_hpmcounter17h] = value;
				break;
			case 129 : // hpmcounter18h
				r_csr[csr_hpmcounter18h] = value;
				break;
			case 130 : // hpmcounter19h
				r_csr[csr_hpmcounter19h] = value;
				break;
			case 131 : // hpmcounter20h
				r_csr[csr_hpmcounter20h] = value;
				break;
			case 132 : // hpmcounter21h
				r_csr[csr_hpmcounter21h] = value;
				break;
			case 133 : // hpmcounter22h
				r_csr[csr_hpmcounter22h] = value;
				break;
			case 134 : // hpmcounter23h
				r_csr[csr_hpmcounter23h] = value;
				break;
			case 135 : // hpmcounter24h
				r_csr[csr_hpmcounter24h] = value;
				break;
			case 136 : // hpmcounter25h
				r_csr[csr_hpmcounter25h] = value;
				break;
			case 137 : // hpmcounter26h
				r_csr[csr_hpmcounter26h] = value;
				break;
			case 138 : // hpmcounter27h
				r_csr[csr_hpmcounter27h] = value;
				break;
			case 139 : // hpmcounter28h
				r_csr[csr_hpmcounter28h] = value;
				break;
			case 140 : // hpmcounter29h
				r_csr[csr_hpmcounter29h] = value;
				break;
			case 141 : // hpmcounter30h
				r_csr[csr_hpmcounter30h] = value;
				break;
			case 142 : // hpmcounter31h
				r_csr[csr_hpmcounter31h] = value;
				break;
			case 143 : // sstatus
				r_csr[csr_sstatus] = value;
				break;
			case 144 : // sedeleg
				r_csr[csr_sedeleg] = value;
				break;
			case 145 : // sideleg
				r_csr[csr_sideleg] = value;
				break;
			case 146 : // sie
				r_csr[csr_sie] = value;
				break;
			case 147 : // stvec
				r_csr[csr_stvec] = value;
				break;
			case 148 : // scounteren
				r_csr[csr_scounteren] = value;
				break;
			case 149 : // sscratch
				r_csr[csr_sscratch] = value;
				break;
			case 150 : // sepc
				r_csr[csr_sepc] = value;
				break;
			case 151 : // scause
				r_csr[csr_scause] = value;
				break;
			case 152 : // stval
				r_csr[csr_stval] = value;
				break;
			case 153 : // sip
				r_csr[csr_sip] = value;
				break;
			case 154 : // satp
				r_csr[csr_satp] = value;
				break;
			case 155 : // mvendorid
				r_csr[csr_mvendorid] = value;
				break;
			case 156 : // marchid
				r_csr[csr_marchid] = value;
				break;
			case 157 : // mimpid
				r_csr[csr_mimpid] = value;
				break;
			case 158 : // mhartid
				r_csr[csr_mhartid] = value;
				break;
			case 159 : // mstatus
				r_csr[csr_mstatus] = value;
				break;
			case 160 : // misa
				r_csr[csr_misa] = value;
				break;
			case 161 : // medeleg
				r_csr[csr_medeleg] = value;
				break;
			case 162 : // mideleg
				r_csr[csr_mideleg] = value;
				break;
			case 163 : // mie
				r_csr[csr_mie] = value;
				break;
			case 164 : // mtvec
				r_csr[csr_mtvec] = value;
				break;
			case 165 : // mcounteren
				r_csr[csr_mcounteren] = value;
				break;
			case 166 : // mscratch
				r_csr[csr_mscratch] = value;
				break;
			case 167 : // mepc
				r_csr[csr_mepc] = value;
				break;
			case 168 : // mcause
				r_csr[csr_mcause] = value;
				break;
			case 169 : // mtval
				r_csr[csr_mtval] = value;
				break;
			case 170 : // mip
				r_csr[csr_mip] = value;
				break;
			case 171 : // pmpcfg0
				r_csr[csr_pmpcfg0] = value;
				break;
			case 172 : // pmpcfg1
				r_csr[csr_pmpcfg1] = value;
				break;
			case 173 : // pmpcfg2
				r_csr[csr_pmpcfg2] = value;
				break;
			case 174 : // pmpcfg3
				r_csr[csr_pmpcfg3] = value;
				break;
			case 175 : // pmpaddr0
				r_csr[csr_pmpaddr0] = value;
				break;
			case 176 : // pmpaddr1
				r_csr[csr_pmpaddr1] = value;
				break;
			case 177 : // pmpaddr2
				r_csr[csr_pmpaddr2] = value;
				break;
			case 178 : // pmpaddr3
				r_csr[csr_pmpaddr3] = value;
				break;
			case 179 : // pmpaddr4
				r_csr[csr_pmpaddr4] = value;
				break;
			case 180 : // pmpaddr5
				r_csr[csr_pmpaddr5] = value;
				break;
			case 181 : // pmpaddr6
				r_csr[csr_pmpaddr6] = value;
				break;
			case 182 : // pmpaddr7
				r_csr[csr_pmpaddr7] = value;
				break;
			case 183 : // pmpaddr8
				r_csr[csr_pmpaddr8] = value;
				break;
			case 184 : // pmpaddr9
				r_csr[csr_pmpaddr9] = value;
				break;
			case 185 : // pmpaddr10
				r_csr[csr_pmpaddr10] = value;
				break;
			case 186 : // pmpaddr11
				r_csr[csr_pmpaddr11] = value;
				break;
			case 187 : // pmpaddr12
				r_csr[csr_pmpaddr12] = value;
				break;
			case 188 : // pmpaddr13
				r_csr[csr_pmpaddr13] = value;
				break;
			case 189 : // pmpaddr14
				r_csr[csr_pmpaddr14] = value;
				break;
			case 190 : // pmpaddr15
				r_csr[csr_pmpaddr15] = value;
				break;
			case 191 : // mcycle
				r_csr[csr_mcycle] = value;
				break;
			case 192 : // minstret
				r_csr[csr_minstret] = value;
				break;
			case 193 : // mhpmcounter3
				r_csr[csr_mhpmcounter3] = value;
				break;
			case 194 : // mhpmcounter4
				r_csr[csr_mhpmcounter4] = value;
				break;
			case 195 : // mhpmcounter5
				r_csr[csr_mhpmcounter5] = value;
				break;
			case 196 : // mhpmcounter6
				r_csr[csr_mhpmcounter6] = value;
				break;
			case 197 : // mhpmcounter7
				r_csr[csr_mhpmcounter7] = value;
				break;
			case 198 : // mhpmcounter8
				r_csr[csr_mhpmcounter8] = value;
				break;
			case 199 : // mhpmcounter9
				r_csr[csr_mhpmcounter9] = value;
				break;
			case 200 : // mhpmcounter10
				r_csr[csr_mhpmcounter10] = value;
				break;
			case 201 : // mhpmcounter11
				r_csr[csr_mhpmcounter11] = value;
				break;
			case 202 : // mhpmcounter12
				r_csr[csr_mhpmcounter12] = value;
				break;
			case 203 : // mhpmcounter13
				r_csr[csr_mhpmcounter13] = value;
				break;
			case 204 : // mhpmcounter14
				r_csr[csr_mhpmcounter14] = value;
				break;
			case 205 : // mhpmcounter15
				r_csr[csr_mhpmcounter15] = value;
				break;
			case 206 : // mhpmcounter16
				r_csr[csr_mhpmcounter16] = value;
				break;
			case 207 : // mhpmcounter17
				r_csr[csr_mhpmcounter17] = value;
				break;
			case 208 : // mhpmcounter18
				r_csr[csr_mhpmcounter18] = value;
				break;
			case 209 : // mhpmcounter19
				r_csr[csr_mhpmcounter19] = value;
				break;
			case 210 : // mhpmcounter20
				r_csr[csr_mhpmcounter20] = value;
				break;
			case 211 : // mhpmcounter21
				r_csr[csr_mhpmcounter21] = value;
				break;
			case 212 : // mhpmcounter22
				r_csr[csr_mhpmcounter22] = value;
				break;
			case 213 : // mhpmcounter23
				r_csr[csr_mhpmcounter23] = value;
				break;
			case 214 : // mhpmcounter24
				r_csr[csr_mhpmcounter24] = value;
				break;
			case 215 : // mhpmcounter25
				r_csr[csr_mhpmcounter25] = value;
				break;
			case 216 : // mhpmcounter26
				r_csr[csr_mhpmcounter26] = value;
				break;
			case 217 : // mhpmcounter27
				r_csr[csr_mhpmcounter27] = value;
				break;
			case 218 : // mhpmcounter28
				r_csr[csr_mhpmcounter28] = value;
				break;
			case 219 : // mhpmcounter29
				r_csr[csr_mhpmcounter29] = value;
				break;
			case 220 : // mhpmcounter30
				r_csr[csr_mhpmcounter30] = value;
				break;
			case 221 : // mhpmcounter31
				r_csr[csr_mhpmcounter31] = value;
				break;
			case 222 : // mcycleh
				r_csr[csr_mcycleh] = value;
				break;
			case 223 : // minstreth
				r_csr[csr_minstreth] = value;
				break;
			case 224 : // mhpmcounter3h
				r_csr[csr_mhpmcounter3h] = value;
				break;
			case 225 : // mhpmcounter4h
				r_csr[csr_mhpmcounter4h] = value;
				break;
			case 226 : // mhpmcounter5h
				r_csr[csr_mhpmcounter5h] = value;
				break;
			case 227 : // mhpmcounter6h
				r_csr[csr_mhpmcounter6h] = value;
				break;
			case 228 : // mhpmcounter7h
				r_csr[csr_mhpmcounter7h] = value;
				break;
			case 229 : // mhpmcounter8h
				r_csr[csr_mhpmcounter8h] = value;
				break;
			case 230 : // mhpmcounter9h
				r_csr[csr_mhpmcounter9h] = value;
				break;
			case 231 : // mhpmcounter10h
				r_csr[csr_mhpmcounter10h] = value;
				break;
			case 232 : // mhpmcounter11h
				r_csr[csr_mhpmcounter11h] = value;
				break;
			case 233 : // mhpmcounter12h
				r_csr[csr_mhpmcounter12h] = value;
				break;
			case 234 : // mhpmcounter13h
				r_csr[csr_mhpmcounter13h] = value;
				break;
			case 235 : // mhpmcounter14h
				r_csr[csr_mhpmcounter14h] = value;
				break;
			case 236 : // mhpmcounter15h
				r_csr[csr_mhpmcounter15h] = value;
				break;
			case 237 : // mhpmcounter16h
				r_csr[csr_mhpmcounter16h] = value;
				break;
			case 238 : // mhpmcounter17h
				r_csr[csr_mhpmcounter17h] = value;
				break;
			case 239 : // mhpmcounter18h
				r_csr[csr_mhpmcounter18h] = value;
				break;
			case 240 : // mhpmcounter19h
				r_csr[csr_mhpmcounter19h] = value;
				break;
			case 241 : // mhpmcounter20h
				r_csr[csr_mhpmcounter20h] = value;
				break;
			case 242 : // mhpmcounter21h
				r_csr[csr_mhpmcounter21h] = value;
				break;
			case 243 : // mhpmcounter22h
				r_csr[csr_mhpmcounter22h] = value;
				break;
			case 244 : // mhpmcounter23h
				r_csr[csr_mhpmcounter23h] = value;
				break;
			case 245 : // mhpmcounter24h
				r_csr[csr_mhpmcounter24h] = value;
				break;
			case 246 : // mhpmcounter25h
				r_csr[csr_mhpmcounter25h] = value;
				break;
			case 247 : // mhpmcounter26h
				r_csr[csr_mhpmcounter26h] = value;
				break;
			case 248 : // mhpmcounter27h
				r_csr[csr_mhpmcounter27h] = value;
				break;
			case 249 : // mhpmcounter28h
				r_csr[csr_mhpmcounter28h] = value;
				break;
			case 250 : // mhpmcounter29h
				r_csr[csr_mhpmcounter29h] = value;
				break;
			case 251 : // mhpmcounter30h
				r_csr[csr_mhpmcounter30h] = value;
				break;
			case 252 : // mhpmcounter31h
				r_csr[csr_mhpmcounter31h] = value;
				break;
			case 253 : // mhpmevent3
				r_csr[csr_mhpmevent3] = value;
				break;
			case 254 : // mhpmevent4
				r_csr[csr_mhpmevent4] = value;
				break;
			case 255 : // mhpmevent5
				r_csr[csr_mhpmevent5] = value;
				break;
			case 256 : // mhpmevent6
				r_csr[csr_mhpmevent6] = value;
				break;
			case 257 : // mhpmevent7
				r_csr[csr_mhpmevent7] = value;
				break;
			case 258 : // mhpmevent8
				r_csr[csr_mhpmevent8] = value;
				break;
			case 259 : // mhpmevent9
				r_csr[csr_mhpmevent9] = value;
				break;
			case 260 : // mhpmevent10
				r_csr[csr_mhpmevent10] = value;
				break;
			case 261 : // mhpmevent11
				r_csr[csr_mhpmevent11] = value;
				break;
			case 262 : // mhpmevent12
				r_csr[csr_mhpmevent12] = value;
				break;
			case 263 : // mhpmevent13
				r_csr[csr_mhpmevent13] = value;
				break;
			case 264 : // mhpmevent14
				r_csr[csr_mhpmevent14] = value;
				break;
			case 265 : // mhpmevent15
				r_csr[csr_mhpmevent15] = value;
				break;
			case 266 : // mhpmevent16
				r_csr[csr_mhpmevent16] = value;
				break;
			case 267 : // mhpmevent17
				r_csr[csr_mhpmevent17] = value;
				break;
			case 268 : // mhpmevent18
				r_csr[csr_mhpmevent18] = value;
				break;
			case 269 : // mhpmevent19
				r_csr[csr_mhpmevent19] = value;
				break;
			case 270 : // mhpmevent20
				r_csr[csr_mhpmevent20] = value;
				break;
			case 271 : // mhpmevent21
				r_csr[csr_mhpmevent21] = value;
				break;
			case 272 : // mhpmevent22
				r_csr[csr_mhpmevent22] = value;
				break;
			case 273 : // mhpmevent23
				r_csr[csr_mhpmevent23] = value;
				break;
			case 274 : // mhpmevent24
				r_csr[csr_mhpmevent24] = value;
				break;
			case 275 : // mhpmevent25
				r_csr[csr_mhpmevent25] = value;
				break;
			case 276 : // mhpmevent26
				r_csr[csr_mhpmevent26] = value;
				break;
			case 277 : // mhpmevent27
				r_csr[csr_mhpmevent27] = value;
				break;
			case 278 : // mhpmevent28
				r_csr[csr_mhpmevent28] = value;
				break;
			case 279 : // mhpmevent29
				r_csr[csr_mhpmevent29] = value;
				break;
			case 280 : // mhpmevent30
				r_csr[csr_mhpmevent30] = value;
				break;
			case 281 : // mhpmevent31
				r_csr[csr_mhpmevent31] = value;
				break;
			case 282 : // tselect
				r_csr[csr_tselect] = value;
				break;
			case 283 : // tdata1
				r_csr[csr_tdata1] = value;
				break;
			case 284 : // tdata2
				r_csr[csr_tdata2] = value;
				break;
			case 285 : // tdata3
				r_csr[csr_tdata3] = value;
				break;
			case 286 : // dcsr
				r_csr[csr_dcsr] = value;
				break;
			case 287 : // dpc
				r_csr[csr_dpc] = value;
				break;
			case 288 : // dscratch
				r_csr[csr_dscratch] = value;
				break;
			case 289 : // hstatus
			case 290 : // hedeleg
			case 291 : // hideleg
			case 292 : // hie
			case 293 : // htvec
			case 294 : // hscratch
			case 295 : // hepc
			case 296 : // hcause
			case 297 : // hbadaddr
			case 298 : // hip
			case 299 : // mbase
			case 300 : // mbound
			case 301 : // mibase
			case 302 : // mibound
			case 303 : // mdbase
			case 304 : // mdbound
			case 305 : // mucounteren
			case 306 : // mscounteren
			case 307 : // mhcounteren
			default:
				break;
		}
	}

} // end common
} // end soclib
