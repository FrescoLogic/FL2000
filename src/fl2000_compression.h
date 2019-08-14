// fl2000_compression.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion file.
//

#ifndef _FL2000_COMPRESSION_H_
#define _FL2000_COMPRESSION_H_

#define COMPRESSION_MASK_23_BIT_INDEX           ( 0 )
#define COMPRESSION_MASK_21_BIT_INDEX           ( 1 )
#define COMPRESSION_MASK_18_BIT_INDEX           ( 2 )
#define COMPRESSION_MASK_15_BIT_INDEX           ( 3 )
#define COMPRESSION_MASK_14_BIT_INDEX           ( 4 )
#define COMPRESSION_MASK_13_BIT_INDEX           ( 5 )
#define COMPRESSION_MASK_12_BIT_INDEX           ( 6 )
#define COMPRESSION_MASK_10_BIT_INDEX           ( 7 )
#define COMPRESSION_MASK_09_BIT_INDEX           ( 8 )
#define COMPRESSION_MASK_08_BIT_INDEX           ( 9 )
#define COMPRESSION_MASK_06_BIT_INDEX           ( 10 )
#define COMPRESSION_MASK_04_BIT_INDEX           ( 11 )
#define COMPRESSION_MASK_03_BIT_INDEX           ( 12 )
#define COMPRESSION_MASK_INDEX_MINIMUM          ( COMPRESSION_MASK_23_BIT_INDEX )
#define COMPRESSION_MASK_INDEX_MAXIMUM          ( COMPRESSION_MASK_04_BIT_INDEX )

#define COMPRESSION_MASK_23_BIT_VALUE           ( 0xFF000000 )
#define COMPRESSION_MASK_21_BIT_VALUE           ( 0xFF010101 )
#define COMPRESSION_MASK_18_BIT_VALUE           ( 0xFF030303 )
#define COMPRESSION_MASK_15_BIT_VALUE           ( 0xFF070707 )
#define COMPRESSION_MASK_14_BIT_VALUE           ( 0xFF07070F )
#define COMPRESSION_MASK_13_BIT_VALUE           ( 0xFF0F070F )
#define COMPRESSION_MASK_12_BIT_VALUE           ( 0xFF0F0F0F )
#define COMPRESSION_MASK_10_BIT_VALUE           ( 0xFF1F0F1F )
#define COMPRESSION_MASK_09_BIT_VALUE           ( 0xFF1F1F1F )
#define COMPRESSION_MASK_08_BIT_VALUE           ( 0xFF1F3F1F )
#define COMPRESSION_MASK_06_BIT_VALUE           ( 0xFF3F3F3F )
#define COMPRESSION_MASK_04_BIT_VALUE           ( 0xFF7F3F7F )
#define COMPRESSION_MASK_03_BIT_VALUE           ( 0xFF7F7F7F )

#define MINIMUM_COMPRESSED_LENGH_IN_BYTES       ( 1025 )

// Mmx compression state.
//
#define MMX_COMPRESSION_STATE_INIT      0
#define MMX_COMPRESSION_STATE_LEADIN    1
#define MMX_COMPRESSION_STATE_WAITRUN1  2
#define MMX_COMPRESSION_STATE_RUN1      3
#define MMX_COMPRESSION_STATE_WAITRUN2  4
#define MMX_COMPRESSION_STATE_RUN2      5
#define MMX_COMPRESSION_STATE_LEADOUT   6

// Fast compression bandwidth target.
//
#define MMX_COMPRESSION_BANDWIDTH_TARGET_PER_FRAME_BYTES 500000

#define MAX_COMPRESSION_RETRY_COUNT_COUNT 120

size_t fl2000_comp_gravity_low(
	struct dev_ctx * dev_ctx,
	size_t data_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint32_t num_of_pixels,
	uint32_t bytes_per_pixel,
	bool NoCompressionToFirst1K
	);

size_t fl2000_compression_gravity(
	struct dev_ctx * dev_ctx,
	size_t DataBufferLength,
	uint8_t * source,
	uint8_t * target,
	uint8_t * working_buffer,
	uint32_t num_of_pixels
	);

size_t fl2000_compression_gravity2(
	struct dev_ctx * dev_ctx,
	size_t data_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint8_t * working_buffer,
	uint32_t num_of_pixels
	);

size_t fl2000_comp_decompress_and_check(
	struct dev_ctx * dev_ctx,
	size_t CompressedBufferLength,
	uint8_t * source,
	uint8_t * target,
	uint32_t PixelBytes,
	uint32_t num_of_pixels
	);

void fl2000_comp_raise_mask(struct dev_ctx * dev_ctx);
void fl2000_comp_lower_mask(struct dev_ctx * dev_ctx);
void fl2000_comp_apply_safest_mask(struct dev_ctx * dev_ctx);

void
fl2000_compression_convert_3_to_2(
	uint8_t * target,
	uint8_t * source,
	size_t num_of_pixels,
	uint32_t color_mode
	);

#endif // _FL2000_COMPRESSION_H_

// eof: fl2000_compression.h
//
