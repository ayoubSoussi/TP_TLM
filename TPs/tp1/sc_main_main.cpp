#include "ensitlm.h"
#include "generator.h"
#include "memory.h"
#include "bus.h"
#include "LCDC.h"
#include "ROM.h"

#define MEM_START 12
#define MEM_SIZE 320 * 240 + 10 * 1024

#define LCD_MEM_START 12
#define LCD_MEM_SIZE 76800

#define LCD_REG_START 0
#define LCD_REG_SIZE 12

using namespace std;
using namespace sc_core;

int sc_main(int argc, char **argv){
        // Add 3 registers of 4 bytes each to the memory
	generator Generator1("Generateur1");
	memory Memory("memoire",MEM_SIZE);
	ROM rom("rom");
	
	
	Bus bus("Bus");
       	LCDC lcdc("ControllerLCDC", sc_time(1.0/25, SC_SEC));
	sc_signal<bool> s("s");

        bus.map(Memory.socket,MEM_START,MEM_SIZE);
	bus.map(rom.socket,MEM_START + MEM_SIZE, ROM_SIZE);
	
	//initiators
	Generator1.socket.bind(bus.target);
	lcdc.initiator_socket(bus.target);
	//targets
	bus.initiator(Memory.socket);
	bus.initiator(rom.socket);
	bus.initiator(lcdc.target_socket);

	bus.map(lcdc.target_socket,LCD_REG_START,LCD_REG_SIZE); 
	// binding the interruption ports
	lcdc.display_intr.bind(s);
	Generator1.display_intr_in.bind(s);
	
	sc_start();		
	
	return 0 ;
}
