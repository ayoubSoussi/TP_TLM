#ifndef NATIVEWRAPPER_H
#define NATIVEWRAPPER_H

#include "ensitlm.h"

SC_MODULE(NativeWrapper) {

	ensitlm::initiator_socket<NativeWrapper> socket;
	sc_core::sc_in<bool>                            irq;

private:
	SC_CTOR(NativeWrapper);
public:
	/*
	   We use a singleton here. This is a limitation since it
	   doesn't allow multiple NativeWrapper instances (multiple CPU
	   in the platform), but it considerably reduces the
	   complexity of Makefiles, hal.h, ...
	*/
	static NativeWrapper * get_instance();

	void hal_write32(unsigned int addr, unsigned int data);
	unsigned int hal_read32(unsigned int addr);

	void hal_cpu_relax();
	void hal_wait_for_irq();

	void compute();

	void interrupt_handler_internal();

	bool              interrupt;
	sc_core::sc_event interrupt_event;
};

#endif
