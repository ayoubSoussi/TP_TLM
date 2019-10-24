/********************************************************************
 * Copyright (C) 2016 by Verimag                                    *
 * Initial author: Matthieu Moy                                     *
 * This file must not be distributed outside Verimag                *
 ********************************************************************/

#ifndef INTC_OFFSETS_H
#define INTC_OFFSETS_H

/* From xintc_l.h */

#define XIN_ISR_OFFSET      0	/* Interrupt Status Register */
#define XIN_IPR_OFFSET      4	/* Interrupt Pending Register */
#define XIN_IER_OFFSET      8	/* Interrupt Enable Register */
#define XIN_IAR_OFFSET      12	/* Interrupt Acknowledge Register */
#define XIN_SIE_OFFSET      16	/* Set Interrupt Enable Register */
#define XIN_CIE_OFFSET      20	/* Clear Interrupt Enable Register */
#define XIN_IVR_OFFSET      24	/* Interrupt Vector Register */
#define XIN_MER_OFFSET      28	/* Master Enable Register */
#define XIN_IMR_OFFSET      32	/* Interrupt Mode Register , this is present
				 *  only for Fast Interrupt */
#define XIN_ILR_OFFSET      36  /* Interrupt level register */
#define XIN_IVAR_OFFSET     0x100 /* Interrupt Vector Address Register
				   * Interrupt 0 Offest, this is present
				   * only for Fast Interrupt */

#endif // INTC_OFFSETS_H
