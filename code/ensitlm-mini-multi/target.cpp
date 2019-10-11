/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag			    *
 * Initial author: Matthieu Moy					    *
 ********************************************************************/

#include "target.h"

using namespace std;

tlm::tlm_response_status target::write(const ensitlm::addr_t &a,
                                       const ensitlm::data_t &d) {
	cout << "j'ai reçu : " << d << endl;
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status target::read(const ensitlm::addr_t &a,
                                      /* */ ensitlm::data_t &d) {
	SC_REPORT_ERROR("TLM", "non implémenté");
	abort();
}
