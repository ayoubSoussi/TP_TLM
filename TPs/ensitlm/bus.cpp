#include "ensitlm.h"
#include "bus.h"

using namespace std;

Bus::Bus(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	cout << name << ": Ensitlm bus" << endl;
}

void Bus::map(ensitlm::compatible_socket &port, ensitlm::addr_t start_addr,
              ensitlm::addr_t size) {
	port_map.insert(std::pair<ensitlm::compatible_socket *, addr_range>(
	    &port, addr_range(start_addr, start_addr + size - 1)));
}

void Bus::end_of_elaboration() {
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

void Bus::print_addr_map() {
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

tlm::tlm_response_status Bus::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	if (a % sizeof(ensitlm::data_t)) {
		std::stringstream s;
		s << "unaligned read at 0x" << std::hex << a;
		SC_REPORT_ERROR(name(), s.str().c_str());
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}

	addr_map_t::iterator it = addr_map.find(addr_range(a, a));
	if (it == addr_map.end()) {
		std::cerr << name() << ": no target at address " << std::hex
		          << a << std::endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}

	tlm::tlm_response_status s =
	    initiator.read(a - (*it).first.begin, d, (*it).second);

#ifdef DEBUG
	std::cout << "Debug: " << name() << ": read access at " << std::hex
	          << std::showbase << a << " (data: " << d << ")\n";
#endif

	return s;
}

tlm::tlm_response_status Bus::write(ensitlm::addr_t a, ensitlm::data_t d) {
	if (a % sizeof(ensitlm::data_t)) {
		std::stringstream s;
		s << "unaligned write at 0x" << std::hex << a;
		SC_REPORT_ERROR(name(), s.str().c_str());
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}

	addr_map_t::iterator it = addr_map.find(addr_range(a, a));
	if (it == addr_map.end()) {
		std::cerr << name() << ": no target at address " << std::hex
		          << a << std::endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}

#ifdef DEBUG
	std::cout << "Debug: " << name() << ": write access at " << std::hex
	          << std::showbase << a << " (data: " << d << ")\n";
#endif

	tlm::tlm_response_status s =
	    initiator.write(a - (*it).first.begin, d, (*it).second);

	return s;
}
