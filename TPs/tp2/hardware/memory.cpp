#include "ensitlm.h"
#include "memory.h"

#if 0
#define DEBUG
#endif

// Constructor
Memory::Memory(sc_core::sc_module_name name, unsigned int size)
    : sc_module(name), m_size(size) {
	storage = new ensitlm::data_t[size / sizeof(ensitlm::data_t)];
}

// Destructor
Memory::~Memory() {
	delete[] storage;
}

// Read transactions
tlm::tlm_response_status Memory::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	// Check if the address is within memory bounds
	if (a >= m_size) {
		std::cerr << name() << ": Read access outside memory range! ("
		          << a << ")" << std::endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	} else {
		d = storage[a / sizeof(ensitlm::data_t)];
#ifdef DEBUG
		std::cout << name() << ": Read  access at " << std::showbase
		       << std::hex << a << " (Data: " << std::showbase << d << ")" << std::endl;
#endif
		return tlm::TLM_OK_RESPONSE;
	}
}

// Write transactions
tlm::tlm_response_status Memory::write(ensitlm::addr_t a, ensitlm::data_t d) {
	// Check if the address is within memory bounds
	if (a >= m_size) {
		std::cerr << name() << ": Write access outside memory range! ("
		          << a << ")" << std::endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	} else {
#ifdef DEBUG
		std::cout << name() << ": Write access at " << std::showbase
		          << std::hex << a << " (Data: " << std::showbase << d << ")" << std::endl;
#endif
		storage[a / sizeof(ensitlm::data_t)] = d;
		return tlm::TLM_OK_RESPONSE;
	}
}
