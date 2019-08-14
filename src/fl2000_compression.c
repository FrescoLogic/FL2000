// fl2000_compression.c
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Compression.
//

#include "fl2000_include.h"

/////////////////////////////////////////////////////////////////////////////////
// P R I V A T E
/////////////////////////////////////////////////////////////////////////////////
//

__inline
uint32_t
GET_BYTES_PER_PIXEL(
	uint32_t image_type
	)
{
	uint32_t bytes_per_pixel;

	switch (image_type) {
	case OUTPUT_IMAGE_TYPE_RGB_8:
		bytes_per_pixel = PIXEL_BYTE_1;
		break;

	case OUTPUT_IMAGE_TYPE_RGB_16:
		bytes_per_pixel = PIXEL_BYTE_2;
		break;

	case OUTPUT_IMAGE_TYPE_RGB_24:
		bytes_per_pixel = PIXEL_BYTE_3;
		break;

	default:
		ASSERT( false );
		bytes_per_pixel = PIXEL_BYTE_3;
		break;
	}
    return (bytes_per_pixel);
}

uint32_t
fl2000_comp_get_current_mask_value(struct dev_ctx * dev_ctx)
{
	uint32_t mask;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	switch (dev_ctx->vr_params.compression_mask_index) {
	case COMPRESSION_MASK_23_BIT_INDEX:
		mask = COMPRESSION_MASK_23_BIT_VALUE;
		break;
	case COMPRESSION_MASK_21_BIT_INDEX:
		mask = COMPRESSION_MASK_21_BIT_VALUE;
		break;
	case COMPRESSION_MASK_18_BIT_INDEX:
		mask = COMPRESSION_MASK_18_BIT_VALUE;
		break;
	case COMPRESSION_MASK_15_BIT_INDEX:
		mask = COMPRESSION_MASK_15_BIT_VALUE;
		break;
	case COMPRESSION_MASK_12_BIT_INDEX:
		mask = COMPRESSION_MASK_12_BIT_VALUE;
		break;
	case COMPRESSION_MASK_09_BIT_INDEX:
		mask = COMPRESSION_MASK_09_BIT_VALUE;
		break;
	case COMPRESSION_MASK_08_BIT_INDEX:
		mask = COMPRESSION_MASK_08_BIT_VALUE;
		break;
	case COMPRESSION_MASK_06_BIT_INDEX:
		mask = COMPRESSION_MASK_06_BIT_VALUE;
		break;
	case COMPRESSION_MASK_04_BIT_INDEX:
		mask = COMPRESSION_MASK_04_BIT_VALUE;
		break;
	case COMPRESSION_MASK_03_BIT_INDEX:
		mask = COMPRESSION_MASK_03_BIT_VALUE;
		break;
	default:
		mask = COMPRESSION_MASK_23_BIT_VALUE;
		break;
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
	return (mask);
}

void fl2000_comp_raise_mask(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	// Raise compression mask make gravity compression algorithm get worse compression rate,
	// But the output image quality will be clear.
	//
	if (dev_ctx->vr_params.compression_mask_index_min <
	    dev_ctx->vr_params.compression_mask_index) {
		dev_ctx->vr_params.compression_mask_index--;
	}

	dbg_msg(TRACE_LEVEL_INFO, DBG_COMPRESSION,
		"Raise compression mask maskIndex=%d.",
		( int )dev_ctx->vr_params.compression_mask_index );

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
}

void fl2000_comp_lower_mask(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	// Lower the compression mask make garvity compression algorithm get
	// better compression rate,
	// But the output image quality will be blurred.
	//
	if (dev_ctx->vr_params.compression_mask_index_max !=
	    dev_ctx->vr_params.compression_mask_index)
		dev_ctx->vr_params.compression_mask_index++;

	ASSERT(dev_ctx->vr_params.compression_mask_index <=
	       COMPRESSION_MASK_INDEX_MAXIMUM);

	dbg_msg(TRACE_LEVEL_INFO, DBG_COMPRESSION,
		"Lower compression mask maskIndex=%d.",
		(int)dev_ctx->vr_params.compression_mask_index);

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
}

void fl2000_comp_apply_safest_mask(struct dev_ctx * dev_ctx)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");
	dev_ctx->vr_params.compression_mask_index = COMPRESSION_MASK_INDEX_MAXIMUM;
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
}

__inline
uint32_t fl2000_comp_get_pixel(
	uint8_t * source,
	uint32_t byte_per_pixel
	)
{
	uint32_t pixelData = 0;

	switch (byte_per_pixel) {
	case PIXEL_BYTE_1:
		pixelData = source[0] ;
		break;
	case PIXEL_BYTE_2:
		pixelData = (source[1] << 8) | source[0] ;
		break;
	case PIXEL_BYTE_3:
		pixelData = (source[2] << 16) | (source[1] << 8) | source[0];
		break;
	default:
		break;
	}
	return (pixelData);
}

__inline
void fl2000_comp_save_repeated_data(
	uint8_t * target,
	uint8_t * source,
	uint32_t bytes_per_pixel
	)
{
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	switch (bytes_per_pixel) {
	case PIXEL_BYTE_1:
		target[0] =  (source[ 0 ] >> 1);
		target[0] |= 0x80;
		break;

	case PIXEL_BYTE_2:
		// No need to shift because it's RGB 555 format.
		//
		target[0] =  source[0];
		target[1] =  source[1];
		target[1] |= 0x80;
		break;

	case PIXEL_BYTE_3:
		target[0] = source[0];
		target[1] = source[1];
		target[2] = (source[ 2 ] >> 1);
		target[2] |= 0x80;
		break;

	default:
		break;
	}
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
}

__inline void
fl2000_comp_save_repeated_count(
	uint8_t** target_ptr,
	uint8_t* repeat_data,
	uint32_t bytes_per_pixel,
	uint32_t repeat_count)
{
	uint32_t repeat_chunk_size;
	uint8_t* target = *target_ptr;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	switch (bytes_per_pixel) {
	case PIXEL_BYTE_1:
		do {
			if (repeat_count > 0x7F)
				repeat_chunk_size = 0x7F;
			else
				repeat_chunk_size = repeat_count;

			target[0] = (uint8_t) repeat_chunk_size;
			target += PIXEL_BYTE_1;

			repeat_count -= repeat_chunk_size;

			if (repeat_count > 0) {
				target[0] = (repeat_data[0] >> 1);
				target[0] |= 0x80;
				target += PIXEL_BYTE_1;
				repeat_count -= 1;
			}
		} while (repeat_count > 1);
		break;

	case PIXEL_BYTE_2:
		do {
			if ( repeat_count > 0x7FFF )
				repeat_chunk_size = 0x7FFF;
			else
				repeat_chunk_size = repeat_count;

			target[0] = (uint8_t)(repeat_chunk_size);
			target[1] = (uint8_t)((repeat_chunk_size & 0x7FFF) >> 8);
			target += PIXEL_BYTE_2;

			repeat_count -= repeat_chunk_size;

			if (repeat_count > 0) {
				// No need to shift because it's RGB 555 format.
				//
				target[0] = repeat_data[0];
				target[1] = repeat_data[1];
				target[1] |= 0x80;
				target += PIXEL_BYTE_2;
				repeat_count -= 1;
			}
		} while ( repeat_count > 1 );
		break;

	case PIXEL_BYTE_3:
		do {
			if (repeat_count > 0x7FFFFF)
				repeat_chunk_size = 0x7FFFFF;
			else
				repeat_chunk_size = repeat_count;

			target[0] = ((uint8_t)(repeat_chunk_size >> 0 )) & 0xFF;
			target[1] = ((uint8_t)(repeat_chunk_size >> 8 )) & 0xFF;
			target[2] = ((uint8_t)(repeat_chunk_size >> 16)) & 0xFF;
			target += PIXEL_BYTE_3;

			repeat_count -= repeat_chunk_size;

			if (repeat_count > 0) {
				target[0] = repeat_data[0];
				target[1] = repeat_data[1];
				target[2] = repeat_data[2] >> 1;
				target[2] |= 0x80;
				target += PIXEL_BYTE_3;
				repeat_count -= 1;
			}
		} while (repeat_count > 1);
		break;

	default:
		break;
	}

	*target_ptr = target;
	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");
}

void fl2000_comp_padding_alignment(
	struct dev_ctx * dev_ctx,
	uint8_t * target,
	size_t *data_buffer_length)
{
	uint32_t alignment;
	size_t length = *data_buffer_length;

	alignment = 4;
	while (length % alignment) {
		// I prefer to treat padding data as single data.
		//
		target[0] = 0x80;
		target += 1;
		length += 1;
	}

	*data_buffer_length = length;
}

size_t fl2000_comp_gravity_low(
	struct dev_ctx * dev_ctx,
	size_t data_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint32_t num_of_pixels,
	uint32_t bytes_per_pixel,
	bool NoCompressionToFirst1K)
{
	uint32_t i;
	uint32_t looselyColorMaskRGB;
	int32_t repeatCount;
	uint32_t markPixel;
	uint32_t currentPixel;
	uint32_t previousPixel;
	uint8_t * repeatDataPointer;
	uint8_t * targetOriginalPointer;
	uint8_t * sourceOriginalPointer;
	uint32_t isochSourcePacketIndex;
	uint8_t *  isochNextBoundry;
	bool isochCompression;
	uint32_t currentPixelWithMask;
	uint32_t markPixelWithMask;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	targetOriginalPointer = target;
	sourceOriginalPointer = source;
	repeatDataPointer = source;

	markPixel = 0;
	previousPixel = 0;
	repeatCount = 0;

	isochSourcePacketIndex = 0;
	isochNextBoundry = source;
	isochCompression = false;

	looselyColorMaskRGB = fl2000_comp_get_current_mask_value(dev_ctx);

	// Check if the start pixel exceeds the boundary of compression by
	// Precise-Isoch compress mode.
	// Mark the first isochNextBoundry.
	if (isochCompression) {
		// TODO:
		//
	}
	else {
		/*
		 * Bulk transfer doesn't need to consider packet boundry, and
		 * set the value to the end of transfer.
		 */
		isochNextBoundry = source + data_buffer_length;
	}


	for (i = 0; i < num_of_pixels; i++) {
		currentPixel = fl2000_comp_get_pixel(source, bytes_per_pixel);

		currentPixelWithMask = (currentPixel | looselyColorMaskRGB);
		markPixelWithMask = (markPixel | looselyColorMaskRGB);

		if (repeatCount > 0) {
			/* It's loosely comparsion, if current pixel after the
			 * mask that looks like repeatDataPointer pixel, then we
			 * treat it as the same group of current compression
			 * run.
			 */
			if (currentPixelWithMask == markPixelWithMask) {
				bool notExactlyTheSameAsMarkPixel;
				bool exactlyTheSameAsPreviousPixel;

				if ( markPixel != currentPixel )
					notExactlyTheSameAsMarkPixel = true;
				else
					notExactlyTheSameAsMarkPixel = false;

				previousPixel = fl2000_comp_get_pixel(
					source - bytes_per_pixel,
					bytes_per_pixel);

				if  (previousPixel == currentPixel)
					exactlyTheSameAsPreviousPixel = true;
				else
					exactlyTheSameAsPreviousPixel = false;

				// This algorithem check two condtions to
				// prevent worse color trailing issue.
				// If we match these two conditions, we have to
				// start a new compression run.
				//
				// 1. Current pixel is *not the same* as
				//    repeatDataPointer pixel.
				// 2. Current pixel is *the same* as previous
				//    pixel.
				//
				if (notExactlyTheSameAsMarkPixel &&
				    exactlyTheSameAsPreviousPixel) {
					// Start over a new compression run.
					//
					if (repeatCount > 1) {
						// A real compression run.
						// And we send device the number
						// of additional copies - not the
						// length
						//
						repeatCount--;
						fl2000_comp_save_repeated_count(
							&target,
							repeatDataPointer,
							bytes_per_pixel,
							repeatCount);
					}
					repeatCount = 0;
				}
				else if (source >= isochNextBoundry) {
					if (repeatCount > 1) {
						repeatCount--;
						fl2000_comp_save_repeated_count(
							&target,
							repeatDataPointer,
							bytes_per_pixel,
							repeatCount);
					}
					repeatCount = 0;
				}
				else {
					// After the mask, this pixel is same as
					// previous one.
					//
					repeatCount++;
					source += bytes_per_pixel;
				}
			}
			else {
				// Start over a new compression run.
				//
				if (repeatCount > 1) {
				// A real compression run.
				// And we send device the number of additional copies - not the length
				//
				repeatCount--;
				fl2000_comp_save_repeated_count(
					&target,
					repeatDataPointer,
					bytes_per_pixel,
					repeatCount);
				}
				repeatCount = 0;
			}
		}

		if ( repeatCount == 0 ) {
			fl2000_comp_save_repeated_data(
				target, source, bytes_per_pixel);
			markPixel = fl2000_comp_get_pixel(
				source, bytes_per_pixel);
			repeatDataPointer = source;

			target += bytes_per_pixel;
			source += bytes_per_pixel;

			if (isochCompression) {
				ASSERT(false);
			}

			repeatCount++;
		}
	}

	// When we exit, we could be in the middle of a run. If so we need to do
	// a special termination.
	//
	if (repeatCount > 1)  {
		// A real compression run.
		// And we send device the number of additional copies - not the length
		//
		repeatCount--;

		if (repeatCount > 1) {
			fl2000_comp_save_repeated_count(
				&target,
				repeatDataPointer,
				bytes_per_pixel,
				repeatCount - 1 );
		}

		// Hardware bug: The end of compressed data must be single pixel.
		//
		fl2000_comp_save_repeated_data(
			target, repeatDataPointer, bytes_per_pixel);
		target += bytes_per_pixel;
	}

	if (dev_ctx->vr_params.use_compression)
		data_buffer_length = target - targetOriginalPointer;

	if (isochCompression) {
		ASSERT(false);
	}
	else {
		// Make sure compressed data buffer length is aligned.
		//
		fl2000_comp_padding_alignment(
			dev_ctx,
			target,
			&data_buffer_length);
		// target is invalid here.
		//
	}

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");

	return (data_buffer_length);
}

size_t
fl2000_comp_decompress_low(
	struct dev_ctx * dev_ctx,
	size_t CompressedBufferLength,
	uint8_t * source,
	uint8_t * target,
	uint32_t bytes_per_pixel,
	uint32_t num_of_pixels)
{
	uint8_t dataByte;
	uint8_t* sourceBoundary = source + CompressedBufferLength;
	uint8_t*  workingSource;
	uint32_t repeatCount = 0;
	uint8_t * repeatData = NULL;
	uint8_t*  savedTargetBeginning = target;
	size_t decompressed_buf_len;
	size_t expectDeCompressedLength = num_of_pixels * bytes_per_pixel;

	ASSERT(bytes_per_pixel == PIXEL_BYTE_2);

	workingSource = source;

	while (workingSource < sourceBoundary) {
		dataByte = workingSource[1];

		if (dataByte & 0x80) {
			// Repeat data.
			//
			target[0] = workingSource[0];
			target[1] = workingSource[1];

			// Clear repeat data flag.
			//
			target[ 1 ] &= ~0x80;
			repeatData = target;
			repeatCount = 0;
			target += bytes_per_pixel;
		}
		else {
			uint32_t index = 0;

			// Repeat Count.
			//
			ASSERT(!repeatCount && repeatData);

			repeatCount = ((workingSource[1] << 8) | workingSource[0]);

			for (index = 0; index < repeatCount; index++) {
				target[0] = repeatData[0];
				target[1] = repeatData[1];

				target += bytes_per_pixel;
			}
		}

		workingSource += bytes_per_pixel;
	}

	decompressed_buf_len = target - savedTargetBeginning;

	ASSERT((decompressed_buf_len == expectDeCompressedLength) ||
	       (decompressed_buf_len == expectDeCompressedLength + 2));

	return (decompressed_buf_len);
}

/////////////////////////////////////////////////////////////////////////////////
// P U B L I C
/////////////////////////////////////////////////////////////////////////////////
//
size_t
fl2000_compression_gravity(
	struct dev_ctx * dev_ctx,
	size_t data_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint8_t * working_buffer,
	uint32_t num_of_pixels
	)
{
	uint32_t pixelBytes;
	size_t compressed_length;

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, ">>>>");

	// Gravity compress picture according to same color of repeat count.
	// 1. MASK - choose the first n bytes of RGB and compare,
	//    for better compress we need bigger mask, but result is lower picture quality.
	//
	// 2. Algorithm - we calculate pixel and how many similar color pixel count.
	//    if no count number is written, the count is equal to 1.
	//
        pixelBytes = GET_BYTES_PER_PIXEL(dev_ctx->vr_params.output_image_type);

	/*
	 * FIXME: we always assumed the input color format is 24bpp.
	 */
	if (pixelBytes == 2)
		fl2000_compression_convert_3_to_2(
			working_buffer,
			source,
			num_of_pixels,
			dev_ctx->vr_params.color_mode_16bit);

recompress:
        switch (pixelBytes)
	{
	case 2:
		compressed_length = fl2000_comp_gravity_low(
			dev_ctx,
			num_of_pixels * 2,	// data_buffer_length,
			working_buffer,		// source,
			target,
			num_of_pixels,
			2,			// 2 bytes per pixel
			true);
		break;

	default:
		compressed_length = fl2000_comp_gravity_low(
			dev_ctx,
			data_buffer_length,
			source,
			target,
			num_of_pixels,
			pixelBytes,
			false);
		break;
	};

	/*
	 * if the compressed buffer size is too large, lower the compression mask
	 * and re-compress again
	 */
	if (dev_ctx->vr_params.dynamic_compression_mask &&
            dev_ctx->vr_params.compress_size_limit) {
		if (compressed_length < (dev_ctx->vr_params.compress_size_limit / 2)) {
			if (dev_ctx->vr_params.compression_mask_index >
			    dev_ctx->vr_params.compression_mask_index_min)
			{
				fl2000_comp_raise_mask(dev_ctx);
				goto recompress;
			}
		}
		else if (compressed_length > dev_ctx->vr_params.compress_size_limit) {
			if (dev_ctx->vr_params.compression_mask_index <
			    dev_ctx->vr_params.compression_mask_index_max)
			{
				fl2000_comp_lower_mask(dev_ctx);
				goto recompress;
			}
		}
	}
	// Compressed data should be always smaller.
	//
	ASSERT((dev_ctx->vr_params.input_bytes_per_pixel * num_of_pixels ) >= compressed_length );

	dbg_msg(TRACE_LEVEL_VERBOSE, DBG_COMPRESSION, "<<<<");

	return (compressed_length);
}

/*
 * the term gravity2 means output pixel size is 2 bytes
 */
size_t
fl2000_compression_gravity2(
	struct dev_ctx * dev_ctx,
	size_t data_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint8_t * working_buffer,
	uint32_t num_of_pixels)
{
	size_t	compressed_length;

	/*
	 * FIXME: we always assumed the input color format is 24bpp.
	 * should check the input color format in the future.
	 * do in-place 24bpp to 16bpp conversion
	 */
	fl2000_compression_convert_3_to_2(
		working_buffer,
		source,
		num_of_pixels,
		dev_ctx->vr_params.color_mode_16bit);

recompress:
	compressed_length = fl2000_comp_gravity_low(
		dev_ctx,
		num_of_pixels * 2,	// data_buffer_length,
		working_buffer,		// source,
		target,
		num_of_pixels,
		2,	// 2 bytes per pixel
		true);

	dbg_msg(TRACE_LEVEL_INFO, DBG_COMPRESSION,
		"compressed_length(0x%x)", (uint32_t) compressed_length);

	/*
	 * if the compressed buffer size is too large, lower the compression mask
	 * and re-compress again
	 */
	if (IS_DEVICE_USB2LINK(dev_ctx) &&
	    dev_ctx->vr_params.dynamic_compression_mask) {
		if (compressed_length < dev_ctx->vr_params.compression_low_water_mark) {
			if (dev_ctx->vr_params.compression_mask_index >
			    dev_ctx->vr_params.compression_mask_index_min)
			{
				fl2000_comp_raise_mask(dev_ctx);
				goto recompress;
			}
		}
		else if (compressed_length > dev_ctx->vr_params.compression_high_water_mark) {
			if (dev_ctx->vr_params.compression_mask_index <
			    dev_ctx->vr_params.compression_mask_index_max)
			{
				fl2000_comp_lower_mask(dev_ctx);
				goto recompress;
			}
		}
	}
	return (compressed_length);
}

size_t
fl2000_comp_decompress_and_check(
	struct dev_ctx * dev_ctx,
	size_t compressed_buffer_length,
	uint8_t * source,
	uint8_t * target,
	uint32_t bytes_per_pixel,
	uint32_t num_of_pixels)
{
	size_t decompressed_buf_len;

	ASSERT(source != target);
	decompressed_buf_len = fl2000_comp_decompress_low(
		dev_ctx,
		compressed_buffer_length,
		source,
		target,
		bytes_per_pixel,
		num_of_pixels);

	return (decompressed_buf_len);
}

void
fl2000_compression_convert_3_to_2(
	uint8_t * target,
	uint8_t * source,
	size_t num_of_pixels,
	uint32_t color_mode)
{
	size_t index;
	uint8_t r, g, b;

	for (index = 0; index < num_of_pixels; index += 1) {
		if (color_mode == VR_16_BIT_COLOR_MODE_565) {
			r = source[2] >> 3;
			g = source[1] >> 2;
			b = source[0] >> 3;

			target[1] = (r << 3) | (g >> 3);
			target[0] = ((g & 0x07) << 5) | b;
		}
		else {
			r = source[2] >> 3;
			g = source[1] >> 3;
			b = source[0] >> 3;

			target[1] = (r << 2) | (g >> 3);
			target[0] = ((g & 0x07) << 5) | b;
		}

		source += 3;
		target += 2;
	}
}


// eof: fl2000_compression.c
//
