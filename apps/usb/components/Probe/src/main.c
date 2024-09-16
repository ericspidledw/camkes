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
#include <string.h>

#define VID 0x1d50 //Black Sphere Tech
#define DID 0x6018 //Black Magic Probe

char *str = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static inline void udelay(uint32_t us)
{
	volatile uint32_t i;
	for (; us > 0; us--) {
		for (i = 0; i < 1000; i++);
	}
}

int run()
{
	uintptr_t probe = NULL;

	printf("Starting the Black Magic Probe\n");
	printf("-------------------\n");

	/* Find the device */
	while (!probe) {
		probe = cdc_find(VID, DID);
		udelay(5000);
	}
	printf("Found Probe\n");

	/* Connect to the sub class driver */
	cdc_connect(probe);

	cdc_configure(probe, 115200, 1, 0, 8);

	/* Send Data */
	memcpy(fdata, str, strlen(str));
//	while (1) {
		udelay(2000);
		cdc_write(probe, strlen(str));
//	}
	printf("Well done, Probe!\n");

	return 0;
}
