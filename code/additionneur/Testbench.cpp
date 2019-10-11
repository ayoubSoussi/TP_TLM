#include "Testbench.h"

Testbench::Testbench(sc_module_name name) : sc_module(name)
{
	SC_THREAD(test);
}

void Testbench::test()
{
	while (true)
	{
		for (int i=0; i<16; i++)
		{
			for (int j=0; j<16; j++)
			{
				a.write(sc_uint<8>(i));
				b.write(sc_uint<8>(j));
			
				wait(5, SC_NS);
			}
		}
	}
}
