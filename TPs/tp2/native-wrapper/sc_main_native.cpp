#include "ensitlm.h"

#include "../address_map.h"
#include "native_wrapper.h"
#include "memory.h"
#include "bus.h"
#include "timer.h"
#include "vga.h"
#include "intc.h"
#include "gpio.h"

int sc_main(int, char**)
{
	NativeWrapper& cpu = *NativeWrapper::get_instance();
	Memory inst_ram("inst_ram", INST_RAM_SIZE);
	Bus bus("bus");
	TIMER timer("timer", sc_core::sc_time(20, sc_core::SC_NS));
	Vga vga("vga");
	Intc intc("intc");
	Gpio gpio("gpio");

	sc_core::sc_signal<bool> timer_irq("timer_irq");
	sc_core::sc_signal<bool> vga_irq("vga_irq");
	sc_core::sc_signal<bool> cpu_irq("cpu_irq");

	// initiators
	cpu.socket.bind(bus.target);
	vga.initiator(bus.target);

	// targets
	// bus.initiator(data_ram.target);
	bus.initiator(inst_ram.target);
	bus.initiator(vga.target);
	bus.initiator(timer.target);
	bus.initiator(gpio.target);
	bus.initiator(intc.target);

	// interrupts
	vga.irq(vga_irq);
	timer.irq(timer_irq);
	intc.in0(vga_irq);
	intc.in1(timer_irq);
	intc.out(cpu_irq);
	cpu.irq(cpu_irq);

	//      port             start addr         size
	bus.map(inst_ram.target, INST_RAM_BASEADDR, INST_RAM_SIZE);
	bus.map(vga.target,      VGA_BASEADDR,      VGA_SIZE);
	bus.map(gpio.target,     GPIO_BASEADDR,     GPIO_SIZE);
	bus.map(timer.target,    TIMER_BASEADDR,    TIMER_SIZE);
	bus.map(intc.target,     INTC_BASEADDR,     INTC_SIZE);

	// start the simulation
	sc_core::sc_start();

	return 0;
}
