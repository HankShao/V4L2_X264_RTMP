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
#include <pthread.h>
#include "v4l2.h"
#include "encoder.h"
#include "rtsp.h"

#ifdef __cplusplus
extern "C" {
#endif
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

void dump_yuyv(char *pdata, int w, int h)
{
	char PATH[20];
	FILE *fp;

	sprintf(PATH, "./%d_%d.yuv", w, h);
	fp = fopen(PATH, "w");
	fwrite(pdata, 1, w*h*2, fp);
	fclose(fp);

	return;
}

void *task_uvc_capture_encode(void *param)
{
	struct v4l2_buffer frame;
	volatile int *pExit = (int *)param;
	void *handle;

	handle = Enc_OpenX264(640, 480, 25);
	while(*pExit == 0)
	{
		if (0 != read_frame(&v4l2_fd, 640, 480,	&n_buffers, buffers, 3, &frame)){
			usleep(40*1000);
			continue;
		}
		
		//dump_yuyv(buffers[frame.index].start, 640, 480);
#ifdef JPEGENC	
		char PATH[10];
		static int cnt = 0;
		sprintf(PATH, "./%d.jpg", cnt++);
		Enc_yuyv2jpg(buffers[frame.index].start, 640, 480, 80, PATH);
#endif
		enc_h264_out out;
		Enc_yuvToh264(buffers[frame.index].start, 640, 480, -1, 0, &out);
		static FILE *fp;
		if (!fp) 
			fp = fopen("./1.264", "a");
		fwrite(out.packetdata[0], 1, out.packetlen[0], fp);

		release_frame(&v4l2_fd, &frame);
		usleep(40000);
	}

	Enc_CloseX264(handle);

	*pExit = 0;
	return NULL;
}

void *task_rtsp(void *param)
{
    rtsp_task_run(param);
	return NULL;
}
int main(int argv, char *argc[])
{
	pthread_t tid1,tid2;
	volatile int exit1 = 0;
	volatile int exit2 = 0;

    v4l2_dev_config();	
	pthread_create(&tid1, NULL, task_uvc_capture_encode, (void *)&exit1);
	//pthread_create(&tid2, NULL, task_rtsp, (void *)&exit2);
	
	while(getchar() != 'q')
		printf("please input 'q' exit!\n");

	exit1 = 1;
	exit2 = 1;

	sleep(2); //wait phtread exit

	printf("demo successfully exited!\n");
    return 0;
}
#ifdef __cplusplus
}
#endif
