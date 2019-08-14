// fl2000_dongle.c
//
// (c)Copyright 2009-2013, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//
void
fl2000_dongle_i2c_detection_enable(struct dev_ctx * dev_ctx)
{
	// Enable I2C VGA Detection
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8020, 30);
	DELAY_MS(DELAY_FOR_I2C_CONNECTION_ENABLE);
}

void
fl2000_dongle_ext_monitor_detect_enable(struct dev_ctx * dev_ctx)
{
	// Enable external monitor detection.
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8020, 28);
}

void fl2000_dongle_init_fl2000dx(struct dev_ctx * dev_ctx)
{
	// Enable interrupt for I2C detection and external monitor.
	//
	fl2000_dongle_i2c_detection_enable(dev_ctx);
	fl2000_dongle_ext_monitor_detect_enable(dev_ctx);

	// BUG: We turn-off hardward reset for now.
	// But we do need it for resolve accumulate interrupt packet issue.
	// Got debug with NJ for this problem.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8088, 10);

	// Disable polling for FL2000DX.
	//
	dev_ctx->registry.UsePollingMonitorConnection = 0;

	dev_ctx->registry.FilterEdidTableEnable = EDID_FILTER_USB2_800_600_60HZ;

	// Compression registry and flags.
	//
	dev_ctx->registry.CompressionEnable = 0;
	dev_ctx->registry.Usb2PixelFormatTransformCompressionEnable = 1;
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

void fl2000_dongle_u1u2_setup(struct dev_ctx * dev_ctx, bool enable)
{
	if (enable) {
		// Set 0x0070 bit 20 = 0, accept U1.
		// Set 0x0070 bit 19 = 0, accept U2.
		//
		fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_0070, 20);
		fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_0070, 19);
	}
	else {
		// Set 0x0070 bit 20 = 1, reject U1.
		// Set 0x0070 bit 19 = 1, reject U2.
		//
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_0070, 20);
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_0070, 19);
	}
}

void fl2000_dongle_reset(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_INIT, ">>>>");

	// REG_OFFSET_8048(0x8048)< bit 15 > = 1, app reset, self clear.
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8048, 15);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_INIT, "<<<<");
}

void fl2000_dongle_stop(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");
	if (!dev_ctx->usb_dev) {
		// The device is not yet enumerated.
		//
		goto exit;
	}
	fl2000_dongle_reset(dev_ctx);

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

int
fl2000_dongle_set_params(struct dev_ctx * dev_ctx, struct vr_params * vr_params)
{
	int ret_val;
	bool ret;
	uint32_t old_pll;
	uint32_t new_pll;
	bool pll_changed;
	struct resolution_entry const * entry = NULL;
	size_t table_num;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	// FileIO thread references to parameters and need to avoid concurrent access.
	//
	ret_val = 0;
	pll_changed = false;

	// Set PLL register takes long time to stabilize, therefore, we set that only
	// found it's different to previous setting.
	//
	old_pll = dev_ctx->vr_params.pll_reg;
	memcpy(&dev_ctx->vr_params, vr_params, sizeof(struct vr_params));

	dev_ctx->vr_params.pll_reg = old_pll;
	dev_ctx->vr_params.end_of_frame_type = EOF_ZERO_LENGTH;

	if (dev_ctx->registry.CompressionEnable ||
		vr_params->use_compression) {
		dev_ctx->vr_params.use_compression = 1;
		dev_ctx->vr_params.dynamic_compression_mask = 1;

		dev_ctx->vr_params.compression_mask_index_min = COMPRESSION_MASK_INDEX_MINIMUM;
		dev_ctx->vr_params.compression_mask_index_max = COMPRESSION_MASK_INDEX_MAXIMUM;

		if (dev_ctx->vr_params.output_image_type == OUTPUT_IMAGE_TYPE_RGB_16) {
			// Bug#6346: Need more aggressive compression mask.
			//
			dev_ctx->vr_params.compression_mask = COMPRESSION_MASK_13_BIT_VALUE;
			dev_ctx->vr_params.compression_mask_index = COMPRESSION_MASK_13_BIT_INDEX;

			// Output is RGB555, and need at most the mask.
			//
			dev_ctx->vr_params.compression_mask_index_min = COMPRESSION_MASK_15_BIT_INDEX;
		}
		else {
			dev_ctx->vr_params.compression_mask = COMPRESSION_MASK_23_BIT_VALUE;
			dev_ctx->vr_params.compression_mask_index = COMPRESSION_MASK_23_BIT_INDEX;
		}

		/*
		 * how are the low_water_mark/high_water_mark values derived?
		 * For USB2 bandwidth, a typical bus provides 480 mb/s. Taking
		 * 8b/10b encoding into account, maximum bandwidth is 480/10*8
		 * = 384 mb/sec = 48 MB/s.
		 * Since EHCI does not reach this theoretically limit, we assume
		 * the maximum bandwidth for the client device is about 40 MB/s
		 * With 40 MB/sec to accomodate 60 frames /sec, each frame
		 * is about 682KB. You have to deduct vsync interval time,
		 * during which the fl2000 refuse any data-in packet, so the
		 * possible maximum frame size is 576KB.
		 * On high speed port connected to xHC, the theoretical throughput
		 * could reach higher value.
		 */
		dev_ctx->vr_params.compression_low_water_mark = 384 * 1000;
		dev_ctx->vr_params.compression_high_water_mark = 640 * 1000;
	}

	switch (dev_ctx->vr_params.output_image_type) {
	case OUTPUT_IMAGE_TYPE_RGB_8:
		table_num = VGA_BIG_TABLE_8BIT_R0;
		break;
	case OUTPUT_IMAGE_TYPE_RGB_16:
		table_num = VGA_BIG_TABLE_16BIT_R0;
		break;
	case OUTPUT_IMAGE_TYPE_RGB_24:
	default:
		table_num = VGA_BIG_TABLE_24BIT_R0;
		break;
	}

	entry = fl2000_table_get_entry(
		table_num,
		dev_ctx->vr_params.width,
		dev_ctx->vr_params.height,
		dev_ctx->vr_params.freq);
	if (entry == NULL) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"ERROR fl2000_table_get_entry failed.");
			ret_val = -EINVAL;
			goto exit;
		}

	dev_ctx->vr_params.h_sync_reg_1 = entry->h_sync_reg_1;
	dev_ctx->vr_params.h_sync_reg_2 = entry->h_sync_reg_2;
	dev_ctx->vr_params.v_sync_reg_1 = entry->v_sync_reg_1;
	dev_ctx->vr_params.v_sync_reg_2 = entry->v_sync_reg_2;

	dev_ctx->vr_params.h_total_time = entry->h_total_time;
	dev_ctx->vr_params.h_sync_time	= entry->h_sync_time;
	dev_ctx->vr_params.h_back_porch = entry->h_back_porch;
	dev_ctx->vr_params.v_total_time = entry->v_total_time;
	dev_ctx->vr_params.v_sync_time	= entry->v_sync_time;
	dev_ctx->vr_params.v_back_porch = entry->v_back_porch;

	if (dev_ctx->hdmi_chip_found)
		fl2000_hdmi_compliance_tweak(dev_ctx);

	new_pll = entry->bulk_asic_pll;

	if (new_pll != dev_ctx->vr_params.pll_reg) {
		pll_changed = true;
		dev_ctx->vr_params.pll_reg = new_pll;
	}

	ret = fl2000_monitor_set_resolution(dev_ctx, pll_changed);
	if (!ret) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] fl2000_monitor_set_resolution failed?");
		ret_val = -EIO;
		goto exit;
	}

	// Select Interface
	//
	ret_val = fl2000_dev_select_interface(dev_ctx);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR fl2000_dev_select_interface failed?");
		goto exit;
	}

	dev_ctx->usb_pipe_bulk_out = usb_sndbulkpipe(dev_ctx->usb_dev, 1);

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
	return (ret_val);
}

int
fl2000_set_display_mode(
	struct dev_ctx * dev_ctx,
	struct display_mode * display_mode)
{
	int ret_val = 0;
	struct vr_params vr_params;
	bool resolution_changed = false;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		 "Display information width:%u, height:%u, use_compression:%u, compress_size_limit:%u",
		 display_mode->width,
		 display_mode->height,
		 display_mode->use_compression,
		 display_mode->compress_size_limit);

	if ((dev_ctx->vr_params.width != display_mode->width) ||
		(dev_ctx->vr_params.height != display_mode->height))
		resolution_changed = true;

	fl2000_render_stop(dev_ctx);
	fl2000_dongle_stop(dev_ctx);

	/*
	 * user want to turn off monitor
	 */
	if (display_mode->width == 0 && display_mode->height == 0)
		goto exit;

	memset(&vr_params, 0, sizeof(struct vr_params));

	vr_params.width = display_mode->width;
	vr_params.height = display_mode->height;
	vr_params.freq = 60;
	vr_params.use_compression = display_mode->use_compression;
	vr_params.compress_size_limit = display_mode->compress_size_limit;
	switch (display_mode->input_color_format) {
	case COLOR_FORMAT_RGB_24:
		vr_params.input_bytes_per_pixel = 3;
		break;

	case COLOR_FORMAT_RGB_16_565:
	default:
		vr_params.input_bytes_per_pixel = 2;
		break;
	}

	switch (display_mode->output_color_format) {
	case COLOR_FORMAT_RGB_24:
		vr_params.output_image_type = OUTPUT_IMAGE_TYPE_RGB_24;
		break;

	case COLOR_FORMAT_RGB_16_565:
		vr_params.output_image_type = OUTPUT_IMAGE_TYPE_RGB_16;
		vr_params.color_mode_16bit = VR_16_BIT_COLOR_MODE_565;
		break;
	default:
		vr_params.output_image_type = OUTPUT_IMAGE_TYPE_RGB_16;
		vr_params.color_mode_16bit = VR_16_BIT_COLOR_MODE_555;
		break;
	}

	vr_params.trasfer_pipe = VR_TRANSFER_PIPE_BULK;

	if (IS_DEVICE_USB2LINK(dev_ctx)) {
		/*
		 * Considering physical bw limitation, force frequency to 60Hz
		 * once user select higher frequency from panel.
		 */
		if (60 < vr_params.freq)
			vr_params.freq = 60;

		// If usb2, then force to pixeltransform ( 24->16(555) ) and compression on.
		//
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"Turn on usb2 compression.");

		vr_params.output_image_type = OUTPUT_IMAGE_TYPE_RGB_16;
		vr_params.color_mode_16bit = VR_16_BIT_COLOR_MODE_555;
		vr_params.use_compression = 1;
	}

	ret_val = fl2000_dongle_set_params(dev_ctx, &vr_params);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"[ERR] fl2000_dongle_set_params failed?");
		goto exit;
	}
	fl2000_render_start(dev_ctx);

	if (dev_ctx->hdmi_chip_found)
		fl2000_hdmi_init(dev_ctx, resolution_changed);

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
	return ret_val;
}

void fl2000_dongle_card_initialize(struct dev_ctx * dev_ctx)
{
	bool hdmi_chip_found;

	fl2000_dongle_reset(dev_ctx);

	hdmi_chip_found = fl2000_hdmi_find_chip(dev_ctx);
	dev_ctx->hdmi_chip_found = hdmi_chip_found;
	if (hdmi_chip_found) {
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"found ITE hdmi chip, initializing it.");
		fl2000_hdmi_reset(dev_ctx);
		if (!dev_ctx->hdmi_powered_up) {
			fl2000_hdmi_power_up(dev_ctx);
		}
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"ITE hdmi chip powered up");
	}
	fl2000_dongle_init_fl2000dx(dev_ctx);
}

// eof: fl2000_dongle.c
//
