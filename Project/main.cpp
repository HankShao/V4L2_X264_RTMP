/*
 * File:main.c
 * note:V4L2.X264.RTSP学习主模块
 * time：2021年11月8日22:01:41
 */

#include <iostream>
#include <memory>
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
#include "rtsp.hpp"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
int v4l2_fd = -1;
int n_buffers = 0;
struct  buffer *buffers = NULL;
rtsp_server *rtsp = nullptr;

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

void dump_enc(char *pdata, int len)
{
	static FILE *fp;
	if (!fp) 
		fp = fopen("./1.264", "a");
	fwrite(pdata, 1, len, fp);
}

void *task_uvc_capture_encode(void *param)
{
	struct v4l2_buffer frame;
	volatile int *pExit = (int *)param;
	void *handle;

	while (rtsp == nullptr) //等待rtsp启动
		usleep(100000);

	handle = Enc_OpenX264(640, 480, 25);
	while(*pExit == 0)
	{
		usleep(40000);
		if (0 != v4l2_read_frame(&v4l2_fd, 640, 480, &n_buffers, buffers, 3, &frame)){
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
		Enc_yuvToh264(static_cast<char *>(buffers[frame.index].start), 640, 480, -1, 0, &out);
		
		for (auto i=0; i < out.framenum; i++)
		{

			if(rtsp->m_framedq.size() < rtsp->m_dqlimit)
			{
				shared_ptr<nalu_data> nalu(new nalu_data, [](nalu_data *p){printf("del %p\n", p); delete p;});
				cout << "add:" << nalu << endl;
				nalu->key = out.key;
				nalu->pdata = out.packetdata[i];
				nalu->len = out.packetlen[i];
				rtsp->m_framedq.push_back(nalu);
			}
			else
				cout << "Deque is full limit:" << rtsp->m_dqlimit << endl;
		}
		v4l2_release_frame(&v4l2_fd, &frame);	
	}

	Enc_CloseX264(handle);

	*pExit = 0;
	return NULL;
}

void *task_rtsp(void *param)
{
	rtsp = new rtsp_server;
	rtsp->rtsp_server_start(param);

	return NULL;
}
int main(int argv, char *argc[])
{
	pthread_t tid1,tid2;
	volatile int exit1 = 0;
	volatile int exit2 = 0;

    v4l2_dev_config();	
	pthread_create(&tid1, NULL, task_uvc_capture_encode, (void *)&exit1);
	pthread_create(&tid2, NULL, task_rtsp, (void *)&exit2);
	
	while(getchar() != 'q')
		printf("please input 'q' exit!\n");

	exit1 = 1;
	exit2 = 1;

	pthread_join(tid1, NULL); //wait phtread exit
	pthread_join(tid2, NULL); //wait phtread exit
    delete rtsp;

	printf("demo successfully exited!\n");
    return 0;
}

