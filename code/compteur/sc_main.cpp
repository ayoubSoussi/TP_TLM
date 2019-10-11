#include <systemc>
#include "Compteur.h"
#include "Testbench.h"

using namespace std;

int sc_main(int, char**)
{
	Compteur               compteur("Compteur");
	Testbench              testbench("Testbench");
	sc_signal<bool>        sclk, sreset;
	sc_signal<sc_uint<8> > scount;

	testbench.clk(sclk);
	testbench.reset(sreset);
	testbench.count(scount);

	compteur.clk(sclk);
	compteur.reset(sreset);
	compteur.count(scount);

	sc_trace_file *trace;

	trace = sc_create_vcd_trace_file("trace");

	sc_trace(trace, sclk, "clk");
	sc_trace(trace, sreset, "reset");
	sc_trace(trace, scount, "count");

	sc_start(500, SC_NS);

	sc_close_vcd_trace_file(trace);

	return 0;
}

