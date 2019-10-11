#ifndef C_H
#define C_H

#include "systemc"
#include "QuickChannel.h"

SC_MODULE(C)
{
	sc_core::sc_port<quick_if>     port1;
	sc_core::sc_port<quick_if>     port2;
	sc_core::sc_port<quick_if>     port3;

	SC_CTOR(C);

	void process();
};

#endif
