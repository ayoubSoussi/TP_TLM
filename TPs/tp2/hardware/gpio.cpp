#include "ensitlm.h"
#include "gpio.h"
#include "offsets/gpio.h"
#include "bit_manipulation.h"

#include <SDL.h>

//#define DEBUG

Gpio::Gpio(sc_core::sc_module_name name) : sc_core::sc_module(name) {
#ifdef DEBUG
	std::cout << "Debug: " << sc_module::name() << ": GPIO TLM model\n";
#endif
}

tlm::tlm_response_status Gpio::read(const ensitlm::addr_t &a,
                                    ensitlm::data_t &d) {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	switch (a) {
	case GPIO_DATA_OFFSET:
		d = 0;
		if (keystate[SDLK_x]) {
			SET_BIT(d, GPIO_BTN0);
		}
		if (keystate[SDLK_c]) {
			SET_BIT(d, GPIO_BTN1);
		}
		if (keystate[SDLK_v]) {
			SET_BIT(d, GPIO_BTN2);
		}
#ifdef DEBUG
		std::cout << name() << ": Read GPIO_DATA_OFFSET = " << std::hex
		          << d << std::endl;
#endif
		break;
	case GPIO_TRI_OFFSET:
		d = tri;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status Gpio::write(const ensitlm::addr_t &a,
                                     const ensitlm::data_t &d) {
	switch (a) {
	case GPIO_DATA_OFFSET:
		data = d;
		break;
	case GPIO_TRI_OFFSET:
		tri = d;
		break;
	default:
		SC_REPORT_ERROR(name(), "register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}
