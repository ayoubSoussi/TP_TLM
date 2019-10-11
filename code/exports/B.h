#ifndef B_H
#define B_H

#include "systemc"
#include "QuickChannel.h"

SC_MODULE(B)
{
	sc_core::sc_port<quick_if>     port1;
	sc_core::sc_port<quick_if>     port2;

	SC_CTOR(B);

	void process();
};

#endif
