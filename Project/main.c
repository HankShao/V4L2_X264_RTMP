/*
 * File:main.c
 * note:V4L2.X264.RTSP学习主模块
 * time：2021年11月8日22:01:41
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "v4l2.h"


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

int main(int argv, char *argc[])
{
    v4l2_dev_config();	

	while(1 != read_frame(&v4l2_fd, 640, 480,  &n_buffers, buffers, 3));
		
    return 0;
}

