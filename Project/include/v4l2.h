#ifndef __V4L2_H__
#define __V4L2_H__

#ifdef __cplusplus
extern "C" {
#endif

struct buffer {    
	void *                  start;
	size_t                  length;
};

void v4l2_open_device (int * fd, char * dev_name);
void v4l2_start_capturing (int * fd, int * n_buffers );
struct buffer *v4l2_init_device (int * fd, char * dev_name, int width, \
                                    int height, int * n_buffers, int pixel_format);
int v4l2_read_frame  (int * fd, int width, int height, int * n_buffers, 
						struct buffer * buffers, int pixel_format, struct v4l2_buffer *frame);
int v4l2_release_frame(int * fd, struct v4l2_buffer *frame);


#ifdef __cplusplus
}
#endif
#endif

