#include "ROM.h"

#include <iostream>

#include "ROM_content.h"
#include "ensitlm.h"

using namespace std;
using namespace sc_core;

ROM::ROM(sc_module_name name) : sc_module(name) {
	content = testimg;
}

tlm::tlm_response_status ROM::read(const ensitlm::addr_t &a,
                                   ensitlm::data_t &d) {
	assert(a < sizeof(testimg));
	d = content[a / sizeof(ensitlm::data_t)];
#ifdef DEBUG
	cout << name() << ": read(" << a << ", " << d << ");" << endl;
#endif
	return tlm::TLM_OK_RESPONSE;
}

ROM::~ROM() {
	/* */
}
