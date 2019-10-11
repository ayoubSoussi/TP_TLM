#include "Producteur.h"

Producteur::Producteur(sc_module_name name) : sc_module(name)
{
	SC_THREAD(production);
}

void Producteur::production()
{
	for (int i=0; i<10; i++)
	{
                std::cout << "Envoi de " << i << std::endl;
	
		sortie->put(i);
	}
}
