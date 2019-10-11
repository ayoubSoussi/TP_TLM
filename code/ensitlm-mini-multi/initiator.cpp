/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag                        *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

#include "initiator.h"

using namespace std;

void initiator::thread(void) {
	ensitlm::data_t val = 1;
	ensitlm::addr_t addr = 4;
	while (true) {
		cout << "Entrer un nombre" << endl;
		cin >> val;
		cout << "je vais envoyer : " << std::dec << val << endl;
		socket.write(addr, val);
	}
}

initiator::initiator(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	SC_THREAD(thread);
}
