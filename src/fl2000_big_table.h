// fl2000_big_table.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_BIG_TABLE_H_
#define _FL2000_BIG_TABLE_H_

#define VGA_BIG_TABLE_SIZE              73

#define VGA_BIG_TABLE_24BIT_R0		0
#define VGA_BIG_TABLE_24BIT_R1		1
#define VGA_BIG_TABLE_16BIT_R0		2
#define VGA_BIG_TABLE_16BIT_R1		3
#define VGA_BIG_TABLE_8BIT_R0		4
#define VGA_BIG_TABLE_8BIT_R1		5

struct resolution_entry const *
fl2000_table_get_entry(
	uint32_t table_num,
	uint32_t width,
	uint32_t height,
	uint32_t freq);

#endif // _FL2000_BIG_TABLE_H_

// eof: fl2000_big_table.h
//
