// fl2000_i2c.c
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
int
fl2000_i2c_xfer(
	struct dev_ctx * dev_ctx,
	uint32_t is_read,
	uint32_t offset,
	uint32_t* data)
{
	int 	ret_val;
	uint8_t bRequest;
	uint8_t req_type;
	uint16_t req_index;
	unsigned int pipe;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_HW, ">>>>");

	dev_ctx->ctrl_xfer_buf = *data;

	if (is_read) {
		bRequest = REQUEST_I2C_COMMAND_READ;
		req_type = (USB_DIR_IN | USB_TYPE_VENDOR);
		pipe = usb_rcvctrlpipe(dev_ctx->usb_dev, 0);
	}
	else {
		bRequest = REQUEST_I2C_COMMAND_WRITE;
		req_type = (USB_DIR_OUT | USB_TYPE_VENDOR);
		pipe = usb_sndctrlpipe(dev_ctx->usb_dev, 0);
	}

	req_index = (uint16_t) offset;
	ret_val = usb_control_msg(
		dev_ctx->usb_dev,
		pipe,
		bRequest,
		req_type,
		0,
		req_index,
		&dev_ctx->ctrl_xfer_buf,
		REQUEST_I2C_RW_DATA_COMMAND_LENGTH,
		CTRL_XFER_TIMEOUT);

	if (is_read)
		*data = dev_ctx->ctrl_xfer_buf;

	return ret_val;
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//

int
fl2000_i2c_read(
	struct dev_ctx * dev_ctx,
	uint8_t i2c_addr,
	uint8_t offset,
	uint32_t* ret_dword)
{
	int ret_val;
	I2C_DATA i2c_data;
	uint32_t read_back_data;
	uint32_t retry;
	bool i2c_done;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_HW, ">>>>");

	*ret_dword = 0;

	// Step 1: Readback 0x8020 data.
	//
	read_back_data = 0;
	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_READ,
		REG_OFFSET_8020,
		&read_back_data);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed." );
		goto exit;
	}

	// BUG: Bit 28 always return zero.
	// Need to debug this issue.
	//
	read_back_data |= 0x10000000;

	// Step 2: Program data to REG_OFFSET_8020.
	//
	i2c_data.value = read_back_data;
	i2c_data.s.Addr = i2c_addr;
	i2c_data.s.RW = I2C_READ;
	i2c_data.s.offset = offset;
	i2c_data.s.IsSpiOperation = 0;
	i2c_data.s.SpiEraseEnable = 0;
	i2c_data.s.OpStatus = 0;

	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_WRITE,
		REG_OFFSET_8020,
		(uint32_t*)&i2c_data);

	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed.");
		goto exit;
	}

	// I2C is very slow
	//
	DELAY_MS(3);

	// Step 3: Read back REG_OFFSET_8020, and make sure bit31 is set to 1.
	//         Because I2C is very slow, we should retry to make sure it done.
	//
	i2c_done = false;

	for (retry = 0; retry < I2C_RETRY_MAX; retry++) {
		read_back_data = 0;
		ret_val = fl2000_i2c_xfer(
			dev_ctx,
			VGA_MMIO_READ,
			REG_OFFSET_8020,
			&read_back_data);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
				"WARNING I2c transfer failed.");
			goto exit;
		}

		if (read_back_data & 0x80000000) {
			// I2C program done.
			//
			i2c_done = true;
			break;
		}

		// I2C programming not finish, wait for a while and keep trying.
		//
		DELAY_MS( 10 );
	}

	if (!i2c_done) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c read back data not done." );
		ret_val = -EIO;
		goto exit;
	}

	// Step 4: Read back REG_OFFSET_8024 to get return data.
	//
	read_back_data = 0;
	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_READ,
		REG_OFFSET_8024,
		&read_back_data);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed.");
		goto exit;
	}

	*ret_dword = read_back_data;

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_HW, "<<<<");
	return ret_val;
}

int fl2000_i2c_write(
	struct dev_ctx * dev_ctx,
	uint8_t i2c_addr,
	uint8_t offset,
	uint32_t* write_dword)
{
	int ret_val;
	I2C_DATA i2c_data;
	uint32_t read_back_data;
	uint32_t retry;
	bool i2c_done;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_HW, ">>>>");

	// Step 1: Put write_dword to 0x8028.
	//
	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_WRITE,
		REG_OFFSET_8028,
		write_dword);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed.");
		goto exit;
	}

	// Step 2: Readback 0x8020 data.
	//
	read_back_data = 0;
	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_READ,
		REG_OFFSET_8020,
		&read_back_data);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed.");
		goto exit;
	}

	// BUG: Bit 28 always return zero.
	// Need to debug this issue.
	//
	read_back_data |= 0x10000000;

	// Step 3: Porgram data to REG_OFFSET_8020.
	//
	i2c_data.value = read_back_data;
	i2c_data.s.Addr = i2c_addr;
	i2c_data.s.RW = I2C_WRITE;
	i2c_data.s.offset = offset;
	i2c_data.s.IsSpiOperation = 0;
	i2c_data.s.SpiEraseEnable = 0;
	i2c_data.s.OpStatus = 0;

	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_WRITE,
		REG_OFFSET_8020,
		(uint32_t*)&i2c_data);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
			"WARNING I2c transfer failed.");
		goto exit;
	}

	// I2C is very slow
	//
	DELAY_MS(3);

	// Step 3: Read back REG_OFFSET_8020, and make sure bit31 is set to 1.
	//         Because I2C is very slow, we should retry to make sure it done.
	//
	i2c_done = false;

	for (retry = 0; retry < I2C_RETRY_MAX; retry++) {
		read_back_data = 0;
		ret_val = fl2000_i2c_xfer(
			dev_ctx,
			VGA_MMIO_READ,
			REG_OFFSET_8020,
			&read_back_data);
		if (ret_val < 0) {
			dbg_msg(TRACE_LEVEL_WARNING, DBG_HW,
				"WARNING I2c transfer failed." );
			goto exit;
		}

		if (read_back_data & 0x80000000) {
			// I2C program done.
			//
			i2c_done = true;
			break;
		}

		// I2C programming not finish, wait for a while and keep trying.
		//
		DELAY_MS(10);
	}

	if (!i2c_done) {
		ret_val = -EIO;
		goto exit;
	}

exit:
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_HW, "<<<<");
	return ret_val;
}

// eof: vid_i2c.c
//
