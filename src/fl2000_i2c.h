// vid_i2c.h
//
// (c)Copyright 2009-2013, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _VID_I2C_H_
#define _VID_I2C_H_

#define I2C_ADDRESS_HDMI                        ( 0x4C )
#define I2C_ADDRESS_DSUB                        ( 0x50 )
#define I2C_ADDRESS_EEPROM                      ( 0x54 )
#define I2C_READ                                ( 1 )
#define I2C_WRITE                               ( 0 )
#define I2C_STATUS_PASS                         ( 0 )
#define I2C_STATUS_FAIL                         ( 1 )
#define I2C_OP_STATUS_PROGRESS                  ( 0 )
#define I2C_OP_STATUS_DONE                      ( 1 )
#define I2C_RETRY_MAX                           ( 10 )

#define REQUEST_I2C_RW_DATA_COMMAND_LENGTH      4
#define REQUEST_I2C_COMMAND_READ                64
#define REQUEST_I2C_COMMAND_WRITE               65

typedef union _I2C_DATA_
{
    struct
    {
	u32 Addr:7;            // I2C address
	u32 RW:1;              // 1=I2C read, 0=I2C write
	u32 offset:8;          // I2C offset.  Bit 9:8 shall be written 0.
	u32 IsSpiOperation:1;  // 1=SPI, 0=EEPROM.
	u32 SpiEraseEnable:1;  // 1=SPI, 0=EEPROM.
	u32 Resv1:6;
	u32 DataStatus:4;      // I2C status indicating passed/failed per byte. 0=passed, 1=failed
	u32 Resv2:3;
	u32 OpStatus:1;        // I2C operation status.  0=in progress, 1=done.  SW shall not perform the next I2C operation when this bit is 0.  SW shall write 0 to clear this bit to start the next i2C
    } s;

    u32 value;
} I2C_DATA, *PI2C_DATA;

int fl2000_i2c_xfer(
	struct dev_ctx * dev_ctx,
	uint32_t is_read,
	uint32_t offset,
	uint32_t* data);

int fl2000_i2c_read(
	struct dev_ctx * dev_ctx,
	uint8_t i2c_addr,
	uint8_t offset,
	uint32_t* ret_dword);

int fl2000_i2c_write(
	struct dev_ctx * dev_ctx,
	uint8_t i2c_addr,
	uint8_t offset,
	uint32_t* write_word);

#endif // _VID_I2C_H_

// eof: vid_i2c.h
//
