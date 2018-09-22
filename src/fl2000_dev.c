// fl2000_dev.c
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//

static void
fl2000_init_flags(struct dev_ctx * dev_ctx)
{
	dev_ctx->card_name = CARD_NAME_FL2000DX;
	dev_ctx->monitor_plugged_in = false;
	init_waitqueue_head(&dev_ctx->ioctl_wait_q);
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

int
fl2000_dev_select_interface(struct dev_ctx * dev_ctx)
{
	struct usb_host_interface *cur_altsetting;
	int ret_val;

	cur_altsetting = dev_ctx->usb_ifc_streaming->cur_altsetting;
	ASSERT( cur_altsetting );
	ret_val = usb_set_interface(
		dev_ctx->usb_dev,
		cur_altsetting->desc.bInterfaceNumber,
		cur_altsetting->desc.bAlternateSetting);
	if (0 != ret_val) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
		"ERROR usb_set_interface failed ret_val=%d",
		ret_val);
	}

	return ret_val;
}

int fl2000_dev_init(struct dev_ctx * dev_ctx)
{
	int ret_val;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	spin_lock_init(&dev_ctx->count_lock);
	fl2000_init_flags(dev_ctx);

	ret_val = fl2000_dev_select_interface(dev_ctx);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR fl2000_dev_select_interface failed.");
		goto exit;
	}

	fl2000_dongle_u1u2_setup(dev_ctx, false);
	fl2000_dongle_card_initialize(dev_ctx);

	ret_val = fl2000_render_create(dev_ctx);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR DRENDER_RenderCreate failed." );
		goto exit;
	}

	fl2000_monitor_manual_check_connection(dev_ctx);

exit:
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Device initialize failed to destory context.");

		fl2000_dev_destroy(dev_ctx);
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
	return ret_val;
}

void fl2000_dev_destroy(
	struct dev_ctx * dev_ctx
	)
{
	fl2000_render_stop(dev_ctx);
	fl2000_dongle_stop(dev_ctx);
	fl2000_render_destroy(dev_ctx);
	fl2000_surface_destroy_all(dev_ctx);
}

// eof: fl2000_dev.c
//
