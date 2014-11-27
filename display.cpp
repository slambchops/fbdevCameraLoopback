#include "display.h"

int Display::fb_init_device(void) {
	int ret;

	/* Open the display device */
	if ((fd = open(name, O_RDWR)) <= 0) {
		printf("%s: Cannot open device\n", name);
		close(fd);
		return -1;
	}

	printf("\n%s: Opened device\n", name);

	ret = ioctl(fd, FBIOGET_FSCREENINFO, &fixinfo);
	if (ret < 0) {
		printf("%s: Error reading fixed info\n", name);
		close(fd);
		return -1;
	}

        ret = ioctl(fd, FBIOGET_VSCREENINFO, &varinfo);
        if (ret < 0) {
                printf("%s: Error reading variable info\n", name);
                close(fd);
		return -1;
        }

	varinfo.xres = width;
	varinfo.yres = height;
	varinfo.xres_virtual = width;
	varinfo.yres_virtual = height;

        ret = ioctl(fd, FBIOPUT_VSCREENINFO, &varinfo);
        if (ret < 0) {
                printf("%s: Error setting variable info\n", name);
                close(fd);
                return -1;
        }

        printf("%s: xres = %d\n", name, varinfo.xres);
        printf("%s: yres = %d\n", name, varinfo.yres);
        printf("%s: xres virtual = %d\n", name, varinfo.xres_virtual);
        printf("%s: yres virtual = %d\n", name, varinfo.yres_virtual);
        printf("%s: bits per pixel = %d\n", name, varinfo.bits_per_pixel);

	width = varinfo.xres;
	height = varinfo.yres;

	/* Mmap the driver buffers in application space so that application
	 * can write on to them. Driver allocates contiguous memory for
	 * three buffers. These buffers can be displayed one by one. */
	buf_size = fixinfo.line_length * varinfo.yres;
	buf_addr = (unsigned char *)mmap (0, buf_size,
		(PROT_READ|PROT_WRITE), MAP_SHARED, fd, 0);
 
	if (buf_addr == MAP_FAILED) {
		printf("%s: mmap failed\n", name);
		close(fd);
		return -1;
	}

	printf("%s: Init done successfully\n", name);

	return 0;
}

int Display::fb_exit_device(void) {
	munmap(buf_addr, buf_size);
	close(fd);

	return 0;
}

int Display::init(void) {
	if (fb_init_device() < 0)
		return -1;

	return 0;
}

int Display::deinit(void) {
	fb_exit_device();

	return 0;
}

int Display::displayFrame(char *frameAddr, int frameSize)
{
	memcpy(buf_addr, frameAddr, frameSize);

	return 0;
}

int Display::getW(void)
{
        return width;
}

int Display::getH(void)
{
        return height;
}

Display::Display(char *dev, int w, int h) {
        height = h;
        width = w;
        strcpy(name, dev);
}
