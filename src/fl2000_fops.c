// fl2000_fops.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

#define	USE_VM_INSERT_PFN	0

int fl2000_open(struct inode * inode, struct file * file)
{
	int const minor = iminor(inode);
	struct usb_interface * interface;
	struct dev_ctx * dev_ctx;
	int ret_val;
	uint32_t open_count;
	unsigned long flags;

	ret_val = 0;
	interface = usb_find_interface(&fl2000_driver, minor);
	if (!interface) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no interface?.");
		ret_val = -ENODEV;
		goto exit;
	}

	dev_ctx = usb_get_intfdata(interface);
	if (!dev_ctx) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"no dev_ctx?.");
		ret_val = -ENODEV;
		goto exit;
	}

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	open_count = ++dev_ctx->open_count;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
	if (open_count > 1) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"open_count(%u) exceeds 1?", open_count);
		spin_lock_irqsave(&dev_ctx->count_lock, flags);
		dev_ctx->open_count--;
		spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
		ret_val = -EBUSY;
		goto exit;
	}

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP, "open_count(%u)", open_count);

	file->private_data = dev_ctx;
	kref_get(&dev_ctx->kref);
exit:
	return ret_val;
}

int fl2000_release(struct inode * inode, struct file * file)
{
	struct dev_ctx * const dev_ctx = file->private_data;
	uint32_t open_count;
	unsigned long flags;

	if (dev_ctx == NULL)
		return -ENODEV;

	// wake up any sleeping process.
	//
	if (waitqueue_active(&dev_ctx->ioctl_wait_q)) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"wake up pending process");
		wake_up_interruptible(&dev_ctx->ioctl_wait_q);
	}

	fl2000_render_stop(dev_ctx);
	fl2000_dongle_stop(dev_ctx);
	fl2000_surface_destroy_all(dev_ctx);

	spin_lock_irqsave(&dev_ctx->count_lock, flags);
	open_count = --dev_ctx->open_count;
	spin_unlock_irqrestore(&dev_ctx->count_lock, flags);
	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP, "open_count(%u)", open_count);
	kref_put(&dev_ctx->kref, fl2000_module_free);

	return 0;
}

/*
 * this function is triggered from vm_mmap() in fl2000_ioctl_test_alloc_surface.
 * finish the last step of mapping system ram.
 * treat the memory as device ram.
 */
int fl2000_mmap(struct file * file, struct vm_area_struct *vma)
{
	struct dev_ctx * const dev_ctx = file->private_data;
	unsigned long len = vma->vm_end - vma->vm_start;
	unsigned long num_pages = (len + PAGE_SIZE - 1) >> PAGE_SHIFT;
	unsigned int ret_val = 0;
	unsigned int i;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"vm_start(0x%lx), vm_end(0x%lx), num_pages(0x%lx)",
		vma->vm_start, vma->vm_end, num_pages);

	vma->vm_private_data = dev_ctx;

	/*
	 * vm_insert_pfn required either VM_PFNMAP or VM_MIXEDMAP
	 * set, but not both.
	 * if VM_MIXEDMAP is set, pfn_valid(pfn) must be false.
	 * in our case, the memory is from system memory, and we are
	 * not able to use VM_MIXEDMAP
	 *
	 * vm_insert_page requires VM_PFNMAP be cleared, and called
	 * with vma->vm_mm->mmap held.
	 */
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_flags &= ~VM_PFNMAP;

	for (i = 0; i < num_pages; i++) {
		unsigned long usr_addr = vma->vm_start + (i << PAGE_SHIFT);
		struct page * page = dev_ctx->start_page + i;

		ret_val = vm_insert_page(vma, usr_addr, page);
		if (ret_val) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"vm_insert_page(usr_addr 0x%lx), page_count(%u) failed %d",
				usr_addr, page_count(page), ret_val);
			break;
		}
	}

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP, "vm_flags(0x%lx)", vma->vm_flags);
	return ret_val;
}

