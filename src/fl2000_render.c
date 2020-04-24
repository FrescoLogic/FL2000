// fl2000_render.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Render Device Support
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//

/*
 * push render_ctx to the bus, with dev_ctx->render.busy_list_lock held
 */
int
fl2000_render_with_busy_list_lock(
	struct dev_ctx * dev_ctx,
	struct render_ctx * render_ctx
	)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	int ret_val = 0;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	if (!dev_ctx->monitor_plugged_in) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER,
			"WARNING Monitor is not attached.");
		/*
		 * put the render_ctx into free_list_head
		 */
		spin_lock_bh(&dev_ctx->render.free_list_lock);
		list_add_tail(&render_ctx->list_entry, free_list_head);
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		InterlockedIncrement(&dev_ctx->render.free_list_count);
		goto exit;
	}

	/*
	 * put this render_ctx into busy_list
	 */
	list_add_tail(&render_ctx->list_entry, &dev_ctx->render.busy_list);
	InterlockedIncrement(&dev_ctx->render.busy_list_count);

	fl2000_bulk_prepare_urb(dev_ctx, render_ctx);
	InterlockedIncrement(&render_ctx->pending_count);

	ret_val = usb_submit_urb(render_ctx->main_urb, GFP_ATOMIC);
	if (ret_val != 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] usb_submit-urb(%p) failed with %d!",
			render_ctx->main_urb,
			ret_val);

		InterlockedDecrement(&render_ctx->pending_count);

		/*
		 * remove this render_ctx from busy_list
		 */
		list_del(&render_ctx->list_entry);

		InterlockedDecrement(&dev_ctx->render.busy_list_count);

		/*
		 * put the render_ctx into free_list_head
		 */
		spin_lock_bh(&dev_ctx->render.free_list_lock);
		list_add_tail(&render_ctx->list_entry, free_list_head);
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		InterlockedIncrement(&dev_ctx->render.free_list_count);

		if (-ENODEV == ret_val || -ENOENT == ret_val) {
			/*
			 * mark the fl2000 device gone
			 */
			dev_ctx->dev_gone = 1;
		}
	    goto exit;
	}

	if ((dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) &&
	    (VR_TRANSFER_PIPE_BULK == dev_ctx->vr_params.trasfer_pipe)) {
		InterlockedIncrement(&render_ctx->pending_count);

		ret_val = usb_submit_urb(render_ctx->zero_length_urb, GFP_ATOMIC);
		if (ret_val != 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"[ERR] zero_length_urb submit fails with %d.",
				ret_val);

			/*
			 * the main_urb is already schedule, we wait until
			 * the completion to move the render_ctx to free_list
			 */
			InterlockedDecrement(&render_ctx->pending_count);

			if (-ENODEV == ret_val || -ENOENT == ret_val) {
				/*
				 * mark the fl2000 device gone
				 */
				dev_ctx->dev_gone = 1;
			}
			goto exit;
		}
	}

exit:
    dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
    return ret_val;
}

/*
 * bug#12414: on customer's platform, the in-flight URBs get stuck
 * for some reason. We try to kill in-flight URBs
 */
void
fl2000_kill_in_flight_render_ctx(
	struct dev_ctx * dev_ctx
	)
{
	struct list_head* const	busy_list = &dev_ctx->render.busy_list;
	struct render_ctx * render_ctx;

	if (dev_ctx->render.busy_list_count == 0)
		return;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"kill in-flight requests now");

	dev_ctx->in_flight_cancelling = 1;

	/*
	 * for every render_ctx on busy_list, kill and wait for the urb.
	 */
	spin_lock_bh(&dev_ctx->render.busy_list_lock);
	while (!list_empty(busy_list)) {
		render_ctx = list_first_entry(
			busy_list, struct render_ctx, list_entry);

		/*
		 * usb_kill_urb() is synchronous call. no spinlock held!
		 */
		spin_unlock_bh(&dev_ctx->render.busy_list_lock);
		usb_kill_urb(render_ctx->main_urb);
		usb_kill_urb(render_ctx->zero_length_urb);
		spin_lock_bh(&dev_ctx->render.busy_list_lock);
	}
	spin_unlock_bh(&dev_ctx->render.busy_list_lock);

	dev_ctx->in_flight_cancelling = 0;
	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"all in-flight requests killed");
}

int
fl2000_render_ctx_create(
	struct dev_ctx * dev_ctx
	)
{
	struct render_ctx * render_ctx;
	int		ret_val;
	uint32_t	i;

	ret_val = 0;
	for (i = 0; i < NUM_OF_RENDER_CTX; i++) {
		render_ctx = &dev_ctx->render.render_ctx[i];

		INIT_LIST_HEAD(&render_ctx->list_entry);
		render_ctx->dev_ctx = dev_ctx;
		render_ctx->pending_count = 0;

		render_ctx->main_urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!render_ctx->main_urb) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"no main_urb usb_alloc_urb?");
			ret_val = -ENOMEM;
			goto exit;
		}

		render_ctx->zero_length_urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!render_ctx->zero_length_urb) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"no zero_length_urb?" );
			ret_val = -ENOMEM;
			goto exit;
		}

		list_add_tail(&render_ctx->list_entry,
			&dev_ctx->render.free_list);

		InterlockedIncrement(&dev_ctx->render.free_list_count);
	}

exit:
    return ret_val;
}

void
fl2000_render_ctx_destroy(
    struct dev_ctx * dev_ctx
    )
{
	struct render_ctx * render_ctx;
	uint32_t	i;

	for (i = 0; i < NUM_OF_RENDER_CTX; i++) {
		render_ctx = &dev_ctx->render.render_ctx[i];

		// It can be NULL in case of failed initialization.
		//
		if (render_ctx == NULL)
			break;

		if (render_ctx->main_urb) {
		    usb_free_urb( render_ctx->main_urb);
		    render_ctx->main_urb = NULL;
		}

		if (render_ctx->zero_length_urb) {
			usb_free_urb(render_ctx->zero_length_urb);
			render_ctx->zero_length_urb = NULL;
		}

		INIT_LIST_HEAD(&render_ctx->list_entry);

		InterlockedDecrement(&dev_ctx->render.free_list_count);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

int
fl2000_render_create(struct dev_ctx * dev_ctx)
{
	int ret_val;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	INIT_LIST_HEAD(&dev_ctx->render.free_list);
	spin_lock_init(&dev_ctx->render.free_list_lock);
	dev_ctx->render.free_list_count = 0;

	INIT_LIST_HEAD(&dev_ctx->render.ready_list);
	spin_lock_init(&dev_ctx->render.ready_list_lock);
	dev_ctx->render.ready_list_count = 0;

	INIT_LIST_HEAD(&dev_ctx->render.busy_list);
	spin_lock_init(&dev_ctx->render.busy_list_lock);
	dev_ctx->render.busy_list_count = 0;

	ret_val = fl2000_render_ctx_create(dev_ctx);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] fl2000_render_ctx_create failed?");
		goto exit;
	}

	INIT_LIST_HEAD(&dev_ctx->render.surface_list);
	spin_lock_init(&dev_ctx->render.surface_list_lock);
	dev_ctx->render.surface_list_count = 0;

exit:
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] Initialize threads failed.");
		fl2000_render_destroy(dev_ctx);
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
	return ret_val;
}

void
fl2000_render_destroy(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	fl2000_render_ctx_destroy(dev_ctx);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
}

void fl2000_render_completion(struct render_ctx * render_ctx)
{
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	int const urb_status = render_ctx->main_urb->status;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, ">>>>");

	WARN_ON(in_irq());

	/*
	 * remove this render_ctx from busy_list
	 */
	spin_lock_bh(&dev_ctx->render.busy_list_lock);
	list_del(&render_ctx->list_entry);
	spin_unlock_bh(&dev_ctx->render.busy_list_lock);

	InterlockedDecrement(&dev_ctx->render.busy_list_count);

	/*
	 * put the render_ctx into free_list_head
	 */
	spin_lock_bh(&dev_ctx->render.free_list_lock);
	list_add_tail(&render_ctx->list_entry, &dev_ctx->render.free_list);
	spin_unlock_bh(&dev_ctx->render.free_list_lock);

	InterlockedIncrement(&dev_ctx->render.free_list_count);

	if (urb_status < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"urb->status(%d) error", urb_status);
		dev_ctx->render.green_light = 0;
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "turning off green_light");

		if (urb_status == -ESHUTDOWN || urb_status == -ENOENT ||
		    urb_status == -ENODEV) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "mark device gone");
			dev_ctx->dev_gone = true;
		}
		goto exit;
	}

	fl2000_schedule_next_render(dev_ctx);
exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_RENDER, "<<<<");
}

void fl2000_render_completion_tasklet(unsigned long data)
{
	struct render_ctx * render_ctx = (struct render_ctx *) data;
	fl2000_render_completion(render_ctx);
}

/*
 * schedule a frame buffer for update.
 * the input frame_buffer should be pinned down or resident in kernel sapce
 */
void
fl2000_primary_surface_update(
	struct dev_ctx *	dev_ctx,
	struct primary_surface* surface)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	struct list_head* const	ready_list_head = &dev_ctx->render.ready_list;
	struct render_ctx *	render_ctx;
	uint32_t		retry_count = 0;

	might_sleep();

	dev_ctx->render.last_updated_surface = surface;
	dev_ctx->render.last_frame_num = surface->frame_num;

	if (dev_ctx->render.green_light == 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER, "green_light off");
		goto exit;
	}

retry:
	/*
	 * get render_ctx from free_list_head
	 */
	render_ctx = NULL;
	spin_lock_bh(&dev_ctx->render.free_list_lock);
	if (!list_empty(free_list_head)) {
		render_ctx = list_first_entry(
			free_list_head, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);

		InterlockedDecrement(&dev_ctx->render.free_list_count);
	}
	spin_unlock_bh(&dev_ctx->render.free_list_lock);

	if (render_ctx == NULL) {
		if (retry_count > 3) {
			dbg_msg(TRACE_LEVEL_WARNING, DBG_PNP,
				"no render_ctx?, render_ctx(%u free, %u busy)",
				dev_ctx->render.free_list_count,
				dev_ctx->render.busy_list_count);
			return;
		}

		if (retry_count > 1)
			fl2000_kill_in_flight_render_ctx(dev_ctx);

		retry_count++;
		msleep_interruptible(10);
		goto retry;
	}

	/*
	 * by now we have a render_ctx, initialize it and schedule it
	 */
	render_ctx->primary_surface = surface;

	spin_lock_bh(&dev_ctx->render.ready_list_lock);
	list_add_tail(&render_ctx->list_entry, ready_list_head);

	InterlockedIncrement(&dev_ctx->render.ready_list_count);

	spin_unlock_bh(&dev_ctx->render.ready_list_lock);

	dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER,
		"render_ctx(%p) scheduled, free_count(%u)/ready_count(%u)",
		render_ctx,
		dev_ctx->render.free_list_count,
		dev_ctx->render.ready_list_count);

	/*
	 * schedule render context from ready_list
	 */
	fl2000_schedule_next_render(dev_ctx);

exit:
	return;
}

/*
 * schedule all render_ctx on ready_list. We schedule redundant frames if no
 * new frames are available.
 */
void
fl2000_schedule_next_render(struct dev_ctx * dev_ctx)
{
	struct list_head* const	free_list_head = &dev_ctx->render.free_list;
	struct list_head* const	ready_list_head = &dev_ctx->render.ready_list;
	struct list_head	staging_list;
	struct render_ctx *	render_ctx = NULL;
	int			ret_val;

	if (dev_ctx->render.green_light == 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_RENDER, "green_light off");
		goto exit;
	}

	/*
	 * step 1: take out all render_ctx on ready list, and put them to
	 * staging list
	 */
	INIT_LIST_HEAD(&staging_list);
	spin_lock_bh(&dev_ctx->render.ready_list_lock);
	while (!list_empty(ready_list_head)) {
		render_ctx = list_first_entry(
			ready_list_head, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);
		InterlockedDecrement(&dev_ctx->render.ready_list_count);
		list_add_tail(&render_ctx->list_entry, &staging_list);
	}
	spin_unlock_bh(&dev_ctx->render.ready_list_lock);

	/*
	 * step 2, schedule all render_ctx, with busy_list_lock held.
	 * this is critical path where we schedule redundant frames.
	 */
	spin_lock_bh(&dev_ctx->render.busy_list_lock);
reschedule:
	while (!list_empty(&staging_list)) {
		render_ctx = list_first_entry(
			&staging_list, struct render_ctx, list_entry);
		list_del(&render_ctx->list_entry);
		ret_val = fl2000_render_with_busy_list_lock(dev_ctx, render_ctx);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"usb_submit_urb failed %d, "
				"turn off green_light\n", ret_val);
			dev_ctx->render.green_light = false;
			break;
		}
	}

	/*
	 * schedule redundant frames.
	 */
	if (dev_ctx->render.busy_list_count < NUM_RENDER_ON_BUS &&
	    dev_ctx->render.green_light) {
		struct primary_surface* surface;

		spin_lock_bh(&dev_ctx->render.free_list_lock);
		if (!list_empty(free_list_head)) {
			render_ctx = list_first_entry(
				free_list_head, struct render_ctx, list_entry);
			list_del(&render_ctx->list_entry);

			InterlockedDecrement(&dev_ctx->render.free_list_count);
		}
		spin_unlock_bh(&dev_ctx->render.free_list_lock);

		/*
		 * preparing additional frame
		 */
		if (render_ctx != NULL) {
			surface = dev_ctx->render.last_updated_surface;
			render_ctx->primary_surface = surface;
			list_add_tail(&render_ctx->list_entry, &staging_list);
			goto reschedule;
		}
	}
	spin_unlock_bh(&dev_ctx->render.busy_list_lock);

exit:
	return;
}

void fl2000_render_start(struct dev_ctx * dev_ctx)
{
	dev_ctx->render.green_light = 1;
}

void fl2000_render_stop(struct dev_ctx * dev_ctx)
{
	uint32_t delay_ms = 0;

	might_sleep();
	dev_ctx->render.green_light = 0;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"busy_list_count(%u)", dev_ctx->render.busy_list_count);

	fl2000_kill_in_flight_render_ctx(dev_ctx);
	while (dev_ctx->render.busy_list_count != 0) {
		delay_ms += 10;
		DELAY_MS(10);
	}
	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP, "waited %u ms", delay_ms);
}

// eof: fl2000_render.c
//
