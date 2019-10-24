// LCD Controller
// (c) 2005 Jerome Cornet
//     2007 Matthieu Moy

#include "LCDC.h"
#include "LCDC_registermap.h"
#include "ensitlm.h"

using namespace std;
using namespace sc_core;

// constants
const int LCDC::kWidth = 320;
const int LCDC::kHeight = 240;

// Constructor
LCDC::LCDC(sc_module_name name, const sc_time &display_period)
    : sc_module(name), period(display_period) {
	static XSizeHints size_hints;
	Window rootwin;
	int width, height, depth;
	XGCValues values;
	unsigned long valuemask = 0;
	Visual *visual;

	// registers initialisation
	addr_register = 0;
	intr_register = 0;
	started = false;

	// X11 Initialisation
	width = kWidth;
	height = kHeight;

	display = XOpenDisplay(NULL);

	if (display == NULL) {
		cerr << "Failed to open Display!" << endl;
		sc_stop();
	}

	screen = DefaultScreen(display);
	depth = DefaultDepth(display, screen);
	visual = DefaultVisual(display, screen);
	cmap = DefaultColormap(display, screen);
	rootwin = RootWindow(display, screen);
	window = XCreateSimpleWindow(display, rootwin, 10, 10, width, height, 5,
	                             BlackPixel(display, screen),
	                             BlackPixel(display, screen));

	size_hints.flags = PSize | PMinSize | PMaxSize;
	size_hints.min_width = width;
	size_hints.max_width = width;
	size_hints.min_height = height;
	size_hints.max_height = height;

	XSetWindowColormap(display, window, cmap);

	init_colormap();

	gc = XCreateGC(display, window, valuemask, &values);

	XSetStandardProperties(display, window, name, name, None, 0, 0,
	                       &size_hints);

	XSelectInput(display, window, ButtonPressMask | KeyPressMask);
	XMapWindow(display, window);

	buffer = 0;

	image = XCreateImage(display, visual, depth, ZPixmap, 0, buffer, width,
	                     height, 8, 0);
	{
		// tricks to have it working in non 8-bit depth
		int imgsize = image->height * image->bytes_per_line;
		buffer = (char *)malloc(imgsize);
		for (int i = 0; i < imgsize; i++)
			buffer[i] = 0;
	}
	image->data = buffer;

	XPutImage(display, window, gc, image, 0, 0, 0, 0, kWidth, kHeight);

	// SystemC threads declarations
	SC_THREAD(compute);
}

// Destructor
LCDC::~LCDC() {
	// Deallocate internal Ximage
	// also deallocate buffer
	XDestroyImage(image);

	// Close X connection
	XCloseDisplay(display);
}

// Others initialisations
void LCDC::end_of_elaboration() {
	display_intr.write(false);
}

// Colormap init
// The colormap is an array giving the nearest X11 color value
// corresponding to the 8 bit grayscale value passed in index
void LCDC::init_colormap() {
	XColor c;

	for (int i = 0; i < 256; i++) {
		c.red = i * 65535 / 255;
		c.green = i * 65355 / 255;
		c.blue = i * 65535 / 255;
		c.flags = DoRed | DoGreen | DoBlue;

		if (XAllocColor(display, cmap, &c)) {
			color_table[i] = c.pixel;
		} else {
			// If not enough color available
			if (cmap == DefaultColormap(display, screen)) {
				cmap = XCopyColormapAndFree(display, cmap);
				XSetWindowColormap(display, window, cmap);

				c.red = i * 65535 / 255;
				c.green = i * 65535 / 255;
				c.blue = i * 65535 / 255;
				c.flags = DoRed | DoGreen | DoBlue;

				if (XAllocColor(display, cmap, &c)) {
					color_table[i] = c.pixel;
				}
			}
		}
	}
}

// Read transactions
tlm::tlm_response_status LCDC::read(const ensitlm::addr_t &a,
                                    ensitlm::data_t &d) {
	switch (a) {
	case LCDC_ADDR_REG:
		d = addr_register;
		break;
	case LCDC_INT_REG:
		d = intr_register;
		break;
	default:
		cerr << name() << ": Read access outside register range!"
		     << endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

// Write transactions
tlm::tlm_response_status LCDC::write(const ensitlm::addr_t &a,
                                     const ensitlm::data_t &d) {
	switch (a) {
	case LCDC_ADDR_REG:
		addr_register = d;
		break;
	case LCDC_INT_REG:
		intr_register = d;
		if (intr_register == 0)
			display_intr.write(false);
		break;
	default:
		cerr << name() << ": Write access outside register range!"
		     << endl;
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

// main thread
void LCDC::compute() {
	while (!started) {
		wait(start_event);
	}

	cout << name() << ": LCDC starting" << endl;

	while (true) {
		wait(period);

		fill_buffer();
		draw();

		if (intr_register == 0) {
			cout << name() << ": sending display interrupt" << endl;
			intr_register = 1;
			display_intr.write(true);
		}
	}
}

// Transfer the internal XImage buffer to the lcd window
void LCDC::draw() {
	XPutImage(display, window, gc, image, 0, 0, 0, 0, kWidth, kHeight);
}

// Reads an external memory devices and fill the buffer corresponding to the
// internal XImage
void LCDC::fill_buffer() {
	ensitlm::addr_t a = addr_register;
	ensitlm::data_t d;
	tlm::tlm_response_status status;

	for (int y = 0; y < kHeight; y++) {
		for (int x = 0; x < kWidth / 4; x++) {
			status = initiator_socket.read(a, d);

			if (status != tlm::TLM_OK_RESPONSE) {
				cerr << name() << ": error while reading "
				                  "memory (address: 0x" << hex
				     << a << ")" << endl;
			} else {
				int sourcevalues[4];

				// extract each pixel from the 32 bits value
				// data is considered to be in big endian format
				sourcevalues[0] = (d & 0xFF000000) >> 24;
				sourcevalues[1] = (d & 0x00FF0000) >> 16;
				sourcevalues[2] = (d & 0x0000FF00) >> 8;
				sourcevalues[3] = (d & 0x000000FF);

				// blit the pixels in the buffer
				XPutPixel(image, x * 4, y,
				          color_table[sourcevalues[0]]);
				XPutPixel(image, x * 4 + 1, y,
				          color_table[sourcevalues[1]]);
				XPutPixel(image, x * 4 + 2, y,
				          color_table[sourcevalues[2]]);
				XPutPixel(image, x * 4 + 3, y,
				          color_table[sourcevalues[3]]);
			}

			a += 4;
		}
	}
}
