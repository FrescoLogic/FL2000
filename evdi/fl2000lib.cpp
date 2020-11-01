#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fl2000lib.h"

int getmonitor(monitor_info *monitor_info)
{
  int fd = open(FL2K_NAME, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "fl2000 device not connected?\n");
		return -1;
	}
  ioctl(fd, IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT, monitor_info);
	return fd;
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

void parse_edid(uint32_t *width,uint32_t *height,monitor_info monitor_info)
{
	uint8_t i;
	uint8_t * const monitor_edid = monitor_info.edid;

	/*
	 * EDID offset 38 ~ 53. Standard timing information. Upto 8 2-bytes.
	 * Unused fields are filled with 01 01
	 */
	for (i = 38; i < 53; i+= 2) {
		uint8_t	 x = monitor_edid[i];
		uint8_t  ratio = monitor_edid[i + 1] >> 6;
		uint8_t  freq = (monitor_edid[i + 1] & 0x3F) + 60;

		if (monitor_edid[i] == 1 && monitor_edid[i + 1] == 1)
			break;

		fl2000_monitor_ratio_to_dimension(
			x,
			ratio,
			width,
			height);

		fprintf(stdout, "found (%u, %u) @ %u fps\n",
			*width, *height, freq);
	}
}


int set_display_mode(int fd, int width, int height,int mem)
{
	struct display_mode display_mode;
	int ret_val;
	memset(&display_mode, 0, sizeof(display_mode));
	display_mode.width = width;
	display_mode.height = height;
	display_mode.refresh_rate = 60;
	display_mode.input_color_format = COLOR_FORMAT_RGB_24;
	display_mode.output_color_format = OUTPUT_COLOR_FORMAT;
	display_mode.use_compression = USE_COMPRESSION;
	display_mode.compress_size_limit = COMPRESS_SIZE_LIMIT;

	ret_val = ioctl(fd, IOCTL_FL2000_SET_DISPLAY_MODE, &display_mode);
	if (ret_val < 0) {
		fprintf(stderr, "IOCTL_FL2000_SET_DISPLAY_MODE failed %d\n",ret_val);
		return -1;
	}
	return 0;
}

surface_info create_surface(int fd, uint8_t *frame_buffer,int width,int height,int mem_type)
{
	struct surface_info surface_info;
	memset(&surface_info, 0, sizeof(surface_info));
	surface_info.handle		= (unsigned long) frame_buffer;
	surface_info.user_buffer	= (unsigned long) frame_buffer;
	surface_info.buffer_length	= width * height * 3;
	surface_info.width		= width;
	surface_info.height		= height;
	surface_info.pitch		= width * 3;
	surface_info.color_format	= COLOR_FORMAT_RGB_24;
	surface_info.type		= mem_type;
	ioctl(fd, IOCTL_FL2000_CREATE_SURFACE, &surface_info);
	return surface_info;
}


int send_update_to_gpu(int fd, surface_info surface_info)
{
	struct surface_update_info update_info;
	memset(&update_info, 0, sizeof(update_info));
	update_info.handle 		= surface_info.handle;
	update_info.user_buffer 	= surface_info.user_buffer;
	update_info.buffer_length 	= surface_info.width * surface_info.height * 3;

	int ret_val = ioctl(fd, IOCTL_FL2000_NOTIFY_SURFACE_UPDATE, &update_info);
	if (ret_val < 0) {
		fprintf(stderr, "IOCTL_FL2000_NOTIFY_SURFACE_UPDATE failed %d\n",
			ret_val);
			return -1;
	}
	return 0;
}

void disable_output(int fd)
{
	struct display_mode display_mode;
	memset(&display_mode, 0, sizeof(display_mode));
	int ret_val = ioctl(fd, IOCTL_FL2000_SET_DISPLAY_MODE, &display_mode);
	if(ret_val < 0)
	{
		fprintf(stderr, "IOCTL_FL2000_SET_DISPLAY_MODE failed \n");
	}
}

int destroy_surface(int fd, surface_info surface_info)
{
	int ret_val = ioctl(fd, IOCTL_FL2000_DESTROY_SURFACE, &surface_info);
	if (ret_val < 0) {
		fprintf(stderr, "IOCTL_FL2000_DESTROY_SURFACE failed %d\n",
			ret_val);
			return -1;
	}
	return 0;
}

bool fl2000_is_connected(void)
{
	struct stat f_stat;
	int    ret_val;

	ret_val = stat(FL2K_NAME, &f_stat);

	if (ret_val == 0)
		return true;
	return false;
}


//need [] = {int fd, int width, int height}
void * monitor_connection_worker(void *arg)
{
	int fd = ((int *)arg)[0];
	int width = ((int *)arg)[1];
	int height = ((int *)arg)[2];
	int	old_state;
	int	old_type;
	struct	monitor_info	monitor_info;
	struct	display_mode 	display_mode;
	int	ret_val = 0;

	ret_val = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);
	if (ret_val)
		fprintf(stderr, "pthread_setcancelstate failed?\n");

	ret_val = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old_type);
	if (ret_val)
		fprintf(stderr, "pthread_setcanceltype failed?\n");

	while(true) {
		memset(&monitor_info, 0, sizeof(monitor_info));
		ioctl(fd, IOCTL_FL2000_WAIT_FOR_MONITOR_EVENT, &monitor_info);
		if (monitor_info.monitor_flags.connected) {
			fprintf(stdout, "monitor connected! set resolution(%u, %u)\n",
				width, height
				);
			set_display_mode(fd,width,height,0);
		}
		else {
			fprintf(stderr, "monitor disconnected! current resolution(%u, %u)\n",
				width, height
				);
		}
	}
	return NULL;	// nothing to return
}
