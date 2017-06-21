// fl2000_monitor.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose:
//

#ifndef _FL2000_MONITOR_H_
#define _FL2000_MONITOR_H_

bool fl2000_monitor_set_resolution(struct dev_ctx * dev_ctx, bool pll_changed);

void fl2000_monitor_read_edid(struct dev_ctx * dev_ctx);

bool fl2000_monitor_resolution_in_white_table(
	uint32_t width,
	uint32_t height,
	uint32_t freq);

void fl2000_monitor_manual_check_connection(struct dev_ctx * dev_ctx);

void fl2000_monitor_vga_status_handler(
	struct dev_ctx * dev_ctx, uint32_t raw_status
	);

#endif // _FL2000_MONITOR_H_

// eof: fl2000_monitor.h
//
