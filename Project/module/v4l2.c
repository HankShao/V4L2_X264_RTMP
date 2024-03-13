/*
 *File:v4l2.c
 *Note:v4l2功能模块实现，通过该文件代码实现
 *     UVC摄像头的画面采集，对外输出YUV格式
 *     数据，供后级使用。
 *
 *Time:2021年11月8日22:31:42
 *Author:Hank
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include "v4l2.h"

#define UVC_DEVICE  "/dev/video0"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static void errno_exit (const char *s)
{
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	exit (EXIT_FAILURE);
}

void v4l2_open_device (int * fd, char * dev_name)
{
    struct stat st; 

    char dev[20];
    if (dev_name)
        strcpy(dev, dev_name);
    else
        strcpy(dev, UVC_DEVICE);

    if (-1 == stat (dev, &st)) 
    {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n",dev, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }
    
    //确定文件是字符设备
    if (!S_ISCHR (st.st_mode)) 
    {
        fprintf (stderr, "%s is no device\n", dev);
        exit (EXIT_FAILURE);
    }

    *fd = open (dev, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == *fd) 
    {
        fprintf (stderr, "Cannot open '%s': %d, %s\n",dev, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    return;
}

//a blocking wrapper of the ioctl function
static int xioctl (int fd, int request, void *arg)
{
    int r;

    do r = ioctl (fd, request, arg);
    while (-1 == r && EINTR == errno);

    return r;
}

//alloc buffers and configure the shared memory area
struct buffer *init_mmap (int * fd, char * dev_name, int * n_buffers)
{
    struct v4l2_requestbuffers req = {0};
    //buffers is an array of n_buffers length, and every element store a frame
    struct buffer *buffers = NULL;

    req.count               = 4; //申请4块帧缓存
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;    
    if (-1 == xioctl (*fd, VIDIOC_REQBUFS, &req)) 
    {
        if (EINVAL == errno) 
        {
            fprintf (stderr, "%s does not support "
                              "memory mapping\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) 
    {
        fprintf (stderr, "Insufficient buffer memory on %s\n",dev_name);
        exit (EXIT_FAILURE);
    }    
    
    /* 申请内存节点 */
    buffers = calloc (req.count, sizeof (*buffers));

    //map every element of the array buffers to the shared memory
    for (*n_buffers = 0; *n_buffers < req.count; ++*n_buffers) 
    {
        struct v4l2_buffer buf = {0};
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = *n_buffers; /*VIDIOC_REQBUFS 申请的BUFFER ID*/

        /* 把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址 */
        if (-1 == xioctl (*fd, VIDIOC_QUERYBUF, &buf))
            errno_exit ("VIDIOC_QUERYBUF");
        buffers[*n_buffers].length = buf.length;
        /* 物理地址映射得到进程虚拟地址 */
        buffers[*n_buffers].start = mmap (NULL /* start anywhere */,
                                          buf.length,
                                          PROT_READ | PROT_WRITE /* required */,
                                          MAP_SHARED /* recommended */,
                                          *fd, buf.m.offset);
        if (MAP_FAILED == buffers[*n_buffers].start)
            errno_exit ("mmap");       
    }
    
    return buffers;
}

void v4l2_start_capturing (int * fd, int * n_buffers )
{
    unsigned int i = 0;
    enum v4l2_buf_type type;

    for (i = 0; i < *n_buffers; ++i) 
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;      
        /* 将空的待填充的（输出）缓冲区放入驱动程序的传入队列中 */    
        if (-1 == xioctl (*fd, VIDIOC_QBUF, &buf))
            errno_exit ("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //start the capture from the device
    if (-1 == xioctl (*fd, VIDIOC_STREAMON, &type))
        errno_exit ("VIDIOC_STREAMON");
    
    return;
}


//configure and initialize the hardware device 
struct buffer *v4l2_init_device (int * fd, char * dev_name, int width,
                                    int height, int * n_buffers, int pixel_format)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    struct buffer * buffers = NULL;
//    unsigned int min;            

    memset(&cap, 0, sizeof(cap));
    memset(&cropcap, 0, sizeof(cropcap));
    memset(&crop, 0, sizeof(crop));
    memset(&fmt, 0, sizeof(fmt));
    
    if (-1 == xioctl (*fd, VIDIOC_QUERYCAP, &cap)) 
    {
        if (EINVAL == errno) 
        {
            fprintf (stderr, "%s is no V4L2 device\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
    {
        fprintf (stderr, "%s is no video capture device\n",dev_name);
        exit (EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
    {
        fprintf (stderr, "%s does not support streaming i/o\n",dev_name);
        exit (EXIT_FAILURE);
    }   

    /* Select video input, video standard and tune here. */
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl (*fd, VIDIOC_CROPCAP, &cropcap)) 
    {  }

    /* 设置默认裁剪尺寸 */
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */
    if (-1 == xioctl (*fd, VIDIOC_S_CROP, &crop)) 
    {
        switch (errno) {
            case EINVAL:
                    /* Cropping not supported. */
            break;
            default:
                    /* Errors ignored. */
            break;    
        } 
    }

    /* 配置图像格式属性 */
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    if (-1 == xioctl (*fd, VIDIOC_S_FMT, &fmt))
        errno_exit ("\nError: pixel format not supported\n");
    
    /* 检查配置 */
//    min = fmt.fmt.pix.width * 2;
//
    /* 申请帧缓存并映射给用户访问 */
    buffers = init_mmap(fd, dev_name, n_buffers);

       
    return buffers;
}


//read one frame from memory and throws the data to standard output
int v4l2_read_frame(int * fd, int width, int height, int * n_buffers, 
						struct buffer * buffers, int pixel_format, struct v4l2_buffer *frame)
{
	struct v4l2_buffer buf;//needed for memory mapping
//	unsigned int Bpf;//bytes per frame
//	static int cnt;
//	const int dstframe = 8;

	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	while (1) 
	{
        if (-1 == xioctl (*fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno) 
            {
                case EAGAIN:
                    //printf("xioctl error EAGAIN!\n");
                    usleep(40000);
                    break;
                case EIO://EIO ignored
                default:
                    errno_exit ("VIDIOC_DQBUF");
                    return -1;
            }
        }
        else
            break;
	}
		

	*frame = buf;

	return 0;

#if 0
	//assert (buf.index < *n_buffers);

	switch (pixel_format) 
	{  
		case 0: //YUV420
			Bpf = width*height*12/8;           
			break;
		case 1: //RGB565
			Bpf = width*height*2;
			break;
		case 2: //RGB32
			Bpf = width*height*4;
		break;
		case 3: //YUYV
			Bpf = width*height*2;
		break;
	}

	int ret;
	//writing to standard output
	if (cnt++%dstframe==0)
		ret = write(STDOUT_FILENO, buffers[buf.index].start, Bpf);
	
	if (-1 == xioctl (*fd, VIDIOC_QBUF, &buf))
		errno_exit ("VIDIOC_QBUF");

	return 1;
#endif

}

int v4l2_release_frame(int * fd, struct v4l2_buffer *frame)
{
	if (-1 == xioctl (*fd, VIDIOC_QBUF, frame))
		errno_exit ("VIDIOC_QBUF");

	return 0;
}










