#include <pthread.h>
#include "../include/fl2000_ioctl.h"

/*
 * Note: valid values for OUTPUT_COLOR_FORMAT are:
 * COLOR_FORMAT_RGB_24, COLOR_FORMAT_RGB_16_565, COLOR_FORMAT_RGB_16_555
 *
 * when USE_COMPRESSION is set to 1, COLOR_FORMAT_RGB_16_565 is not allowed
 */
#define	OUTPUT_COLOR_FORMAT			COLOR_FORMAT_RGB_24

#define	USE_COMPRESSION				0
#define	COMPRESS_SIZE_LIMIT			0	// no limit on the compress size
//#define	COMPRESS_SIZE_LIMIT			1666666		// for xHC that can do 100MB/s, the driver compress each frame to size no larger than  (100*1000*1000/60) bytes


/*
 * definitions
 */
#define IMAGE_ASPECT_RATIO_16_10                0
#define IMAGE_ASPECT_RATIO_4_3                  1
#define IMAGE_ASPECT_RATIO_5_4                  2
#define IMAGE_ASPECT_RATIO_16_9                 3

#define MAX_FRAME_BUFFER_SIZE			1920*1080*3

#define	FL2K_NAME	"/dev/fl2000-0"

void parse_edid(uint32_t *width,uint32_t *height,monitor_info monitor_info);

void * monitor_connection_worker(void *arg);

int getmonitor(monitor_info *monitor_info);
bool fl2000_is_connected(void);
int destroy_surface(int fd, surface_info surface_info);
void disable_output(int fd);
int send_update_to_gpu(int fd, surface_info surface_info);
surface_info create_surface(int fd, uint8_t *frame_buffer,int width,int height,int mem_type);
int set_display_mode(int fd, int width, int height,int mem);
