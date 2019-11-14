/*\
 * vim: tw=0: cindent: sw=3: ts=3: sts=3: noet: nolist:
 * Copyright (C) 2009, 2012 by Verimag
 * Initial author: Matthieu Moy
 * riscv support : Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr
\*/

/*\
 * Harwdare Abstraction Layer : implementation for risc-v
\*/
#ifndef HAL_H
#define HAL_H

#include <stdint.h>


/* Dummy implementation of abort(): invalid instruction */
#define abort() do {				\
	printf("abort() function called\r\n");  \
	_hw_exception_handler();		\
} while (0)

/* TODO: implement HAL primitives for cross-compilation */
#define hal_read32(a)      abort()
#define hal_write32(a, d)  abort()
#define hal_wait_for_irq() abort()
#define hal_cpu_relax()    abort()

static inline void enable_interrupts(void) {
	__asm("li    t0, 0x8\n"
	      "csrs  mstatus, t0\n"
			"li    t0, 0x800\n"
			"csrs  mie, t0");
}

/* TODO: printf is disabled, for now ... */
#define printf(...) do {} while(0)

#endif /* HAL_H */
