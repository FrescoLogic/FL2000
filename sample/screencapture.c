#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdio.h>

void getScreenPixels(u_int8_t * framebuffer,int width,int height)
{
  Display *d = XOpenDisplay(NULL);

  Window root = DefaultRootWindow(d);

  XImage *image = XGetImage(d,root, 0, 0, width, height, AllPlanes, XYPixmap);
  if(image == NULL)
  {
   printf("error could not grab display capture\n");
   return;
  }

  //converting Ximage to RGB(8*3 bit)
  long unsigned int pixel;
  for(int y = 0 ; y < image->height ;y++)
  {
    for(int x = 0 ; x < image->width ;x++)
    {
      pixel = XGetPixel(image, x, y);
      long offset = (x+y*1920)*3;
      //R
      framebuffer[offset+2]=(u_int8_t)(pixel >> 16);
      //G
      framebuffer[offset+1]=(u_int8_t)(pixel >> 8);
      //B
      framebuffer[offset]=(u_int8_t)pixel;
    }
  }
}
