// fl2000_interrupt.c
//
// (c)Copyright 2010-2013, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose: Interrupt Pipe Support
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

int fl2000_intr_pipe_create(struct dev_ctx * dev_ctx)
{
	struct usb_host_interface * const host_ifc =
		dev_ctx->usb_ifc_intr->cur_altsetting;
	uint8_t const bNumEndpoints = host_ifc->desc.bNumEndpoints;
	int i;
	int ret_val;

	/*
	 * find interrupt in endpoint
	 */
	dev_ctx->ep_intr_in = NULL;
	for (i = 0; i < bNumEndpoints; i++) {
		struct usb_host_endpoint * endpoint;
		struct usb_endpoint_descriptor * desc;

		endpoint = &host_ifc->endpoint[i];
		desc = &endpoint->desc;
		if (usb_endpoint_is_int_in(desc)) {
			dev_ctx->ep_intr_in = endpoint;
			dev_ctx->ep_num_intr_in = usb_endpoint_num(desc);
			dev_ctx->ep_desc_intr_in = desc;
			dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
				"found ep_num_intr_in(%d)",
				dev_ctx->ep_num_intr_in
				);
			break;
		}
	}
	if (dev_ctx->ep_intr_in == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"couldn't find ep_intr_in" );
		ret_val = -EINVAL;
		goto exit;
	}

	/*
	 * dev_ctx->ep_num_intr_in should be 3
	 */
	dev_ctx->usb_pipe_intr_in = usb_rcvintpipe(dev_ctx->usb_dev, 3);
	if (!dev_ctx->usb_pipe_intr_in) {
		ASSERT(false);
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Invalid interrupt pipe." );
		ret_val = -EINVAL;
		goto exit;
	}

	dev_ctx->intr_urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!dev_ctx->intr_urb) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Allocate interrupt urb failed.");
		ret_val = -ENOMEM;
		goto exit;
	}

	dev_ctx->intr_pipe_wq = create_workqueue("intr_pipe_wq");
	if (dev_ctx->intr_pipe_wq == NULL) {
		usb_free_urb(dev_ctx->intr_urb);
		dev_ctx->intr_urb = NULL;

		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Allocate interrupt urb failed.");
		ret_val = -ENOMEM;
		goto exit;
	}

	ret_val = 0;

exit:
	return ret_val;
}

void fl2000_intr_pipe_destroy(struct dev_ctx * dev_ctx)
{
	if (dev_ctx->intr_pipe_wq) {
		destroy_workqueue(dev_ctx->intr_pipe_wq);
		dev_ctx->intr_pipe_wq = NULL;
	}

	if (dev_ctx->intr_urb){
		usb_free_urb(dev_ctx->intr_urb);
		dev_ctx->intr_urb = NULL;
	}
}

int fl2000_intr_pipe_start(struct dev_ctx * dev_ctx)
{
	int ret_val;
	unsigned long flags;

	dev_ctx->intr_pipe_started = true;
	usb_fill_int_urb(
		dev_ctx->intr_urb,
		dev_ctx->usb_dev,
		dev_ctx->usb_pipe_intr_in,
		&dev_ctx->intr_data,
		sizeof(dev_ctx->intr_data),
		fl2000_intr_pipe_completion,
		dev_ctx,
		dev_ctx->ep_desc_intr_in->bInterval);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->intr_pipe_pending_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	ret_val = usb_submit_urb(dev_ctx->intr_urb, GFP_KERNEL);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR usb_submit_urb(intr_urb) failed.");
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_pending_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_started = false;
	}

	return ret_val;
}

/*
 * stop interrupt pipe, and wait for all interrupt request completion
 */
void fl2000_intr_pipe_stop(struct dev_ctx * dev_ctx)
{
	might_sleep();

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"stopping interrupt pipe");

	dev_ctx->intr_pipe_started = false;

	if (dev_ctx->intr_pipe_pending_count != 0)
		usb_kill_urb(dev_ctx->intr_urb);
	drain_workqueue(dev_ctx->intr_pipe_wq);

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"interrupt pipe stopped");
}

void fl2000_intr_pipe_completion(struct urb * urb)
{
	struct dev_ctx * const dev_ctx = urb->context;
	bool work_queued;
	int ret_val;
	unsigned long flags;

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->intr_pipe_pending_count--;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	/*
	 * invoke worker-thread for further processing
	 */
	INIT_WORK(&dev_ctx->intr_pipe_work, &fl2000_intr_pipe_work);
	work_queued = queue_work(dev_ctx->intr_pipe_wq, &dev_ctx->intr_pipe_work);

	if (work_queued)
		goto exit;

	if (!dev_ctx->intr_pipe_started) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"intr_pipe stopped.");
		goto exit;
	}

	/*
	 * unable to queue worker-thread, continue to fire interrupt
	 */
	dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
		"unable to queue fl2000_intr_pipe_work.");

	usb_fill_int_urb(
		dev_ctx->intr_urb,
		dev_ctx->usb_dev,
		dev_ctx->usb_pipe_intr_in,
		&dev_ctx->intr_data,
		sizeof(dev_ctx->intr_data),
		fl2000_intr_pipe_completion,
		dev_ctx,
		dev_ctx->ep_desc_intr_in->bInterval);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->intr_pipe_pending_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	ret_val = usb_submit_urb(dev_ctx->intr_urb, GFP_KERNEL);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR usb_submit_urb(intr_urb) failed.");
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_pending_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_started = false;
	}

exit:
	return;
}

void fl2000_intr_pipe_work(struct work_struct * work_item)
{
	struct dev_ctx * const dev_ctx =
		container_of(work_item, struct dev_ctx, intr_pipe_work);
	int ret_val;
	unsigned long flags;

	/*
	 * read interrupt status, and process it
	 */
	fl2000_intr_process(dev_ctx);

	if (!dev_ctx->intr_pipe_started) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"intr_pipe stopped.");
		goto exit;
	}

	/*
	 * schedule next interrupt request
	 */
	usb_fill_int_urb(
		dev_ctx->intr_urb,
		dev_ctx->usb_dev,
		dev_ctx->usb_pipe_intr_in,
		&dev_ctx->intr_data,
		sizeof(dev_ctx->intr_data),
		fl2000_intr_pipe_completion,
		dev_ctx,
		dev_ctx->ep_desc_intr_in->bInterval);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	dev_ctx->intr_pipe_pending_count++;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);

	ret_val = usb_submit_urb(dev_ctx->intr_urb, GFP_KERNEL);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR usb_submit_urb(intr_urb) failed.");
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_pending_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		dev_ctx->intr_pipe_started = false;
	}

exit:
	return;
}

void
fl2000_intr_process(struct dev_ctx * dev_ctx)
{
	uint32_t data;

	data = 0;

	// Get interrupt status
	//
	if (fl2000_reg_read(dev_ctx, REG_OFFSET_8000, &data)) {
		struct vga_status * vga_status;

		vga_status = (struct vga_status *)&data;

		if (vga_status->edid_connect_changed ||
		    vga_status->ext_mon_connect_changed) {
			fl2000_monitor_vga_status_handler(dev_ctx, data);
		}
		else {
			if ( vga_status->frame_dropped) {
				dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
					"frame drop!");
			}
		}
	}
}


// eof: fl2000_interrupt.c
//
