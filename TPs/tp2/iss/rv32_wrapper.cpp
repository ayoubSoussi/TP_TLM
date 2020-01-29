/*\
 * vim: tw=0: cindent: sw=3: ts=3: sts=3: noet: list
 * Wrapper of Soclib ISS for the ensitlm protocol
 * Largely inspired from the Microblaze Wrapper by Matthieu Moy
 * Contributing authors: Frédéric Pétrot
 *                       <frederic.petrot@univ-grenoble-alpes.fr>
 *
\*/

#include "rv32_wrapper.h"
#include "ensitlm.h"
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
    : sc_core::sc_module(name),
      irq("irq"),
      m_iss(0) /* identifier, not very useful since we have only one instance */
{
	m_iss.reset();
	m_iss.setIrq(false);
	SC_THREAD(run_iss);
	/* The method that is required to forward the interrupts from the SystemC
	 * environment to the ISS needs to be declared here */
	m_irq_counter = 0;
	SC_METHOD(interrupt_forward);
	sensitive << irq;
}

/* IRQ forwarding method to be defined here */

void RV32Wrapper::exec_data_request(enum iss_t::DataOperationType mem_type,
                                    uint32_t mem_addr, uint32_t mem_wdata,
                                    uint32_t mem_be) {
	uint32_t localbuf;
	tlm::tlm_response_status status;

	switch (mem_type) {
	case iss_t::DATA_READ:
		/* The ISS requested a data read (content of mem_addr into localbuf). */
		status = socket.read(mem_addr, localbuf);
		if (status != tlm::TLM_OK_RESPONSE) {
			std::cerr << "Read error at address " << hex << mem_addr
			          << std::endl;
		}
#ifdef DEBUG
		std::cout << hex << "read    " << setw(10) << localbuf << " at address "
		          << mem_addr << std::endl;
#endif
		m_iss.setDataResponse(0, localbuf);
		break;
	case iss_t::DATA_WRITE:
		/* The ISS requested a data write (mem_wdata at mem_addr). */
		status = socket.write(mem_addr, mem_wdata);
		if (status != tlm::TLM_OK_RESPONSE) {
			std::cerr << "Write error at address " << hex << mem_addr
			          << std::endl;
		}
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

void RV32Wrapper::run_iss(void) {
	while (true) {
		if (m_iss.isBusy())
			m_iss.nullStep();
		else {
			bool ins_asked;
			uint32_t ins_addr;
			m_iss.getInstructionRequest(ins_asked, ins_addr);

			if (ins_asked) {
				/* The ISS requested an instruction.
				 * We have to do the instruction fetch by reading from memory.
				 */
				ensitlm::data_t localbuf;
				tlm::tlm_response_status status;
				status = socket.read(ins_addr, localbuf);
				if (status != tlm::TLM_OK_RESPONSE) {
					std::cerr << "Read error at address " << hex << ins_asked
					          << std::endl;
				}

				m_iss.setInstruction(0, localbuf);
#ifdef DEBUG
				std::cout << hex << "Post setInst: asking " << setw(10)
				          << ins_asked << " at address " << ins_addr
				          << " where " << localbuf << std::endl;
#endif
			}

			bool mem_asked;
			enum iss_t::DataOperationType mem_type;
			uint32_t mem_addr;
			uint32_t mem_wdata;
			uint8_t mem_be;
			m_iss.getDataRequest(mem_asked, mem_type, mem_addr, mem_wdata,
			                     mem_be);
#ifdef DEBUG
			std::cout << hex << "Post getDataRequest: reading   " << setw(10)
			          << mem_wdata << " at address " << mem_addr << std::endl;
#endif
			if (mem_asked) {
				exec_data_request(mem_type, mem_addr, mem_wdata, mem_be);
			}
			m_iss.step();

			/* IRQ handling */
			m_irq_counter--;
			if (m_irq_counter < 1) m_iss.setIrq(false);
		}

		wait(PERIOD);
	}
}

void RV32Wrapper::interrupt_forward() {
	// enable interrupts
	m_irq_counter = 5;
	m_iss.setIrq(true);
}