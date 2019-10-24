/********************************************************************
 * Copyright (C) 2017 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file bit_manipulations.h
  \brief Utilities to manipulate int as bit vectors  
*/
#ifndef BIT_MANIPULATIONS_H
#define BIT_MANIPULATIONS_H

#define ALIGN(addr)				\
	((addr) & (-sizeof(uint32_t)))

#define BIT(bit)				\
	(1 << (bit))

#define TEST_BIT(var, bit)			\
	((var) & BIT(bit))

#define SET_BIT(var, bit)			\
	do { (var) |= BIT(bit); } while(0)

#define CLEAR_BIT(var, bit)			\
	do { (var) &= ~BIT(bit); } while(0)

#endif // BIT_MANIPULATIONS_H
