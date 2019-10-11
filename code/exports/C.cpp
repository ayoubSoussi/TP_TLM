#include "C.h"

C::C(sc_core::sc_module_name name) : sc_core::sc_module(name)
{
	SC_THREAD(process);
}

void C::process()
{
	port3->coucou();
}
