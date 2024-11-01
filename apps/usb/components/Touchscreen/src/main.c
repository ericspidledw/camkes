#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define VID 0x2453 // Baanto touchscreen
#define DID 0x0100 // HID touchscreen

static inline void udelay(uint32_t us)
{
	volatile uint32_t i;
	for (; us > 0; us--) {
		for (i = 0; i < 1000; i++);
	}
}

int run() {
    return 0;
    uintptr_t touchscreen = 0;
    printf("Starting the touchscreen component...");

    while(!touchscreen){
        touchscreen = hid_find(VID, DID);
        udelay(5000);
    }

    printf("We found the touchscreen!!!\n");
    hid_connect(touchscreen);
    // hid_configure(touchscreen);

    printf("Successfully connected the touchscreen...\n");
    while(1) {
        hid_read(touchscreen); // n eed to add this
        udelay(1 * 1000 * 1000);
    }

    printf("nice job touchscreen\n");
    return 0;
}