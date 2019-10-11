#include "Add8bits.h"

Add8bits::Add8bits(sc_module_name name) : sc_core::sc_module(name)
{
	SC_METHOD(calcul);
	sensitive << a << b;
}

void Add8bits::calcul()
{
	c.write(a.read() + b.read());
}
