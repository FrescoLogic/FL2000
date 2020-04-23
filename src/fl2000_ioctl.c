// fl2000_ioctl.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Utility Primitives
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//
long
fl2000_ioctl_wait_monitor_event(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct monitor_info monitor_info;
	bool const prev_plug_in = dev_ctx->monitor_plugged_in;
	int wait_status;

	/*
	 * return when dev_ctx->monitor_plugged_in changed.
	 */
	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"wait on plugged_in(%d) change", prev_plug_in);
	wait_status = wait_event_interruptible(dev_ctx->ioctl_wait_q,
		(prev_plug_in != dev_ctx->monitor_plugged_in));

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"wait_completed, current plugged_in(%d), wait_status(%d)",
		dev_ctx->monitor_plugged_in, wait_status);

	memset(&monitor_info, 0, sizeof(monitor_info));
	monitor_info.monitor_flags.connected = (dev_ctx->monitor_plugged_in == true);
	monitor_info.fl2000_flags.connected = (dev_ctx->dev_gone != true);

	/*
	 * assume we have VGA only output. We will check if we have HDMI output
	 * in the future.
	 */
	monitor_info.monitor_type = OUTPUT_TECHNOLOGY_HD15;
	memcpy(monitor_info.edid, dev_ctx->monitor_edid[0], EDID_SIZE);
	if (copy_to_user((void *) arg, &monitor_info, sizeof(monitor_info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_to_user fails?");
		return -EFAULT;
	}

	return wait_status;
}

long
fl2000_ioctl_query_monitor_event(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct monitor_info monitor_info;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"current plugged_in(%d)", dev_ctx->monitor_plugged_in);

	memset(&monitor_info, 0, sizeof(monitor_info));
	monitor_info.monitor_flags.connected = (dev_ctx->monitor_plugged_in == true);
	monitor_info.fl2000_flags.connected = (dev_ctx->dev_gone != true);

	/*
	 * assume we have VGA only output. We will check if we have HDMI output
	 * in the future.
	 */
	monitor_info.monitor_type = OUTPUT_TECHNOLOGY_HD15;
	memcpy(monitor_info.edid, dev_ctx->monitor_edid[0], EDID_SIZE);
	if (copy_to_user((void *) arg, &monitor_info, sizeof(monitor_info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_to_user fails?");
		return -EFAULT;
	}

	return 0;
}

long
fl2000_ioctl_set_display_mode(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct display_mode display_mode;
	int ret;

	if (copy_from_user(&display_mode, (void *) arg, sizeof(display_mode))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		return -EFAULT;
	}
	ret = fl2000_set_display_mode(dev_ctx, &display_mode);
	if (ret < 0) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"fl2000_set_display_mode(width:%u height:%d.) failed",
			display_mode.width,
			display_mode.height);
		return ret;
	}

	dev_ctx->render.display_mode = display_mode;
	return ret;
}

long
fl2000_ioctl_create_surface(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct surface_info info;

	if (copy_from_user(&info, (void *) arg, sizeof(info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		return -EFAULT;
	}

	return fl2000_surface_create(dev_ctx, &info);
}

long
fl2000_ioctl_destroy_surface(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct list_head * const list_head = &dev_ctx->render.surface_list;
	struct surface_info info;
	struct primary_surface* s;
	struct primary_surface* surface;

	if (copy_from_user(&info, (void *) arg, sizeof(info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		return -EFAULT;
	}

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"handle(0x%x)/user_buffer(0x%x)/buffer_length(0x%x)",
		(unsigned int) info.handle,
		(unsigned int) info.user_buffer,
		(unsigned int) info.buffer_length);
	surface = NULL;
	spin_lock_bh(&dev_ctx->render.surface_list_lock);
	list_for_each_entry(s, list_head, list_entry) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"surface(%p), handle(0x%x)\n",
			s,
			(unsigned int) s->handle);
		if (s->handle == info.handle) {

			InterlockedDecrement(&dev_ctx->render.surface_list_count);

			list_del(&s->list_entry);
			surface = s;
			break;
		}
	}
	spin_unlock_bh(&dev_ctx->render.surface_list_lock);

	if (surface == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no surface found for handle(%p)?",
			(void*) (unsigned long) info.handle);
		return -EINVAL;
	}

	fl2000_surface_destroy(dev_ctx, surface);
	return 0;
}

void
pixel_swap(uint8_t * dst, uint8_t * src, uint32_t len)
{
	uint32_t *src_block;
	uint32_t *dst_block;
	uint32_t length;
	unsigned int i;

	src_block = (uint32_t *) src;
	dst_block = (uint32_t *) dst;
	length = (len + 7) & 0xFFFFFFF8; // len round up to multiple of 8
	for (i = 0; i < (length >> 2); i += 2) {
		dst_block[i] = src_block[i + 1];
		dst_block[i + 1] = src_block[i];
	}
}

long
fl2000_ioctl_notify_surface_update(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct surface_update_info info;
	struct primary_surface* s;
	struct primary_surface* surface;
	int			ret = 0;

	if (copy_from_user(&info, (void *) arg, sizeof(info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		ret = -EFAULT;
		goto exit;
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP,
		"handle(%p)/buffer_length(0x%x)",
		(void*) (unsigned long) info.handle,
		(unsigned int) info.buffer_length);

	surface = NULL;
	spin_lock_bh(&dev_ctx->render.surface_list_lock);
	list_for_each_entry(s, &dev_ctx->render.surface_list, list_entry) {
		if (s->handle == info.handle) {
			surface = s;
			break;
		}
	}
	spin_unlock_bh(&dev_ctx->render.surface_list_lock);

	if (surface == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no surface found for handle(%p)?",
			(void*) (unsigned long) info.handle);
		ret = -EFAULT;
		goto exit;
	}

	if (info.buffer_length != surface->buffer_length) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"buffer_length(0x%x) differs from prev created 0x%x?",
			(unsigned int) info.buffer_length,
			(unsigned int) surface->buffer_length);
		ret = -EFAULT;
		goto exit;
	}

	surface->frame_num++;

	/*
	 * on some implementation, the info.user_buffer might not be identical
	 * to the initial user_buffer when created. if this case occurs, use
	 * the current user_buffer, instead of the previous created one.
	 */
	if (info.user_buffer != surface->user_buffer) {
		/*
		 * NOT YET IMPLEMENTED
		 */
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"user_buffer(%p) differs from previously created %p?",
			(void*) (unsigned long) info.user_buffer,
			(void*) (unsigned long) surface->user_buffer);
		ret = -EFAULT;
		goto exit;
	}

	/*
	 * copy pixels from user mode to kernel mode, if shadow buffer
	 * is chosen.
	 */
	if (surface->render_buffer == surface->shadow_buffer) {
		/*
		 * pin down user buffer if surface_type is
		 * SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE, and is not
		 * yet locked down.
		 */
		if (!surface->pre_locked &&
		    surface->type == SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE) {
			ret = fl2000_surface_pin_down(dev_ctx, surface);
			if (ret < 0) {
				dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
					"user_buffer(%p) pin failure?",
					(void*) (unsigned long) info.user_buffer);
				goto unlock_surface;
			}
			ret = fl2000_surface_map(dev_ctx, surface);
			if (ret < 0) {
				dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
					"user_buffer(%p) map failure?",
					(void*) (unsigned long) info.user_buffer);
				goto unlock_surface;
			}

			if (dev_ctx->vr_params.use_compression) {
				uint32_t compressed_size;

				/*
				 * fl2000_compression_gravity2 converts RGB24
				 * to RGB16_555 and then compress the RGB16_555
				 * image using run length encoding algorithm.
				 */
				if (IS_DEVICE_USB2LINK(dev_ctx))
				{
					compressed_size = fl2000_compression_gravity2(
						dev_ctx,
						surface->buffer_length,
						surface->system_buffer,		// source
						surface->compressed_buffer,	// target
						surface->working_buffer,
						surface->width * surface->height
						);
				}
				else
				{
					compressed_size = fl2000_compression_gravity(
						dev_ctx,
						surface->buffer_length,
						surface->system_buffer,		// source
						surface->compressed_buffer,	// target
						surface->working_buffer,
						surface->width * surface->height
						);
				}

				pixel_swap(surface->shadow_buffer,
					surface->compressed_buffer,
					compressed_size);
				surface->xfer_length = compressed_size;

				//dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
				//	"buffer_length(0x%x)/compressed_size(0x%x)\n",
				//	surface->buffer_length,
				//	compressed_size);
			}
			else {
				pixel_swap(surface->shadow_buffer,
					surface->system_buffer,
					surface->buffer_length);
			}

			fl2000_primary_surface_update(
				dev_ctx, surface);
unlock_surface:
			fl2000_surface_unmap(dev_ctx, surface);
			fl2000_surface_unpin(dev_ctx, surface);
		}
		else {
			/*
			 * surface is not SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE
			 * we can safely access system_buffer.
			 */
			if (dev_ctx->vr_params.use_compression) {
				uint32_t compressed_size;

				/*
				 * fl2000_compression_gravity2 converts RGB24
				 * to RGB16_555 and then compress the RGB16_555
				 * image using run length encoding algorithm.
				 */
				if (IS_DEVICE_USB2LINK(dev_ctx))
				{
					compressed_size = fl2000_compression_gravity2(
						dev_ctx,
						surface->buffer_length,
						surface->system_buffer,		// source
						surface->compressed_buffer,	// target
						surface->working_buffer,
						surface->width * surface->height
						);
				}
				else
				{
					compressed_size = fl2000_compression_gravity(
						dev_ctx,
						surface->buffer_length,
						surface->system_buffer,		// source
						surface->compressed_buffer,	// target
						surface->working_buffer,
						surface->width * surface->height
						);
				}

				pixel_swap(surface->shadow_buffer,
					surface->compressed_buffer,
					compressed_size);
				surface->xfer_length = ((compressed_size + 7) / 8) * 8;

				//dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
				//	"buffer_length(0x%x)/compressed_size(0x%x)/xfer_length(0x%x)\n",
				//	surface->buffer_length,
				//	compressed_size,
				//	surface->xfer_length
				//	);
			}
			else {
				pixel_swap(surface->shadow_buffer,
					surface->system_buffer,
					surface->buffer_length);
			}

			fl2000_primary_surface_update(
				dev_ctx, surface);
		}
	}
	else if (surface->render_buffer == surface->system_buffer) {
		fl2000_primary_surface_update(dev_ctx, surface);
	}
	else {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"software error :render_buffer(%p)/"
			"mapped_buffer(%p)/shadow_buffer(%p)/type(%x)",
			surface->render_buffer,
			surface->mapped_buffer,
			surface->shadow_buffer,
			surface->type);
	}

exit:
	return ret;
}

long
fl2000_ioctl_lock_surface(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct surface_update_info info;
	struct primary_surface* s;
	struct primary_surface* surface;
	int			ret = 0;

	if (copy_from_user(&info, (void *) arg, sizeof(info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		ret = -EFAULT;
		goto exit;
	}

	dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
		"handle(%p)/buffer_length(0x%x)",
		(void*) (unsigned long) info.handle,
		(unsigned int) info.buffer_length);

	surface = NULL;
	spin_lock_bh(&dev_ctx->render.surface_list_lock);
	list_for_each_entry(s, &dev_ctx->render.surface_list, list_entry) {
		if (s->handle == info.handle) {
			surface = s;
			break;
		}
	}
	spin_unlock_bh(&dev_ctx->render.surface_list_lock);

	if (surface == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no surface found for handle(%p)?",
			(void*) (unsigned long) info.handle);
		ret = -EFAULT;
		goto exit;
	}

	if (info.buffer_length != surface->buffer_length) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"buffer_length(0x%x) differs from prev created 0x%x?",
			(unsigned int) info.buffer_length,
			(unsigned int) surface->buffer_length);
		ret = -EFAULT;
		goto exit;
	}

	/*
	 * pin down user buffer if the user_buffer is vmalloc style.
	 */
	if (surface->type == SURFACE_TYPE_VIRTUAL_CONTIGUOUS ||
	    surface->type == SURFACE_TYPE_PHYSICAL_CONTIGUOUS) {
		goto exit;
	}

	if (!surface->pre_locked) {
		ret = fl2000_surface_pin_down(dev_ctx, surface);
		if (ret < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"user_buffer(%p) pin failure?",
				(void*) (unsigned long) info.user_buffer);
			goto unlock_surface;
		}
		ret = fl2000_surface_map(dev_ctx, surface);
		if (ret < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"user_buffer(%p) map failure?",
				(void*) (unsigned long) info.user_buffer);
			goto unlock_surface;
		}

		surface->pre_locked = true;
		goto exit;

unlock_surface:
		fl2000_surface_unmap(dev_ctx, surface);
		fl2000_surface_unpin(dev_ctx, surface);
	}

exit:
	return ret;
}

long
fl2000_ioctl_unlock_surface(struct dev_ctx * dev_ctx, unsigned long arg)
{
	struct surface_update_info info;
	struct primary_surface* s;
	struct primary_surface* surface;
	int			ret = 0;

	if (copy_from_user(&info, (void *) arg, sizeof(info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		ret = -EFAULT;
		goto exit;
	}

	dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
		"handle(%p)/buffer_length(0x%x)",
		(void*) (unsigned long) info.handle,
		(unsigned int) info.buffer_length);

	surface = NULL;
	spin_lock_bh(&dev_ctx->render.surface_list_lock);
	list_for_each_entry(s, &dev_ctx->render.surface_list, list_entry) {
		if (s->handle == info.handle) {
			surface = s;
			break;
		}
	}
	spin_unlock_bh(&dev_ctx->render.surface_list_lock);

	if (surface == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no surface found for handle(%p)?",
			(void*) (unsigned long) info.handle);
		ret = -EFAULT;
		goto exit;
	}

	if (info.buffer_length != surface->buffer_length) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"buffer_length(0x%x) differs from prev created 0x%x?",
			(unsigned int) info.buffer_length,
			(unsigned int) surface->buffer_length);
		ret = -EFAULT;
		goto exit;
	}

	if (surface->type == SURFACE_TYPE_VIRTUAL_CONTIGUOUS ||
	    surface->type == SURFACE_TYPE_PHYSICAL_CONTIGUOUS) {
		goto exit;
	}

	/*
	 * unlock user buffer
	 */
	if (surface->pre_locked) {
		fl2000_surface_unmap(dev_ctx, surface);
		fl2000_surface_unpin(dev_ctx, surface);
		surface->pre_locked = false;
	}

exit:
	return ret;
}

long
fl2000_ioctl_test_alloc_surface(struct file *file, unsigned long arg)
{
	struct dev_ctx * const dev_ctx = file->private_data;
	struct test_alloc alloc_info;
	unsigned long usr_addr;
	unsigned long start_pfn;
	unsigned long len;
	unsigned long num_pages;
	unsigned int order;
	struct page * page;
	int ret = 0;

	if (copy_from_user(&alloc_info, (void *) arg, sizeof(alloc_info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		ret = -EFAULT;
		goto exit;
	}

	/*
	 * 1st step, allocate contiguous physical memory
	 */
	len = (unsigned long) alloc_info.buffer_size;
	num_pages = (len + PAGE_SIZE - 1) >> PAGE_SHIFT;
	order = get_order(len);
	page = alloc_pages(GFP_HIGHUSER | __GFP_COMP, order);
	if (!page) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"alloc_pages order(%d)/num_pages(%lu) failed?",
			order, num_pages);
		ret = -ENOMEM;
		goto exit;
	}
	start_pfn = page_to_pfn(page);
	dev_ctx->start_page = page;

	/*
	 * 2nd step, simulate user mmap() call by vm_mmap.
	 */
	usr_addr = vm_mmap(file, 0, len, PROT_READ | PROT_WRITE, MAP_SHARED, 0);
	if (IS_ERR((void*) usr_addr)) {
		ret = PTR_ERR((void *) usr_addr);
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "vm_mmap fails %d?",
			(int) usr_addr);
		goto exit;
	}

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP, "vm_mmap returns 0x%lx", usr_addr);

	alloc_info.buffer_size = len;
	alloc_info.usr_addr = usr_addr;
	alloc_info.phy_addr = (uint64_t) (start_pfn << PAGE_SHIFT);
	ret = copy_to_user((void*) arg, &alloc_info, sizeof(alloc_info));

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"usr_addr(0x%lx)/phy_addr(0x%lx) returned",
		usr_addr,
		start_pfn << PAGE_SHIFT);

exit:
	return ret;
}

long
fl2000_ioctl_test_release_surface(struct file *file, unsigned long arg)
{
	struct test_alloc alloc_info;
	unsigned long usr_addr;
	unsigned long len;
	unsigned long num_pages;
	unsigned long start_pfn;
	unsigned int order;
	struct page * page;

	if (copy_from_user(&alloc_info, (void *) arg, sizeof(alloc_info))) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "copy_from_user fails?");
		return -EFAULT;
	}

	usr_addr = (unsigned long) alloc_info.usr_addr;
	start_pfn = (unsigned long) alloc_info.phy_addr >> PAGE_SHIFT;
	len = (unsigned long) alloc_info.buffer_size;
	num_pages = (len + PAGE_SIZE - 1) >> PAGE_SHIFT;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"usr_addr(0x%lx)/phy_addr(0x%lx)/len(0x%lx)/num_pages(0x%lx)",
		usr_addr, start_pfn << PAGE_SHIFT,
		len, num_pages);

	page = pfn_to_page(start_pfn);
	order = get_order(len);
	vm_munmap(usr_addr, len);

	/*
	 * call free_pages
	 */
	__free_pages(page, order);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

long fl2000_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dev_ctx * const dev_ctx = file->private_data;
	long ret_val;

	ret_val = -EINVAL;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	if (dev_ctx == NULL) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP, "no dev_ctx?");
		ret_val = -ENODEV;
		goto exit;
	}

	switch (cmd) {
	case IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT:
		ret_val = fl2000_ioctl_wait_monitor_event(dev_ctx, arg);
		break;

	case IOCTL_FL2000_QUERY_MONITOR_INFO:
		ret_val = fl2000_ioctl_query_monitor_event(dev_ctx, arg);
		break;

	case IOCTL_FL2000_SET_DISPLAY_MODE:
		ret_val = fl2000_ioctl_set_display_mode(dev_ctx, arg);
		break;

	case IOCTL_FL2000_CREATE_SURFACE:
		ret_val = fl2000_ioctl_create_surface(dev_ctx, arg);
		break;

	case IOCTL_FL2000_DESTROY_SURFACE:
		ret_val = fl2000_ioctl_destroy_surface(dev_ctx, arg);
		break;

	case IOCTL_FL2000_NOTIFY_SURFACE_UPDATE:
		ret_val = fl2000_ioctl_notify_surface_update(dev_ctx, arg);
		break;

	case IOCTL_FL2000_LOCK_SURFACE:
		ret_val = fl2000_ioctl_lock_surface(dev_ctx, arg);
		break;

	case IOCTL_FL2000_UNLOCK_SURFACE:
		ret_val = fl2000_ioctl_unlock_surface(dev_ctx, arg);
		break;

	case IOCTL_FL2000_TEST_ALLOC_SURFACE:
		ret_val = fl2000_ioctl_test_alloc_surface(file, arg);
		break;

	case IOCTL_FL2000_TEST_RELEASE_SURFACE:
		ret_val = fl2000_ioctl_test_release_surface(file, arg);
		break;

	case IOCTL_FL2000_NOTIFY_POINTER_POSITION_UPDATE:
	case IOCTL_FL2000_NOTIFY_POINTER_SHAPE_UPDATE:
	default:
		break;
	}

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
	return ret_val;
}

// eof: fl2000_ioctl.c
//
