/*
 * PRU0 Firmware for BeagleLogic
 * 
 * Copyright (C) 2014 Kumar Abhishek
 *
 * Licensed under the GNU GPL and provided "as-is" without any expressed 
 * or implied warranties / liabilities.
 */

/* We are compiling for PRU0 */
#define PRU0

#include "pru_syscalls.h"
#include "pru_defs.h"

/*
 * Define firmware version
 * This is version 0.2 [v0.1 was the PASM firmware]
 */
#define MAJORVER	0
#define MINORVER	2

/* Maximum number of SG entries; each entry is 8 bytes */
#define MAX_BUFLIST_ENTRIES	128

/* Downcall API. To be kept in sync with the kernel driver */
#define DC_SM_TOGGLE	8   /* Arm the LA (start sampling) */

/* Define magic bytes for the structure. This "looks like" BEAGLELO */
#define FW_MAGIC	0xBEA61E10

/* Just a basic structure describing the start and end buffer addresses */
typedef struct buflist {
	u32 dma_start_addr;
	u32 dma_end_addr;
} bufferlist;

/* Structure describing the context */
struct capture_context {
	u32 magic;
	u32 errorCode;

	u32 interrupt1count;

	bufferlist list[MAX_BUFLIST_ENTRIES];
} cxt = {0};


extern void sc_downcall(int (*handler)(u32 nr, u32 arg0, u32 arg1, 
	u32 arg2, u32 arg3, u32 arg4));


void toggle_led()
{	
        __R30 ^= 0xFFFF;
}
/* Handle downcalls */
static int handle_downcall(u32 id, u32 arg0, u32 arg1, u32 arg2,
		u32 arg3, u32 arg4) {
	switch (id)
	{
		case DC_SM_TOGGLE:
			toggle_led();
			return 0;
	}

	return -1;
}

extern void run(struct capture_context *ctx, u32 trigger_flags);

int main(void)
{	
	/* Enable OCP Master Port */
	PRUCFG_SYSCFG &= ~SYSCFG_STANDBY_INIT;
	cxt.magic = FW_MAGIC;
	while (1) {

		/* Poll for downcalls */
		if(PINTC_SRSR0 & BIT(SYSEV_ARM_TO_PRU0)) {
			PINTC_SICR = SYSEV_ARM_TO_PRU0;
			sc_downcall(handle_downcall);
		}
		
		
	}
}

