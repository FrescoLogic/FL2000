// fl2000_module.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

uint32_t currentTraceLevel = TRACE_LEVEL_INFO;
uint32_t currentTraceFlags = DEFAULT_DBG_FLAGS;

static int
fl2000_device_probe(
	struct usb_interface* usb_interface,
	const struct usb_device_id* usb_device_id
	);

static void
fl2000_disconnect(
	struct usb_interface *usb_interface
	);

static struct usb_device_id fl2000_id_table[] = {
	{
	.idVendor =		0x1D5C,
	.idProduct =		0x2000,
	.bInterfaceClass =	0x10,
	.bInterfaceSubClass =	0x02,
	.bInterfaceProtocol =	0x00,
	.match_flags =		USB_DEVICE_ID_MATCH_VENDOR | 	\
				USB_DEVICE_ID_MATCH_PRODUCT | 	\
				USB_DEVICE_ID_MATCH_INT_SUBCLASS,
	},
	{},
};
MODULE_DEVICE_TABLE(usb, fl2000_id_table);

struct usb_driver fl2000_driver = {
	.name 		= "fl2000",
	.probe 		= fl2000_device_probe,
	.disconnect 	= fl2000_disconnect,
	.id_table 	= fl2000_id_table,
};

static const struct file_operations fl2000_fops = {
	.owner		= THIS_MODULE,
	.open		= fl2000_open,
	.release	= fl2000_release,
	.unlocked_ioctl	= fl2000_ioctl,
	.compat_ioctl	= fl2000_ioctl,
	.mmap		= fl2000_mmap,
};

static struct usb_class_driver fl2000_class_driver = {
	.name		= "fl2000-%d",
	.fops		= &fl2000_fops,
	.minor_base	= 128,
};

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//
static int
fl2000_device_probe(
	struct usb_interface*  ifc,
	const struct usb_device_id* usb_device_id)
{
	struct usb_device*	usb_dev;
	struct dev_ctx *	dev_ctx = NULL;
	int			ret_val;
	struct usb_host_config*	config;
	struct usb_interface*	other_ifc;
	int			index;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"bInterfaceNumber:%d bAlternateSetting:%d",
		ifc->cur_altsetting->desc.bInterfaceNumber,
		ifc->cur_altsetting->desc.bAlternateSetting);

	ret_val = 0;
	usb_dev = interface_to_usbdev(ifc);

	config = usb_dev->actconfig;
	for (index = 0; index < USB_MAXINTERFACES; index ++) {
		other_ifc = config->interface[index];
		if (other_ifc && other_ifc != ifc) {
			dev_ctx = usb_get_intfdata(other_ifc);
			if (dev_ctx)
				break;
		}
	}

	if (!dev_ctx) {
		dev_ctx = kzalloc(sizeof(struct dev_ctx), GFP_KERNEL);
		if (!dev_ctx) {
		dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP,
			"ERROR Allocate dev_ctx failed." );
		goto exit;
		}

		kref_init(&dev_ctx->kref);
	}
	else {
		kref_get(&dev_ctx->kref);
	}

	dev_ctx->usb_dev = usb_dev;
	usb_set_intfdata(ifc, dev_ctx);

	switch (ifc->cur_altsetting->desc.bInterfaceNumber) {
	case FL2000_IFC_STREAMING:
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"streaming interface detected");
		ret_val = usb_register_dev(ifc, &fl2000_class_driver);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"usb_register_dev failed with %d?.", ret_val);
		}

		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"usb_dev(%p), minor_num(%d), dev_name(%s) created",
			ifc->usb_dev,
			ifc->minor,
			dev_name(ifc->usb_dev));

		dev_ctx->usb_ifc_streaming = ifc;
		fl2000_desc_init(dev_ctx);

		ret_val = fl2000_dev_init(dev_ctx);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"fl2000_dev_init failed?.");
			goto exit;
		}
		break;

	case FL2000_IFC_INTERRUPT:
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"Detect interrupt interface.");
		dev_ctx->usb_ifc_intr = ifc;

		ret_val = fl2000_intr_pipe_create(dev_ctx);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"fl2000_intr_pipe_create failed?.");
			goto exit;
		}

		ret_val = fl2000_intr_pipe_start(dev_ctx);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"fl2000_intr_pipe_start failed?.");
			fl2000_intr_pipe_destroy(dev_ctx);
			goto exit;
		}
		break;

	default:
		/*
		 * don't care AVCONTROL interface
		 */
		break;
	}

exit:
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"Interface(ifc_num:%d, alt_setting:%d) failed!",
			ifc->cur_altsetting->desc.bInterfaceNumber,
			ifc->cur_altsetting->desc.bAlternateSetting);

		if (dev_ctx)
			fl2000_module_free(&dev_ctx->kref);
	}

    return ret_val;
}

static void
fl2000_disconnect(struct usb_interface *ifc)
{
	struct dev_ctx * dev_ctx;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"bInterfaceNumber:%d bAlternateSetting:%d",
		ifc->cur_altsetting->desc.bInterfaceNumber,
		ifc->cur_altsetting->desc.bAlternateSetting);

	dev_ctx = usb_get_intfdata(ifc);
	dev_ctx->dev_gone = true;

	// wake up any sleeping process.
	//
	if (waitqueue_active(&dev_ctx->ioctl_wait_q)) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"wake up pending process");
		wake_up_interruptible(&dev_ctx->ioctl_wait_q);
	}

	switch (ifc->cur_altsetting->desc.bInterfaceNumber) {
	case FL2000_IFC_STREAMING:
		fl2000_render_stop(dev_ctx);
		fl2000_dongle_stop(dev_ctx);
		usb_deregister_dev(ifc, &fl2000_class_driver);
		usb_set_interface(
			dev_ctx->usb_dev,
			ifc->cur_altsetting->desc.bInterfaceNumber,
			0);
		break;
	case FL2000_IFC_INTERRUPT:
		fl2000_intr_pipe_stop(dev_ctx);
		fl2000_intr_pipe_destroy(dev_ctx);
		break;

	default:
		/*
		 * don't care AVCONTROL interface
		 */
		break;
	}

	usb_set_intfdata(ifc, NULL);

	kref_put(&dev_ctx->kref, fl2000_module_free);

	//dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
	//	"usb_dev ReferenceCount:%d after kref_put.",
	//	dev_ctx->kref.refcount.counter);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

void
fl2000_module_free(struct kref *kref)
{
	struct dev_ctx * dev_ctx;

	dev_ctx = container_of(kref, struct dev_ctx, kref);
	fl2000_dev_destroy(dev_ctx);
	kfree(dev_ctx);
}

int __init
fl2000_module_init(void)
{
	int result;

	result = usb_register(&fl2000_driver);
	return result;
}

void __exit
fl2000_module_exit(void)
{
	usb_deregister(&fl2000_driver);
}

module_init(fl2000_module_init);
module_exit(fl2000_module_exit);

MODULE_AUTHOR("Fresco Logic");
MODULE_DESCRIPTION("USB VGA device driver - Version 0.0.0.1");
MODULE_LICENSE("GPL");

// eof: fl2000_module.c
//
