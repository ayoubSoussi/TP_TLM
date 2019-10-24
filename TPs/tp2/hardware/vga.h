/*
 * vim: tw=0: ci: sw=4: ts=4: sts=2: lbr: noet: list
 */
#ifndef VGA_H
#define VGA_H

#include "ensitlm.h"

#include <SDL.h>

SC_MODULE(Vga) {
	SC_HAS_PROCESS(Vga);

	ensitlm::initiator_socket<Vga> initiator;
	ensitlm::target_socket<Vga> target;

	sc_core::sc_out<bool> irq;

	explicit Vga(sc_core::sc_module_name name);

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t & d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

private:
	ensitlm::addr_t address;
	bool intr;

	/* sdl2 objects useful in different parts of the code */
	SDL_Surface *screen;
	SDL_Texture *texture;
	SDL_Renderer *renderer;

	void vsync();
	void thread();
	void draw();
};

#endif
