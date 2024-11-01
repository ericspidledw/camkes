#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define VID 0x0518 // EzKEY corp
#define DID 0x0001 // USB to PS2 Adaptor v1.09

static inline void udelay(uint32_t us)
{
	volatile uint32_t i;
	for (; us > 0; us--) {
		for (i = 0; i < 1000; i++);
	}
}

int run() {
    uintptr_t keyboard = 0;
    printf("Starting the keyboard component...");

    while(!keyboard){
        keyboard = hid_find(VID, DID);
        udelay(5000);
    }

    printf("We found the keyboard!!!\n");
    hid_configure(keyboard);
    printf("Done with configuring the keyboard!!!\n");

    printf("Successfully connected the keyboard...\n");
    while(1) {
        hid_read(keyboard); // need to add
        udelay(8000);
    }

    printf("nice job keyboard\n");
    return 0;
}