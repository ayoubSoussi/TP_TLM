#include <systemc>

SC_MODULE(Add8bits)
{
	sc_core::sc_in<sc_core::sc_uint<8> > a, b;
	sc_core::sc_out<sc_core::sc_uint<8> > c;

	SC_CTOR(Add8bits);

	void calcul();
};
