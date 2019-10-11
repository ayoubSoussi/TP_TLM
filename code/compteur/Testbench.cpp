#include "Testbench.h"

Testbench::Testbench(sc_module_name name) : sc_module(name)
{
	SC_THREAD(genClk);
	SC_THREAD(genReset);
}

void Testbench::genClk()
{
	clk.write(false);
	wait(10, SC_NS);

	while (true)
	{
		wait(3, SC_NS);
		if (clk.read() == false)
			clk.write(true);
		else
			clk.write(false);
	}
}

void Testbench::genReset()
{
	reset.write(false);
	wait(2,SC_NS);
	reset.write(true);
	wait(5,SC_NS);
	reset.write(false);
}
