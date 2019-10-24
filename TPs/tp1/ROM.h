#ifndef ROM_H
#define ROM_H

#include "ensitlm.h"

#define ROM_SIZE (320 * 240 / 2)

struct ROM : public sc_core::sc_module {
	ensitlm::target_socket<ROM> socket;
	ensitlm::data_t *content;

	tlm::tlm_response_status read(const ensitlm::addr_t &a,
	                              ensitlm::data_t &d);

	tlm::tlm_response_status write(const ensitlm::addr_t &a,
	                               const ensitlm::data_t &d) {
		(void)a;
		(void)d;
		abort();
	};

	SC_CTOR(ROM);
	~ROM();
};

#endif // ROM_H
