// fl2000_surface.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion File.
//
#ifndef _FL2000_SURFACE_H_
#define _FL2000_SURFACE_H_

int fl2000_surface_pin_down(
	struct dev_ctx * dev_ctx,
	struct primary_surface* surface);

void fl2000_surface_unpin(
	struct dev_ctx * dev_ctx,
	struct primary_surface* surface);

int fl2000_surface_map(
	struct dev_ctx * dev_ctx,
	struct primary_surface* surface);

void fl2000_surface_unmap(
	struct dev_ctx * dev_ctx,
	struct primary_surface* surface);

int fl2000_surface_create(
	struct dev_ctx * dev_ctx,
	struct surface_info * info);

void fl2000_surface_destroy(
	struct dev_ctx * dev_ctx,
	struct primary_surface* surface);

void fl2000_surface_destroy_all(struct dev_ctx * dev_ctx);
#endif