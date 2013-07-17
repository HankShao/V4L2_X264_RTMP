#ifndef __V4L2_H__
#define __V4L2_H__

struct buffer {    
	void *                  start;
	size_t                  length;
};

void v4l2_open_device (int * fd, char * dev_name);
void v4l2_start_capturing (int * fd, int * n_buffers );
struct buffer *v4l2_init_device (int * fd, char * dev_name, int width, \
                                    int height, int * n_buffers, int pixel_format);
int read_frame  (int * fd, int width, int height, int * n_buffers, 
						struct buffer * buffers, int pixel_format);

#endif

