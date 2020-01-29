/********************************************************************
 * Copyright (C) 2012 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file uart.cpp
  \brief body for the file uart.h


*/

#include "uart.h"
#include "offsets/uart.h"

using namespace std;

tlm::tlm_response_status UART::write(ensitlm::addr_t a, ensitlm::data_t d) {
	char c = (char)d;
	switch (a) {
	case UART_FIFO_WRITE:
#ifdef DEBUG
		std::cout << "Debug: " << name() << "fifo access"  << "\n";
#endif
		if (c == '\n') {
			cout << endl;
		} else {
			cout << c;
		}
		break;
	default:
		SC_REPORT_ERROR(name(), "write register not implemented");
		return tlm::TLM_ADDRESS_ERROR_RESPONSE;
	}
	return tlm::TLM_OK_RESPONSE;
}

tlm::tlm_response_status UART::read(ensitlm::addr_t a, ensitlm::data_t &d) {
	(void)a;
	(void)d;
	SC_REPORT_ERROR(name(), "read not implemented");
	return tlm::TLM_ADDRESS_ERROR_RESPONSE;
}
