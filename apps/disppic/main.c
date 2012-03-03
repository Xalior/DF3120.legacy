/*
  * DispPict - A picture viewer for the DF3120 frame.
 *
 * Copyright      2011 D. D. Rimron.
 *
 * This is massively butchered from "The beautiful
 * code" from at http://zarb.org/~gc/html/libpng.html
 * "A simple libpng example program" - and therefore the
 * original copyright message from that tutorial remains
 * below.
 *
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

#define SCR_WIDTH	320
#define SCR_HEIGHT 240

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

// Backing file
int g_fb = 0;

// shared memory area
char* g_Screen = NULL;

int x,y;
int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;


void read_png_file(char* file_name)
{
	char header[8];		// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading", file_name);

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
	
	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during init_io");


	png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[read_png_file] Error during read_image");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);
}

void process_file(void)
{
        unsigned short* buffer_ptr = (unsigned short*)g_Screen;

        /* Expand any grayscale, RGB, or palette images to RGBA */
        png_set_expand(png_ptr);

        /* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
        png_set_strip_16(png_ptr);

        for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {
                        png_byte* ptr = &(row[x*4]);
                        printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n",
                               x, y, ptr[0], ptr[1], ptr[2], ptr[3]);
                       *buffer_ptr = (ptr[0]<<11)+(ptr[1]<<5)+ptr[2];
                        buffer_ptr++;

                        /* perform whatever modifications needed, for example to set red value to 0 and green value to the blue one:
                           ptr[0] = 0;
                           ptr[1] = ptr[2]; */
                }
        }
}

char* g_DisplayBuffer;

// Initialise and fill with something sensible
void InitBuffer()
{
	g_DisplayBuffer = malloc(SCR_WIDTH*SCR_HEIGHT);
	memset((void*)g_DisplayBuffer, '.', SCR_WIDTH*SCR_HEIGHT);
}

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

	bytes = var_info.xres * var_info.yres * var_info.bits_per_pixel /8;

	g_Screen = (char*)mmap(0, bytes, PROT_READ|PROT_WRITE, MAP_SHARED, g_fb, 0);
	//printf("mmap pointer is 0x%x\n", (unsigned)screen);
	
	if ((int)screen == -1) 
	{
		printf("mmap() failed.\n"); 
		exit(1);
	}
}

int main(int argc, char* argv[])
{

	if (argc != 2)
		abort_("Usage: disppict <pngfile>");

	InitFrameBuffer(320, 200);

	read_png_file(argv[1]);
	process_file();	
	return 0;
}

