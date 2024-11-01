#include <camkes.h>
#include <usb/usb.h>
#include <usb/drivers/touchscreen.h>
#include <usb/drivers/usbkbd.h>

extern usb_t usb;

uintptr_t hid_find(int vid, int did)
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

    if(usbdev){
        ZF_LOGE("!-!-!-Found usb device with VID 0x%lx and DID of 0x%lx !-!-!-", usbdev->vend_id, usbdev->prod_id);
    }
	return (uintptr_t)usbdev;
}

int hid_connect(uintptr_t dev)
{
    // assume touchscreen for now because duh
    // usb_touchscreen_bind((usb_dev_t*)dev);

    return 0;
}

void hid_configure(uintptr_t dev)
{
    // assume touchscreen for now because duh
    // usb_touchscreen_configure((usb_dev_t*)dev);
	ps_chardevice_t* c_keyb = malloc(sizeof(*c_keyb));
	assert(c_keyb);
	ZF_LOGE("Bind keyboard driver");
	usb_kbd_driver_bind((usb_dev_t*)dev, c_keyb);
	ZF_LOGE("leaving bind");
}

void hid_read(uintptr_t dev){
	kbd_poll((usb_dev_t*)dev);
	// usb_touchscreen_read((usb_dev_t*) dev);
}