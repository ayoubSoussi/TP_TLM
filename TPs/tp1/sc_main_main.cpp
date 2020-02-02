#include "ensitlm.h"
#include "generator.h"
#include "memory.h"
#include "bus.h"
#include "LCDC.h"
#include "ROM.h"

#define MEMORY_START 12
#define MEMORY_SIZE 240 * 320 + 10 * 1024

#define LCD_REG_START 0
#define LCD_REG_SIZE 12

#define ROM_START MEMORY_START + MEMORY_SIZE
#define ROM_SIZE 320 * 240 / 2

using namespace std;
using namespace sc_core;

int sc_main(int argc, char **argv){
        // Add 3 registers of 4 bytes each to the memory
	generator Generator("Generateur");
	memory Memory("memoire",MEMORY_SIZE);
	ROM rom("rom");
	Bus bus("Bus");
  LCDC lcdc("ControllerLCDC", sc_time(1.0/25, SC_SEC));
	sc_signal<bool> s("s");

	// Addresses mapping
	bus.map(lcdc.target_socket,LCD_REG_START,LCD_REG_SIZE);
  bus.map(Memory.socket,MEMORY_START,MEMORY_SIZE);
	bus.map(rom.socket,ROM_START , ROM_SIZE);
	//initiators
	Generator.socket.bind(bus.target);
	lcdc.initiator_socket(bus.target);
	//targets
	bus.initiator(Memory.socket);
	bus.initiator(rom.socket);
	bus.initiator(lcdc.target_socket);
	// binding the interruption ports
	lcdc.display_intr.bind(s);
	Generator.display_intr_in.bind(s);

	sc_start();

	return 0 ;
}
