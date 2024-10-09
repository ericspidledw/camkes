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

#define VID 0x067b //Prolific Technology Inc.
#define DID 0x2303 //USB-Serial Controller

char *str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\n";

static inline void udelay(uint32_t us)
{
	volatile uint32_t i;
	for (; us > 0; us--) {
		for (i = 0; i < 1000; i++);
	}
}

int run()
{
	uintptr_t pl2303 = 0;

	printf("Starting the PL2303 adaptor\n");
	printf("-------------------\n");

	/* Find the device */
	while (!pl2303) {
		pl2303 = cdc_find(VID, DID);
		udelay(5000);
	}
	printf("Found PL2303 it is at addr %p\n", pl2303);

	/* Connect to the sub class driver */
	cdc_connect(pl2303); // call connect

	cdc_configure(pl2303, 115200, 1, 0, 8); // then configure...

	/* Send Data */
	// memcpy(fdata, str, strlen(str));
	// printf("Strlength of str is %d\n", strlen(str));
	// cdc_write(pl2303, strlen(str)); // then write...

	while(1){
		int chars_read = cdc_read(pl2303, sizeof(char));
		if(chars_read){
			ZF_LOGE("Read %d chars", chars_read);
			while(1);
		}
		udelay(500000);
	}

	printf("Well done, PL2303\n");
	return 0;
}
