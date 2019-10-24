#ifndef TIMER_H_OFFSETS
#define TIMER_H_OFFSETS

#define TIMER_0_CSR_OFFSET   0x0 /* Timer 0 Control/Status Register */
#define TIMER_0_TLR_OFFSET   0x4 /* Timer 0 Load Register */
#define TIMER_0_TCR_OFFSET   0x8 /* Timer 0 Counter Register */
#define TIMER_1_CSR_OFFSET  0x10 /* Timer 1 Control/Status Register */
#define TIMER_1_TLR_OFFSET  0x14 /* Timer 1 Load Register */
#define TIMER_1_TCR_OFFSET  0x18 /* Timer 1 Counter Register */

/* bits 0:20 reserved */
#define TIMER_ENALL (32 - 21 - 1) /* Enable All Timers (not implemented) */
#define TIMER_PWMA  (32 - 22 - 1) /* Enable Pulse Width Modulation (not implemented) */
#define TIMER_TINT  (32 - 23 - 1) /* Interrupt */
#define TIMER_ENT   (32 - 24 - 1) /* Enable Timer */
#define TIMER_ENIT  (32 - 25 - 1) /* Enable Interrupt */
#define TIMER_LOAD  (32 - 26 - 1) /* Load */
#define TIMER_ARHT  (32 - 27 - 1) /* Auto Reload/Hold */
#define TIMER_CAPT  (32 - 28 - 1) /* Enable External Capture Trigger (not implemented) */
#define TIMER_GENT  (32 - 29 - 1) /* Enable External Generate Signal (not implemented) */
#define TIMER_UDT   (32 - 30 - 1) /* Up/Down Count */
#define TIMER_MTD   (32 - 31 - 1) /* Timer mode (not implemented) */

/* check (if read) or clear (if write) interrupt flag */
#define TIMER_INTERRUPT				\
	BIT(TIMER_TINT)

/* load and clear interrupt flag */
#define TIMER_RELOAD				\
	BIT(TIMER_TINT) | BIT(TIMER_LOAD)

/* start timer, enable irq, auto reload and count down */
#define TIMER_START				\
	BIT(TIMER_ENT) | BIT(TIMER_ENIT) |	\
	BIT(TIMER_ARHT) | BIT(TIMER_UDT)

#endif
