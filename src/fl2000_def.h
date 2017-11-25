// fl2000_def.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_DEF_H_
#define _FL2000_DEF_H_

/////////////////////////////////////////////////////////////////////////////////
// Global Constants Definition
/////////////////////////////////////////////////////////////////////////////////
//

#define VID_FRESCO_LOGIC			0x1D5C
#define PID_FL2000                              0x2000

#define DEVICE_ID_FL2000                        0x0100
#define DEVICE_ID_FL2000DX                      0x0200

#define CARD_NAME_UNDEFINED                     0
#define CARD_NAME_SHUTTLE_ASIC                  1
#define CARD_NAME_FL2000                        2
#define CARD_NAME_FL2000DX                      3

#define DELAY_FOR_WAIT_PLL_STABLE               300

#define DELAY_FOR_I2C_CONNECTION_ENABLE         750

#define FREQUENCY_BUFFER_MAX                    20

#define POOL_TAG 				(uint32_t) 'fl2k'

#define VGA_MMIO_READ                           1
#define VGA_MMIO_WRITE                          0

#define IMAGE_ASPECT_RATIO_16_10                0
#define IMAGE_ASPECT_RATIO_4_3                  1
#define IMAGE_ASPECT_RATIO_5_4                  2
#define IMAGE_ASPECT_RATIO_16_9                 3

#define FL2000_IFC_AV				0
#define FL2000_IFC_STREAMING			1
#define FL2000_IFC_INTERRUPT			2
#define FL2000_IFC_MASS_STORAGE			3

#define INT_STATUS_ERROR                        0x00
#define INT_STATUS_VGA_EVENT                    0x01
#define INT_STATUS_ISOCH_ADJUST_FRAME           0x02

#define VGA_EVENT_MONITOR_PLUG_OUT              0x80000000
#define VGA_EVENT_MONITOR_PLUG_IN               0x80000001
#define VGA_EVENT_MONITOR_PLUG_IN_ERROR_PATCH   0x20000041
#define VGA_EVENT_ISOCH_SW_RECOVER              0x80000003

#define DROP_FRAME_ERROR                        0x00000002 
#define BUFFER_UNDERFLOW                        0x00000100 
#define BUFFER_OVERFLOW                         0x00000200 

#define USB2_BCD                              	0x0210
#define USB3_BCD                              	0x0300

#define MAX_WIDTH                          	1920
#define MAX_HEIGHT                         	1080
#define	MAX_BUFFER_SIZE				(MAX_WIDTH * MAX_HEIGHT * 3)

#define EDID_SIZE                          	128
#define NUM_OF_RENDER_CTX                       4

#define IS_DEVICE_USB3LINK(dev_ctx)             (USB3_BCD == dev_ctx->usb_dev_desc.bcdUSB)
#define IS_DEVICE_USB2LINK(dev_ctx)             (!IS_DEVICE_USB3LINK(dev_ctx))

#define OUTPUT_IMAGE_TYPE_RGB_8              	0
#define OUTPUT_IMAGE_TYPE_RGB_16             	1
#define OUTPUT_IMAGE_TYPE_RGB_24             	2

#define VR_USB_LINKUP_TYPE_NONE             	0
#define VR_USB_LINKUP_TYPE_USB2             	1
#define VR_USB_LINKUP_TYPE_USB3             	2

#define VR_TRANSFER_PIPE_BULK                	0
#define VR_TRANSFER_PIPE_ISOCH               	1
#define VR_TRANSFER_PIPE_MAX                 	2

#define PIXEL_BYTE_1                            1
#define PIXEL_BYTE_2                            2
#define PIXEL_BYTE_3                            3

#define EOF_PENDING_BIT       			0
#define EOF_ZERO_LENGTH       			1

#define VR_16_BIT_COLOR_MODE_565                0
#define VR_16_BIT_COLOR_MODE_555                1

#define EDID_FILTER_USB2_800_600_60HZ		1
#define EDID_FILTER_USB2_640_480_60HZ           2

#define MAX_SIZE_OF_EDIT_TABLE                  128

#define EDID_HEADER_DWORD1                      0xFFFFFF00
#define EDID_HEADER_DWORD2                      0x00FFFFFF

#define CTRL_XFER_TIMEOUT       		2000

#endif // _FL2000_DEF_H_

// eof: fl2000_def.h
//
