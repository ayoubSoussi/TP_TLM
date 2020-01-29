#include "ensitlm.h"
#include "bus.h"

SC_MODULE(memory){
	ensitlm::target_socket<memory> socket;
	ensitlm::data_t *storage;
	int size;
	tlm::tlm_response_status write(const ensitlm::addr_t &a,
				       const ensitlm::data_t &d);

	tlm::tlm_response_status read(const ensitlm::addr_t &a,
				            ensitlm::data_t &d);
	
	// the constructor 
	SC_HAS_PROCESS(memory);
	memory(sc_core::sc_module_name name, int memory_size);
	// the destructor
	~memory();
};
