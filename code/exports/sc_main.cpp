#include "systemc"
#include "AetB.h"
#include "C.h"

using namespace std;
using namespace sc_core;

int sc_main(int, char**)
{
	AetB             aetb("AetB");
	C                c("C");
	QuickChannel     q1("q1"), q2("q2");

	aetb.port1.bind(q1);
	c.port1.bind(q1);

	aetb.port2.bind(q2);
	c.port2.bind(q2);

	c.port3.bind(aetb.port3);

	sc_start();

	return 0;
}

