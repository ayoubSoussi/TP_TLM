#ifndef RV32_WRAPPER_H
#define RV32_WRAPPER_H

#include "ensitlm.h"
#include "rv32.h"

/*\
 * Wrapper for the RISCV ISS using the ensitlm protocol.
\*/
struct RV32Wrapper : sc_core::sc_module {
	ensitlm::initiator_socket<RV32Wrapper> socket;
	sc_core::sc_in<bool> irq;

	void run_iss(void);
	/* Add stuff relative to irq handling */
	void irq_handler(void);
	void interrupt_forward(void); ///////
	int m_irq_counter = 0; /* cycles counter */

	SC_CTOR(RV32Wrapper);

private:
	typedef soclib::common::Rv32Iss iss_t;
	void exec_data_request(enum iss_t::DataOperationType mem_type,
	                       uint32_t mem_addr, uint32_t mem_wdata, uint32_t mem_be);
	iss_t m_iss;
};

#endif // RV32_WRAPPER_H
