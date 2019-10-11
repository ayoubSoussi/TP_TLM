/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag			    *
 * Initial author: Matthieu Moy					    *
 ********************************************************************/

#include "ensitlm.h"
#include "bus.h"

struct initiator : sc_core::sc_module {
	ensitlm::initiator_socket<initiator> socket;
	void thread(void);

	SC_CTOR(initiator);
};
