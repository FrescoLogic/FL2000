// fl2000_bulk.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Bulk Pipe Support
//

#include "fl2000_include.h"

/*
 * do not enable MERGE_ADJACENT_PAGES
 * it turned out that ehci-pci does not accept a scatterlist which describes a
 * large contiguous buffer (> 30KB ?) when we merge multiple configuous
 * pages into a scatterlist (MERGE_ADJACENT_PAGES = 1).
 * When the fl2000 dongle is attached to intel xHC, the issue disappears.
 */
#define	MERGE_ADJACENT_PAGES	0

/*
 * this routine is called typically from hard_irq context, as of the latest
 * xHCI implementation. We do our processing in lower IRQL context by launching
 * a tasklet.
 */
void fl2000_bulk_main_completion(
	struct urb *urb
	)
{
	struct render_ctx * const render_ctx = urb->context;
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	uint32_t pending_count;

	UNREFERENCED_PARAMETER(dev_ctx);

	pending_count = InterlockedDecrement(&render_ctx->pending_count);

	if (pending_count == 0) {
		if (in_irq() || irqs_disabled()) {
			struct tasklet_struct * tasklet = &render_ctx->tasklet;

			tasklet_init(
				tasklet,
				fl2000_render_completion_tasklet,
				(unsigned long) render_ctx);
			tasklet_schedule(tasklet);
		} else {
			fl2000_render_completion(render_ctx);
		}
	}
}

/*
 * this routine is called typically from hard_irq context, as of the latest
 * xHCI implementation. We do our processing in lower IRQL context by launching
 * a tasklet.
 */
void fl2000_bulk_zero_length_completion(
	struct urb *urb
	)
{
	struct render_ctx * const render_ctx = urb->context;
	struct dev_ctx * const dev_ctx = render_ctx->dev_ctx;
	uint32_t pending_count;

	UNREFERENCED_PARAMETER(dev_ctx);

	pending_count = InterlockedDecrement(&render_ctx->pending_count);

	if (pending_count == 0) {
		if (in_irq() || irqs_disabled()) {
			struct tasklet_struct * tasklet = &render_ctx->tasklet;

			tasklet_init(
				tasklet,
				fl2000_render_completion_tasklet,
				(unsigned long) render_ctx);
			tasklet_schedule(tasklet);
		} else {
			fl2000_render_completion(render_ctx);
		}
	}
}

void fl2000_bulk_prepare_urb(
	struct dev_ctx * dev_ctx,
	struct render_ctx * render_ctx
	)
{
	struct primary_surface* const surface = render_ctx->primary_surface;
	struct scatterlist * const sglist = &surface->sglist[0];
	struct scatterlist * sg;
	unsigned int len = surface->xfer_length;
	unsigned int nr_pages = 0;
	unsigned int num_sgs = 0;
	unsigned int i;

	render_ctx->transfer_buffer = surface->render_buffer;
	render_ctx->transfer_buffer_length = surface->xfer_length;

	sg = &sglist[0];
	if (surface->render_buffer == surface->system_buffer &&
	    surface->type == SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT) {
		nr_pages = surface->nr_pages;

		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"surface->nr_pages(%u), start_offset(0x%x)",
			nr_pages, surface->start_offset);

		/*
		 * the buffer is from user mode address, where the start offset
		 * might not be zero
		 */
		sg_init_table(sglist, nr_pages);
		sg_set_page(
			sg,
			surface->pages[0],
			PAGE_SIZE - surface->start_offset,
			surface->start_offset);
		len -= PAGE_SIZE - surface->start_offset;
		num_sgs++;
		for (i = 1; i < nr_pages; i++) {
			struct page * pg = surface->pages[i];
#if (MERGE_ADJACENT_PAGES)
			struct page * prev_pg = surface->pages[i - 1];
			unsigned long this_pfn = page_to_pfn(pg);
			unsigned long prev_pfn = page_to_pfn(prev_pg);

			if (this_pfn != prev_pfn + 1) {
				sg = &sglist[num_sgs];
				num_sgs++;
				sg_set_page(sg, pg, 0, 0);
			}
#else
			sg = &sglist[num_sgs];
			num_sgs++;
			sg_set_page(sg, pg, 0, 0);
#endif

			/*
			 * update sg
			 */
			if (len > PAGE_SIZE) {
				sg->length += PAGE_SIZE;
				len -= PAGE_SIZE;
				}
			else {
				sg->length += len;
				dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
					"sglist[%u], len = 0x%x",
					num_sgs - 1, len);
			}
		}
	}
	else if (surface->render_buffer == surface->system_buffer &&
		 (surface->type == SURFACE_TYPE_VIRTUAL_CONTIGUOUS ||
		  surface->type == SURFACE_TYPE_PHYSICAL_CONTIGUOUS)) {
		sg_init_table(sglist, 1);
		sg_set_page(
			sg,
			surface->first_page,
			len - surface->start_offset,
			surface->start_offset);
		num_sgs++;
		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"sglist[%u], len = 0x%x",
			num_sgs - 1, len);
	}
	else {
		/*
		 * the buffer is allocated in kernel vmalloc space. the start
		 * offset should be zero.
		 */
		unsigned long start;
		unsigned long end;
		unsigned int start_offset;
		uint8_t * buf = surface->render_buffer;
		uint8_t * prev_buf;

		start = (unsigned long) buf & PAGE_MASK;
		end = (unsigned long) (buf + surface->xfer_length);
		start_offset = ((unsigned long) buf) & ~PAGE_MASK;
		nr_pages = (end - start + PAGE_SIZE - 1) >> PAGE_SHIFT;

		dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			"len(x%x), nr_pages(%u), start_offset(0x%x)",
			len, nr_pages, (unsigned int) start_offset);

		sg_init_table(sglist, nr_pages);
		sg_set_page(
			sg,
			vmalloc_to_page(buf),
			PAGE_SIZE - start_offset,
			start_offset);
		num_sgs++;
		prev_buf = buf;
		len -= PAGE_SIZE - start_offset;
		buf += PAGE_SIZE - start_offset;

		for (i = 1; i < nr_pages; i++) {
			struct page * pg = vmalloc_to_page(buf);
#if (MERGE_ADJACENT_PAGES)
			struct page * prev_pg = vmalloc_to_page(prev_buf);
			unsigned long this_pfn = page_to_pfn(pg);
			unsigned long prev_pfn = page_to_pfn(prev_pg);

			//dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			//	"this_pfn(%lx)/prev_pfn(%lx)", this_pfn, prev_pfn);

			if (this_pfn != prev_pfn + 1) {
				sg = &sglist[num_sgs];
				num_sgs++;
				sg_set_page(sg, pg, 0, 0);
			}
#else
			sg = &sglist[num_sgs];
			num_sgs++;
			sg_set_page(sg, pg, 0, 0);
#endif

			/*
			 * update sg
			 */
			if (len > PAGE_SIZE) {
				sg->length += PAGE_SIZE;
				len -= PAGE_SIZE;
				prev_buf = buf;
				buf += PAGE_SIZE;
				}
			else {
				sg->length += len;
				prev_buf = buf;
				buf += len;
			}
			//dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
			//	"sg[%u]->length(0x%x)", num_sgs - 1, sg->length);
		}
	}
	sg_mark_end(sg);

	dbg_msg(TRACE_LEVEL_INFO, DBG_RENDER,
		"num_sgs(%u)", num_sgs);

	usb_init_urb(render_ctx->main_urb);
	render_ctx->main_urb->num_sgs = num_sgs;
	render_ctx->main_urb->sg = sglist;

	usb_fill_bulk_urb(
		render_ctx->main_urb,
		dev_ctx->usb_dev,
		dev_ctx->usb_pipe_bulk_out,
		render_ctx->transfer_buffer,
		render_ctx->transfer_buffer_length,
		fl2000_bulk_main_completion,
		render_ctx);

	if (dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) {
		usb_init_urb(render_ctx->zero_length_urb);
		usb_fill_bulk_urb(
			render_ctx->zero_length_urb,
			dev_ctx->usb_dev,
			dev_ctx->usb_pipe_bulk_out,
			NULL,
			0,
			fl2000_bulk_zero_length_completion,
			render_ctx);
	}
}

// eof: fl2000_bulk.c
//
