#include "AetB.h"

AetB::AetB(sc_core::sc_module_name name)
	: sc_module(name),
	  a(sc_core::sc_gen_unique_name("A")),
	  b(sc_core::sc_gen_unique_name("B")),
	  s("bonchannel")
{
	a.port2(s);
	b.port1(s);
	
	a.port1(port1);
	b.port2(port2);
	
	port3.bind(s);
}

