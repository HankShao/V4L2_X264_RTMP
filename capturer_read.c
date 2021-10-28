/*This sample program was made by:

   Aquiles Yáñez C. 
   (yanez<at>elo<dot>utfsm<dot>cl)

   Under the design guidance of:

   Agustín González V.

version 0.1 - Lanzada en Enero del 2005
version 0.2 - Lanzada en Febrero del 2005
version 0.3 - Lanzada en Octubre del 2006
version 0.4 - The same of 0.3 but in English (November 2009)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define MAX_INPUT   16
#define MAX_NORM    16

//info needed to store one video frame in memory
struct buffer {    
	void *                  start;
	size_t                  length;
};

static void errno_exit (const char *s)
{
	fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
	exit (EXIT_FAILURE);
}

//a blocking wrapper of the ioctl function
static int xioctl (int fd, int request, void *arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

//read one frame from memory and throws the data to standard output
static int read_frame  (int * fd, int width, int height, int * n_buffers,
						struct buffer * buffers, int pixel_format)
{
	unsigned int Bpf; //bytes per frame
	unsigned int i;

	if (-1 == read (*fd, buffers[0].start, buffers[0].length)) 
	{
		switch (errno) 
		{
			case EAGAIN:
				return 0;

			case EIO:
								//EIO ignored
			default:
				errno_exit ("read");
		}
	}

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
	}

	int ret;
	//writing to standard output
	ret = write(STDOUT_FILENO, buffers[0].start, Bpf);
	return 1;
}

//just the main loop of this program 
static void mainloop (int * fd, int width, int height, int * n_buffers, 
					struct buffer * buffers, int pixel_format)
{
	unsigned int count;

	count = 100;
	for (;;) 
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO (&fds);
		FD_SET (*fd, &fds);

		/* needed for select timeout */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		//the classic select function, who allows to wait up to 2 seconds, until we have captured data,
		r = select (*fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) 
		{//error
			if (EINTR == errno)
				continue;
			errno_exit ("select");
		}

		if (0 == r) 
		{
			fprintf (stderr, "select timeout\n");
			exit (EXIT_FAILURE);
		}

		//read one frame from the device and put on the buffer
		read_frame (fd, width, height, n_buffers, buffers, pixel_format);
					
	}        
}

//dummy function, that represents the stop of capturing 
static void stop_capturing (int * fd)
{
	enum v4l2_buf_type type;
	// Nothing to do. 

}

//dummy function, that represents the start of capturing 
static void start_capturing (int * fd, int * n_buffers )
{
	unsigned int i;
	enum v4l2_buf_type type;
	// Nothing to do. 

}

//free the buffers
static void uninit_device (int * n_buffers, struct buffer * buffers)
{
	unsigned int i;

	free (buffers[0].start);
	free (buffers);
}

//allocate memory for buffers, the buffer must have capacity for one video frame. 
static struct buffer *init_read (unsigned int buffer_size)
{
	struct buffer *buffers = NULL;
	buffers = calloc (1, sizeof (*buffers));

	if (!buffers) 
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	buffers[0].length = buffer_size;
	buffers[0].start = malloc (buffer_size);

	if (!buffers[0].start) 
	{
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	return buffers;
}

//configure and initialize the hardware device 
static struct buffer *init_device (int * fd, char * dev_name, int width,
								int height, int * n_buffers, int pixel_format)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct buffer * buffers = NULL;
	unsigned int min;

	if (-1 == xioctl (*fd, VIDIOC_QUERYCAP, &cap)) 
	{
		if (EINVAL == errno) 
		{
			fprintf (stderr, "%s is no V4L2 device\n",dev_name);
			exit (EXIT_FAILURE);
		} else 
		{
			errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
		fprintf (stderr, "%s is not a video capture device\n",dev_name);
		exit (EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_READWRITE)) 
	{
		fprintf (stderr, "%s does not support read i/o\n",dev_name);
		exit (EXIT_FAILURE);
	}
	//select video input, standard(not used) and tuner(not used) here
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (*fd, VIDIOC_CROPCAP, &cropcap)) 
	{
				/* Errors ignored. */
	}
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; /* reset to default */

	if (-1 == xioctl (*fd, VIDIOC_S_CROP, &crop)) 
	{
		switch (errno) 
		{
			case EINVAL:
				/* Cropping not supported. */
				break;
			default:
				/* Errors ignored. */
				break;
		}
	}
	CLEAR (fmt);
	//set image properties
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width; 
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	//fmt.fmt.pix.colorspace  = V4L2_COLORSPACE_SRGB;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	switch (pixel_format) 
	{
		case 0:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
			break;
		case 1:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
			break;
		case 2:
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
			break;
	}
	if (-1 == xioctl (*fd, VIDIOC_S_FMT, &fmt))
		errno_exit ("VIDIOC_S_FMT");

	/* Note VIDIOC_S_FMT may change width and height. */

	//check the configuration data
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	fprintf(stderr,"Video bytespreline = %d\n",fmt.fmt.pix.bytesperline);
	fprintf(stderr,"Using READ IO Method\n");
	buffers=init_read (fmt.fmt.pix.sizeimage);

	return buffers;
}

static void close_device (int * fd)
{
	if (-1 == close (*fd))
		errno_exit ("close");

	*fd = -1;
}

static void open_device (int * fd, char * dev_name)
{
	struct stat st; 

	if (-1 == stat (dev_name, &st)) 
	{
		fprintf (stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (!S_ISCHR (st.st_mode)) 
	{
		fprintf (stderr, "%s is no device\n", dev_name);
		exit (EXIT_FAILURE);
	}

	*fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == *fd) 
	{
		fprintf (stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
}

//show the usage
static void usage (FILE *fp, int argc, char **argv)
{
	fprintf (fp,
				"Usage: %s [options]\n\n"
				"Options:\n"
				"-D | --device       name               Select device name [/dev/video0]\n"
				"-d | --device-info  name               Show device info\n"
				"-i | --input        number             Video input number \n"
				"-s | --standard     number             Video standard \n"
				"-w | --window-size  <640*480|          Video size\n"
				"                      320*240>\n"
				"-p | --pixel-format number             Pixel Format (0 = YUV420)\n"
				"                                                    (1 = RGB565)\n"
				"                                                    (2 = RGB32 )\n"
				"-h | --help                            Print this message\n"
				"\n",
				argv[0]);
}

//used by getopt_long to know the possible inputs
static const char short_options [] = "D:d:i:s:w:p:h";

//long version of the previous function
static const struct option
long_options [] = 
{
		{ "device",      required_argument,      NULL,           'D' },
		{ "device-info", required_argument,      NULL,           'd' },	
		{ "input",       required_argument,      NULL,           'i' },
		{ "standard",    required_argument,      NULL,           's' },
		{ "window-size", required_argument,      NULL,           'w' },
		{ "pixel-format",required_argument,      NULL,           'p' },
		{ "help",        no_argument,            NULL,           'h' },
		{ 0, 0, 0, 0 }
};

//show all the available devices
static void enum_inputs (int * fd)
{
	int  ninputs;
	struct v4l2_input  inp[MAX_INPUT];
	printf("Available Inputs:\n");
	for (ninputs = 0; ninputs < MAX_INPUT; ninputs++) 
	{
		inp[ninputs].index = ninputs;
		if (-1 == ioctl(*fd, VIDIOC_ENUMINPUT, &inp[ninputs]))
			break;
		printf("number = %d      description = %s\n",ninputs,inp[ninputs].name); 
	}
}

//show the available standards(norms) for capture 
static void enum_standards (int * fd )
{
	struct v4l2_standard  std[MAX_NORM];
	int  nstds;
	printf("Available Standards:\n");
	for (nstds = 0; nstds < MAX_NORM; nstds++) 
	{
		std[nstds].index = nstds;
		if (-1 == ioctl(*fd, VIDIOC_ENUMSTD, &std[nstds]))
			break;
		printf("number = %d     name = %s\n",nstds,std[nstds].name);
	}
}

//configure the video input
static void set_input(int * fd, int dev_input)
{
	struct v4l2_input input;
	//set the input
	int index = dev_input;
	if (-1 == ioctl (*fd, VIDIOC_S_INPUT, &index)) 
	{
		perror ("VIDIOC_S_INPUT");
		exit (EXIT_FAILURE);
	}
	//check the input
	if (-1 == ioctl (*fd, VIDIOC_G_INPUT, &index)) 
	{
		perror ("VIDIOC_G_INPUT");
		exit (EXIT_FAILURE);
	}
	memset (&input, 0, sizeof (input));
	input.index = index;
	if (-1 == ioctl (*fd, VIDIOC_ENUMINPUT, &input)) 
	{
		perror ("VIDIOC_ENUMINPUT");
		exit (EXIT_FAILURE);
	}
	fprintf (stderr,"input: %s\n", input.name); 
}

//configure the capture standard
static void set_standard(int * fd, int dev_standard)
{
	struct v4l2_standard standard;
	v4l2_std_id st;
	standard.index = dev_standard;;
	if (-1 == ioctl (*fd, VIDIOC_ENUMSTD, &standard)) 
	{
		perror ("VIDIOC_ENUMSTD");
	}
	st=standard.id;
	
	if (-1 == ioctl (*fd, VIDIOC_S_STD, &st)) 
	{
		perror ("VIDIOC_S_STD");
	}
	fprintf (stderr,"standard: %s\n", standard.name); 
}

//enum for pixel format
typedef enum 
{      
	PIX_FMT_YUV420P,
	PIX_FMT_RGB565,
	PIX_FMT_RGB32
} pix_fmt;

int main (int argc, char ** argv)
{

	int                 dev_standard; 
	int                 dev_input;
	int                 set_inp              = 0;
	int                 set_std              = 0;
	char                *dev_name            = "/dev/video0";
	int                 fd                   = -1;
	int                 width                = 640;
	int                 height               = 480;
	int                 n_buffers;
	int                 pixel_format         = 0;
	struct buffer       *buffers             = NULL;

	//process all the command line arguments
	for (;;) 
	{	
		int index;
		int c;
				
		c = getopt_long (argc, argv,short_options, long_options,&index);

		if (-1 == c)
			break; //no more arguments (quit from for)

		switch (c) 
		{
			case 0: // getopt_long() flag
				break;

			case 'D':
				dev_name = optarg;
				break;
					
			case 'd':
				dev_name = optarg;
				open_device (&fd,dev_name);
				printf("\n");
				printf("Device info: %s\n\n",dev_name);
				enum_inputs(&fd);
				printf("\n");
				enum_standards(&fd);
				printf("\n");
				close_device (&fd);
				exit (EXIT_SUCCESS);
				//break;
				
			case 'i':  
				dev_input = atoi(optarg);              
				set_inp=1;
				break;

			case 's':
				dev_standard = atoi(optarg);
				set_std=1;
				break;

			case 'w':
				if (strcmp(optarg,"640*480")==0)
				{
					printf("window size 640*480\n");
					width=640;
					height=480;
				}
				if (strcmp(optarg,"320*240")==0)
				{
					printf("window size 320*240\n");
					width=320;
					height=240;
				}
				if ((strcmp(optarg,"320*240")!=0)&&(strcmp(optarg,"640*480")!=0))
				{
					printf("\nError: window size not supported\n");
					exit(EXIT_FAILURE);
				}                
				break;
				
			case 'p':
				pixel_format=atoi(optarg);
				break;

			case 'h':
				usage (stdout, argc, argv);
				exit (EXIT_SUCCESS);

			default:
				usage (stderr, argc, argv);
				exit (EXIT_FAILURE);
		}
	}
	
	open_device (&fd, dev_name);
	
	//set the input if needed
	if (set_inp==1)
		set_input(&fd, dev_input);
	
	//set the standard if needed
	if (set_std==1)
		set_standard(&fd, dev_standard);

	buffers = init_device (&fd, dev_name, width, height, &n_buffers, pixel_format);

	start_capturing (&fd, &n_buffers);

	mainloop (&fd, width, height, &n_buffers, buffers, pixel_format);

	//TODO: main loop never exits, a break method must be implemented to execute 
	//the following code

	stop_capturing (&fd);

	uninit_device (&n_buffers, buffers);

	close_device (&fd);

	exit (EXIT_SUCCESS);

}
