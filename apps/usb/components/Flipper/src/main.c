/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the term of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(D61_BSD)
 */

#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef CONFIG_ARCH_X86
#define VID 0x03EB //Atmel
#define DID 0x0008 //Flipper
#else
#define VID 0x1d50 //Black Sphere Tech
#define DID 0x6018 //Black Magic Probe
#endif

extern void set_flipper_postion(uintptr_t udev, int angle, int velocity);
extern uint16_t report_flipper_postion(uintptr_t udev);
extern void set_flipper_effort(uintptr_t udev, uint16_t effort);
extern void clear_fault(uintptr_t udev, uint16_t fault);
extern void set_status(uintptr_t udev, uint8_t status);

static inline void udelay(uint32_t us)
{
	volatile uint32_t i;
	for (; us > 0; us--) {
		for (i = 0; i < 1000; i++);
	}
}

int run()
{
	uintptr_t flipper = 0;
	uint16_t angle = 0;
 	printf("dummy flipper comp\n");
	printf("Starting the Flipper\n");
	printf("-------------------\n");

	/* Find the device */
	while (!flipper) {
		flipper = cdc_find(VID, DID);
		udelay(5000);
	}
	printf("Found flipper\n");

	/* Connect to the sub class driver */
	cdc_connect(flipper);

	cdc_configure(flipper, 115200, 1, 0, 8);

	/* Send Data */
	clear_fault(flipper, 0xFFFF);
	set_status(flipper, 4);

	angle = report_flipper_postion(flipper);
	printf("Flipper angle: %u\n", angle);

	set_flipper_effort(flipper, 30000);

	angle = report_flipper_postion(flipper);
	printf("Flipper angle: %u\n", angle);

	printf("Well done, Flipper\n");
	return 0;
}
