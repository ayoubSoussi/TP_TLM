/*\
 * vim: tw=0: cindent: sw=3: ts=3: sts=3: noet: list
 * Wrapper of Soclib ISS for the ensitlm protocol
 * Largely inspired from the Microblaze Wrapper by Matthieu Moy
 * Contributing authors: Frédéric Pétrot
 *                       <frederic.petrot@univ-grenoble-alpes.fr>
 *
\*/

#include "ensitlm.h"
#include "rv32_wrapper.h"
#include "rv32.h"
#include <iomanip>

#if 0
#define DEBUG
#endif
#define NB_INST 50
static const sc_core::sc_time PERIOD_MULT(20 * NB_INST, sc_core::SC_NS);

/* Time between two step()s */
static const sc_core::sc_time PERIOD(20, sc_core::SC_NS);

using namespace std;

RV32Wrapper::RV32Wrapper(sc_core::sc_module_name name)
	: sc_core::sc_module(name), irq("irq"),
	m_iss(0) /* identifier, not very useful since we have only one instance */
{
	m_iss.reset();
	m_iss.setIrq(false);
	SC_THREAD(run_iss);
	/* The method that is required to forward the interrupts from the SystemC
	 * environment to the ISS needs to be declared here */
}

/* IRQ forwarding method to be defined here */

void RV32Wrapper::exec_data_request(enum iss_t::DataOperationType mem_type,
                                  uint32_t mem_addr, uint32_t mem_wdata, uint32_t mem_be)
{
	uint32_t localbuf;
	int      shift;
	tlm::tlm_response_status status;

	// FIXME: No byte_enable yet in the tlm bus, assume aligned on the lsb lanes
	// From what I get, the ISS produces be that is either 0b0001, 0b0011 or 0b1111
	switch (mem_type) {
		case iss_t::DATA_READ:
			/* The ISS requested a data read (content of mem_addr into localbuf). */
			abort(); // TODO
#ifdef DEBUG
			std::cout << hex << "read    " << setw(10) << localbuf
						 << " at address " << mem_addr << std::endl;
#endif
			m_iss.setDataResponse(0, localbuf);
			break;
		case iss_t::DATA_WRITE:
			/* The ISS requested a data write (mem_wdata at mem_addr). */
			abort(); // TODO
#ifdef DEBUG
			std::cout << hex << "wrote   " << setw(10) << mem_wdata
						 << " at address " << mem_addr << std::endl;
#endif
			m_iss.setDataResponse(0, 0);
			break;
		case iss_t::DATA_LL:
		case iss_t::DATA_SC:
		default:
			std::cerr << "Operation " << mem_type << " unsupported for "
						 << std::showbase << std::hex << mem_addr << std::endl;
			abort();
	}
}

void RV32Wrapper::run_iss(void)
{
	int inst_count = 0;

	while (true) {
		if (m_iss.isBusy())
			m_iss.nullStep();
		else {
			bool ins_asked;
			uint32_t ins_addr;
			m_iss.getInstructionRequest(ins_asked, ins_addr);

			// FIXME: riscv accepts compressed instructions, thus instructions
			// may be half-word aligned. 
			// For now assume the rv32im profile
			if (ins_asked) {
				/* The ISS requested an instruction.
				 * We have to do the instruction fetch by reading from memory. */
				abort(); // TODO
				uint32_t localbuf;
				m_iss.setInstruction(0, localbuf);
			}

			bool mem_asked;
			enum iss_t::DataOperationType mem_type;
			uint32_t mem_addr;
			uint32_t mem_wdata;
			uint8_t mem_be;
			m_iss.getDataRequest(mem_asked, mem_type, mem_addr, mem_wdata, mem_be);

			if (mem_asked) {
				exec_data_request(mem_type, mem_addr, mem_wdata, mem_be);
			}
			m_iss.step();

			/* IRQ handling to be done */
		}

		wait(PERIOD);
	}
}
