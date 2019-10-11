#ifndef A_H
#define A_H

#include "systemc"
#include "QuickChannel.h"

SC_MODULE(A)
{
	sc_core::sc_port<quick_if>     port1;
	sc_core::sc_port<quick_if>     port2;

	SC_CTOR(A);

	void process();
};

#endif
