/*
  Quick and dirty way to write characters to the df3120 screen.

  bifferos@yahoo.co.uk
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
// set g_FontData
#include "font.h"


void Help(const char* name)
{
  printf("Usage: %s <text to display>\n", name);
  _exit(1);
}


// Something we can print
static int MaxX;
static int MaxY;

char* g_DisplayBuffer;

// Initialise and fill with something sensible
void InitBuffer(int x, int y)
{
  MaxX = x;
  MaxY = y;
  g_DisplayBuffer = malloc(MaxX*MaxY);
  memset((void*)g_DisplayBuffer, '.', MaxX*MaxY);
}


// Backing file
int g_fb = 0;

// shared memory area
char* g_Screen = NULL;

void InitFrameBuffer(int max_x, int max_y)
{
  int bytes;
  struct fb_fix_screeninfo fixed_info;
  struct fb_var_screeninfo var_info;
  char* screen;
  MaxX = max_x;
  MaxY = max_y;
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



// Print display Buffer
void PrintBuffer()
{
  int x, y, count;
  count = 0;
  for (y = 0; y<MaxY; y++)
  {
    for (x = 0; x<MaxX; x++)
    {
      printf("%c", g_DisplayBuffer[count]);
      count++;      
    }
    printf("\n");
  }    
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
  buffer_ptr += (g_CursorX + MaxX*g_CursorY);
  for (y=0;y<CHAR_HEIGHT;y++)
  {
    memcpy(buffer_ptr, font_ptr, CHAR_WIDTH);
    buffer_ptr += MaxX;
    font_ptr += CHAR_WIDTH;
  }
  g_CursorX += CHAR_WIDTH+1;
  if (g_CursorX >= (MaxX - CHAR_WIDTH+1))
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
  buffer_ptr += (g_CursorX + MaxX*g_CursorY);
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
        *buffer_ptr = 1000;
        buffer_ptr++;
      }
      font_ptr++;
    }
    buffer_ptr -= CHAR_WIDTH;
    buffer_ptr += MaxX;  // onto the next line
  }
  g_CursorX += CHAR_WIDTH+1;
  if (g_CursorX >= (MaxX - CHAR_WIDTH+1))
  {
    g_CursorX = 0;
    g_CursorY += CHAR_HEIGHT;
  }
  // TODO: test Ypos
}





void PrintChars(const char* txt, int test)
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
    if (test)
    {
      AddChar(*txt);
    } 
    else 
    {
      PutCharOnScreen(*txt);
    }
    
    txt++;
  }
  
  
}


int main(int argc, char* argv[])
{
  int test_font = 0;  // set to 1 to print on text console
  if (argc!=2)
  {
    Help(argv[0]);
  }
  if (test_font)
  {
    InitBuffer(80, 30);
  }
  else
  {
    InitFrameBuffer(320, 200);
  }
  
  PrintChars(argv[1], test_font);
  
  if (test_font)
  {
    PrintBuffer();
  }
  return 0;
}

