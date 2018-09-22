// fl2000_desc.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//


/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//
void fl2000_desc_init(struct dev_ctx * dev_ctx)
{
	memcpy(	&dev_ctx->usb_dev_desc,
		&dev_ctx->usb_dev->descriptor,
		sizeof(struct usb_device_descriptor));

	dev_ctx->card_name = CARD_NAME_UNDEFINED;

	if (VID_FRESCO_LOGIC == dev_ctx->usb_dev_desc.idVendor &&
	    PID_FL2000 == dev_ctx->usb_dev_desc.idProduct &&
	    DEVICE_ID_FL2000DX == dev_ctx->usb_dev_desc.bcdDevice)
		dev_ctx->card_name = CARD_NAME_FL2000DX;
}

// eof: f2l000_desc.c
//