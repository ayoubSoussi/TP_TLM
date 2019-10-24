/********************************************************************
 * Copyright (C) 2009, 2012 by Verimag                              *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file hal.h
  \brief Hardware Abstraction Layer : implementation for native
  simulation in SystemC.


*/
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

/*
 * The C compiler does a bit of magic on the main() function. Trick it
 * by changing the name to something else.
 */
#define main __start

/* fonctions déclarées ici, et implémentées dans native_wrapper.cpp */
extern void     hal_write32(uint32_t addr, uint32_t data);
extern uint32_t hal_read32(uint32_t addr);
extern void     hal_cpu_relax(void);
extern void     hal_wait_for_irq(void);

/* Nothing: we assume IT are active all the time */
static inline void enable_interrupts(void) {};

#endif /* HAL_H */
