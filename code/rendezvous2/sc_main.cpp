#include <systemc>
#include "Producteur.h"
#include "Consommateur.h"
#include "RendezVous.h"

using namespace std;

int sc_main(int, char**)
{
	Consommateur    consommateur1("Consommateur1");
	Producteur      producteur1("Producteur1");
	Producteur      producteur2("Producteur2");
	Consommateur    consommateur2("Consommateur2");
	RendezVous<int> rendezvous1("RendezVous1"), rendezvous2("RendezVous2");

	producteur1.sortie(rendezvous1);
	consommateur1.entree(rendezvous1);

	producteur2.sortie(rendezvous2);
	consommateur2.entree(rendezvous2);

	sc_start();

	return 0;
}

