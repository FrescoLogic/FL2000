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
#if (!GCC_ATOMIC_SUPPORT)
uint32_t fl2000_interlocked_increment(
	struct dev_ctx * 	dev_ctx,
	volatile uint32_t *	target
	)
{
	unsigned long 	flags;
	uint32_t	ret_val;

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	*target = *target + 1;
	ret_val = *target;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	return ret_val;
}

uint32_t fl2000_interlocked_decrement(
	struct dev_ctx * 	dev_ctx,
	volatile uint32_t *	target
	)
{
	unsigned long 	flags;
	uint32_t	ret_val;

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	*target = *target - 1;
	ret_val = *target;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	return ret_val;
}
#endif /* GCC_ATOMIC_SUPPORT */

int
fl2000_dev_select_interface(struct dev_ctx * dev_ctx)
{
	struct usb_host_interface *alt;
	int ret_val;

	alt = dev_ctx->usb_ifc_streaming->cur_altsetting;
	ASSERT( alt );

	/*
	 * started from kernel 5.x.x, the USB core is picky on the buggy fl2000
	 * descriptors where the fl2000 descriptor reports redudant endpoints
	 * in the interface[0,1,2] group. this descritor bug causes the usb core
	 * to ignore the redundant endpoints in interface[1,2]. Therefore the
	 * endpoints are ignored. As a workaround, we select interface 0,
	 * alternate setting 1 where the bulk endpoints are recognized
	 * by the core.
	 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
	ret_val = usb_set_interface(
		dev_ctx->usb_dev,
		alt->desc.bInterfaceNumber,
		alt->desc.bAlternateSetting
		);
#else
	ret_val = usb_set_interface(
		dev_ctx->usb_dev,
		0,	//alt->desc.bInterfaceNumber,
		1	//alt->desc.bAlternateSetting
		);
#endif
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

#if (!GCC_ATOMIC_SUPPORT)
	spin_lock_init(&dev_ctx->count_lock);
#endif
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
