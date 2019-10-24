/********************************************************************
 * Copyright (C) 2012 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file uart.h
  \brief UART module


*/
#ifndef UART_H
#define UART_H

#include "ensitlm.h"

class UART : public sc_core::sc_module {
public:
	ensitlm::target_socket<UART> target;

	tlm::tlm_response_status read(ensitlm::addr_t a, ensitlm::data_t &d);

	tlm::tlm_response_status write(ensitlm::addr_t a, ensitlm::data_t d);

	SC_CTOR(UART){
		// Make output unbuffed so that we instantly see each char
		std::cout.setf(std::ios::unitbuf);
	};

private:
};

#endif // UART_H
