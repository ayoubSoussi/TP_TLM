#include "Consommateur.h"

Consommateur::Consommateur(sc_module_name name) : sc_module(name)
{
	SC_THREAD(consommation);
}

void Consommateur::consommation()
{
	while (true)
	{
		int valeur_recue = entree->get();

                std::cout << "Recu : " << valeur_recue << std::endl;
	}
}
