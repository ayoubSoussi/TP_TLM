#include <systemc>
#include "Producteur.h"
#include "Consommateur.h"

using namespace std;

int sc_main(int, char**)
{
	Producteur   producteur("Producteur");
	Consommateur consommateur("Consommateur");
	sc_fifo<int> fifo("Fifo", 4);

	producteur.sortie(fifo);
	consommateur.entree(fifo);

	sc_start();

	return 0;
}

