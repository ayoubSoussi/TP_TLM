#include "ensitlm.h"
#include "fast-bus.h"

using namespace std;

FastBus::FastBus(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	cout << name << ": Ensitlm fast bus" << endl;
	target.bind(*this);
	initiator.bind(*this);
}

void FastBus::map(ensitlm::compatible_socket &port, ensitlm::addr_t start_addr,
                  ensitlm::addr_t size) {
	port_map.insert(std::pair<ensitlm::compatible_socket *, addr_range>(
	    &port, addr_range(start_addr, start_addr + size - 1)));
}

void FastBus::b_transport(tlm::tlm_generic_payload &trans,
                          sc_core::sc_time &t) {
	ensitlm::addr_t a = trans.get_address();
	addr_map_t::iterator it = addr_map.find(addr_range(a, a));
	if (it == addr_map.end()) {
		std::cerr << name() << ": no target at address "
		          << std::showbase << std::hex << a << std::endl;
		trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
	}
	trans.set_address(a - (*it).first.begin);
	initiator[(*it).second]->b_transport(trans, t);
}

void FastBus::end_of_elaboration() {
	// for each target connected to this bus initiator port
	for (int i = 0; i < initiator.size(); ++i) {
		// get the target
		ensitlm::compatible_socket *target =
		    dynamic_cast<ensitlm::compatible_socket *>(initiator[i]);
		if (!target) {
			std::cerr << name()
			          << ": target is not a tlm_target_socket\n";
			abort();
		}
		// get the set of port maps which correspond to this name
		std::pair<port_map_t::iterator, port_map_t::iterator> it =
		    port_map.equal_range(target);
		// if no port map corresponds
		if (it.first == it.second) {
			std::cerr << name() << ": no address map information "
			                       "available for target "
			          << target->name() << "\n";
			abort();
		}
		// iterate through port maps
		for (port_map_t::iterator j = it.first; j != it.second; ++j) {
			std::pair<addr_range, int> map_entry((*j).second, i);
			// add to address map and check for conflicts
			if (!addr_map.insert(map_entry).second) {
				std::pair<addr_range, int> map_entry_bis =
				    (*addr_map.find((*j).second));
				int k = map_entry_bis.second;
				ensitlm::compatible_socket *target_bis =
				    dynamic_cast<ensitlm::compatible_socket *>(
				        initiator[k]);
				std::cerr << name() << ": address map conflict "
				                       "between target ports "
				          << target->name() << " and "
				          << target_bis->name() << "\n";
				abort();
			}
		}
	}
	//   #ifdef DEBUG
	print_addr_map();
	//   #endif
}

void FastBus::print_addr_map() {
	// iterate through port maps
	for (addr_map_t::iterator i = addr_map.begin(); i != addr_map.end();
	     ++i) {
		std::cout << name() << ": range [" << std::hex
		          << (*i).first.begin << "-" << (*i).first.end + 1
		          << "[ is mapped to target '"
		          << dynamic_cast<ensitlm::compatible_socket *>(
		                 initiator[((*i).second)])->name() << "'\n";
	}
}
