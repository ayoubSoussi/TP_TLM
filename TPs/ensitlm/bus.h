#ifndef BUS_H
#define BUS_H

#include "ensitlm.h"

#include <map>

SC_MODULE(Bus) {
	// The bus is the only component needing this "true" template
	// parameter, to allow multi-port connections.
	ensitlm::initiator_socket<Bus, true> initiator;
	ensitlm::target_socket<Bus, true> target;

	Bus(sc_core::sc_module_name name);

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t & d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

	void map(ensitlm::compatible_socket & port, ensitlm::addr_t start_addr,
	         ensitlm::addr_t size);

private:
	void print_addr_map();
	void end_of_elaboration();

	class addr_range {
	public:
		addr_range(ensitlm::addr_t b, ensitlm::addr_t e)
		    : begin(b), end(e) {
		}
		const ensitlm::addr_t begin;
		const ensitlm::addr_t end;
		bool operator<(const addr_range &ar) const {
			return (end < ar.begin);
		}
	};

	typedef std::multimap<ensitlm::compatible_socket *, addr_range>
	    port_map_t;
	port_map_t port_map;

	typedef std::map<addr_range, int> addr_map_t;
	addr_map_t addr_map;
};

#endif
