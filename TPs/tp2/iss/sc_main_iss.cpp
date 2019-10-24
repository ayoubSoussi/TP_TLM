#include "ensitlm.h"

#include "rv32_wrapper.h"
#include "memory.h"
#include "bus.h"
#include "fast-bus.h"
#include "timer.h"
#include "uart.h"
#include "vga.h"
#include "intc.h"
#include "gpio.h"

#include "../address_map.h"

#include "../elf-loader/loader/include/loader.h"
#include "../elf-loader/loader/include/exception.h"

namespace soclib {
namespace common {
   extern bool elf_load(const std::string &filename,
                        soclib::common::Loader &loader);
   }
};
#define SOFT_SIZE 0xB000

int sc_main(int, char **) {
	RV32Wrapper cpu("risc-v");
	Memory inst_ram("inst_ram", INST_RAM_SIZE);
	Bus bus("bus");
	TIMER timer("timer", sc_core::sc_time(20, sc_core::SC_NS));
	Vga vga("vga");
	Intc intc("intc");
	Gpio gpio("gpio");

	sc_core::sc_signal<bool> timer_irq("timer_irq");
	sc_core::sc_signal<bool> vga_irq("vga_irq");
	sc_core::sc_signal<bool> cpu_irq("cpu_irq");

	// Load the program in RAM
	soclib::common::Loader::register_loader("elf", soclib::common::elf_load);
	try {
		soclib::common::Loader loader("../software/cross/a.out");
		loader.load(inst_ram.storage, 0x80000000, SOFT_SIZE);
		for (int i = 0; i < SOFT_SIZE / 4; i++) {
			inst_ram.storage[i] = uint32_le_to_machine(inst_ram.storage[i]);
		}
	} catch (soclib::exception::RunTimeError &e) {
		std::cerr << "unable to load ELF file in memory:" << std::endl;
		std::cerr << e.what() << std::endl;
		abort();
	}

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
