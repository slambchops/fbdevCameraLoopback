#ifndef DISPLAY_H
#define DISPLAY_H

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

class Display
{
	private:
		int fd;
		int height;
		int width;
		unsigned int buf_size;
		unsigned char *buf_addr;
		char name[10];
		struct fb_fix_screeninfo fixinfo;
		struct fb_var_screeninfo varinfo;

		int fb_init_device(void);
		int fb_exit_device(void);
	public:
		Display(char *dev, int w, int h);

		int init(void);
		int deinit(void);
		int getW(void);
		int getH(void);
		int displayFrame(char *frameAddr, int frameSize);
};

#endif // DISPLAY_H
