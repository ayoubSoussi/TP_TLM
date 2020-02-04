#include "ensitlm.h"
#include "bus.h"

struct generator : sc_core::sc_module {
	ensitlm::initiator_socket<generator> socket;
	sc_core::sc_in<bool> display_intr_in;
	bool irq_received;
	void thread(void);
	void irq_handler();
	SC_CTOR(generator);
};
