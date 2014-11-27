#ifndef CAMERA_H
#define CAMERA_H

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct buf_info
{
        unsigned int length;
        char *start;
};

class Camera
{
	private:
		int fd;
		int type;
		int height;
		int width;
		int buf_size;
		int num_buffers;
		char name[10];
		enum v4l2_memory memory_mode;
		struct buf_info *buffers;
		struct v4l2_buffer buf;
		struct v4l2_format fmt;

		int v4l2_init_device(void);
		int v4l2_exit_device(void);
		int v4l2_stream_on(void);
		int v4l2_stream_off(void);
		int v4l2_queue_buffer(void);
		int v4l2_dequeue_buffer(void);
	public:
		Camera(char *dev, int w, int h);

		int init(void);
		int deinit(void);
		char *getFrame(void);
		int getFrameSize(void);
};

#endif // CAMERA_H
