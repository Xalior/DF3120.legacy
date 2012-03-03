#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define SCR_WIDTH  320
#define SCR_HEIGHT 240

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
// set g_FontData
#include "font_5x8.h"


void Help(const char* name)
{
  printf("Usage: %s <text to display>\n", name);
  _exit(1);
}

char* g_DisplayBuffer;

// Initialise and fill with something sensible
void InitBuffer()
{
  g_DisplayBuffer = malloc(SCR_WIDTH*SCR_HEIGHT);
  memset((void*)g_DisplayBuffer, '.', SCR_WIDTH*SCR_HEIGHT);
}

// Backing file
int g_fb = 0;

// shared memory area
char* g_Screen = NULL;

void InitFrameBuffer()
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

  g_Screen = (char*)mmap(0, bytes, PROT_READ|PROT_WRITE, MAP_SHARED, g_fb, 0);
  //printf("mmap pointer is 0x%x\n", (unsigned)screen);
  
  if ((int)screen == -1) 
  {
    printf("mmap() failed.\n"); 
    exit(1);
  }
  printf("mmap() OK\n");
}

// X position
int g_CursorX = 0;
int g_CursorY = 0;
// add a character and advance cursor
void AddChar(char ch)
{
  const char* font_ptr = g_FontData;
  char* buffer_ptr = g_DisplayBuffer;
  int font_index = ch;
  int y;
  // don't bother with non-printable
  if (ch<32) return;
  if (ch>126) return;
  
  // Move to this char in the font.
  font_index -= 32;
  font_index*= (CHAR_HEIGHT*CHAR_WIDTH);
  font_ptr += font_index;
  
  // blit it into position row at a time
  buffer_ptr += (g_CursorX + SCR_WIDTH*g_CursorY);
  for (y=0;y<CHAR_HEIGHT;y++)
  {
    memcpy(buffer_ptr, font_ptr, CHAR_WIDTH);
    buffer_ptr += SCR_WIDTH;
    font_ptr += CHAR_WIDTH;
  }
  g_CursorX += CHAR_WIDTH+1;
  if (g_CursorX >= (SCR_WIDTH - CHAR_WIDTH+1))
  {
    g_CursorX = 0;
    g_CursorY += CHAR_HEIGHT;
  }
}



// add a character and advance cursor
void PutCharOnScreen(char ch)
{
  const char* font_ptr = g_FontData;
  unsigned short* buffer_ptr = (unsigned short*)g_Screen;
  int font_index = ch;
  int y, x;
  // don't bother with non-printable
  if (ch<32) return;
  if (ch>126) return;
  
  // Move to this char in the font.
  font_index -= 32;
  font_index*= (CHAR_HEIGHT*CHAR_WIDTH);
  font_ptr += font_index;
  
  // Move screen ptr to top-left of character
  buffer_ptr += (g_CursorX + SCR_WIDTH*g_CursorY);
  for (y=0;y<CHAR_HEIGHT;y++)
  {
    for (x=0;x<CHAR_WIDTH;x++)
    {
      if (*font_ptr == '.')
      {
        //printf("Writing black\n");
        *buffer_ptr = 0;
        buffer_ptr++;
      } 
      else
      {
        //printf("Writing white\n");
        *buffer_ptr = 65535;
        buffer_ptr++;
      }
      font_ptr++;
    }
    buffer_ptr -= CHAR_WIDTH;
    buffer_ptr += SCR_WIDTH;  // onto the next line
  }
  g_CursorX += CHAR_WIDTH+1;
  if (g_CursorX >= (SCR_WIDTH - CHAR_WIDTH+1))
  {
    g_CursorX = 0;
    g_CursorY += CHAR_HEIGHT;
  }
  // TODO: test Ypos
}





void PrintChars(const char* txt)
{
/*  int i;
  int colour;
  sscanf(txt, "%d", &colour);
  unsigned short* ptr = (unsigned short*)g_Screen;
  for (i=0;i<5000;i++)
  {
    *ptr = colour & 0xffff;
    ptr++;
  }
*/
  while (*txt)
  {
    PutCharOnScreen(*txt);
    txt++;
  }
  
  
}


int main(int argc, char* argv[])
{
  if (argc!=2)
  {
    Help(argv[0]);
  }
  InitFrameBuffer(320, 200);
  
  PrintChars(argv[1]);
  
  return 0;
}

