#include "Compteur.h"

Compteur::Compteur(sc_module_name name) : sc_module(name)
{
	SC_METHOD(calcul);
	sensitive << clk.pos();
	sensitive << reset;
}

void Compteur::calcul()
{
	if (reset.read() == true)
		count.write(0);
	else if (clk.posedge())
	{
		count.write(count.read() + 1);
	}
}
