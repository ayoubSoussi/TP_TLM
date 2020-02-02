#include "generator.h"
#include "LCDC_registermap.h"

#define MEMORY_START 12
#define MEMORY_SIZE 240 * 320 + 10 * 1024

#define LCD_START 12
#define LCD_SIZE 76800

using namespace std;


void generator::thread(void){
  ensitlm::addr_t memory_addr = MEMORY_START;
	ensitlm::addr_t rom_addr = MEMORY_START + MEMORY_SIZE;
	ensitlm::data_t data_pixels, var;

  // writing to the registers of the LCDC
  socket.write(LCDC_ADDR_REG, MEMORY_START);
  socket.write(LCDC_START_REG, 0X00000001);

  for (int i = 0; i < 76800/8; i++){
  	// read from ROM
  	socket.read(rom_addr, data_pixels);

  	var = 0;
  	var = var | (data_pixels & 0xF0000000);
  	var = var | ((data_pixels & 0x0F000000) >> 4);
  	var = var | ((data_pixels & 0x00F00000) >> 8);
  	var = var | ((data_pixels & 0x000F0000) >> 12);
  	socket.write(memory_addr, var);

  	var = 0;
  	var = var | ((data_pixels & 0x0000F000) << 16);
  	var = var | ((data_pixels & 0x00000F00) << 12);
  	var = var | ((data_pixels & 0x000000F0) << 8);
  	var = var | ((data_pixels & 0x0000000F) << 4);
    memory_addr += 4;
    socket.write(memory_addr, data_pixels);

    memory_addr += 4;
  	rom_addr += 4;
  }
}

generator::generator(sc_core::sc_module_name name) : sc_core::sc_module(name) {
	SC_THREAD(thread);
}
