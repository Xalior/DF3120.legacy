#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define SCR_WIDTH	320
#define SCR_HEIGHT 240

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
// and size	
int bytes;

void InitFrameBuffer()
{
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
	printf("	 length: %d\n", var_info.red.length);
	printf("	 offset: %d\n", var_info.red.offset);
	printf("green:\n");
	printf("	 length: %d\n", var_info.green.length);
	printf("	 offset: %d\n", var_info.green.offset);
	printf("blue:\n");
	printf("	 length: %d\n", var_info.blue.length);
	printf("	 offset: %d\n", var_info.blue.offset);
	bytes = var_info.xres * var_info.yres * var_info.bits_per_pixel /8;
	printf("Screen buffer size is %d bytes\n", bytes);

	g_Screen = (char*)mmap(0, bytes, PROT_READ|PROT_WRITE, MAP_SHARED, g_fb, 0);
	
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

void DrawPattern()
{
	unsigned short* buffer_ptr = (unsigned short*)g_Screen;
	int y, x;
	
	// Move screen ptr to top-left of character
	for (y=1;y<=SCR_HEIGHT;y++)
	{
		for (x=1;x<=SCR_WIDTH-1;x++)
		{
			int red,green,blue;
            
            blue = x/10;
            red = x%32;
            green = y/5;
            printf(" Plotting:\n    x: %d y: %d R:%d G:%d B:%d\n", x, y, red, green, blue);
			*buffer_ptr = (red<<11)+(green<<5)+blue;
			buffer_ptr++;
		}
		buffer_ptr += 1;
	}
}

int main(int argc, char* argv[])
{
	InitFrameBuffer();
	DrawPattern();
 
	return 0;
}

