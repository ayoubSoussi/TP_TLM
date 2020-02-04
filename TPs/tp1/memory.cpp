#include "memory.h"

using namespace std;

tlm::tlm_response_status memory::write(const ensitlm::addr_t &a,
																			const ensitlm::data_t &d) {
  // if the address is valid
	if (a < (unsigned int)size && a%4 == 0) {
    storage[a/4] = d;
    return tlm::TLM_OK_RESPONSE;
  }
  else {
    //report an error if the adress is not valid
    SC_REPORT_ERROR("write","writing in an invalid memory address");
    abort();
  }
}

tlm::tlm_response_status memory::read(const ensitlm::addr_t &a,
				            								ensitlm::data_t &d) {
  if (a < (unsigned int)size && a%4 == 0) {
    d = storage[a/4];
    return tlm::TLM_OK_RESPONSE;
  }
  else {
    //report an error if the adress is not valid
    SC_REPORT_ERROR("read","reading in an invalid memory address");
    abort();
  }
}

memory::memory(sc_core::sc_module_name name, int memory_size)
 									: sc_core::sc_module(name) {
  // memory_size is the size of storage in Bytes
  size = memory_size;
  storage = new ensitlm::data_t[(int)size/4];
}
memory::~memory() {
  delete storage;
}
