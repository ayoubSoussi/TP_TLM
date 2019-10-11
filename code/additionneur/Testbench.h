#include <systemc>

SC_MODULE(Testbench)
{
	sc_core::sc_out<sc_uint<8> > a, b;
	sc_core::sc_in<sc_uint<8> > c;

	SC_CTOR(Testbench);

	void test();
};
