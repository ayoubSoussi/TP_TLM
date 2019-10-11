/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag                        *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/* Plateforme SystemC utilisant le protocole TLM Ensitlm.

   Cette version est en un seul fichier pour faciliter la
   compréhension. Une version multi-fichiers (donc plus propre) est
   également disponible.
*/

#include "ensitlm.h"
#include "bus.h"

using namespace std;
using namespace sc_core;

struct initiator : sc_module {
	ensitlm::initiator_socket<initiator> socket;
	void thread(void) {
		ensitlm::data_t val = 1;
		ensitlm::addr_t addr = 4;
		while (true) {
			cout << "Entrer un nombre" << endl;
			cin >> val;
			cout << "je vais envoyer : " << std::dec << val << endl;
			socket.write(addr, val);
		}
	}
	SC_CTOR(initiator) {
		SC_THREAD(thread);
	}
};

struct target : sc_module {
	ensitlm::target_socket<target> socket;
	tlm::tlm_response_status write(const ensitlm::addr_t &a,
	                               const ensitlm::data_t &d) {
		cout << "j'ai reçu : " << d << endl;
		return tlm::TLM_OK_RESPONSE;
	}
	tlm::tlm_response_status read(const ensitlm::addr_t &a,
	                              /* */ ensitlm::data_t &d) {
		SC_REPORT_ERROR("TLM", "non implémenté");
		abort();
	}
	SC_CTOR(target) { /* */	}
};

int sc_main(int argc, char **argv) {
	/*
	 +---------+	+-------------+	   +--------+
	 |	  +++  +-+	     +++  +++	    |
	 | Alice  | +--+ |  Router   | +--+ |  Bob  |
	 |	  +++  +-+	     +++  +++	    |
	 +---------+	+-------------+	   +--------+
	 */

	initiator a("Alice");
	target b("Bob");
	Bus router("Router");

	/* Bob is mapped at addresses [0, 100[, i.e. ... */
	router.map(b.socket, 0, 100);

	/* connect components to the bus */
	a.socket.bind(router.target);
	router.initiator.bind(b.socket);

	/* and start simulation */
	sc_start();
	return 0;
}
