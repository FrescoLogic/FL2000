// fl2000_dev.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose:
//

#ifndef _FL2000_DEV_H_
#define _FL2000_DEV_H_

int fl2000_dev_init(struct dev_ctx * dev_ctx);
void fl2000_dev_destroy(struct dev_ctx * dev_ctx);
int fl2000_dev_select_interface(struct dev_ctx * dev_ctx);

#endif // _FL2000_DEV_H_

// eof: fl2000_dev.h
//
