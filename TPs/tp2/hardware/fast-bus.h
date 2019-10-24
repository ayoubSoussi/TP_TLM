#ifndef FAST_BUS_H
#define FAST_BUS_H

#include "ensitlm.h"

#include <map>

// Alternative implementation for the bus. Instead of relying on
// ensitlm's read and write functions, we use the raw initiator/target
// sockets, and implement b_transport. We do not unpack/repack the
// payload, but only transmit it to the the right component.
// This does less work than the Bus implementation, but the
// performance gain is not visible in practice (it can even be
// slower).
SC_MODULE(FastBus), tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>,
    tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types> {

	tlm::tlm_target_socket<CHAR_BIT * sizeof(ensitlm::data_t),
	                       tlm::tlm_base_protocol_types, 0> target;
	tlm::tlm_initiator_socket<CHAR_BIT * sizeof(ensitlm::data_t),
	                          tlm::tlm_base_protocol_types, 0> initiator;

	FastBus(sc_core::sc_module_name name);

	void map(ensitlm::compatible_socket & port, ensitlm::addr_t start_addr,
	         ensitlm::addr_t size);

	// Function that does the job of transporting the payload.
	void b_transport(tlm::tlm_generic_payload & trans,
	                 sc_core::sc_time & t);

	// Default implementation of methods to be able to derive from
	// tlm_fw_transport_if and tlm_bw_transport_if.
	bool get_direct_mem_ptr(tlm::tlm_generic_payload &, tlm::tlm_dmi &) {
		std::cerr << "get_direct_mem_ptr not implemented" << std::endl;
		abort();
	}

	unsigned int transport_dbg(tlm::tlm_generic_payload &) {
		std::cerr << "transport_dbg not implemented" << std::endl;
		abort();
	}

	tlm::tlm_sync_enum nb_transport_fw(
	    tlm::tlm_generic_payload &, tlm::tlm_phase &, sc_core::sc_time &) {
		std::cerr << "nb_transport_fw not implemented" << std::endl;
		abort();
	}

	void invalidate_direct_mem_ptr(sc_dt::uint64, sc_dt::uint64) {
		std::cerr << "invalidate_direct_mem_ptr not implemented"
		          << std::endl;
		abort();
	}

	tlm::tlm_sync_enum nb_transport_bw(
	    tlm::tlm_generic_payload &, tlm::tlm_phase &, sc_core::sc_time &) {
		std::cerr << "nb_transport_bw not implemented" << std::endl;
		abort();
	}

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
