#ifndef AB_H
#define AB_H

#include "systemc"
#include "A.h"
#include "B.h"
#include "QuickChannel.h"

SC_MODULE(AetB)
{
	sc_core::sc_port<quick_if> port1;
	sc_core::sc_port<quick_if> port2;
	sc_core::sc_export<quick_if> port3;

	SC_CTOR(AetB);

	A a;
	B b;
	QuickChannel s;
};

#endif
