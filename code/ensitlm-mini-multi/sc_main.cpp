/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag			    *
 * Initial author: Matthieu Moy					    *
 ********************************************************************/

#include "ensitlm.h"
#include "initiator.h"
#include "target.h"
#include "bus.h"

int sc_main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	/*
	  +---------+	 +-------------+    +--------+
	  |	   +++	+-+	      +++  +++	     |
	  | Alice  | +--+ |  Router   | +--+ |	Bob  |
	  |	   +++	+-+	      +++  +++	     |
	  +---------+	 +-------------+    +--------+
	*/

	initiator a("Alice");
	target b("Bob");
	Bus router("Router");

	/* Bob is mapped at addresses [0, 100[, i.e. ... */
	router.map(b.socket, 0, 100);

	/* connect components to the bus */
	a.socket.bind(router.target);
	router.initiator.bind(b.socket);

	/* and start simulation */
	sc_core::sc_start();
	return 0;
}
