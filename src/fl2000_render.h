// fl2000_render.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion file.
//

#ifndef _FL2000_RENDER_H_
#define _FL2000_RENDER_H_

int fl2000_render_create(struct dev_ctx * dev_ctx);
void fl2000_render_destroy(struct dev_ctx * dev_ctx);

void fl2000_render_start(struct dev_ctx * dev_ctx);
void fl2000_render_stop(struct dev_ctx * dev_ctx);

void fl2000_render_completion(struct render_ctx * render_ctx);
void fl2000_render_completion_tasklet(unsigned long data);

void fl2000_primary_surface_update(
	struct dev_ctx * 	dev_ctx,
	struct primary_surface* surface);

void fl2000_schedule_next_render(struct dev_ctx * dev_ctx);

#endif // _FL2000_RENDER_H_

// eof: fl2000_render.h
//
