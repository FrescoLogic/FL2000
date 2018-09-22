// fl2000_dongle.h
//
// (c)Copyright 2009-2013, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_DONGLE_H_
#define _FL2000_DONGLE_H_

void fl2000_dongle_u1u2_setup(struct dev_ctx * dev_ctx, bool enable);

void fl2000_dongle_reset(struct dev_ctx * dev_ctx);
void fl2000_dongle_stop(struct dev_ctx * dev_ctx);

int fl2000_set_display_mode(
	struct dev_ctx * dev_ctx,
	struct display_mode * display_mode
	);

void fl2000_dongle_card_initialize(struct dev_ctx * dev_ctx);

#endif // _FL2000_DONGLE_H_

// eof: fl2000_dongle.h
//
