#ifndef PRODUCTEUR_H
#define PRODUCTEUR_H

#include <systemc>
#include "rendezvous_ifs.h"

SC_MODULE(Producteur)
{
	sc_port<rendezvous_out_if<int> >    sortie;

	SC_CTOR(Producteur);

	void production();
};

#endif
