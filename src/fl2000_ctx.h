// fl2000_ctx.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_CTX_H_
#define _FL2000_CTX_H_

#define	GCC_ATOMIC_SUPPORT	1

/*
 * forward declaration
 */
struct dev_ctx;
struct render_ctx;

/*
 * schedule at least 2 video frames to the bus, to buy driver response time
 */
#define	NUM_RENDER_ON_BUS	2

struct resolution_entry
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	freq;
	uint32_t	h_total_time;
	uint32_t	h_sync_time;
	uint32_t	h_back_porch;
	uint32_t	v_total_time;
	uint32_t	v_sync_time;
	uint32_t	v_back_porch;
	uint32_t	isoch_num_of_pkt;
	uint32_t	isoch_td_size_in_kb;
	uint32_t	isoch_zero_bytes;
	uint32_t	itp_per_frame;
	uint32_t	num_of_idle;
	uint32_t	td_size;
	uint32_t	h_sync_reg_1;
	uint32_t	h_sync_reg_2;
	uint32_t	v_sync_reg_1;
	uint32_t	v_sync_reg_2;
	uint32_t	iso_reg;
	uint32_t	bulk_fpga_pll;
	uint32_t	bulk_asic_pll;
	uint32_t	isoch_fpga_pll;
	uint32_t	isoch_asic_pll;
	uint32_t	isoch_fpga_h_sync_reg_1;
	uint32_t	isoch_asic_h_sync_reg_1;
	uint32_t	bus_interval_adjust;
};

struct registry
{
	uint32_t FilterEdidTableEnable;
	uint32_t CurrentNumberOfDevices;
	uint32_t UsePollingMonitorConnection;

	uint32_t CompressionEnable;
	uint32_t Usb2PixelFormatTransformCompressionEnable;
};

struct vr_params
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	freq;
	uint32_t	h_total_time;
	uint32_t	h_sync_time;
	uint32_t	h_back_porch;
	uint32_t	v_total_time;
	uint32_t	v_sync_time;
	uint32_t	v_back_porch;
	uint32_t	trasfer_pipe;

	// Compression.
	//
	uint32_t	use_compression;
	uint32_t	compression_mask;
	uint32_t	compression_mask_index;
	uint32_t	compression_mask_index_min;
	uint32_t	compression_mask_index_max;
	uint32_t	compression_low_water_mark;	// this is for USB2.0 only
	uint32_t	compression_high_water_mark;	// this is for USB2.0 only
	uint32_t	compress_size_limit;		// generic compression size limit
	bool		dynamic_compression_mask;

	uint32_t	h_sync_reg_1;
	uint32_t	h_sync_reg_2;
	uint32_t	v_sync_reg_1;
	uint32_t	v_sync_reg_2;
	uint32_t	pll_reg;
	uint32_t	input_bytes_per_pixel;
	uint32_t	output_image_type;
	uint32_t	end_of_frame_type;
	uint32_t	color_mode_16bit;
};

#define	MAX_NUM_FRAGMENT	((MAX_BUFFER_SIZE + PAGE_SIZE - 1) >> PAGE_SHIFT)

struct primary_surface {
	struct list_head	list_entry;
	uint64_t		handle;
	uint64_t		user_buffer;
	uint32_t		buffer_length;
	uint32_t		xfer_length;
	uint32_t		width;
	uint32_t		height;
	uint32_t		pitch;
	uint32_t		color_format;
	uint32_t		type;
	uint32_t		frame_num;
	uint32_t		start_offset;
	uint64_t		physical_address;
	struct page *		first_page;
	uint8_t *		shadow_buffer;
	uint8_t *		mapped_buffer;	/* return from vm_map_ram */
	uint8_t *		system_buffer;	/* offset corrected buffer */
	uint8_t *		render_buffer;
	bool			pre_locked;

	struct page **		pages;
	unsigned int		nr_pages;
	int			pages_pinned;
	struct scatterlist 	sglist[MAX_NUM_FRAGMENT];

	/*
	 * compression working buffer
	 */
	uint8_t			compressed_buffer[MAX_BUFFER_SIZE];
	uint8_t			working_buffer[MAX_BUFFER_SIZE];
	uint32_t		compressed_buffer_size;
};

struct render_ctx {
	struct list_head	list_entry;

	struct dev_ctx *	dev_ctx;
	struct primary_surface*	primary_surface;
	void *			transfer_buffer;
	uint32_t		transfer_buffer_length;
	struct urb*		main_urb;
	struct urb*		zero_length_urb;
	uint32_t		pending_count;
	struct tasklet_struct	tasklet;
};

struct render {
	struct list_head	free_list;
	uint32_t		free_list_count;
	spinlock_t		free_list_lock;

	struct list_head	ready_list;
	uint32_t		ready_list_count;
	spinlock_t		ready_list_lock;

	struct list_head	busy_list;
	uint32_t		busy_list_count;
	spinlock_t		busy_list_lock;

	struct render_ctx	render_ctx[NUM_OF_RENDER_CTX];

	struct list_head	surface_list;
	uint32_t		surface_list_count;
	spinlock_t		surface_list_lock;

	struct display_mode	display_mode;
	uint32_t		last_frame_num;
	struct primary_surface* last_updated_surface;

	uint32_t		green_light;
};

struct dev_ctx {
	struct usb_device*		usb_dev;
	struct usb_device_descriptor	usb_dev_desc;
	struct kref			kref;

	/*
	 * control transfer scratch area.
	 * starting from some kernel version, the usb_control_msg no longer
	 * accept buffer from stack (which is in vmalloc area).
	 * so we have to allocate our scratch area so that the buffer
	 * usb core is happy with the buffer.
	 */
	uint32_t			ctrl_xfer_buf;

	/*
	 * some compiler (eg. arm-hisiv200-linux-gcc-4.4.1) does not provide
	 * __sync_xxx_and_fetch. kind of sucks. we use our sync lock here.
	 */
#if (!GCC_ATOMIC_SUPPORT)
	spinlock_t			count_lock;
#endif

	/*
	 * bulk out interface
	 */
	struct usb_interface*		usb_ifc_streaming;
	int				usb_pipe_bulk_out;

	/*
	 * interrupt in pipe related
	 */
	struct usb_interface*		usb_ifc_intr;
	struct usb_host_endpoint*	ep_intr_in;
	struct usb_endpoint_descriptor*	ep_desc_intr_in;
	int					ep_num_intr_in;
	int				usb_pipe_intr_in;
	struct urb*			intr_urb;
	uint8_t				intr_data;
	uint32_t			intr_pipe_pending_count;
	bool				intr_pipe_started;
	struct workqueue_struct *	intr_pipe_wq;
	struct work_struct		intr_pipe_work;

	struct registry			registry;

	/*
	 * flags and counters
	 */
	bool				monitor_plugged_in;
	bool				dev_gone;
	bool				in_flight_cancelling;
	uint32_t			card_name;

	bool				hdmi_chip_found;
	bool				hdmi_running_in_dvi_mode;
	bool				hdmi_powered_up;
	uint32_t			hdmi_audio_use_spdif;

	struct vr_params		vr_params;
	struct render			render;
	uint8_t				monitor_edid[8][EDID_SIZE];

	/*
	 * user mode app management
	 */
	uint32_t			open_count;
	wait_queue_head_t		ioctl_wait_q;

	/*
	 * SURFACE_TYPE_VIRTUAL_CONTIGUOUS/SURFACE_TYPE_PHYSICAL_CONTIGUOUS
	 * allocation management
	 */
	struct page *			start_page;
};

#if (GCC_ATOMIC_SUPPORT)
#define	InterlockedIncrement(x)		__sync_add_and_fetch(x, 1)
#define	InterlockedDecrement(x)		__sync_sub_and_fetch(x, 1)
#else
uint32_t fl2000_interlocked_increment(
	struct dev_ctx * 	dev_ctx,
	volatile uint32_t *	target
	);

uint32_t fl2000_interlocked_decrement(
	struct dev_ctx * 	dev_ctx,
	volatile uint32_t *	target
	);

#define	InterlockedIncrement(x)	fl2000_interlocked_increment(dev_ctx, x)
#define	InterlockedDecrement(x)	fl2000_interlocked_decrement(dev_ctx, x)
#endif // GCC_ATOMIC_SUPPORT

#endif // _FL2000_CTX_H_

// eof: fl2000_ctx.h
//
