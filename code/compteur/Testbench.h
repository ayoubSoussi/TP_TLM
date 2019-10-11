#include <systemc>

SC_MODULE(Testbench)
{
	sc_out<bool>         clk;
	sc_out<bool>         reset;
	sc_in<sc_uint<8> >   count;

	SC_CTOR(Testbench);

	void genClk();
	void genReset();
};
