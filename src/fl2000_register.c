// fl2000_register.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: USBVGA Device Register Primitives
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//
//
bool fl2000_reg_write(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t* data)
{
	int ret_val;

	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_WRITE,
		offset,
		data);
	if (ret_val < 0)
		return false;

	return true;
}

bool fl2000_reg_read(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t* data)
{
	int ret_val;
	bool status;

	status = true;
	*data = 0;

	ret_val = fl2000_i2c_xfer(
		dev_ctx,
		VGA_MMIO_READ,
		offset,
		data);
	if (ret_val < 0) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_HW,
			"Read register failed.");
		status = false;
	}
	return status;
}

bool fl2000_reg_check_bit(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset)
{
	bool status;
	uint32_t data;

	status = fl2000_reg_read(dev_ctx, offset, &data);
	if (!status)
		goto exit;

	data >>= bit_offset;
	data &= 0x1;

	if (data == 0x01)
		status = true;
	else
		status = false;

exit:
	return status;
}

void fl2000_reg_bit_set(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset)
{
	bool status;
	uint32_t data;

	status = fl2000_reg_read(dev_ctx, offset, &data);
	if (!status) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_HW,
			"fl2000_reg_read failed.");
		goto exit;
	}

	data |= (1 << bit_offset);
	status = fl2000_reg_write(dev_ctx, offset, &data);
	if (!status) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_HW,
			"fl2000_reg_write failed.");
		goto exit;
	}

exit:
	return;
}

void fl2000_reg_bit_clear(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset)
{
	bool status;
	uint32_t data;

	status = fl2000_reg_read(dev_ctx, offset, &data);
	if (!status) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_HW,
			"fl2000_reg_read failed.");
		goto exit;
	}

	data &= ~(1 << bit_offset);

	status = fl2000_reg_write(dev_ctx, offset, &data);
	if (!status) {
		dbg_msg(TRACE_LEVEL_ERROR, DBG_HW,
			"fl2000_reg_write failed.");
		goto exit;
	}

exit:
	return;
}

// eof: fl2000_register.c
//
