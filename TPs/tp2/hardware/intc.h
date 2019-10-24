#ifndef INTC_H
#define INTC_H

#include "ensitlm.h"

#include <vector>

SC_MODULE(Intc) {
	ensitlm::target_socket<Intc> target;

	SC_HAS_PROCESS(Intc);

	sc_core::sc_in<bool> in0;
	sc_core::sc_in<bool> in1;
	sc_core::sc_out<bool> out;

	explicit Intc(sc_core::sc_module_name name);

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t & d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

private:
	void process_in_irq(int N);
	void process_in_irq0() {
		process_in_irq(0);
	}
	void process_in_irq1() {
		process_in_irq(1);
	}
	void send_irq();
	void send_irq_maybe(ensitlm::data_t mask);

	// IRQ non-masquée
	ensitlm::data_t m_enabled_it;
	// IRQ recue et non-acquitée
	ensitlm::data_t m_active_it;

	bool irq_to_send_notified;
	sc_core::sc_event irq_to_send;
};

#endif
