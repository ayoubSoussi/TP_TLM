#include "generator.h"
#include "LCDC_registermap.h"

#define MEMORY_BASE_ADDRESS 0x10000000
#define MVID_BASE_ADDRESS MEMORY_BASE_ADDRESS+10240
#define LCDC_BASE_ADDRESS MVID_BASE_ADDRESS+76800

#define MEM_START 12
#define MEM_SIZE 240 * 320 + 10 * 1024


using namespace std;


void generator::thread(void){
  cout << "hi" <<endl ;
  ensitlm::addr_t memory_addr = MEM_START;
  
  // writing to the registers of the LCDC
  socket.write(LCDC_ADDR_REG, MEM_START);
  socket.write(LCDC_START_REG, 0X00000001);

  for (int i = 0; i < 76800/4; i++){
    socket.write(memory_addr, 0xFFFFFFFF);
    memory_addr += 4;
    }

  
  // socket.write(LCDC_INT_REG, );
  
  

  
}

generator::generator(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	SC_THREAD(thread);
}

