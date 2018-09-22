// fl2000_register.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_REGISTER_H_
#define _FL2000_REGISTER_H_

#define REG_OFFSET_8000		0x8000
#define REG_OFFSET_8004		0x8004
#define REG_OFFSET_8008		0x8008
#define REG_OFFSET_800C		0x800C
#define REG_OFFSET_8010		0x8010
#define REG_OFFSET_8014		0x8014
#define REG_OFFSET_8018		0x8018
#define REG_OFFSET_801C		0x801C
#define REG_OFFSET_8020		0x8020
#define REG_OFFSET_8024		0x8024
#define REG_OFFSET_8028		0x8028
#define REG_OFFSET_802C		0x802C
#define REG_OFFSET_8030		0x8030
#define REG_OFFSET_8034		0x8034
#define REG_OFFSET_8038		0x8038
#define REG_OFFSET_803C		0x803C
#define REG_OFFSET_8040		0x8040
#define REG_OFFSET_8044		0x8044
#define REG_OFFSET_8048		0x8048
#define REG_OFFSET_804C		0x804C
#define REG_OFFSET_8050		0x8050
#define REG_OFFSET_8054		0x8054
#define REG_OFFSET_8058		0x8058
#define REG_OFFSET_805C		0x805C
#define REG_OFFSET_8064		0x8064
#define REG_OFFSET_8070		0x8070
#define REG_OFFSET_8074		0x8074
#define REG_OFFSET_8078		0x8078
#define REG_OFFSET_807C		0x807C
#define REG_OFFSET_8088		0x8088

#define REG_OFFSET_0070		0x0070
#define REG_OFFSET_0078		0x0078

bool fl2000_reg_write(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t* data);

bool fl2000_reg_read(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t* data);

bool fl2000_reg_check_bit(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset);

void fl2000_reg_bit_set(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset);

void fl2000_reg_bit_clear(
	struct dev_ctx * dev_ctx,
	uint32_t offset,
	uint32_t bit_offset);

#endif // _FL2000_REGISTER_H_

// eof: fl2000_register.h
//
