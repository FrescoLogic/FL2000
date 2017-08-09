// fl2000_include.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// The contents of this file are property of Fresco Logic, Incorporated and are strictly protected
// by Non Disclosure Agreements. Distribution in any form to unauthorized parties is strictly prohibited.
//
// Purpose:
//

#ifndef _FL2000_INCLUDE_H_
#define _FL2000_INCLUDE_H_

/////////////////////////////////////////////////////////////////////////////////
// Driver Compiliation Parameters
/////////////////////////////////////////////////////////////////////////////////
//

#define NO_USE_SUPPORT_MMX

/////////////////////////////////////////////////////////////////////////////////
// OS and C Includes
/////////////////////////////////////////////////////////////////////////////////
//

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/prefetch.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/pagemap.h>
#include <linux/scatterlist.h>

#include "fl2000_ioctl.h"
#include "fl2000_linux.h"
#include "fl2000_def.h"
#include "fl2000_ctx.h"
#include "fl2000_dev.h"

#include "fl2000_bulk.h"
#include "fl2000_dongle.h"
#include "fl2000_render.h"
#include "fl2000_log.h"
#include "fl2000_register.h"
#include "fl2000_i2c.h"
#include "fl2000_monitor.h"
#include "fl2000_desc.h"

#include "fl2000_big_table.h"
#include "fl2000_compression.h"

#include "fl2000_module.h"
#include "fl2000_interrupt.h"
#include "fl2000_surface.h"

#include "fl2000_hdmi.h"

#endif // _FL2000_INCLUDE_H_

// eof: fl2000_include.h
//
