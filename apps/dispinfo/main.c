#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
int g_fb = 0;

void CheckFrameBuffer()
{

  int bytes;
  struct fb_fix_screeninfo fixed_info;
  struct fb_var_screeninfo var_info;
  char* screen;
  g_fb = open("/dev/fb0", O_RDWR);
  if (g_fb == -1)
  {
    printf("Unable to open /dev/fb0\n");
    _exit(1);
  }

  printf("Opened /dev/fb0, using handle 0x%x\n", (unsigned)g_fb);
  
  if (ioctl(g_fb, FBIOGET_FSCREENINFO, &fixed_info))
  {
    printf("Unable to get fixed screen info\n");
    _exit(1);
  }
  if (ioctl(g_fb, FBIOGET_VSCREENINFO, &var_info))
  {
    printf("Unable to get var screen info\n");
    _exit(1);
  }

  printf("xres: %d\n", var_info.xres);
  printf("yres: %d\n", var_info.yres);
  printf("bpp: %d\n", var_info.bits_per_pixel);
  printf("height: %d\n", var_info.height);
  printf("width: %d\n", var_info.width);
  printf("red:\n");
  printf("   length: %d\n", var_info.red.length);
  printf("   offset: %d\n", var_info.red.offset);
  printf("green:\n");
  printf("   length: %d\n", var_info.green.length);
  printf("   offset: %d\n", var_info.green.offset);
  printf("blue:\n");
  printf("   length: %d\n", var_info.blue.length);
  printf("   offset: %d\n", var_info.blue.offset);
  bytes = var_info.xres * var_info.yres * var_info.bits_per_pixel /8;
  printf("Screen buffer size is %d bytes\n", bytes);
}

int main(int argc, char* argv[])
{
  CheckFrameBuffer();
  
  return 0;
}

