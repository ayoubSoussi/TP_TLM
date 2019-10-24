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
 *
\*/

/*\
 * Macros to extract fields depending on instruction X-type
 * so as to avoid duplicating the code too much
\*/

#define decode_csr_type(rd, csr, val)              \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	csr = (m_ir >> 20) & 0xfffff;                   \
	val = (m_ir >> 15) & 0x1f;                      \
} while (0)

#define decode_u_type(rd, imm)                     \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	imm = m_ir & 0xfffff000;                        \
} while (0)

#define decode_j_type(rd, imm)                     \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	imm = ((int32_t)(m_ir & 0x80000000) >> 11)      \
			| (m_ir & 0x000ff000)                     \
			| ((m_ir & 0x00100000) >> 9)              \
			| ((m_ir & 0x7fe00000) >> 20);            \
} while (0)

#define decode_i_type(rd, rs1, imm)                \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	rs1 = (m_ir >> 15) & 0x1f;                      \
	imm = (int32_t)(m_ir & 0xfff00000) >> 20;       \
} while (0)

#define decode_b_type(rs1, rs2, imm)               \
do {                                               \
	rs1 = (m_ir >> 15) & 0x1f;                      \
	rs2 = (m_ir >> 20) & 0x1f;                      \
	imm = ((int32_t)(m_ir & 0x80000000) >> 19)      \
			| ((m_ir & 0x00000080) << 4)              \
			| ((m_ir & 0x00000f00) >> 7)              \
			| ((m_ir & 0x7e000000) >> 20);            \
} while (0)

#define decode_s_type(rs1, rs2, imm)               \
do {                                               \
	rs1 = (m_ir >> 15) & 0x1f;                      \
	rs2 = (m_ir >> 20) & 0x1f;                      \
	imm = ((int32_t)(m_ir & 0xfe000000) >> 20)      \
			| ((m_ir & 0x00000f80) >> 7);             \
} while (0)

#define decode_r_type(rd, rs1, rs2)                \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	rs1 = (m_ir >> 15) & 0x1f;                      \
	rs2 = (m_ir >> 20) & 0x1f;                      \
} while (0)

/*\
 * Used in 3 operand fp operations
\*/
#define decode_r4_type(rd, rs1, rs2, rs3, fmt, rm) \
do {                                               \
	rd  = (m_ir >>  7) & 0x1f;                      \
	rs1 = (m_ir >> 15) & 0x1f;                      \
	rs2 = (m_ir >> 20) & 0x1f;                      \
	rs3 = (m_ir >> 27) & 0x1f;                      \
	fmt = (m_ir >> 25) & 0x03;                      \
	rm  = (m_ir >> 12) & 0x07;                      \
} while (0)

/*\
 * Used in the compressed instruction set
 * Adding parenthesis as the g++-8 seems more
 * pedantic on that topic and issues otherwise a
 * bunch of warnings
\*/
#define decode_cr_type(rd, rs2)                    \
do {                                               \
	rd  = (m_ir >> 7) & 0x1f;                       \
	rs2 = (m_ir >> 2) & 0x1f;                       \
} while (0)

#define decode_ci_type(rd, imm)                   \
do {                                              \
	rd  = (m_ir >> 7) & 0x1f;                      \
	imm = (m_ir & 0x1000)                          \
			| ((m_ir << 5) & 0xf80);                 \
	imm = (int32_t)(imm << 19) >> 26;              \
} while (0)

// For right shifts and and with immediat
#define decode_cis_type(rd, imm)                  \
do {                                              \
	rd  = ((m_ir >> 7) & 0x7) + 8;                 \
	imm = (m_ir & 0x1000)                          \
			| ((m_ir << 5) & 0xf80);                 \
	imm = (int32_t)(imm << 19) >> 26;              \
} while (0)


// Special version of ci for the sp relative loads
#define decode_cils_type(rd, imm)                 \
do {                                              \
	rd  = (m_ir >> 7) & 0x1f;                      \
	imm = ((m_ir << 4) &  0xc0)                    \
			| ((m_ir >> 7) & 0x20)                   \
			| ((m_ir >> 2) & 0x1c);                  \
} while (0)

// Special version of ci for c.addi4spn
#define decode_ciw_type(rd, imm)                  \
do {                                              \
	rd  = 8 + ((m_ir >> 2) & 0x7);                 \
	imm = ((m_ir >> 7) & 0x30)                     \
			| ((m_ir >> 1) & 0x3c0)                  \
			| ((m_ir >> 4) & 0x4)                    \
			| ((m_ir >> 2) & 0x8);                   \
} while (0)

#define decode_css_type(rs2, imm)                 \
do {                                              \
	rs2 = (m_ir >> 2) & 0x1f;                      \
	imm = ((m_ir >> 7) & 0x3c)                     \
			| ((m_ir >> 1) & 0xc0);                  \
} while (0)

#define decode_cisp_type(rd, imm)                 \
do {                                              \
	rd   = ((m_ir >> 7) & 0x7);                    \
	imm  = (m_ir & 0x1000)                         \
				| ((m_ir & 0x18) << 7)                \
				| ((m_ir & 0x20) << 4)                \
				| ((m_ir & 0x4)  << 6)                \
				| ((m_ir & 0x40) << 1);               \
	imm = (int32_t)(imm << 19) >> 22;              \
} while (0)

#define decode_cl_type(rd, rs1, imm)              \
do {                                              \
	rd  = 8 + ((m_ir >> 2) & 0x7);                 \
	rs1 = 8 + ((m_ir >> 7) & 0x7);                 \
	imm = ((m_ir << 1) & 0x40)                     \
			| ((m_ir >> 7) & 0x38)                   \
			| ((m_ir >> 4) & 0x4);                   \
} while (0)

/* Strangely enough, identical to cl */
#define decode_cs_type(rs2, rs1, imm)             \
do {                                              \
	rs2 = 8 + ((m_ir >> 2) & 0x7);                 \
	rs1 = 8 + ((m_ir >> 7) & 0x7);                 \
	imm = ((m_ir << 1) & 0x40)                     \
			| ((m_ir >> 7) & 0x38)                   \
			| ((m_ir >> 4) & 0x4);                   \
} while (0)

#define decode_cj_type(imm)                       \
do {                                              \
	imm = ((m_ir >> 1) & 0x800)                    \
			| ((m_ir >> 7) & 0x10)                   \
			| ((m_ir >> 1) & 0x300)                  \
			| ((m_ir << 2) & 0x400)                  \
			| ((m_ir >> 1) & 0x40)                   \
			| ((m_ir << 1) & 0x80)                   \
			| ((m_ir >> 2) & 0xe)                    \
			| ((m_ir << 3) & 0x20);                  \
	imm = (int32_t)(imm << 20) >> 20;              \
} while (0)

#define decode_cb_type(rs1, imm)                  \
do {                                              \
	rs1 = 8 + ((m_ir >> 7) & 0x7);                 \
	imm = ((m_ir >> 4) & 0x100)                    \
			| ((m_ir >> 7) & 0x18)                   \
			| ((m_ir << 1) & 0xc0)                   \
			| ((m_ir >> 2) & 0x6)                    \
			| ((m_ir << 3) & 0x20);                  \
	imm = (int32_t)(imm << 23) >> 23;              \
} while (0)
