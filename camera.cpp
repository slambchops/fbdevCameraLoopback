#include "camera.h"

/*
 * V4L2 capture device initialization
 */
int Camera::v4l2_init_device(void)
{
	int ret, i, j;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	struct v4l2_format temp_fmt;
	struct buf_info *temp_buffers;
	struct v4l2_capability capability;

	/* Open the capture device */
	fd = open((const char *) name, O_RDWR);
	if (fd <= 0) {
		printf("%s: Cannot open device\n", name);
		goto ERROR;
	}

	printf("\n%s: Opened device\n", name);

	/* Check if the device is capable of streaming */
	if (ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0) {
		perror("VIDIOC_QUERYCAP");
		goto ERROR;
	}

	if (capability.capabilities & V4L2_CAP_STREAMING)
		printf("%s: Capable of streaming\n", name);
	else {
		printf("%s: Not capable of streaming\n", name);
		goto ERROR;
	}

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	temp_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	temp_fmt.fmt.pix.width = width;
	temp_fmt.fmt.pix.height = height;
	temp_fmt.fmt.pix.pixelformat = fmt.fmt.pix.pixelformat;

	ret = ioctl(fd, VIDIOC_S_FMT, &temp_fmt);
	if (ret < 0) {
		perror("VIDIOC_S_FMT");
		goto ERROR;
	}

	printf("%s: xres = %d\n", name, temp_fmt.fmt.pix.width);
        printf("%s: yres = %d\n", name, temp_fmt.fmt.pix.height);

	fmt = temp_fmt;

	reqbuf.count = num_buffers;
	reqbuf.memory = memory_mode;
	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		perror("Cannot allocate memory");
		goto ERROR;
	}
	num_buffers = reqbuf.count;
	printf("%s: Number of requested buffers = %u\n", name,
		num_buffers);

	temp_buffers = (struct buf_info *) malloc(sizeof(struct buf_info) *
		num_buffers);
	if (!temp_buffers) {
		printf("Cannot allocate memory\n");
		goto ERROR;
	}

	for (i = 0; i < num_buffers; i++) {
		buf.type = reqbuf.type;
		buf.index = i;
		buf.memory = reqbuf.memory;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QUERYCAP");
			num_buffers = i;
			goto ERROR_MEM;
		}

		temp_buffers[i].length = buf.length;
		temp_buffers[i].start = (char*)mmap(NULL, buf.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED, fd,
			buf.m.offset);
		if (temp_buffers[i].start == MAP_FAILED) {
			printf("Cannot mmap = %d buffer\n", i);
			num_buffers = i;
			goto ERROR_MEM;
		}
	}

	buffers = temp_buffers;

	printf("%s: Init done successfully\n", name);
	return 0;

ERROR_MEM:
	for (j = 0; j < num_buffers; j++)
		munmap(temp_buffers[j].start,
			temp_buffers[j].length);
	free(temp_buffers);

ERROR:
	close(fd);

	return -1;
}

int Camera::v4l2_exit_device(void)
{
	int i;

	for (i = 0; i < num_buffers; i++) {
		munmap(buffers[i].start,
			buffers[i].length);
	}

	free(buffers);
	close(fd);

	return 0;
}


/*
 * Enable streaming for V4L2 capture device
 */
int Camera::v4l2_stream_on(void)
{
	int a, i, ret;

	for (i = 0; i < num_buffers; ++i) {
		struct v4l2_buffer buf;

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		ret = ioctl(fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QBUF");
			num_buffers = i;
			return -1;
		}
	}

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.index = 0;
	buf.memory = memory_mode;

	a = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, &a);
	if (ret < 0) {
		perror("VIDIOC_STREAMON");
		return -1;
	}
	printf("%s: Stream on\n", name);

	return 0;
}

/*
 * Disable streaming for V4L2 capture device
 */
int Camera::v4l2_stream_off(void)
{
	int a, ret;

	a = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMOFF, &a);
	if (ret < 0) {
		perror("VIDIOC_STREAMOFF");
		return -1;
	}
	printf("%s: Stream off\n", name);

	return 0;
}

/*
 * Queue V4L2 buffer
 */
int Camera::v4l2_queue_buffer(void)
{
	int ret;
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/* Timeout. */
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	ret = select(fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == ret) {
		if (EINTR == errno) {
			perror( "select\n");
			return -1;
		}
	}

	if (0 == ret) {
		perror( "select timeout\n");
		return -1;
	}

	/* Queue buffer for the v4l2 capture device */
	ret = ioctl(fd, VIDIOC_QBUF,
	&buf);
	if (ret < 0) {
		perror("VIDIOC_QBUF");
		return -1;
	}

	return 0;
}

/*
 * DeQueue V4L2 buffer
 */
int Camera::v4l2_dequeue_buffer(void)
{
	int ret;
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	/* Timeout. */
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	ret = select(fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == ret) {
		if (EINTR == errno) {
			perror( "select\n");
			return -1;
		}
	}

	if (0 == ret) {
		perror( "select timeout\n");
		return -1;
	}

	/* Dequeue buffer for the v4l2 capture device */
	ret = ioctl(fd, VIDIOC_DQBUF,
	&buf);
	if (ret < 0) {
		perror("VIDIOC_DQBUF");
		return -1;
	}

	return 0;
}

/*
 * Initializes camera for streaming
 */
int Camera::init(void)
{
	/* Initialize the v4l2 capture devices */
	if (v4l2_init_device() < 0)
		return -1;

	/* Enable streaming for the v4l2 capture devices */
	if (v4l2_stream_on() < 0) {
		deinit();
		return -1;
	}

	/* Get a buffer for consumption */
	if (v4l2_dequeue_buffer() < 0) {
		deinit();
		return -1;
	}	

	return 0;
}

/*
 * Closes down the camera
 */
int Camera::deinit(void)
{
	v4l2_stream_off();
	v4l2_exit_device();

	return 0;
}

char *Camera::getFrame(void)
{
	/* Return previously used buffer */
        if (v4l2_queue_buffer() < 0)
		return (char *)-1;

        /* Get a buffer for consumption */
        if (v4l2_dequeue_buffer() < 0)
		return (char *)-1;

	return buffers[buf.index].start;
}

int Camera::getFrameSize(void)
{
	return buf_size;
}

Camera::Camera(char *dev, int w, int h)
{
	height = h;
	width = w;
	buf_size = height * width * 2;
	strcpy(name, dev);

	memory_mode = V4L2_MEMORY_MMAP;
	num_buffers = 3;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
}
