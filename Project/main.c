/*
 * File:main.c
 * note:V4L2.X264.RTSP学习主模块
 * time：2021年11月8日22:01:41
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "v4l2.h"
#include "jpegenc.h"

int v4l2_fd = -1;
int n_buffers = 0;
struct  buffer *buffers = NULL;

int v4l2_dev_config(void)
{
    v4l2_open_device (&v4l2_fd, NULL);

    buffers = v4l2_init_device (&v4l2_fd, NULL, 640, 480, &n_buffers, 0);

    v4l2_start_capturing (&v4l2_fd, &n_buffers);

    return 0;
}

void dump_yuv_422(char *pdata, int w, int h)
{
	char PATH[20];
	FILE *fp;

	sprintf(PATH, "./%d_%d.yuv", w, h);
	fp = fopen(PATH, "w");
	fwrite(pdata, 1, w*h*2, fp);
	fclose(fp);

	return;
}

int main(int argv, char *argc[])
{
	struct v4l2_buffer frame;
	int cnt = 0;

    v4l2_dev_config();	

	while(1)
	{
		if (0 != read_frame(&v4l2_fd, 640, 480,	&n_buffers, buffers, 3, &frame)){
			usleep(40*1000);
			continue;
		}

		//write(STDOUT_FILENO, buffers[frame.index].start, 640*480*2);
		//dump_yuv_422(buffers[frame.index].start, 640, 480);
		
		char PATH[10];
		sprintf(PATH, "./%d.jpg", cnt++);
		encode_yuyvjpg(buffers[frame.index].start, 640, 480, 80, PATH);
		release_frame(&v4l2_fd, &frame);
		sleep(1);
	}
		
    return 0;
}

