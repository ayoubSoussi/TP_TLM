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
	 * environment to the ISS */
	cmpt = 3;
	SC_METHOD(irq_handler);
	sensitive << irq;
}


void RV32Wrapper::irq_handler(void){
	m_iss.setIrq(true);
	cmpt = 0;
}
void RV32Wrapper::exec_data_request(enum iss_t::DataOperationType mem_type,
                                    uint32_t mem_addr, uint32_t mem_wdata, uint32_t mem_be)
{
	uint32_t localbuf;
	int      shift;
	tlm::tlm_response_status status;

	switch (mem_type) {
    case iss_t::DATA_READ:
			// read data in the address mem_addr (The ISS requested a data read)
			status = socket.read(mem_addr, localbuf);
			if (status != tlm::TLM_OK_RESPONSE ){
                std::cerr << "Read error in address " << hex << mem_addr << std::endl;
			}
#ifdef DEBUG
			std::cout << hex << "read    " << setw(10) << localbuf
						 << " at address " << mem_addr << std::endl;
#endif
			m_iss.setDataResponse(0, localbuf);
			break;
		case iss_t::DATA_WRITE:
			// write data in the address mem_addr to the mem_wdata (The ISS requested a data write)
			status = socket.write(mem_addr, mem_wdata);
			if (status != tlm::TLM_OK_RESPONSE ){
				std::cerr << "Write error in address " << hex << mem_addr << std::endl;
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

void RV32Wrapper::run_iss(void){
	while (true) {
		if (m_iss.isBusy())
			m_iss.nullStep();
		else {
			bool ins_asked;
			uint32_t ins_addr;
			tlm::tlm_response_status status;
			m_iss.getInstructionRequest(ins_asked, ins_addr);

			if (ins_asked) {
				uint32_t localbuf;
				/* The ISS requested an instruction.
				 * We have to do the instruction fetch by reading from memory. */
				
				status = socket.read(ins_addr, localbuf);
				if (status != tlm::TLM_OK_RESPONSE ){
				std::cerr << "Fetch error in address " << hex << ins_addr << std::endl;
				}
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

			/* IRQ handling */
			cmpt++;
			if (cmpt > 3) m_iss.setIrq(false);
		}

		wait(PERIOD);
	}
}
