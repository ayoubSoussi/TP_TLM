#ifndef CONSOMMATEUR_H
#define CONSOMMATEUR_H

#include <systemc>
#include "rendezvous_ifs.h"

SC_MODULE(Consommateur)
{
	sc_port<rendezvous_in_if<int> >    entree;

	SC_CTOR(Consommateur);

	void consommation();
};

#endif
