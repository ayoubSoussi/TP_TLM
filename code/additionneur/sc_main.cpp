#include <systemc>
#include "Add8bits.h"
#include "Testbench.h"

using namespace std;

int sc_main(int, char**)
{
	Add8bits  add("Add");
	Testbench testbench("Testbench");
	sc_core::sc_signal<sc_core::sc_uint<8> > sa, sb, sc;

	testbench.a(sa);
	testbench.b(sb);
	testbench.c(sc);

	add.a(sa);
	add.b(sb);
	add.c(sc);

	sc_trace_file *trace;

	trace = sc_create_vcd_trace_file("trace");

	sc_trace(trace, sa, "a");
	sc_trace(trace, sb, "b");
	sc_trace(trace, sc, "c");

	sc_start(500, SC_NS);

	sc_close_vcd_trace_file(trace);

	return 0;
}

