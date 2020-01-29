#include "ensitlm.h"
#include "generator.h"
#include "memory.h"
#include "bus.h"
#include "LCDC.h"

using namespace std;
using namespace sc_core;

int sc_main(int argc, char **argv){
        // Add 3 registers of 4 bytes each to the memory
	int plage_memoire = 0x00015400;
	generator Generator1("Generateur1");
	memory Memory("memoire",plage_memoire);
	Bus bus("Bus");
       	LCDC lcdc("ControllerLCDC", sc_time(1.0/25, SC_SEC));
	sc_signal<bool> s("s");

        bus.map(Memory.socket,0x10000000,plage_memoire);
	
	//initiators
	Generator1.socket.bind(bus.target);
	lcdc.initiator_socket(bus.target);
	//targets
	bus.initiator(Memory.socket);
	bus.initiator(lcdc.target_socket);

	bus.map(lcdc.target_socket,0x00000000,0x09000000); 
	// binding the interruption ports
	lcdc.display_intr.bind(s);
	Generator1.display_intr_in.bind(s);
	
	sc_start();		
	
	return 0 ;
}
