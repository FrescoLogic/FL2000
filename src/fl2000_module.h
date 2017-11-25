// fl2000_module.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_MOUDLE_H_
#define _FL2000_MOUDLE_H_

extern struct usb_driver fl2000_driver;

void fl2000_module_free(struct kref *kref);
int fl2000_open(struct inode * inode, struct file * file);
int fl2000_release(struct inode * inode, struct file * file);
long fl2000_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int fl2000_mmap(struct file * file, struct vm_area_struct *vma);
#endif // _FL2000_MODULE_H_

// eof: fl2000_module.h
//
