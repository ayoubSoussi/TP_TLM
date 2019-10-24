/*
 * vim: tw=0: ci: sw=4: ts=4: noet: list
 */
#include "ensitlm.h"
#include "vga.h"
#include "offsets/vga.h"
#include "bit_manipulation.h"

#if 0
#define DEBUG
#endif

#define INFO

const sc_core::sc_time period((double)1 / VGA_FREQUENCY, sc_core::SC_SEC);

static int filter(void *userdata, SDL_Event *event)
{
	switch (event->type) {
	case SDL_QUIT:
		sc_core::sc_stop();
		break;
	default:
		break;
	}
	return 0;
}

Uint16 black;
Uint16 white;

Vga::Vga(sc_core::sc_module_name name)
    : sc_core::sc_module(name), address(0), intr(false)
{

	SC_THREAD(thread);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SC_REPORT_FATAL(sc_module::name(), SDL_GetError());
	}

	atexit(SDL_Quit);

	/* Monkey code: doing what I am told to do, but without really getting the point, ... */
	SDL_Window *window = SDL_CreateWindow(sc_module::name(),
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      SDL_WINDOWPOS_UNDEFINED,
	                                      VGA_WIDTH, VGA_HEIGHT,
	                                      0);
	assert(window != NULL);
	renderer = SDL_CreateRenderer(window, -1, 0);
	assert(renderer != NULL);
	/* We're running on a little endian host, so the masks are as follows */
	screen = SDL_CreateRGBSurface(0, VGA_WIDTH, VGA_HEIGHT, 32,
	                              0x000000ff,
	                              0x0000ff00,
	                              0x00ff0000,
	                              0xff000000);
	assert(screen != NULL);
	//texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, VGA_WIDTH, VGA_HEIGHT);
	texture = SDL_CreateTextureFromSurface(renderer, screen);
	assert(texture != NULL);


	if (screen->format->BytesPerPixel != 4) {
		SC_REPORT_FATAL(sc_module::name(), SDL_GetError());
	}

	black = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	white = SDL_MapRGB(screen->format, 0xff, 0xff, 0xff);

	SDL_SetEventFilter(filter, NULL);


#ifdef DEBUG
	std::cout << "Debug: " << sc_module::name()
	          << ": LCD controller TLM model\n";
#endif
}

void Vga::thread()
{
	while (true) {
		wait(period);
		if (address != 0) {
			vsync();
		}
	}
}

void Vga::vsync()
{
	SDL_PumpEvents();
	draw();

#ifdef DEBUG
	std::cout << "Debug: " << sc_module::name() << ": vsync @ "
	          << sc_core::sc_time_stamp() << "\n";
#endif

	intr = true;
	irq.write(1);
	wait(sc_core::SC_ZERO_TIME);
	irq.write(0);
}

void Vga::draw()
{
	if (SDL_MUSTLOCK(screen)) {
		SDL_LockSurface(screen);
	}

	for (int y = 0; y < VGA_HEIGHT; ++y) {
		for (int x = 0;
			 x < VGA_WIDTH;
			 x += (sizeof(ensitlm::data_t) * CHAR_BIT)) {

			ensitlm::data_t d;
			initiator.read(address + ((x + (y * VGA_WIDTH)) / CHAR_BIT), d);

			for (unsigned int bit = 0;
				bit < (sizeof(ensitlm::data_t) * CHAR_BIT); ++bit) {
				Uint32 offset = x + y * VGA_WIDTH + (sizeof(ensitlm::data_t) * CHAR_BIT - 1) - bit;
				Uint32 *bufp = (Uint32 *)screen->pixels + offset;
				if (TEST_BIT(d, bit)) {
					*bufp = 0xffffffff;
				} else {
					*bufp = 0xff000000;
				}
			}
		}
	}

	if (SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
	SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

tlm::tlm_response_status Vga::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	switch (a) {
	case VGA_CFG_OFFSET:
		d = address;
		break;
	case VGA_STT_OFFSET:
		d = 0xFFFFFFFF;
		break;
	case VGA_INT_OFFSET:
		d = intr;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status Vga::write(ensitlm::addr_t a, ensitlm::data_t d) {
	switch (a) {
	case VGA_CFG_OFFSET:
		address = d;
#ifdef INFO
		std::cout << name() << ": VGA_CFG_OFFSET changed to "
		          << std::hex << address << std::endl;
#endif
		break;
	case VGA_STT_OFFSET:
		/* see VHDL */
		break;
	case VGA_INT_OFFSET:
		intr = false;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}
