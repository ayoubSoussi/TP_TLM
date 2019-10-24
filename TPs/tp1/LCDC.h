// LCD Controller
// (c) 2005 Jerome Cornet
//     2007 Matthieu Moy

#ifndef LCDC_H
#define LCDC_H

#include "ensitlm.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct LCDC : sc_core::sc_module {
	ensitlm::initiator_socket<LCDC> initiator_socket;
	ensitlm::target_socket<LCDC> target_socket;
	sc_core::sc_out<bool> display_intr;

	SC_HAS_PROCESS(LCDC);
	LCDC(sc_core::sc_module_name name,
	     const sc_core::sc_time &display_period);

	~LCDC();

	void compute();
	void draw();
	void fill_buffer();

	void end_of_elaboration();
	void init_colormap();

	tlm::tlm_response_status read(const ensitlm::addr_t &a,
	                              ensitlm::data_t &d);

	tlm::tlm_response_status write(const ensitlm::addr_t &a,
	                               const ensitlm::data_t &d);

	Display *display;
	Window window;
	int screen;
	int depth;
	GC gc;
	char *buffer;
	XImage *image;
	Colormap cmap;
	int color_table[256];

	unsigned long addr_register;
	unsigned long intr_register;

	bool started;
	sc_core::sc_event start_event;

	sc_core::sc_time period;

	static const int kWidth;
	static const int kHeight;
};

#endif
