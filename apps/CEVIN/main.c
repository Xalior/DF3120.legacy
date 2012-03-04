/*
 * CEVIN_head - process remote display commands from CEVIN_iod over TCP
 *
 * Copyright	  2011 D. D. Rimron.
 *
 * This code draws from many places:
 *   http://www.linuxhowtos.org/C_C++/socket.htm
 *   http://zarb.org/~gc/html/libpng.html
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SCR_WIDTH	320
#define SCR_HEIGHT 240

#define PNG_DEBUG 3
#include <png.h>

#include "pictools.c"

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

char extractBits(unsigned short byte, int startingPos, int offset) {
   return (byte >> startingPos) & ((1 << offset)-1);
}

void process_file(void)
{
    int image_type=0;
    unsigned short* buffer_ptr = (unsigned short*)g_Screen;
		/* Expand any grayscale, RGB, or palette images to RGBA */
		png_set_expand(png_ptr);
		/* Reduce any 16-bits-per-sample images to 8-bits-per-sample */
		png_set_strip_16(png_ptr);
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        image_type=3;

    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
        image_type=4;

	for (y=0; y<height; y++) {
		png_byte* row = row_pointers[y];
		for (x=0; x<width; x++) {
			png_byte* ptr = &(row[x*image_type]);
            if((image_type==3) || ((image_type==4) && (ptr[3]==255)))
            {
                *buffer_ptr = mkPixel(ptr[0],ptr[1],ptr[2]);
            }
            else 
            {
                *buffer_ptr = mkPixel(mkBlend(((extractBits(*buffer_ptr,11,5))<<3),ptr[0],ptr[3]),
                                      mkBlend(((extractBits(*buffer_ptr,5,6))<<2),ptr[1],ptr[3]),
                                      mkBlend(((extractBits(*buffer_ptr,0,5))<<3),ptr[2],ptr[3]));
            }
			buffer_ptr++;
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
		abort_("Unable to open /dev/fb0\n");

	if (ioctl(g_fb, FBIOGET_FSCREENINFO, &fixed_info))
		abort_("Unable to get fixed screen info\n");

	if (ioctl(g_fb, FBIOGET_VSCREENINFO, &var_info))
		abort_("Unable to get var screen info\n");

	bytes = var_info.xres * var_info.yres * var_info.bits_per_pixel/8;

	g_Screen = (char*)mmap(0, bytes, PROT_READ|PROT_WRITE, MAP_SHARED, g_fb, 0);
	
	if ((int)screen == -1) 
		abort_("mmap() failed.\n"); 
}

int main(int argc, char* argv[])
{
	int sockfd, clientsock, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (argc < 2)
		abort_("ERROR, no port provided\n");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		abort_("ERROR opening socket");


	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		abort_("ERROR on binding");


	InitFrameBuffer(320, 200);


    while(1) {
    	listen(sockfd,5);
    	clilen = sizeof(cli_addr);
    	clientsock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    	if (clientsock < 0)
    		error("ERROR on accept");
    	bzero(buffer,256);
    	n = read(clientsock,buffer,255);
    	if (n < 0)
    		error("ERROR reading from socket");
    	printf("COMMAND: %s\n",buffer);
        int len = strlen(buffer);
        printf("COMMAND LEN: %d\n",len);
        if( buffer[len-1] == '\n' ) {
            buffer[len-1] = '\0';
        }
        int newlen = strlen(buffer);
    	printf("NEW COMMAND: %s\n",buffer);
        printf("NEW COMMAND LEN: %d\n",newlen);
        if( buffer[newlen-1] == '\r' ) {
            buffer[newlen-1] = '\0';
        }
        int newnewlen = strlen(buffer);
    	printf("NEWNEW COMMAND: %s\n",buffer);
        printf("NEWNEW COMMAND LEN: %d\n",newnewlen);
	    read_png_file(buffer);
	    process_file();
        
    	n = write(clientsock,"DONE\n",5);
    	if (n < 0)
    		error("ERROR writing to socket");
    	close(clientsock);
    }

	close(sockfd);
	return 0;
}

