#include <unistd.h>
#include <evdi_lib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <stdbool.h>
#include "../include/fl2000_ioctl.h"
#include "fl2000lib.h"

#define true 1
#define false 0

volatile static sig_atomic_t sigtermcatched = 0;

uint32_t fd,width,height;
bool readyToUpdate = true;
evdi_handle evdihandle;
evdi_buffer evdibuffer;

void term(int signum)
{
   printf("SIG recieved exiting!\n");
   sigtermcatched = 1;
}

int exist(char const * fname)
{
  if( access( fname, F_OK ) != -1 ) {
    return true;
  } else {
    return false;
  }
}

 void update_ready_handler(int buffer_to_be_updated, void* user_data)
 {
   /*evdi_rect evdi_rect;
   int num_rects;
   evdi_grab_pixels(evdihandle,&evdi_rect,&num_rects);

   surface_info surface_info = create_surface(fd,(uint8_t*)evdibuffer.buffer,width,height,0);
   send_update_to_gpu(fd, surface_info);
   destroy_surface(fd, surface_info);*/
   std::cout << "updated !" << std::endl;
   readyToUpdate=false;
 }

//return the number of devices
int searchdri(int *dri)
{
  for(int i=0;;i++)
  {
    char str[10];
    sprintf(str,"/dev/dri%d",i);
    if(exist(str))
    {
      if(*dri == -1 && AVAILABLE == evdi_check_device(i))
      {
        std::cout << "Found dri device" << i << std::endl;
        *dri=i;
      }
    }else{
      std::cout << "Error dri device not found creating...";
      return i-1;
    }
  }
}

int main(int argc, char const *argv[]) {
  if (geteuid() != 0) {
		fprintf(stderr, "need root privilege. try sudo %s\n", argv[0]);
		return 1;
	}

  std::cout << "loading the fl2000 usb driver";
  std::cout.flush();
  int ret_val = system("/sbin/insmod fl2000.ko");
  if ( ret_val != 0 ) {
  //don't precise /sbin since on arch-linux it's /bin
    ret_val = system("rmmod fl2000");
    if(ret_val == 0)
    {
      ret_val = system("/sbin/insmod fl2000.ko");
    }
    if(ret_val != 0)
    {
      std::cerr << "\nCould not load evdi module" << std::endl;
      if(exist("fl2000.ko"))
      {
        std::cerr << "you have to move fl2000.ko inside the evdi folder" << std::endl;
        std::cerr << "fl2000.ko can be compiled with the src folder" << std::endl;
      }
      return 6;
    }
  }
  std::cout << "... done" << std::endl;

  std::cout << "requesting monitor info";
  std::cout.flush();
  //detect the fl2000 and check if it is connected and a monitor is hooked up to it
  struct	monitor_info monitor_info;
  fd = getmonitor(&monitor_info);
  if(fd == -1)
  {
  	ret_val = system("/usr/bin/rmmod fl2000");
  	 return 4;
  }
  if (monitor_info.monitor_flags.connected == 0) {
	fprintf(stderr, "\nno monitor connected to FL2000?\n");
	return 5;
  }
  std::cout << "done" << std::endl;
  printf("%s\n", monitor_info.edid);

  int dri=-1;
  int nbdevices = searchdri(&dri);
  pthread_t monitor_thread;

  std::cout << "loading the evid kernel module" << std::endl;
  ret_val = system("/sbin/modprobe evdi");
  if ( ret_val != 0 ) {
      fprintf( stderr, "Could not load evdi module");
  }


  //checking the dri and adding one if none are found
  if(dri == -1)
  {
    if(evdi_add_device())
    {
      if(evdi_check_device(++nbdevices))
      {
        dri = nbdevices;
        std::cout << " DONE!" <<std::endl;
      }else
      {
        std::cerr << "\nError device not found after creation" << std::endl;
        return 2;
      }
    }else{
      std::cerr << "\nError could not create dri device" << std::endl;
      return 3;
    }
  }

  evdihandle = evdi_open(dri);
  if(EVDI_INVALID_HANDLE == evdihandle)
  {
    std::cerr << "could not open dri device" << std::endl;
  }

  parse_edid(&width,&height,monitor_info);

  evdi_connect(evdihandle,
          monitor_info.edid,
          EDID_SIZE,
          width*height);

  evdibuffer.width=width;
  evdibuffer.id=0;
  evdibuffer.height=height;
  evdibuffer.buffer=(uint8_t*)malloc(sizeof(uint8_t)*width*height*3);//rgb * width * height
  evdibuffer.stride=width * 3;

  evdi_register_buffer(evdihandle, evdibuffer);


  signal(SIGTERM, term);

  int monitor_thread_args[3];
  monitor_thread_args[0]=fd;
  monitor_thread_args[1]=width;
  monitor_thread_args[2]=height;
  ret_val = pthread_create(&monitor_thread, NULL, monitor_connection_worker, (void*) monitor_thread_args);
  if(ret_val != 0)
  {
    fprintf(stderr, "failed to create monitor_connection_worker %d", ret_val);
  }

  //assuming 0
  if(set_display_mode(fd,width,height,0))
  {
    std::cerr << "ERROR COULD NOT SET DISPLAY_MODE" << std::endl;
  }

  evdi_get_event_ready(evdihandle);
  evdi_event_context context;
  context.update_ready_handler = update_ready_handler;
  evdi_handle_events(evdihandle, &context);

  while(!sigtermcatched)
  {
    std::cout << "update requested" << std::endl;
    bool isready = evdi_request_update(evdihandle, evdibuffer.id);
    readyToUpdate=false;
    while(!readyToUpdate);
    if(isready)
    {
      //update_ready_handler(evdibuffer.id,NULL);
    }
  }

  /*
	 * kill monitor_connection_worker, and wait for monitor_thread to die.
	 */
	pthread_cancel(monitor_thread);
	pthread_join(monitor_thread, NULL);	// expect nothing from the thread

  evdi_unregister_buffer(evdihandle,evdibuffer.id);
  evdi_close(evdihandle);

  close(fd);
  return 0;
}
