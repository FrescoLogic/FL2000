// fl2000_monitor.c
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//
void
fl2000_monitor_filter_established_timing(struct dev_ctx * dev_ctx)
{
	// Check byte 35
	//
	// Bit 0 800x600 @ 60 Hz
	// Bit 1 800x600 @ 56 Hz
	// Bit 2 640x480 @ 75 Hz
	// Bit 3 640x480 @ 72 Hz
	// Bit 4 640x480 @ 67 Hz
	// Bit 5 640x480 @ 60 Hz
	// Bit 6 720x400 @ 88 Hz
	// Bit 7 720x400 @ 70 Hz
	//
	dev_ctx->monitor_edid[0][35] = 0x21;

	// Check byte 36
	//
	// Bit 0 1280x1024 @ 75 Hz
	// Bit 1 1024x768  @ 75 Hz
	// Bit 2 1024x768  @ 72 Hz
	// Bit 3 1024x768  @ 60 Hz
	// Bit 4 1024x768  @ 87 Hz, interlaced (1024*768i)
	// Bit 5 832x624   @ 75 Hz
	// Bit 6 800x600   @ 75 Hz
	// Bit 7 800x600   @ 72 Hz
	//
	dev_ctx->monitor_edid[0][36] = 0x8;

	// Check Byte 37
	//
	// Bit 7 for 1152x870@75Hz.
	//
	dev_ctx->monitor_edid[0][37] = 0;
}

bool
fl2000_monitor_resolution_in_white_table(
	uint32_t width,
	uint32_t height,
	uint32_t freq)
{
	bool in_white_list;
	const struct resolution_entry * entry;

	if (freq != 60) {
		in_white_list = false;
		goto exit;
	}

	if (width * height > MAX_WIDTH * MAX_HEIGHT) {
		in_white_list = false;
		goto exit;
	}

	entry = fl2000_table_get_entry(
		VGA_BIG_TABLE_24BIT_R0,
		width,
		height,
		freq);
	in_white_list = (entry != NULL);

exit:
	return in_white_list;
}

void fl2000_monitor_ratio_to_dimension(
	uint8_t x,
	uint8_t aspect_ratio,
	uint32_t* width,
	uint32_t* height)
{
	uint32_t temp_width;
	uint32_t temp_height;

	temp_width = (x + 31) * 8;
	switch (aspect_ratio) {
	case IMAGE_ASPECT_RATIO_16_10:
		temp_height = (temp_width / 16) * 10;
		break;

	case IMAGE_ASPECT_RATIO_4_3:
		temp_height = (temp_width / 4) * 3;
		break;

	case IMAGE_ASPECT_RATIO_5_4:
	    temp_height = (temp_width / 5) * 4;
	    break;

	case IMAGE_ASPECT_RATIO_16_9:
	default:
	    temp_height = ( temp_width / 16 ) * 9;
	    break;

	}

	*width = temp_width;
	*height = temp_height;
}

void fl2000_monitor_filter_std_timing(struct dev_ctx * dev_ctx)
{
	uint8_t i;
	uint32_t width;
	uint32_t height;
	uint32_t freq;
	bool in_white_table;

	/*
	 * EDID offset 38 ~ 53. Standard timing information. Upto 8 2-bytes.
	 * Unused fields are filled with 01 01
	 */
	for (i = 38; i < 53; i+= 2) {
		uint8_t	 x = dev_ctx->monitor_edid[0][i];
		uint8_t  ratio = dev_ctx->monitor_edid[0][i + 1] >> 6;

		freq = (dev_ctx->monitor_edid[0][i + 1] & 0x3F) + 60;
		if (dev_ctx->monitor_edid[0][i] == 1 &&
		    dev_ctx->monitor_edid[0][i + 1] == 1)
			break;

		fl2000_monitor_ratio_to_dimension(
			x,
			ratio,
			&width,
			&height);

		in_white_table = fl2000_monitor_resolution_in_white_table(
			width,
			height,
			freq);
		if (!in_white_table) {
			/*
			 * make it a 1024x768
			 */
			dev_ctx->monitor_edid[0][i] = 97;
			dev_ctx->monitor_edid[0][i + 1]= IMAGE_ASPECT_RATIO_4_3 << 6;
		}

	}
}

void fl2000_monitor_filter_detailed_timing(struct dev_ctx * dev_ctx)
{
	uint32_t pixel_clock;
	uint32_t h_active;
	uint32_t h_blanking;
	uint32_t v_active;
	uint32_t v_blanking;
	uint32_t i;

	for (i = 54; i < 125; i+= 18) {
		uint8_t *  entry = &dev_ctx->monitor_edid[0][i];

		/*
		 * NOT detailed timing descriptor
		 */
		if (entry[0] == 0 && entry[1] == 0)
			break;

		pixel_clock = entry[1] << 8 | entry[0];
		pixel_clock *= 10000;

		h_active = (entry[4] >> 4) << 8 | entry[2];
		h_blanking = (entry[4] & 0x0F) << 8 | entry[3];
		v_active = (entry[7] >> 4) << 8 | entry[5];
		v_blanking = (entry[7] & 0x0F) << 8 | entry[6];

		/*
		 * if image is larger than 1920x1080, downgrade it
		 */
		if (h_active * v_active > MAX_WIDTH * MAX_HEIGHT) {
			h_active = MAX_WIDTH;
			v_active = MAX_HEIGHT;
			h_blanking = 128;
			v_blanking = 32;
			pixel_clock = (h_active + h_blanking) *
				(v_active + v_blanking) * 60;
			pixel_clock /= 10000;
			entry[0] = pixel_clock & 0xFF;
			entry[1] = (pixel_clock >> 8) & 0xFF;
			entry[2] = h_active & 0xFF;
			entry[3] = h_blanking & 0xFF;
			entry[4] = (h_active >> 8) << 4 | (h_blanking >> 8);
			entry[5] = v_active & 0xFF;
			entry[6] = v_blanking & 0xFF;
			entry[7] = (v_active >> 8) << 4 | (v_blanking >> 8);
			/*
			 * don't care entry[8 ~ 17], leave them as is.
			 */
		}
	}
}

bool fl2000_monitor_read_edid_dsub(struct dev_ctx * dev_ctx)
{
	uint8_t index;
	uint32_t data;
	bool ret_val;
	int read_status;

	ret_val = false;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	// EDID Header check.
	//
	read_status = fl2000_i2c_read(dev_ctx, I2C_ADDRESS_DSUB, 0, &data);
	if (read_status < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Read Edid table failed.");
		goto exit;
	}

	if (EDID_HEADER_DWORD1 != data) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Read Edid data incorrect.");
		goto exit;
	}

	read_status = fl2000_i2c_read(dev_ctx, I2C_ADDRESS_DSUB, 4, &data);
	if (read_status < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Read Edid table failed.");
		goto exit;
	}

	if (EDID_HEADER_DWORD2 != data) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Read Edid data incorrect.");
		goto exit;
	}

	for (index = 0; index < EDID_SIZE; index += 4) {
		read_status = fl2000_i2c_read(
			dev_ctx, I2C_ADDRESS_DSUB, (uint8_t) index, &data);
		if (read_status < 0) {
			dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
				"ERROR Read Edid table failed.");
			goto exit;
		}

		memcpy(&dev_ctx->monitor_edid[0][index], &data, 4);

		// Because I2C is slow, we have to delay a while.
		//
		DELAY_MS(10);
	}
	ret_val = true;

exit:
    return ret_val;
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

bool fl2000_monitor_set_resolution(struct dev_ctx * dev_ctx, bool pll_changed)
{
	uint32_t data;
	bool ret_val;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");
	ret_val = true;

	if (pll_changed) {
		// REG_OFFSET_802C
		//
		data = dev_ctx->vr_params.pll_reg;
		if (fl2000_reg_write(dev_ctx, REG_OFFSET_802C, &data)) {
			// From Ni Jie, only isoch transfer needs to wait until
			// PLL stabilized.
			if (VR_TRANSFER_PIPE_ISOCH ==
			    dev_ctx->vr_params.trasfer_pipe)
				DELAY_MS(1000);
		}
	}

	// REG_OFFSET_8048 ( 0x8048 )< bit 15 > = 1, app reset, self clear.
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8048, 15);

	// Confirm PLL setting.
	//
	data = 0;
	fl2000_reg_read(dev_ctx, REG_OFFSET_802C, &data);
	if (dev_ctx->vr_params.pll_reg != data) {
		ret_val = false;
		goto exit;
	}

	// REG_OFFSET_803C
	//

	// Clear bit 22 - Disable BIA.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 22);

	// Clear bit 24 - Disable isoch error interrupt.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 24);

	// Clear bit 19,21 - Disable isoch auto recover.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 19);
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 21);

	// Clear bit 13 - Disable isoch feedback interrupt.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 13);

	// Clear bit 27:29 - End Of Frame Type
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 27);
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 28);
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 29);

	if (dev_ctx->vr_params.end_of_frame_type == EOF_ZERO_LENGTH) {
		// Zero Length Bulk.
		//
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_803C, 28);
	}
	else  {
		// Pending Bit.
		//
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_803C, 29);
	}

	// REG_OFFSET_8004
	//

	// Clear bit 28, Default setting.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 28);

	// Clear bit 6( 565 ) & 31( 555 ), 16 bit color mode.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 6);
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 31);

	// Clear bit 24, Disable compression.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 24);

	// Clear bit 25, Disable 8 bit color mode.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 25);

	// Clear bit 26, Disable 256 color palette.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 26);

	// Clear bit 27, Disable first byte mask.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_8004, 27);

	// Set bit 0, Reset VGA CCS.
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 0);

	if (dev_ctx->vr_params.use_compression) {
		// Set bit 24, Enable compression mode.
		//
		dbg_msg(TRACE_LEVEL_INFO, DBG_HW, "enable compression mode");
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 24);
	}

	if (OUTPUT_IMAGE_TYPE_RGB_16 == dev_ctx->vr_params.output_image_type) {
		if (VR_16_BIT_COLOR_MODE_555 ==
		    dev_ctx->vr_params.color_mode_16bit) {
			// Bit 31 for 555 mode.
			//
			dbg_msg(TRACE_LEVEL_INFO, DBG_HW, "16bit 555 mode");
			fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 31);
		}
		else {
			// Bit 6 for 565 mode.
			//
			dbg_msg(TRACE_LEVEL_INFO, DBG_HW, "16bit 565 mode");
			fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 6);
		}
	}
	else if (OUTPUT_IMAGE_TYPE_RGB_8 ==
		 dev_ctx->vr_params.output_image_type) {
		// Bit 25 for enable eight bit color mode.
		//
		dbg_msg(TRACE_LEVEL_INFO, DBG_HW, "8bit mode");
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 25);
	}

	// External DAC Control
	//
	// Set bit 7, Enable external DAC.
	//
	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_8004, 7);

	// REG_OFFSET_8008
	//
	data = dev_ctx->vr_params.h_sync_reg_1;
	if (fl2000_reg_write(dev_ctx, REG_OFFSET_8008, &data)) {
		data = 0;
		fl2000_reg_read(dev_ctx, REG_OFFSET_8008, &data);
		if (dev_ctx->vr_params.h_sync_reg_1 != data) {
			ret_val = false;
			goto exit;
		}
	}

	// REG_OFFSET_800C
	//
	data = dev_ctx->vr_params.h_sync_reg_2;
	if (fl2000_reg_write(dev_ctx, REG_OFFSET_800C, &data)) {
		fl2000_reg_read(dev_ctx, REG_OFFSET_800C, &data);
		if (dev_ctx->vr_params.h_sync_reg_2 != data) {
			ret_val = false;
			goto exit;
		}
	}

	// REG_OFFSET_8010
	//
	data = dev_ctx->vr_params.v_sync_reg_1;
	if (fl2000_reg_write(dev_ctx, REG_OFFSET_8010, &data)) {
		fl2000_reg_read(dev_ctx, REG_OFFSET_8010, &data);
		if (dev_ctx->vr_params.v_sync_reg_1 != data) {
			ret_val = false;
			goto exit;
		}
	}

	// REG_OFFSET_8014
	//
	data = dev_ctx->vr_params.v_sync_reg_2;
	if (fl2000_reg_write(dev_ctx, REG_OFFSET_8014, &data)) {
		fl2000_reg_read(dev_ctx, REG_OFFSET_8014, &data);
		if ( dev_ctx->vr_params.v_sync_reg_2 != data ) {
			ret_val = false;
			goto exit;
		}
	}

	// REG_OFFSET_801C
	//

	// Clear bit 29:16 - Iso Register
	//
	if (fl2000_reg_read(dev_ctx, REG_OFFSET_801C, &data)) {
		data &= 0xC000FFFF;
		if (!fl2000_reg_write( dev_ctx, REG_OFFSET_801C, &data)) {
			ret_val = false;
			goto exit;
		}
	}

	fl2000_reg_bit_set(dev_ctx, REG_OFFSET_0070, 13);

exit:
    dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");

    return ret_val;
}

void fl2000_monitor_read_edid(struct dev_ctx * dev_ctx)
{
	uint8_t index;
	uint8_t check_sum;
	bool edid_ok;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	// Try to read EDID from two places:
	// 1. DSUB EDID
	// 2. HDMI EDID
	//
	if (dev_ctx->hdmi_chip_found) {
		unsigned int	i;
		uint8_t num_ext;

		// read the block 0 first, then determine the number of extensions
		// at offset 126.
		edid_ok = fl2000_hdmi_read_block(dev_ctx, 0);
		if (!edid_ok)
			goto edid_exit;

		num_ext = dev_ctx->monitor_edid[0][126];
		dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
			"%u EDID extensions found", num_ext);

		// ignore num_ext if greater than 7.
		if (num_ext > 7)
			num_ext = 0;
		for (i = 0; i < num_ext; i++) {
			bool read_ok;

			read_ok = fl2000_hdmi_read_block(dev_ctx, i + 1);
			dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
				"block[%u] %s", i + 1, read_ok ? "ok" : "failed");
			if (!read_ok)
				break;
		}

	}
	else {
		edid_ok = fl2000_monitor_read_edid_dsub(dev_ctx);
	}

edid_exit:
	if (!edid_ok) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_PNP,
			"ERROR Read DSUB Edid table failed.");

		// Can't get correct EDID table from I2C
		//
		memset(dev_ctx->monitor_edid[0], 0, EDID_SIZE);
		goto exit;
	}

	if (dev_ctx->registry.FilterEdidTableEnable) {
		// Filter EDID
		//
		if (IS_DEVICE_USB3LINK(dev_ctx)) {
			fl2000_monitor_filter_established_timing(dev_ctx);
			fl2000_monitor_filter_std_timing(dev_ctx);
			fl2000_monitor_filter_detailed_timing(dev_ctx);
		}
		else {
			switch (dev_ctx->registry.FilterEdidTableEnable) {
			case EDID_FILTER_USB2_800_600_60HZ:
				// 800x600@60Hz.
				//
				// PrivateParseEdidEstablishedTimingBitmap
				//
				dev_ctx->monitor_edid[0][35] = 1;
				dev_ctx->monitor_edid[0][36] = 0;
				dev_ctx->monitor_edid[0][37] = 0;

				// PrivateParseEdidStandardTimingInformation
				//
				for (index = 38; index < 54; index++)
					dev_ctx->monitor_edid[0][index] = 0x01;

				// PrivateParseEdidDetailedTimingDescriptors
				//
				dev_ctx->monitor_edid[0][54] = 0x40;
				dev_ctx->monitor_edid[0][55] = 0x0B;
				dev_ctx->monitor_edid[0][56] = 0x20;
				dev_ctx->monitor_edid[0][57] = 0x00;
				dev_ctx->monitor_edid[0][58] = 0x30;
				dev_ctx->monitor_edid[0][59] = 0x58;
				dev_ctx->monitor_edid[0][60] = 0x00;
				dev_ctx->monitor_edid[0][61] = 0x20;
				break;
			case EDID_FILTER_USB2_640_480_60HZ:
				//
				// PrivateParseEdidEstablishedTimingBitmap
				//
				dev_ctx->monitor_edid[0][35] = 0x20;
				dev_ctx->monitor_edid[0][36] = 0;
				dev_ctx->monitor_edid[0][37] = 0;

				// PrivateParseEdidStandardTimingInformation
				//
				for (index = 38; index < 54; index++)
					dev_ctx->monitor_edid[0][index] = 0x01;

				// PrivateParseEdidDetailedTimingDescriptors
				//
				dev_ctx->monitor_edid[0][54] = 0x3F;
				dev_ctx->monitor_edid[0][55] = 0x07;
				dev_ctx->monitor_edid[0][56] = 0x80;
				dev_ctx->monitor_edid[0][57] = 0x00;
				dev_ctx->monitor_edid[0][58] = 0x20;
				dev_ctx->monitor_edid[0][59] = 0xE0;
				dev_ctx->monitor_edid[0][60] = 0x0;
				dev_ctx->monitor_edid[0][61] = 0x10;
				break;
			default:
				break;
			}
		}
	}

	check_sum = 0;
	for (index = 0; index < (EDID_SIZE - 1); index++)
	    check_sum += dev_ctx->monitor_edid[0][index];

	check_sum = -check_sum;
	dev_ctx->monitor_edid[0][127] = check_sum;

exit:
    dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

void
fl2000_monitor_plugin_handler(
	struct dev_ctx * dev_ctx,
	bool external_connected,
	bool edid_connected)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	// Bug #6147 - After hot plug VGA connector, the monitor can't display
	// We need mutex to protect plug-in and plug-out procedure.
	// Just to prevent the U1U2 step is not synchronize for each plug-in and plug-out.
	//

	// Per NJ's description:
	// Register 0x78 bit17 is used to control a bug where we did not wake up U1/U2 even
	// when NRDY has been sent and ERDY is not yet due to OBUF not ready.
	// After it is ready, we do not wake up from U1/U2.
	// This Register bit should be set to 1
	// because otherwise it causes U1 exit too frequently when there is no monitor.
	//
	if (CARD_NAME_FL2000DX == dev_ctx->card_name)
		fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_0078, 17);

	// Disable U1U2
	//
	fl2000_dongle_u1u2_setup(dev_ctx, false);

	memset(dev_ctx->monitor_edid, 0, sizeof(dev_ctx->monitor_edid));

	// Get EDID table.
	//
	fl2000_monitor_read_edid(dev_ctx);

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"Notify system to add monitor.");

	// Monitor Plug-In flag.
	// this flag should only be set after we finished EDID.
	dev_ctx->monitor_plugged_in = true;

	// wake up a sleeping process.
	//
	if (waitqueue_active(&dev_ctx->ioctl_wait_q))
		wake_up_interruptible(&dev_ctx->ioctl_wait_q);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

void
fl2000_monitor_plugout_handler(
	struct dev_ctx * dev_ctx
	)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	/*
	 * Bug #6147 - After hot plug VGA connector, the monitor can't display
	 * We need mutex to protect plug-in and plug-out procedure.
	 * Just to prevent the U1U2 step is not synchronize for each plug-in and plug-out.
	 */
	dev_ctx->monitor_plugged_in = false;

	dbg_msg(TRACE_LEVEL_INFO, DBG_PNP,
		"Notify system to delete monitor.");

	// wake up any sleeping process.
	//
	if (waitqueue_active(&dev_ctx->ioctl_wait_q))
		wake_up_interruptible(&dev_ctx->ioctl_wait_q);

	/*
	 * Stop Thread, but don't do hardware reset to VGA dongle.
	 */
	fl2000_render_stop(dev_ctx);

	memset(dev_ctx->monitor_edid, 0, sizeof(dev_ctx->monitor_edid));

	// Bug #6167 : DUT screen black after S4
	// Because our dongle will compare PLL value for reduce the bootup time.
	// But for S3/S4 non-powered platform, the dongle will be toggled.
	// So we got to cleanup internal PLL value and let it at least set again at boot time.
	//
	dev_ctx->vr_params.pll_reg = 0;

	// Bug #6514 : The monitor back light is still on at FL2000 side when system shutdown
	// Turn off "Force PLL always on".
	// TODO: FL2000DX should not need this step per Stanley's description.
	//       This maybe hardware issue, and Jun is checking now.
	//
	fl2000_reg_bit_clear(dev_ctx, REG_OFFSET_803C, 26);

	// Per NJ's description:
	// Register 0x78 bit17 is used to control a bug where we did not wake up U1/U2 even
	// when NRDY has been sent and ERDY is not yet due to OBUF not ready.
	// After it is ready, we do not wake up from U1/U2.
	// This Register bit should be set to 1
	// because otherwise it causes U1 exit too frequently when there is no monitor.
	//
	if (CARD_NAME_FL2000DX == dev_ctx->card_name)
		fl2000_reg_bit_set(dev_ctx, REG_OFFSET_0078, 17);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

void
fl2000_monitor_vga_status_handler(
	struct dev_ctx * dev_ctx,
	uint32_t raw_status)
{
	struct vga_status *  vga_status;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	vga_status = (struct vga_status *) &raw_status;
	if (vga_status->connected) {
		/*
		 * not previously connected
		 */
		if (!dev_ctx->monitor_plugged_in) {
			bool external_connected;
			bool edid_connected;

			if ( vga_status->ext_mon_connected )
				external_connected = true;
			else
				external_connected = false;

			if (vga_status->edid_connected)
				edid_connected = true;
			else
				edid_connected = false;

			fl2000_monitor_plugin_handler(
				dev_ctx,
				external_connected,
				edid_connected);
		}
		else {
			dbg_msg(TRACE_LEVEL_WARNING, DBG_PNP,
				"WARNING Ignore MonitorPlugin event");
		}
	}
	else {
		// Monitor Plug Out.
		//
		if (dev_ctx->monitor_plugged_in)
			fl2000_monitor_plugout_handler(dev_ctx);
		else
			dbg_msg(TRACE_LEVEL_WARNING, DBG_PNP,
				"Ignore MonitorPlugout event, monitor not attached.");
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

void
fl2000_monitor_manual_check_connection(struct dev_ctx * dev_ctx)
{
	uint32_t data;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, ">>>>");

	data = 0;
	if (fl2000_reg_read(dev_ctx, REG_OFFSET_8000, &data)) {
		fl2000_monitor_vga_status_handler(dev_ctx, data);
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_PNP, "<<<<");
}

// eof: fl2000_monitor.c
//
