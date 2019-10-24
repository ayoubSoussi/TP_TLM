#ifndef TIMER_H
#define TIMER_H

#include "ensitlm.h"

SC_MODULE(TIMER) {
	SC_HAS_PROCESS(TIMER);

	ensitlm::target_socket<TIMER> target;
	sc_core::sc_out<bool> irq;

	TIMER(sc_core::sc_module_name name, sc_core::sc_time p);

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t & d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

private:
	sc_core::sc_time period;
	sc_core::sc_event irq_event;
	sc_core::sc_event csr_event[2];

	void interrupt();
	void timer(int i);
	void timer0();
	void timer1();

	bool refresh[2];
	ensitlm::data_t csr[2];
	ensitlm::data_t tlr[2];
	ensitlm::data_t tcr[2];
};

#endif
