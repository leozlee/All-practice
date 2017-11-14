/*
 *
 * Copyright (C) 2007 Texas Instruments Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * File name   : saMmapLoopback.c
 * Description : Application used to do NTSC loopback in MMAP memory mode
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <assert.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include "../include/dbg.h"

#define _DISPLAY_		1 //enable display? 1:Y 0:N
#define _FILE_RECORD_	0 //enable file record capture data? 1:Y 0:N

#define IMG_WIDTH	    720 //src image width
#define IMG_HEITHT 	576 //src image height

/* structure used to store information of the buffers */
struct buf_info {
	int index;
	unsigned int length;
	char *start;
};

/* Changing the following will result in different number of buffers used */
#define CAPTURE_MAX_BUFFER		4 //max buffer at least 3, two for ping-pong, one for safty processed by alg
#if _DISPLAY_
#define DISPLAY_MAX_BUFFER		4
#endif

/* device to be used for capture */
#define CAPTURE_DEVICE		"/dev/video0"
#define CAPTURE_NAME		"Capture"
#if _DISPLAY_
/* device to be used for display */
#define DISPLAY_DEVICE		"/dev/video1"
#define DISPLAY_NAME		"Display"
#endif

#if 0
/* absolute path of the sysfs entry for controlling video1 to channel0 */
#define OUTPUTPATH      "/sys/class/display_control/omap_disp_control/video1"
/* absolute path of the sysfs entry for controlling channel0 to LCD */
#define OUTPUTPATH_1      "/sys/class/display_control/omap_disp_control/ch0_output"
#endif

///////////////////////////////////////////////////////////////////////////////////
//?è?????????????÷????
#define DEF_PIX_FMT	V4L2_PIX_FMT_UYVY	//?????è??????????????????UYVY
///////////////////////////////////////////////////////////////////////////////////


#if _DISPLAY_
#define DISP_WIDTH		IMG_WIDTH		//?è????·????????í??
#define DISP_HEIGHT	IMG_HEITHT		//?è????·???????????
static struct buf_info display_buff_info[DISPLAY_MAX_BUFFER];
#endif


/*===============================initCapture==================================*/
/* This function initializes capture device. It selects an active input
 * and detects the standard on that input. It then allocates buffers in the
 * driver's memory space and mmaps them in the application space.
 */
static int initCapture(int *capture_fd, int *numbuffers, char *inputname,
				char *stdname, struct buf_info *capture_buff_info, struct v4l2_format *fmt)
{
	int ret, i, j;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	struct v4l2_capability capability;
	struct v4l2_input input;
	v4l2_std_id std_id;
	struct v4l2_standard standard;
	int index;

	/* Open the capture device　打开摄像头设备*/
	*capture_fd  = open((const char *) CAPTURE_DEVICE, O_RDWR);
	if (*capture_fd  <= 0) {
		printf("Cannot open = %s device\n", CAPTURE_DEVICE);
		return -1;
	}
	printf("\n%s: Opened Channel\n", CAPTURE_NAME);

	/* Get any active input */
	/* VIDIOC_G_INPUT: ?????±?°??????????input???? */

	if (ioctl(*capture_fd, VIDIOC_G_INPUT, &index) < 0) {
		perror("VIDIOC_G_INPUT");
		goto ERROR;
	}

	printf("index = %d\n",index);

	/* Enumerate input to get the name of the input detected */
	/* VIDIOC_ENUMINPUT:?????ù??input???? */
	memset(&input, 0, sizeof(input));
	input.index = index;
	if (ioctl(*capture_fd, VIDIOC_ENUMINPUT, &input) < 0) {
		perror("VIDIOC_ENUMINPUT");
		goto ERROR;
	}

	printf("%s: Current Input: %s\n", CAPTURE_NAME, input.name);

	/* Store the name of the output as per the input detected */
	strcpy(inputname, input.name);

	/* Detect the standard in the input detected */
	/* VIDIOC_QUERYSTD: ?é???ì??????????±ê×? */
	if (ioctl(*capture_fd, VIDIOC_QUERYSTD, &std_id) < 0) {
		perror("VIDIOC_QUERYSTD");
		goto ERROR;
	}

	printf("VIDIOC_QUERYSTD std_id: %x\n",std_id);

	/* Get the standard*/
	/* VIDIOC_G_STD: ?????±?°??????????±ê×? */
	if (ioctl(*capture_fd, VIDIOC_G_STD, &std_id) < 0) {
		/* Note when VIDIOC_ENUMSTD always returns EINVAL this
		   is no video device or it falls under the USB exception,
		   and VIDIOC_G_STD returning EINVAL is no error. */
		perror("VIDIOC_G_STD");
		goto ERROR;
	}

	printf("VIDIOC_G_STD std_id: %x\n",std_id);

	memset(&standard, 0, sizeof(standard));
	standard.index = 0;
	while (1) {
		/* VIDIOC_ENUMSTD: ?????è±??§?????ù??±ê×? */
		if (ioctl(*capture_fd, VIDIOC_ENUMSTD, &standard) < 0) {
			perror("VIDIOC_ENUMSTD");
			goto ERROR;
		}

		/* Store the name of the standard */
		if (standard.id & std_id) {
			strcpy(stdname, standard.name);
			printf("%s: Current standard: %s\n",
			       CAPTURE_NAME, standard.name);
			break;
		}
		standard.index++;
	}

	/* Check if the device is capable of streaming */
	if (ioctl(*capture_fd, VIDIOC_QUERYCAP, &capability) < 0) {
		perror("VIDIOC_QUERYCAP");
		goto ERROR;
	}
	if (capability.capabilities & V4L2_CAP_STREAMING)
		printf("%s: Capable of streaming\n", CAPTURE_NAME);
	else {
		printf("%s: Not capable of streaming\n", CAPTURE_NAME);
		goto ERROR;
	}

	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/* VIDIOC_G_FMT: ???????????? */
	ret = ioctl(*capture_fd, VIDIOC_G_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_G_FMT");
		goto ERROR;
	}

	printf("VIDIOC_G_FMT fmt: %c-%c-%c-%c\n",
		fmt->fmt.pix.pixelformat & 0xff,
		(fmt->fmt.pix.pixelformat >> 8) & 0xff,
		(fmt->fmt.pix.pixelformat >> 16) & 0xff,
		(fmt->fmt.pix.pixelformat >> 24) & 0xff);

	printf("Default (w - h): (%d - %d)\n",fmt->fmt.pix.width, fmt->fmt.pix.height);

	fmt->fmt.pix.pixelformat = DEF_PIX_FMT;
	/* VIDIOC_S_FMT: ?è?????????? */
	ret = ioctl(*capture_fd, VIDIOC_S_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_S_FMT");
		goto ERROR;
	}

	printf("Actually (w - h): (%d - %d)\n",fmt->fmt.pix.width, fmt->fmt.pix.height);

	ret = ioctl(*capture_fd, VIDIOC_G_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_G_FMT");
		goto ERROR;
	}

	if (fmt->fmt.pix.pixelformat != DEF_PIX_FMT) {
		printf("%s: Requested pixel format not supported\n",
		       CAPTURE_NAME);
		goto ERROR;
	}

	/* Buffer allocation
	 * Buffer can be allocated either from capture driver or
	 * user pointer can be used
	 */
	/* Request for MAX_BUFFER input buffers. As far as Physically contiguous
	 * memory is available, driver can allocate as many buffers as
	 * possible. If memory is not available, it returns number of
	 * buffers it has allocated in count member of reqbuf.
	 * HERE count = number of buffer to be allocated.
	 * type = type of device for which buffers are to be allocated.
	 * memory = type of the buffers requested i.e. driver allocated or
	 * user pointer */
	reqbuf.count = *numbuffers;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	/* VIDIOC_REQBUFS: ?ò?è±????ó?????????????????????????????? */
	ret = ioctl(*capture_fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0)
	{
		perror("Cannot allocate memory");
		goto ERROR;
	}
	/* Store the number of buffers actually allocated */
	*numbuffers = reqbuf.count;
	printf("%s: Number of requested buffers = %d\n", CAPTURE_NAME,
	       *numbuffers);

	memset(&buf, 0, sizeof(buf));

	/* Mmap the buffers
	 * To access driver allocated buffer in application space, they have
	 * to be mmapped in the application space using mmap system call */
	for (i = 0; i < (*numbuffers); i++)
	{
		buf.type = reqbuf.type;
		buf.index = i;
		buf.memory = reqbuf.memory;
		/* VIDIOC_QUERYBUF: ?é??????????×??? */
		ret = ioctl(*capture_fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QUERYCAP");
			*numbuffers = i;
			goto ERROR1;
		}

		printf("bug_length:%d offset:%x\n", buf.length, buf.m.offset);

		capture_buff_info[i].length = buf.length;
		capture_buff_info[i].index = i;
		capture_buff_info[i].start = (char *)mmap(NULL,  //???????????????·???è????0?±±í???????????????????????????·
						buf.length,	//?????????€??,?€????????×?????????????×?????????°????????????í
						PROT_READ | PROT_WRITE, //??????????±??€±ê?????????????????ò??????????,±??????????????????????????è????????????
						MAP_SHARED, //???????????ó???à??????????????????????·????????í???????è?????????í
						*capture_fd,//???§???????è????????°?????open()????·????????????????è????-1?????±?è??????flags????????MAP_ANON,±í?÷????????????????
						buf.m.offset); //±????????ó??????????

		if (capture_buff_info[i].start == MAP_FAILED) {
			printf("Cannot mmap = %d buffer\n", i);
			*numbuffers = i;
			goto ERROR1;
		}

		//memset bug by 128
		memset((void *) capture_buff_info[i].start, 0x80,capture_buff_info[i].length);

		/* Enqueue buffers
		 * Before starting streaming, all the buffers needs to be
		 * en-queued in the driver incoming queue. These buffers will
		 * be used by the drive for storing captured frames. */
		/* ???????????????÷????(VIDIOC_STREAMON)?°????????buffer???è???????????????????? */
		/* VIDIOC_QBUF: ???????????????????????????????? */
		ret = ioctl(*capture_fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QBUF");
			*numbuffers = i + 1;
			goto ERROR1;
		}

		printf("cap index:%d\n",buf.index);
	}

	printf("%s: Init done successfully\n\n", CAPTURE_NAME);
	return 0;

ERROR1:
	for (j = 0; j < *numbuffers; j++)
		munmap(capture_buff_info[j].start,
		       capture_buff_info[j].length);
ERROR:
	close(*capture_fd);

	return -1;
}


//??????
int CaptureInit(int *capture_fd, struct buf_info *capture_buff_info, struct v4l2_format *capture_fmt, struct v4l2_buffer *capture_buf)
{
	int i = 0;
	char inputname[15];
	char stdname[15];
	int capture_numbuffers = CAPTURE_MAX_BUFFER;
	//params check


	//init capture buf
	for(i = 0; i < CAPTURE_MAX_BUFFER; i++)
	{
		capture_buff_info[i].start = NULL;
	}

	/*
	 * Initialization section
	 * Initialize capture and display devices.
	 * Here one capture channel is opened and input and standard is
	 * detected on that channel.
	 * Display channel is opened with the same standard that is detected at
	 * capture channel.
	 * */
	if(initCapture(capture_fd, &capture_numbuffers,inputname,
		stdname, capture_buff_info, capture_fmt) < 0)
	{
		printf("Error in opening display device\n");
		return -1;
	}

	/* Set the capture buffers for queuing and dqueueing operation */
	capture_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	capture_buf->index = 0;
	capture_buf->memory = V4L2_MEMORY_MMAP;
	return 0;
}

//????
int CaptureStart(int capture_fd)
{
	/* run section
	 * Here display and capture channels are started for streaming. After
	 * this capture device will start capture frames into enqueued
	 * buffers and display device will start displaying buffers from
	 * the qneueued buffers */
	/* Start Streaming. on capture device */
	int a = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(capture_fd, VIDIOC_STREAMON, &a) < 0)
	{
		perror("VIDIOC_STREAMON");
		return -1;
	}
	printf("%s: Stream on...\n", CAPTURE_NAME);
	return 0;
}

	fd_set rfds,wfds;

struct timeval tv;

int count=0;

//????????????????
int CaptureGetFrame(int capture_fd, struct v4l2_buffer *capture_buf)
{
	// check params

	// Dequeue capture buffer
	if (ioctl(capture_fd, VIDIOC_DQBUF, capture_buf) < 0) {
		dbg("ioctl-----------\n");
		perror("VIDIOC_DQBUF");
		return -1;
	}

//	if(capture_buf->index == 3)
//		dbg("capture_buf.index = 3");
	if(count != capture_buf->index)
	{
	  dbg("cap_buf.index=%d,count=%d\n",capture_buf->index,count);
	  count = capture_buf->index;
	}
	else
	{
//		dbg("count = %d\n",count);
		count++;
		if(count > CAPTURE_MAX_BUFFER-1)
			count = 0;
	}


	return 0;
}

//??????????????
int CapturePutBuf(int capture_fd, struct v4l2_buffer *capture_buf)
{
	//check params

	// Queue capture buffer
	if (ioctl(capture_fd, VIDIOC_QBUF, capture_buf) < 0) {
		perror("VIDIOC_QBUF");
		return -1;
	}
	return 0;
}

//·???????capture
int CaptureDeinit(int capture_fd)
{
	int a = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	/* VIDIOC_STREAMOFF: ??±??÷I/O??×÷ */
	if (ioctl(capture_fd, VIDIOC_STREAMOFF, &a) < 0) {
		perror("VIDIOC_STREAMOFF");
		return -1;
	}
	printf("%s: Stream off!!\n", CAPTURE_NAME);
}

//?????í?ó???í
int CaptureError(int capture_fd, struct buf_info *capture_buff_info)
{
	int i = 0;
	/* Un-map the buffers */
	for (i = 0; i < CAPTURE_MAX_BUFFER; i++) {
		munmap(capture_buff_info[i].start,
			capture_buff_info[i].length);
		capture_buff_info[i].start = NULL;
	}
	/* Close the file handle */
	close(capture_fd);
	exit(0);
}

#if _DISPLAY_
/*===============================initDisplay==================================*/
/* This function initializes display device. It sets output and standard for
 * LCD. These output and standard are same as those detected in capture device.
 * It, then, allocates buffers in the driver's memory space and mmaps them in
 * the application space */
static int initDisplay(int *display_fd, int *numbuffers, char *stdname,
				struct v4l2_format *fmt)
{
	int ret, i, j;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	struct v4l2_capability capability;
	int rotation;
	char str[200];

	/* Set the video1 pipeline to channel0 overlay */
#if 0
#if 1
	strcpy(str, "echo channel0 > ");
#else
	strcpy(str, "echo channel1 > ");
#endif
	strcat(str, OUTPUTPATH);
	if (system(str)) {
		printf("Cannot set video1 pipeline to TV\n");
		exit(0);
	}
	/* Set the output of channel0 to LCD */
#if 1
	strcpy(str, "echo LCD > ");
	strcat(str, OUTPUTPATH_1);
	if (system(str)) {
		printf("Cannot set output to LCD\n");
		exit(0);
	}
#endif
#endif
	/* Open the video display device */
	*display_fd = open((const char *) DISPLAY_DEVICE, O_RDWR);
	if (*display_fd <= 0) {
		printf("Cannot open = %s device\n", DISPLAY_DEVICE);
		return -1;
	}
	printf("\n%s: Opened Channel\n", DISPLAY_NAME);

	/* Check if the device is capable of streaming */
	if (ioctl(*display_fd, VIDIOC_QUERYCAP, &capability) < 0) {
		perror("VIDIOC_QUERYCAP");
		goto ERROR;
	}

	if (capability.capabilities & V4L2_CAP_STREAMING)
		printf("%s: Capable of streaming\n", DISPLAY_NAME);
	else {
		printf("%s: Not capable of streaming\n", DISPLAY_NAME);
		goto ERROR;
	}

	/* Rotate by 90 degree so that 480x640 resolution will become 640x480 */
#if 0
	rotation = 90;
	ret = ioctl(*display_fd, VIDIOC_S_OMAP2_ROTATION, &rotation);
	if (ret < 0) {
		perror("VIDIOC_S_OMAP2_ROTATION");
		goto ERROR;
	}
#endif
	/* Get the format */
	fmt->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret = ioctl(*display_fd, VIDIOC_G_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_G_FMT");
		goto ERROR;
	}

	fmt->fmt.pix.width = DISP_WIDTH;
	fmt->fmt.pix.height = DISP_HEIGHT;
	fmt->fmt.pix.pixelformat = DEF_PIX_FMT;
	ret = ioctl(*display_fd, VIDIOC_S_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_S_FMT");
		goto ERROR;
	}

	ret = ioctl(*display_fd, VIDIOC_G_FMT, fmt);
	if (ret < 0) {
		perror("VIDIOC_G_FMT");
		goto ERROR;
	}

	if (fmt->fmt.pix.pixelformat != DEF_PIX_FMT) {
		printf("%s: Requested pixel format not supported\n",
		       CAPTURE_NAME);
		goto ERROR;
	}

	/* Buffer allocation
	 * Buffer can be allocated either from capture driver or
	 * user pointer can be used
	 */
	/* Request for MAX_BUFFER input buffers. As far as Physically contiguous
	 * memory is available, driver can allocate as many buffers as
	 * possible. If memory is not available, it returns number of
	 * buffers it has allocated in count member of reqbuf.
	 * HERE count = number of buffer to be allocated.
	 * type = type of device for which buffers are to be allocated.
	 * memory = type of the buffers requested i.e. driver allocated or
	 * user pointer */
	reqbuf.count = *numbuffers;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(*display_fd, VIDIOC_REQBUFS, &reqbuf);
	if (ret < 0) {
		perror("Cannot allocate memory");
		goto ERROR;
	}
	/* Store the numbfer of buffers allocated */
	*numbuffers = reqbuf.count;
	printf("%s: Number of requested buffers = %d\n", DISPLAY_NAME,
	       (*numbuffers));

	memset(&buf, 0, sizeof(buf));

	/* Mmap the buffers
	 * To access driver allocated buffer in application space, they have
	 * to be mmapped in the application space using mmap system call */
	for (i = 0; i < (*numbuffers); i++) {
		/* Query physical address of the buffers */
		buf.type = reqbuf.type;
		buf.index = i;
		buf.memory = reqbuf.memory;
		ret = ioctl(*display_fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QUERYCAP");
			(*numbuffers) = i;
			goto ERROR1;
		}

		/* Mmap the buffers in application space */
		display_buff_info[i].length = buf.length;
		display_buff_info[i].index =  i;
		display_buff_info[i].start = mmap(NULL, buf.length,
					     PROT_READ |
					     PROT_WRITE,
					     MAP_SHARED,
					     *display_fd,
					     buf.m.offset);

		if (display_buff_info[i].start == MAP_FAILED) {
			printf("Cannot mmap = %d buffer\n", i);
			(*numbuffers) = i;
			goto ERROR1;
		}
		memset((void *) display_buff_info[i].start, 0x80,
		       display_buff_info[i].length);

		/* Enqueue buffers
		 * Before starting streaming, all the buffers needs to be
		 * en-queued in the driver incoming queue. These buffers will
		 * be used by thedrive for storing captured frames. */
		ret = ioctl(*display_fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("VIDIOC_QBUF");
			(*numbuffers) = i + 1;
			goto ERROR1;
		}
	}
	printf("%s: Init done successfully\n\n", DISPLAY_NAME);
	return 0;

ERROR1:
	for (j = 0; j < *numbuffers; j++)
		munmap(display_buff_info[j].start,
			display_buff_info[j].length);
ERROR:
	close(*display_fd);

	return -1;
}

//??????
int DisplayInit(int *display_fd, struct buf_info *display_buff_info, struct v4l2_format *display_fmt, struct v4l2_buffer *display_buf)
{
	int i = 0;
	int ret = 0;
	char stdname[15];
	int display_numbuffers = DISPLAY_MAX_BUFFER;

	for(i = 0; i < DISPLAY_MAX_BUFFER; i++) {
		display_buff_info[i].start = NULL;
	}

	/* open display channel */
	ret = initDisplay(display_fd, &display_numbuffers,
		stdname, display_fmt);

	/* Set the display buffers for queuing and dqueueing operation */
	display_buf->type	= V4L2_BUF_TYPE_VIDEO_OUTPUT;
	display_buf->index	= 0;
	display_buf->memory = V4L2_MEMORY_MMAP;

	return 0;
}

int DisplayStart(int display_fd)
{
	/* Start Streaming. on display device */
	int a = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	/* VIDIOC_STREAMON: ?ò???????÷ */
	if (ioctl(display_fd, VIDIOC_STREAMON, &a) < 0) {
		perror("VIDIOC_STREAMON");
		return -1;
	}
	printf("%s: Stream on...\n", DISPLAY_NAME);
	return 0;
}

int DisplayGetFrame(int display_fd, struct v4l2_buffer *display_buf)
{
	/* Dequeue display buffer */
	if (ioctl(display_fd, VIDIOC_DQBUF, display_buf) < 0)
	{
		perror("VIDIOC_DQBUF");
		return -1;
	}
	return 0;
}
int discount=0;
int DisplayPutBuf(int display_fd, struct v4l2_buffer *display_buf)
{
	/* Queue display buffer */
	if (ioctl(display_fd, VIDIOC_QBUF, display_buf) < 0)
	{
		perror("VIDIOC_QBUF");
		return -1;
	}

//	if(display_buf->index == 3)
//		dbg("display_buf.index = 3");
	if(discount != display_buf->index)
	{
	  dbg("display_buf.index=%d,discount=%d\n",display_buf->index,discount);
	  discount = display_buf->index;
	}
	else
	{
//		dbg("count = %d\n",count);
		discount++;
		if(discount > DISPLAY_MAX_BUFFER-1)
			discount = 0;
	}
	return 0;
}

int DisplayDeinit(int display_fd)
{
	int a = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	/* VIDIOC_STREAMOFF: ??±??÷I/O??×÷ */
	if (ioctl(display_fd, VIDIOC_STREAMOFF, &a) < 0) {
		perror("VIDIOC_STREAMOFF");
		return -1;
	}
	printf("\n%s: Stream off!!\n", DISPLAY_NAME);
	return 0;
}

int DisplayError(int display_fd, struct buf_info *display_buff_info)
{
	int i = 0;
	/* Un-map the buffers */
	for (i = 0; i < DISPLAY_MAX_BUFFER; i++) {
		munmap(display_buff_info[i].start,
			display_buff_info[i].length);
		display_buff_info[i].start = NULL;
	}
	/* Close the file handle */
	close(display_fd);
}

#endif

int main(int argc, char *argv[])
{

	int i = 0;
	int ret = 0;
	struct v4l2_format capture_fmt;
	int capture_fd;
	char stdname[15];
	struct v4l2_buffer capture_buf;
	unsigned char captureImage[IMG_WIDTH * IMG_HEITHT];
	/* capture_buff_info and display_buff_info stores buffer information of capture
	and display respectively. */
	struct buf_info capture_buff_info[CAPTURE_MAX_BUFFER];
	int a;

#if _DISPLAY_
	struct v4l2_format display_fmt;
	int display_fd;
	struct v4l2_buffer display_buf;

#endif


#if _FILE_RECORD_
	int record_idx			= 0;
	int record_frame_max	= 50; //??????×??ó????
	FILE * fp = fopen("capture_data.txt","wb");
	if (fp == NULL)
	{
		printf("File open error!\n");
		return -1;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	//????????????????/????????
	printf("-----------------------------------------------------------------\n");
	printf("-------------------------- 10 ------------------------------------\n");
	printf("-----------------------------------------------------------------\n");
	memset(captureImage, 0, IMG_WIDTH * IMG_HEITHT * sizeof(unsigned char));

	//init capture
	if(CaptureInit(&capture_fd, capture_buff_info, &capture_fmt, &capture_buf) < 0)
	{
		printf("Error in opening capture device for channel 0\n");
	}

	//start capture
	if(CaptureStart(capture_fd) < 0)
	{
		perror("CaptureStart\n");
	}

#if _DISPLAY_
	DisplayInit(&display_fd, display_buff_info, &display_fmt, &display_buf);

	DisplayStart(display_fd);
#endif

	//////////////////////////////////////////////////////////////////////////
	//

	/* One buffer is dequeued from display and capture channels.
	 * Capture buffer will be copied to display buffer.
	 * All two buffers are put back to respective channels.
	 * This sequence is repeated in loop.
	 * After completion of this loop, channels are stopped.
	 */
	while(1)
	{
		int h;
		char *cap_ptr, *dis_ptr;

#if _DISPLAY_
		DisplayGetFrame(display_fd, &display_buf);
#endif

	  FD_ZERO(&rfds);
	  FD_ZERO(&wfds);
	  FD_SET(capture_fd, &rfds);
	  FD_SET(capture_fd, &wfds);
	  tv.tv_sec = 1;
	  tv.tv_usec = 0;

//	  dbg("will enter select\n");
	  select(capture_fd + 1, &rfds, &wfds, NULL, &tv);
	  /*数据可获得*/
	  if (FD_ISSET(capture_fd, &rfds))
	  {
//		dbg("Device can be read now\n");
		//get a frame
		CaptureGetFrame(capture_fd, &capture_buf);

	  }
	  else
	  {
//		  dbg("will init capture_fd=%d\n",capture_fd);

			CaptureDeinit(capture_fd);

			memset(captureImage, 0, IMG_WIDTH * IMG_HEITHT * sizeof(unsigned char));

			usleep(10);
			//init capture
			if(CaptureInit(&capture_fd, capture_buff_info, &capture_fmt, &capture_buf) < 0)
			{
				dbg("Error in opening capture device for channel 0\n");
			}
		//	return 0;

			//start capture
			if(CaptureStart(capture_fd) < 0)
			{
				perror("CaptureStart\n");
			}

			sleep(1);


	  }



//	  dbg("cap_buf.index=%d\n",display_buf.index);

	  //point to frame buf
	  cap_ptr = capture_buff_info[capture_buf.index].start;


#if _DISPLAY_
		dis_ptr = display_buff_info[display_buf.index].start;

		assert(capture_fmt.fmt.pix.width == IMG_WIDTH || capture_fmt.fmt.pix.height == IMG_HEITHT);
		//printf("disp: width:%d height:%d\n", capture_fmt.fmt.pix.width, capture_fmt.fmt.pix.height);

		//?????????????????÷????????????????????????×ó????????????
		for (h = 0; h < display_fmt.fmt.pix.height; h++) {
			//????????????
			memcpy(dis_ptr, cap_ptr, display_fmt.fmt.pix.width * 2);
			cap_ptr += capture_fmt.fmt.pix.width * 2;
			dis_ptr += display_fmt.fmt.pix.width * 2;
		}

		int ret = DisplayPutBuf(display_fd, &display_buf);
		if(ret == -1)
		{
			dbg("display put buf error\n");
		}
#endif

		//put the buf
		ret = CapturePutBuf(capture_fd,&capture_buf);
		if(ret == -1)
		{
			dbg("capture put buf error\n");
		}

#if _FILE_RECORD_
		if(record_idx > 2 && record_idx < record_frame_max)
		{
			//uyvy?????????ó??
			int cap_size =  capture_fmt.fmt.pix.width * capture_fmt.fmt.pix.height * 2;
			printf("frame:%d size:%d cap:(width:%d height:%d)\n",record_idx, cap_size, capture_fmt.fmt.pix.width, capture_fmt.fmt.pix.height);

			//?????????????????÷??????????????
			fwrite(cap_ptr, cap_size, 1, fp);
			fflush(fp);
		}

		record_idx ++;
#endif
	}

#if _FILE_RECORD_
	fclose(fp);
#endif

#if _DISPLAY_
	DisplayDeinit(display_fd);
#endif

	//de-init
	CaptureDeinit(capture_fd);

	return 0;
}
