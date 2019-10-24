#ifndef GPIO_H
#define GPIO_H

#include "ensitlm.h"

SC_MODULE(Gpio) {

	ensitlm::target_socket<Gpio> target;

	Gpio(sc_core::sc_module_name name);

	tlm::tlm_response_status read(const ensitlm::addr_t &a,
	                              ensitlm::data_t &d);

	tlm::tlm_response_status write(const ensitlm::addr_t &a,
	                               const ensitlm::data_t &d);

private:
	ensitlm::data_t data;
	ensitlm::data_t tri;
};

#endif
