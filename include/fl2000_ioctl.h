// fl2000_ioctl.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: IOCTL definitions for FL2000 Linux kernel driver.
//
// NOTE: THIS FILE MAY BE SENT OUTSIDE OF FRESCO LOGIC.

#ifndef _FL2000_IOCTL_H_
#define _FL2000_IOCTL_H_

#ifndef _EXTERN_C
  #ifdef __cplusplus
    #define _EXTERN_C	    extern "C"
    #define _EXTERN_C_START extern "C" {
    #define _EXTERN_C_END   }
  #else
    #define _EXTERN_C
    #define _EXTERN_C_START
    #define _EXTERN_C_END
  #endif
#endif

_EXTERN_C_START

#define FL2000_IOCTL_BASE	     0x200

/*
 * Theory of Operation
 *
 * This document describes interaction behavior between Linux user mode app and
 * Linux FL2000 kernel driver.
 *
 * 1. Upon FL2000 kernel mode driver initialization, the driver creates a
 *    usb class device in the sysfs tree by calling usb_register_dev(). The
 *    user mode app locates the USB device instance by locating FL2000 instance
 *    in sysfs tree.
 *
 * 2. The user app sends IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT to wait for
 *    monitor attach/detach event. In the case of monitor arrival, EDID data
 *    is returned along with the IOCTL. In the case of monitor detach event,
 *    the monitor connection status is cleared.
 *
 * 3. In addition to IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT, the user app could query
 *    the current monitor connection status by IOCTL_FL2000_QUERY_MONITOR_INFO
 *
 * 4. User app reports the monitor event to the graphics system, which later will
 *    commit a monitor mode. In response to mode change, the user mode app
 *    sends IOCTL_FL2000_SET_DISPLAY_MODE to enable a specific mode.
 *
 * 5. The user app creates one or more primary surfaces which will be displayed
 *    on the FL2000 monitor. The user app notifies the size and pitch of the
 *    primary surface by IOCTL_FL2000_CREATE_SURFACE. The kernel mode driver upon
 *    receiving such a IOCTL allocates a internal buffer for holding the pixels
 *    that will be updated later. The kernel driver uses internal buffer instead
 *    of source user mode buffer for following reasons:
 *
 *    - Cursor image will be overlaid to the shadow copy if the image, without
 *	touching the original user mode buffer.
 *    - The user mode surface buffer might not always be valid at all time. In
 *	some cases, the user mode buffer is only valid when it is mapped from GPU.
 *    - The source surface format is not identical to 24bpp, and kernel mode driver
 *	need convert it to 24bpp for hardware use.
 *
 *    Depending on the user app implementation, it is possible to optimize the
 *    system by skipping extra copy of kernel buffer. If the user app guarantees
 *    that the user mode buffer is always resident, and it does not require kernel
 *    driver to overlay cursor image to the driver, the kernel driver could directly
 *    maps the user buffer for USB transfer.
 *
 * 6. When the user app determines that a surface is updated, it notifies the
 *    kernel driver by IOCTL_FL2000_NOTIFY_SURFACE_UPDATE. Along with the IOCTL,
 *    SURFACE_UPDATE_INFO structure is passed to the kernel driver. The kernel
 *    driver locks down the user buffer, and copy the user buffer pixels to
 *    its internal buffer. If the primary surface is cerated with type set to
 *    SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT, SURFACE_TYPE_VIRTUAL_CONTIGUOUS,
 *    or SURFACE_TYPE_PHYSICAL_CONTIGUOUS, the kernel driver skips the memory
 *    copy operation, and directly maps the user buffer for DMA operation. Note
 *    that the DMA operation is subjected to underlying host stack (eg. XHC driver)
 *    scheduling, the user buffer is locked down until the DMA complete.
 *
 *    If user app makes sure that a user mode buffer is always resident, the user
 *    app could help to optimize the driver by pre-locking down the user buffer.
 *    The user app could prelock down user buffer by IOCTL_FL2000_LOCK_SURFACE.
 *
 * 7. If the app is no longer using FL2000, the app should unlock previous locked
 *    down surfaces by IOCTL_FL2000_UNLOCK_SURFACE. Likewise, the app should release
 *    previously created primary surface by IOCTL_FL2000_DESTROY_SURFACE.
 *
 * 8. When a monitor is set to "disabled" state, the user app sends
 *    IOCTL_FL2000_SET_DISPLAY_MODE with width/height = 0. The kernel driver cuts
 *    off the monitor signal (HDMI/DSUB/...etc) when receiving such an IOCTL.
 *
 * 7. Cursor is supported by IOCTL_FL2000_NOTIFY_POINTER_POSITION_UPDATE/
 *    IOCTL_FL2000_NOTIFY_POINTER_SHAPE_UPDATE. The user app is responsible
 *    acquiring cursor image, position, and position before notifying the cursor
 *    event to kernel driver.
 */

/*
 * FL2000 device presence detection
 *
 * Due to PNP nature, the FL2000 device can be unplugged during normal user app
 * operation. When a FL2000 device is disconnected, the user app must release
 * any opened file to the /dev/fl2000-0(or similar) for the driver to completely
 * releases itself. Failure to do so would cause driver lock-up.
 *
 * The following method is provided to detect FL2000 device connection.
 * 1. Polling
 *    The user app can constantly checking the existence of /dev/fl2000-0.
 *    If the FL2000 device is unplugged, the driver deregister this device name.
 * 2. Using IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT.
 *    User app can issues this IOCTL in a separate thread.  The kernel driver
 *    wakes up the waiting thread, and updates fl2000_flags and monitor_flags.
 *    User app can detect monitor and FL2000 arrival/departure event by using
 *    this IOCTL.
 */

/*
 * Name:  IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT
 *
 * details
 *  If a monitor is detected, the kernel driver updates the flags.connected = 1,
 *  and return EDID (128 bytes) along the way. If monitor departure event is reported,
 *  the kernel driver updates flags.Connected = FALSE;
 *
 * parameters
 *    InputBuffer:	    NULL
 *    InputBufferSize:	    0
 *    OutputBuffer:	    pointer to (struct monitor_info)
 *    OutputBufferSize:	    sizeof(struct monitor_info)
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
struct monitor_flags {
	uint32_t	connected: 1;	// currently only Connected bit is defined.
};

struct fl2000_flags {
	uint32_t	connected: 1;	// set to 1 is fl2000 is connected.
};

/*
 * definitions borrowed/adapted from wingdi.h
 */
enum output_technology {
	OUTPUT_TECHNOLOGY_OTHER			= -1,
	OUTPUT_TECHNOLOGY_HD15			= 0,
	OUTPUT_TECHNOLOGY_SVIDEO		= 1,
	OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO	= 2,
	OUTPUT_TECHNOLOGY_COMPONENT_VIDEO	= 3,
	OUTPUT_TECHNOLOGY_DVI			= 4,
	OUTPUT_TECHNOLOGY_HDMI			= 5,
	OUTPUT_TECHNOLOGY_LVDS			= 6,
	OUTPUT_TECHNOLOGY_D_JPN			= 8,
	OUTPUT_TECHNOLOGY_SDI			= 9,
	OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL	= 10,
	OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED	= 11,
	OUTPUT_TECHNOLOGY_UDI_EXTERNAL		= 12,
	OUTPUT_TECHNOLOGY_UDI_EMBEDDED		= 13,
	OUTPUT_TECHNOLOGY_SDTVDONGLE		= 14,
	OUTPUT_TECHNOLOGY_MIRACAST		= 15,
	OUTPUT_TECHNOLOGY_INDIRECT_WIRED	= 16,
	OUTPUT_TECHNOLOGY_INTERNAL		= 0x80000000,
	OUTPUT_TECHNOLOGY_FORCE_UINT32		= 0xFFFFFFFF
};

#define EDID_SIZE	128
struct monitor_info {
	struct monitor_flags	monitor_flags;
	struct fl2000_flags	fl2000_flags;
	enum output_technology	monitor_type;
	uint8_t			edid[EDID_SIZE];
};

#define IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT	    (FL2000_IOCTL_BASE)

/*
 * Name:  IOCTL_FL2000_QUERY_MONITOR_INFO
 *
 * details
 *  If a monitor is currently attached, the kernel driver updates the flags.connected = 1,
 *  and return EDID (128 bytes) along the way. If monitor is not currently attached,
 *  the kernel driver updates flags.connected = 0;
 *
 * parameters
 *    InputBuffer:	    NULL
 *    InputBufferSize:	    0
 *    OutputBuffer:	    pointer to struct monitor_info
 *    OutputBufferSize:	    sizeof(struct monitor_info)
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
#define IOCTL_FL2000_QUERY_MONITOR_INFO		    (FL2000_IOCTL_BASE + 1)

/*
 * Color format
 *
 * The FL2000 hardware supports the following surface format:
 *
 * RGB_24 : Each pixel is comprised of 3 bytes and B occurs on byte[0], and
 * 	G occurs on byte[1], and R occurs on byte[2]. This is the default format.
 *
 * RGB_16_565: Each pixel is comprised by 16 bits in little endian order. The
 *  	byte[0] is the least significant byte. Color B occurs on bits[0~4] of
 *      byte[0], Color G occurs on bits[5~7] of byte[0] and bits[0~2] of byte[1],
 *    	Color R occurs on bits[3~7] of byte[1]. This color format is not encourged
 *      since it gives worse user experience.
 *
 * RGB_16_555: Each pixel is comprised by 16 bits in little endian order. The
 *  	byte[0] is the least significant byte. Color B occurs on bits[0~4] of
 *      byte[0], Color G occurs on bits[5~7] of byte[0] and bits[0~1] of byte[1],
 *    	Color R occurs on bits[2~6] of byte[1]. The most signicant bit of byte[1]
 *      is not used. This color format is not encourged since it delivers worse
 *      user experience.
 *
 * The API allows the user input color format differs from the actual color format
 * used by hardware. When there is color format mismatch, the kernel driver convert
 * user input color format into output color format. When color format mismatch
 * occurs, significant CPU penalty is paid to compensate the color conversion.
 * It is suggested that user input color format always matches the output color
 * format.
 */
#define	COLOR_FORMAT_RGB_24		0
#define	COLOR_FORMAT_RGB_16_565		1
#define	COLOR_FORMAT_RGB_16_555		2

/*
 * Name:  IOCTL_FL2000_SET_DISPLAY_MODE
 *
 * details
 *  The user app sends this IOCTL to enable a certain display mode. Upon receiving
 *  such IOCTL, the driver programs the FL2000 hardware to select specific
 *  resolution, and particular surface format (24bpp or 16bpp). The default format
 *  is RGB 24bpp, where the B occurs on the byte[0], and G occurs on byte[1],
 *  and R occurs on byte[2].
 *
 * parameters
 *    InputBuffer:	    pointer to display_mode
 *    InputBufferSize:	    sizeof(display_mode)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */

struct display_mode {
	uint32_t	width;
	uint32_t	height;
	uint32_t	refresh_rate;
	uint32_t	input_color_format;
	uint32_t	output_color_format;
	uint32_t	use_compression;
	uint32_t	transfer_pipe;

	/*
	 * work-around FL2000 compatibility with some low-bandwidth host controller
	 * FL2000 is a USB bandwith hog, and requires the host driver to pump 60 frames
	 * per second. For some low performance xHC, the bandwidth required by FL2000
	 * might not meet what the xHC can offer. For example, some SoC can do 100MB/s.
	 * In order to send 60 frames in 1 second, each frame has to be compressed to
	 * below 100*1000*1000 / 60 = 1,666,666 bytes.
	 * In this case, we need to set compress_size_limit = 1,666,666
	 */
	uint32_t	compress_size_limit;
};
#define IOCTL_FL2000_SET_DISPLAY_MODE		    (FL2000_IOCTL_BASE + 2)

/*
 * Name:  IOCTL_FL2000_CREATE_SURFACE
 *
 * details
 *  The user app notifies the kernel driver that a primary surface is created.
 *  Along with the creation, the user app specifies primary surface handle, the
 *  width/height of the surface. The kernel driver then creates a shadow buffer
 *  which will be used later. When later a user mode primary surface is updated,
 *  the kernel driver copies pixels to the shadow buffer before starting a USB
 *  transfer.
 *
 *  Depending on how the surface created, the memory storage where the pixels are
 *  backed falls into the following categories:
 *
 *  1. fragmented virtual memory
 *  The surface buffer is created by malloc() or anonymous mmap(). In this case,
 *  the physical memory of the buffer is fragmented in most cases. The kernel
 *  driver locks down the physical pages associated with the user buffer before
 *  accessing the buffer. Since the physical pages lock-down takes considerable
 *  CPU overhead, user app can help to optimize the overhead by specifying that
 *  the buffer is persistent. To do so, the user app can set type to
 *  SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT. When driver sees such a surface,
 *  the driver locks down the user pages in response to IOCTL_FL2000_CREATE_SURFACE.
 *  The kernel driver unlocks the user pages until after IOCTL_FL2000_DESTROY_SURFACE
 *  is made.
 *
 *  If the user buffer would be destroyed after IOCTL_FL2000_NOTIFY_SURFACE_UPDATE,
 *  then user app should set type to SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE.
 *  The primary surface buffer is only accessed during IOCTL_FL2000_NOTIFY_SURFACE_UPDATE
 *  processing, and kernel driver would not lock the buffer after
 *  IOCTL_FL2000_NOTIFY_SURFACE_UPDATE is returned to the caller.
 *
 *  2. contiguous virtual memory
 *  The surface buffer is allocated by graphics driver where the buffer resides
 *  on contiguous physical memory, and is mapped to user space. In this case,
 *  the user app could set type to SURFACE_TYPE_VIRTUAL_CONTIGUOUS.
 *
 *  3. contiguous physical memory
 *  The surface buffer is allocated by graphics driver where the buffer resides
 *  on contiguous physical memory, and the buffer is not mapped to user space.
 *  In this case, the user app could set type to SURFACE_TYPE_PHYSICAL_CONTIGUOUS.
 *
 * parameters
 *    InputBuffer:	    pointer to surface_info
 *    InputBufferSize:	    sizeof(surface_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
#define SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE        0
#define SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT      1
#define SURFACE_TYPE_VIRTUAL_CONTIGUOUS                 2
#define SURFACE_TYPE_PHYSICAL_CONTIGUOUS                3

struct surface_info {
	uint64_t	handle;
	uint64_t	user_buffer;
	uint64_t	buffer_length;
	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;
	uint32_t	color_format;
	uint32_t	type;
};

#define IOCTL_FL2000_CREATE_SURFACE		    (FL2000_IOCTL_BASE + 3)

/*
 * Name:  IOCTL_FL2000_DESTROY_SURFACE
 *
 * details
 *  The user app notifies kernel driver that a primary surface is to be destroyed.
 *  The user app specifies primary surface handle, as well as the  width/height/
 *  pitch of the surface. The kernel driver destroys the shadow buffer previously
 *  created in IOCTL_FL2000_CREATE_SURFACE.
 *
 * parameters
 *    InputBuffer:	    pointer to surface_info
 *    InputBufferSize:	    sizeof(surface_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
#define IOCTL_FL2000_DESTROY_SURFACE		    (FL2000_IOCTL_BASE + 4)

/*
 * Name:  IOCTL_FL2000_NOTIFY_SURFACE_UPDATE
 *
 * details
 *  The client notifies kernel driver that a primary surface is updated.
 *  The user app specifies primary surface handle, as well as the user buffer
 *  information. The kernel driver copies the user buffer into shadow buffer
 *  which was created previously, and send the updated pixels to device.
 *
 *  Depending on the surface creation type, the kernel driver does different
 *  processing as described below:
 *
 *  1. The surface is created with SURFACE_TYPE_VIRTUAL_FRAGMENTED_VOLATILE:
 *  The kernel driver locks down the user buffer, and copy the pixels to its
 *  shadow buffer before unlocking the user buffer. Since the page lock-down
 *  and unlock operation is costly, it is not suggested to use this mode.
 *
 *  2. The surface is created with SURFACE_TYPE_VIRTUAL_FRAGMENTED_PERSISTENT:
 *  The kernel driver pre-lock down the user buffer, and do not copy the pixels
 *  to shadow buffer. The kernel driver programs the URB such that it utilizes
 *  scatter-gather capability and the user pages are directly fed to the underlying
 *  xHC driver. This mode hence improves CPU performance.
 *
 *  3. The surface is created with SURFACE_TYPE_VIRTUAL_CONTIGUOUS:
 *  The kernel driver does not lock down the user buffer, since it is persistent
 *  by its nature. The driver derives the physical address of the virutal address
 *  during IOCTL_FL2000_CREATE_SURFACE, and uses the physical address directly
 *  in programming the URB.
 *
 *  4. The surface is created with SURFACE_TYPE_PHYSICAL_CONTIGUOUS:
 *  The kernel driver does not lock down the user buffer, since it is persistent
 *  by its nature.
 *
 *  Once the buffer is displayed, the kernel driver would rescan the same buffer
 *  until a new buffer update event arrives. The kernel driver continues to scan
 *  out the same buffer until next IOCTL_FL2000_NOTIFY_SURFACE_UPDATE arrives.
 *
 * parameters
 *    InputBuffer:	    pointer to surface_update_info
 *    InputBufferSize:	    sizeof(surface_update_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
struct surface_update_info {
	uint64_t	handle;
	uint64_t	user_buffer;
	uint64_t	buffer_length;
};
#define IOCTL_FL2000_NOTIFY_SURFACE_UPDATE	    (FL2000_IOCTL_BASE + 5)

/*
 * Name:  IOCTL_FL2000_LOCK_SURFACE/IOCTL_FL2000_UNLOCK_SURFACE
 *
 * details
 *  The client notifies kernel driver that a primary surface is always resident,
 *  and that kernel driver locks down the user mode buffer for optimization. The
 *  user buffer stays locked until IOCTL_FL2000_UNLOCK_SURFACE is made.
 *
 *  The kernel driver get_user_pages() on the input buffer when IOCTL_FL2000_LOCK_SURFACE
 *  is called. Once the user_buffer is locked down, the kernel driver skips the
 *  buffer lock down operation in subsequent IOCTL_FL2000_NOTIFY_SURFACE_UPDATE.
 *  This effectively saves the page table operation overhead.
 *
 *  When a user_buffer is no longer in used, the user app could notify the kernel
 *  driver to unlock the buffer previously locked down by IOCTL_FL2000_LOCK_SURFACE.
 *  The kernel driver proceed to release the locked pages with page_cache_release().
 *
 *  The kernel driver does not prohibit user app to touch the content of a buffer.
 *
 * parameters
 *    InputBuffer:	    pointer to surface_update_info
 *    InputBufferSize:	    sizeof(surface_update_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
#define IOCTL_FL2000_LOCK_SURFACE		    (FL2000_IOCTL_BASE + 6)
#define IOCTL_FL2000_UNLOCK_SURFACE		    (FL2000_IOCTL_BASE + 7)

/*
 * Name:  IOCTL_FL2000_NOTIFY_POINTER_POSITION_UPDATE
 *
 * details
 *  The user app notifies kernel driver that a pointer position is updated.
 *
 * parameters
 *    InputBuffer:	    pointer to pointer_position_info
 *    InputBufferSize:	    sizeof(pointer_position_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
struct pointer_position_info {
	/*
	 * updated by IddCxMonitorQueryHardwareCursor
	 */
	int32_t		x;
	int32_t		y;
	uint8_t		visible;
};

#define IOCTL_FL2000_NOTIFY_POINTER_POSITION_UPDATE (FL2000_IOCTL_BASE + 8)

/*
 * Name:  IOCTL_FL2000_NOTIFY_POINTER_SHAPE_UPDATE
 *
 * details
 *  The user app notifies kernel driver that the pointer shape is updated.
 *
 * parameters
 *    InputBuffer:	    pointer to pointer_shape_info
 *    InputBufferSize:	    sizeof(pointer_shape_info)
 *    OutputBuffer:	    NULL
 *    OutputBufferSize:	    0
 *
 * return value
 *  0 if succeeded. -1 on error.
 */
#define CURSOR_WIDTH	    64
#define CURSOR_HEIGHT	    64
#define CURSOR_SHAPE_SIZE   (CURSOR_WIDTH * CURSOR_HEIGHT * 4 * 2)
struct pointer_shape_info {
	uint32_t	cursor_type;
#define CURSOR_SHAPE_TYPE_MASKED_COLOR	1
#define CURSOR_SHAPE_TYPE_ALPHA		2

	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;
	uint8_t		bitmap[CURSOR_SHAPE_SIZE];
	uint32_t	x_hot;
	uint32_t	y_hot;
};

#define IOCTL_FL2000_NOTIFY_POINTER_SHAPE_UPDATE    (FL2000_IOCTL_BASE + 9)

#define IOCTL_FL2000_TEST_ALLOC_SURFACE             (FL2000_IOCTL_BASE + 10)
#define IOCTL_FL2000_TEST_RELEASE_SURFACE           (FL2000_IOCTL_BASE + 11)
struct test_alloc {
	uint64_t	buffer_size;		// input
	uint64_t	usr_addr;		// output
	uint64_t	phy_addr;		// output
};
_EXTERN_C_END

#endif /*  _FL2000_IOCTL_H_ */
