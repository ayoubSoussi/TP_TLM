#include "ensitlm.h"
#include "bus.h"

struct generator : sc_core::sc_module{
	ensitlm::initiator_socket<generator> socket ;
	sc_core::sc_in<bool> display_intr_in;
	void thread(void) ;

	SC_CTOR(generator) ;
};
