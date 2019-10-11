#ifndef QUICKCHANNEL_H
#define QUICKCHANNEL_H

#include "systemc"

struct quick_if : virtual sc_core::sc_interface
{
	virtual void coucou() = 0;
};

struct QuickChannel : sc_core::sc_channel, quick_if
{
	SC_CTOR(QuickChannel) {}

	void coucou() {
		std::cout << name() << " : coucou" << std::endl;
	}
};

#endif
