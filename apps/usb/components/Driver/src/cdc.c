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
#include <usb/usb.h>
#include <usb/drivers/cdc.h>

#include <platsupport/timer.h>
#include <platsupport/arch/tsc.h>

extern usb_t usb;
// extern pstimer_t *tsc_timer;

uintptr_t cdc_find(int vid, int did)
{
	int cnt = 1;
	usb_dev_t* usbdev = NULL;

	do {
		usbdev = usb_get_device(&usb, cnt);
		if (usbdev &&
			usbdev->vend_id == vid && usbdev->prod_id == did) {
			break;
		}
		cnt++;
	} while (usbdev);

	return (uintptr_t)usbdev;
}

int cdc_connect(uintptr_t dev)
{
	return usb_cdc_bind((usb_dev_t*)dev);
}

void cdc_configure(uintptr_t dev, int bps, int stop, int parity, int bits)
{
	struct acm_line_coding coding;

	coding.dwDTERate = bps;

	switch (stop) {
		case 1:
			coding.bCharFormat = ACM_STOP_1BIT;
			break;
		case 2:
			coding.bCharFormat = ACM_STOP_2BIT;
			break;
		default:
			assert(0);
			break;
	}

	switch (parity) {
		case 0:
			coding.bParityType = ACM_PARITY_NONE;
			break;
		case 1:
			coding.bParityType = ACM_PARITY_ODD;
			break;
		default:
			assert(0);
			break;
	}

	coding.bDataBits = bits;

	acm_set_line_coding((usb_dev_t*)dev, &coding);
	acm_set_ctrl_line_state((usb_dev_t*)dev, ACM_CTRL_RTS | ACM_CTRL_DTR);
}

void cdc_write(uintptr_t dev, int size)
{
	// uint64_t now = timer_get_time(tsc_timer);s
	usb_cdc_write((usb_dev_t*)dev, (void*)buf, size);
	printf("Write USB\n");
	// printf("Write Time: %llu\n", timer_get_time(tsc_timer) - now);
}

int cdc_read(uintptr_t dev, int size)
{
	int ret;
	// uint64_t now = timer_get_time(tsc_timer);

	ret =  usb_cdc_read((usb_dev_t*)dev, (void*)buf, size);
	printf("Read USB\n");
	// printf("Read Time: %llu\n", timer_get_time(tsc_timer) - now);

	return ret;
}

