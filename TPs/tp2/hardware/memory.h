#ifndef MEMORY_H
#define MEMORY_H

#include "ensitlm.h"

SC_MODULE(Memory) {
	ensitlm::target_socket<Memory> target;

	Memory(sc_core::sc_module_name name, unsigned int size);

	~Memory();

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t & d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

private:
	unsigned int m_size;

public:
	/* The loader must have access to the storage */
	ensitlm::data_t *storage;
};

#endif
