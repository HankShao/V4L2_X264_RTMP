/*This sample program was made by:

   Aquiles Yáñez C. 
   (yanez<at>elo<dot>utfsm<dot>cl)

   Under the design guidance of:

   Agustín González V.

version 0.1 - Lanzada en Enero del 2005
version 0.2 - Lanzada en Febrero del 2005
version 0.3 - Lanzada en Octubre del 2006
version 0.4 - Translation to English  (November 2009)
              deletion of types.h and image.cpp/image.h
version 0.5 - Bug fixed, missing pointer initialization. (January 2010)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>             /* getopt_long() */
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

typedef struct 
{
	Display         *display;
	GC              gc;
	Visual          *visual;
	int             depth;
	Bool            isShared; // MITSHM
	XImage          *xImage;
	Pixmap          sharedPixmap; // None (0L) if unassigned
	XShmSegmentInfo shmInfo;

	XEvent               *event;
	Window               window;
	int	                 screenNumber;
	Atom                 atomWMDeleteWindow;
	Screen               *screen;
	Colormap             colormap;
	XWindowAttributes    windowAttributes;

} image_context;

//convert a pixel from YUV to RGB 24 (3 bytes)
static void yuv_to_rgb_24(unsigned char y, unsigned char u, unsigned char v,
			unsigned char* r, unsigned char* g, unsigned char* b)
{
	int amp=255;
	double R,G,B;
	
	//conversion equations
	B=amp*(0.004565*y+0.000001*u+0.006250*v-0.872);
	G=amp*(0.004565*y-0.001542*u-0.003183*v+0.531);
	R=amp*(0.004565*y+0.007935*u-1.088);

	//R, G and B must be in the range from 0 to 255    
	if (R < 0)
		R=0;
	if (G < 0)
		G=0;
	if (B < 0)
		B=0;
	
	if (R > 255)
		R=255;
	if (G > 255)
		G=255;
	if (B > 255)
		B=255;

	*r=(unsigned char)(R);
	*g=(unsigned char)(G);
	*b=(unsigned char)(B);

}

//convert a pixel from YUV to RGB 16 (2 bytes)
static void yuv_to_rgb_16(unsigned char y, unsigned char u, unsigned char v,
						unsigned char* rg, unsigned char* gb)
{
	double R,G,B;
	
	//conversion equations
	B=31*(0.004565*y+0.000001*u+0.006250*v-0.872);
	G=63*(0.004565*y-0.001542*u-0.003183*v+0.531);
	R=31*(0.004565*y+0.007935*u-1.088);

	//R and B must be in the range from 0 to 31, and G in the range from 0 to 63
	if (R < 0)
		R=0;
	if (G < 0)
		G=0;
	if (B < 0)
		B=0;
	
	if (R > 31)
		R=31;
	if (G > 63)
		G=63;
	if (B > 31)
		B=31;

	*rg= (((unsigned char)(R))<<3) + (((unsigned char)(G)>>3)/*&0xe0*/);
	*gb= (((unsigned char)(G)& 0x1f)<<5) +(unsigned char)(B);

}

//read the data from memory, converts that data to RGB, and call Put (shows the picture) 
static void process_image_yuv420 (uint8_t * videoFrame,  image_context image_ctx, int width, int height)
{
	XImage * xImage1 = image_ctx.xImage;
	uint8_t * imageLine1;
	int    xx,yy;
	int    x,y;
	int bpl,Bpp,amp;
	double r,g,b;
	unsigned char Y,U,V;
	unsigned char R,G,B,RG,GB;
	imageLine1 = (uint8_t *) xImage1->data;
	bpl=xImage1->bytes_per_line;
	Bpp=xImage1->bits_per_pixel/8;
	int ret;

	ret = read(STDIN_FILENO, videoFrame , width*height*12/8);
	
	switch (xImage1->depth)
	{
		case 16://process one entire frame
			for (yy = 0; yy < (height/2); yy++)
			{
				for (xx =0; xx < (width/2); xx++)
				{
					//in every loop 4 pixels are processed   
					x=2*xx;
					y=2*yy;

					V = videoFrame[(width*height*1)+(width/2*1*yy)+(1*xx)];
					U = videoFrame[(width*height*1)+(height*width*1/4)+(width/2*1*yy)+(1*xx)];

					Y = videoFrame[(width*1*y)+(1*x)];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*y)+(Bpp*x)]=GB;
					imageLine1[(bpl*y)+(Bpp*x)+1]=RG;

					Y = videoFrame[(width*1*y)+(1*(x+1))];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*y)+(Bpp*(x+1))]=GB;
					imageLine1[(bpl*y)+(Bpp*(x+1))+1]=RG;

					Y = videoFrame[(width*1*(y+1))+(1*x)];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*(y+1))+(Bpp*x)]=GB;
					imageLine1[(bpl*(y+1))+(Bpp*x)+1]=RG;

					Y = videoFrame[(width*1*(y+1))+(1*(x+1))];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*(y+1))+(Bpp*(x+1))]=GB;
					imageLine1[(bpl*(y+1))+(Bpp*(x+1))+1]=RG; 
				}
			}
			break;
			
		case 24:
			for (yy = 0; yy < (height/2); yy++)
			{
				for (xx =0; xx < (width/2); xx++)
				{
					//in every loop 4 pixels are processed
					x=2*xx;
					y=2*yy;

					V = videoFrame[(width*height*1)+(width/2*1*yy)+(1*xx)];
					U = videoFrame[(width*height*1)+(height*width*1/4)+(width/2*1*yy)+(1*xx)];

					Y = videoFrame[(width*1*y)+(1*x)];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*y)+(Bpp*x)]=B;
					imageLine1[(bpl*y)+(Bpp*x)+1]=G;
					imageLine1[(bpl*y)+(Bpp*x)+2]=R;

					Y = videoFrame[(width*1*y)+(1*(x+1))];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*y)+(Bpp*(x+1))]=B;
					imageLine1[(bpl*y)+(Bpp*(x+1))+1]=G;
					imageLine1[(bpl*y)+(Bpp*(x+1))+2]=R;

					Y = videoFrame[(width*1*(y+1))+(1*x)];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*(y+1))+(Bpp*x)]=B;
					imageLine1[(bpl*(y+1))+(Bpp*x)+1]=G;
					imageLine1[(bpl*(y+1))+(Bpp*x)+2]=R;

					Y = videoFrame[(width*1*(y+1))+(1*(x+1))];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*(y+1))+(Bpp*(x+1))]=B;
					imageLine1[(bpl*(y+1))+(Bpp*(x+1))+1]=G;
					imageLine1[(bpl*(y+1))+(Bpp*(x+1))+2]=R;
				}
			}
			break;
		default:
			fprintf(stderr,"\nError: Color depth not supported\n");
			exit(EXIT_FAILURE);
			break;
	}
	image_put(image_ctx, 0, 0, 0, 0, width, height);

	if (XPending(image_ctx.display) > 0)
		XNextEvent(image_ctx.display, image_ctx.event); //refresh the picture
}

//read the data from memory, converts that data to RGB, and call Put (shows the picture) 
static void process_image_yuv422 (uint8_t * videoFrame,  image_context image_ctx, int width, int height)
{
	XImage * xImage1 = image_ctx.xImage;
	uint8_t * imageLine1;
	int    xx,yy;
	int    x,y;
	int bpl,Bpp,amp;
	double r,g,b;
	unsigned char Y,U,V;
	unsigned char R,G,B,RG,GB;
	imageLine1 = (uint8_t *) xImage1->data;
	bpl=xImage1->bytes_per_line;
	Bpp=xImage1->bits_per_pixel/8;
	int ret;

	ret = read(STDIN_FILENO, videoFrame , width*height*2);
	
	switch (xImage1->depth)
	{
		case 16://process one entire frame
			for (yy = 0; yy < (height); yy++)
			{
				for (xx =0; xx < (width/2); xx++)
				{
					//in every loop 2 pixels are processed   
					V = videoFrame[(width*2*y)+(1*x+1)];
					U = videoFrame[(width*2*y)+(1*x+3)];

					Y = videoFrame[(width*2*y)+(1*x)];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*y)+(Bpp*x)]=GB;
					imageLine1[(bpl*y)+(Bpp*x)+1]=RG;

					Y = videoFrame[(width*2*y)+(1*(x+2))];
					yuv_to_rgb_16(Y, U, V, &RG, &GB);
					imageLine1[(bpl*y)+(Bpp*(x+1))]=GB;
					imageLine1[(bpl*y)+(Bpp*(x+1))+1]=RG;
				}
			}
			break;
			
		case 24:
			for (yy = 0; yy < (height); yy++)
			{
				for (xx =0; xx < (width/2); xx++)
				{
					x=2*xx;
					y=yy;
					
					//in every loop 2 pixels are processed   
					V = videoFrame[(width*2*y)+(2*x+1)];
					U = videoFrame[(width*2*y)+(2*x+3)];

					Y = videoFrame[(width*2*y)+(2*x)];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*y)+(Bpp*x)]=B;
					imageLine1[(bpl*y)+(Bpp*x)+1]=G;
					imageLine1[(bpl*y)+(Bpp*x)+2]=R;
					
					Y = videoFrame[(width*2*y)+(2*(x+2))];
					yuv_to_rgb_24(Y, U, V, &R, &G, &B);
					imageLine1[(bpl*y)+(Bpp*(x+1))]=B;
					imageLine1[(bpl*y)+(Bpp*(x+1))+1]=G;
					imageLine1[(bpl*y)+(Bpp*(x+1))+2]=R;
				}
			}
			break;
		default:
			fprintf(stderr,"\nError: Color depth not supported\n");
			exit(EXIT_FAILURE);
			break;
	}
	image_put(image_ctx, 0, 0, 0, 0, width, height);

	if (XPending(image_ctx.display) > 0)
		XNextEvent(image_ctx.display, image_ctx.event); //refresh the picture
}

static void process_image_rgb565 (uint8_t * videoFrame,  image_context image_ctx, int width, int height)
{
	XImage *xImage1 = image_ctx.xImage;
	uint8_t  *imageLine1;
	int    xx,yy;
	int    x,y;
	int bpl,Bpp,amp;
	double r,g,b;
	unsigned char Y,U,V;
	unsigned char R,G,B,RG,GB;
	imageLine1 = (uint8_t *) xImage1->data;
	bpl=xImage1->bytes_per_line;
	Bpp=xImage1->bits_per_pixel/8;
	int ret;

	ret = read(STDIN_FILENO, videoFrame , width*height*2);
	
	switch (xImage1->depth)
	{
		case 16:
		
			for (y = 0; y < height; y++)
			{
				for (x =0; x < width; x++)
				{
					imageLine1[(bpl*y)+(Bpp*x)+1]=videoFrame[(width*2*y)+(2*x)+1];
					imageLine1[(bpl*y)+(Bpp*x)+0]=videoFrame[(width*2*y)+(2*x)+0];
				}
			}
			break;
			
		case 24:

			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					imageLine1[(bpl*y)+(Bpp*x)]=8*(videoFrame[(width*2*y)+(2*x)+0]&0x1f);//blue
					imageLine1[(bpl*y)+(Bpp*x)+1]=4*(((videoFrame[(width*2*y)+((2*x)+0)]&0xe0)>>5)
												+((videoFrame[(width*2*y)+(2*x)+1]&0x07)<<3));//green
					imageLine1[(bpl*y)+(Bpp*x)+2]=8*((videoFrame[(width*2*y)+((2*x)+1)]&0xf8)>>3);//red
				}
			}
			
			break;
		default:
			fprintf(stderr,"\nError: Color depth not supported\n");
			exit(EXIT_FAILURE);
			break;
	}
	image_put(image_ctx, 0, 0, 0, 0, width, height);
	if (XPending(image_ctx.display) > 0)
		XNextEvent(image_ctx.display, image_ctx.event); //refresh the picture
}

static void process_image_rgb32 (uint8_t * videoFrame,  image_context image_ctx, int width, int height)
{
	XImage *xImage1 = image_ctx.xImage;
	uint8_t * imageLine1;
	int    xx,yy;
	int    x,y;
	int bpl,Bpp,amp;
	double r,g,b;
	unsigned char Y,U,V;
	unsigned char R,G,B,RG,GB;
	imageLine1 = (uint8_t *) xImage1->data;
	bpl=xImage1->bytes_per_line;
	Bpp=xImage1->bits_per_pixel/8;
	int ret;

	ret = read(STDIN_FILENO, videoFrame , width*height*4);
	
	switch (xImage1->depth)
	{
		case 16:
			for (y = 0; y < height; y++)
			{
				for (x =0; x < width; x++)
				{
					B = (unsigned char) (videoFrame[(width*4*y)+(4*x)+3] / 8);//blue
					G = (unsigned char) (videoFrame[(width*4*y)+(4*x)+2] / 4);//green
					R = (unsigned char) (videoFrame[(width*4*y)+(4*x)+1] / 8); //red

					imageLine1[(bpl*y)+(Bpp*x)+1]=(R<<3)+(G>>3);
					imageLine1[(bpl*y)+(Bpp*x)+0]=((G& 0x1f)<<5)+B;
				}
			}
			break;
			
		case 24:
			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					imageLine1[(bpl*y)+(Bpp*x)]=videoFrame[(width*4*y)+(4*x)+3];//blue
					imageLine1[(bpl*y)+(Bpp*x)+1]=videoFrame[(width*4*y)+((4*x)+2)];//green
					imageLine1[(bpl*y)+(Bpp*x)+2]=videoFrame[(width*4*y)+((4*x)+1)];//red
				}
			}
			
			break;
		default:
			fprintf(stderr,"\nError: Color depth not supported\n");
			exit(EXIT_FAILURE);
			break;
	}
	image_put(image_ctx, 0, 0, 0, 0, width, height);
	if (XPending(image_ctx.display) > 0)
		XNextEvent(image_ctx.display, image_ctx.event); //refresh the picture
}

static void usage (FILE *fp, int argc, char **argv)
{
	fprintf (fp,
				"Usage: %s [options]\n\n"
				"Options:\n"
				"-p | --pixel-format   number          Pixel Format \n"
				"                                      (0 = YUV420)\n"
				"                                      (1 = RGB565 )\n"                 
				"                                      (2 = RGB32  )\n"  
				"-w | --window-size    <640*480|       Video size\n"
				"                       320*240>\n"
				"-h | --help                           Print this message\n"
				"\n",
				argv[0]);
}

static char *ByteOrderName(int byteOrder)
{
	switch (byteOrder) 
	{
		case LSBFirst: return ("LSBFirst");
		case MSBFirst: return ("MSBFirst");
		default:       return ("?");
	} 
}

static char *VisualClassName(int visualClass)
{
	switch (visualClass) 
	{
		case StaticGray:  return ("StaticGray");
		case GrayScale:   return ("GrayScale");
		case StaticColor: return ("StaticColor");
		case PseudoColor: return ("PseudoColor");
		case TrueColor:   return ("TrueColor");
		case DirectColor: return ("DirectColor");
		default:          return ("?");
	} 
}

//needed to parse command line arguments with getopt_long
static const char short_options [] = "p:w:h";

//also needed to parse command line arguments with getopt_long
static const struct option
long_options [] = 
{
	{ "pixel-format",required_argument,      NULL,           'p' },
	{ "window-size", required_argument,      NULL,           'w' },
	{ "help",        no_argument,            NULL,           'h' },
	{ 0, 0, 0, 0 }
};

typedef enum 
{      
	PIX_FMT_YUV420P,
	PIX_FMT_RGB565,
	PIX_FMT_RGB32,
	PIX_FMT_YUYV
} pix_fmt;  

int image_create(image_context * img_ctx, int width, int height, Bool wantShared,
				Bool wantSharedPixmap)
{
	int                majorVersion;
	int                minorVersion;
	Bool               sharedPixmapsSupported;
	XGCValues          gcValues;
	ulong              gcValuesMask;
	XWindowAttributes  windowAttributes;

	if (img_ctx->xImage != NULL) 
	{
		image_destroy(img_ctx);
	}

	gcValues.function = GXcopy;
	gcValuesMask = GCFunction;
	img_ctx->gc = XCreateGC(img_ctx->display, img_ctx->window, gcValuesMask, &gcValues);

	XGetWindowAttributes(img_ctx->display, img_ctx->window, &windowAttributes);

	img_ctx->visual = windowAttributes.visual;
	img_ctx->depth = windowAttributes.depth;

	if (wantShared && XShmQueryExtension(img_ctx->display)) 
	{
		img_ctx->isShared = 1;
	} else 
	{
		img_ctx->isShared = 0;
	}

	errno = 0;
	img_ctx->xImage = NULL;
	img_ctx->sharedPixmap = None;
	if (img_ctx->isShared) 
	{
		img_ctx->shmInfo.shmid = -1;
		img_ctx->shmInfo.shmaddr = NULL;
		if ((img_ctx->xImage = XShmCreateImage(img_ctx->display, img_ctx->visual, 
			img_ctx->depth, ZPixmap, NULL, &(img_ctx->shmInfo), width, height)) == NULL) 
		{
			return -1;
		}
		if ((img_ctx->shmInfo.shmid = shmget(IPC_PRIVATE,
			img_ctx->xImage->bytes_per_line * img_ctx->xImage->height, 
			IPC_CREAT | 0777)) < 0) 
		{ // Create segment
			return -1;
		}
		if ((img_ctx->shmInfo.shmaddr = (char *) shmat(img_ctx->shmInfo.shmid, 0, 0)) < 0) 
		{  // We attach
			img_ctx->shmInfo.shmaddr = NULL;
			return -1;
		}
		img_ctx->xImage->data = img_ctx->shmInfo.shmaddr;
		img_ctx->shmInfo.readOnly = False;
		if (!XShmAttach(img_ctx->display, &(img_ctx->shmInfo))) 
		{ // X attaches
			return -1;
		}
		if (wantSharedPixmap && (XShmPixmapFormat(img_ctx->display) == ZPixmap)) 
		{
			if ((img_ctx->sharedPixmap = XShmCreatePixmap(img_ctx->display, img_ctx->window, 
				img_ctx->shmInfo.shmaddr, &(img_ctx->shmInfo), width, height, img_ctx->depth))
				== None) 
			{
				return -1;
			}
		}
	} else 
	{
		if ((img_ctx->xImage = XCreateImage(img_ctx->display, img_ctx->visual,
			img_ctx->depth, ZPixmap, 0, NULL, width, height, 16, 0)) == NULL) 
		{
			return -1;
		}

		img_ctx->xImage->data = 
			(char *) malloc(img_ctx->xImage->bytes_per_line * img_ctx->xImage->height);

		if (img_ctx->xImage->data == NULL) 
		{
			return -1;
		}
	}
	return 0;
}

int image_destroy(image_context * img_ctx)
{
	if (img_ctx->xImage == NULL) return (0); // Nothing to do

	if (img_ctx->isShared) 
	{
		if (img_ctx->shmInfo.shmid >= 0) 
		{
			XShmDetach(img_ctx->display, &(img_ctx->shmInfo)); // X detaches
			shmdt(img_ctx->shmInfo.shmaddr); // We detach
			img_ctx->shmInfo.shmaddr = NULL;
			shmctl(img_ctx->shmInfo.shmid, IPC_RMID, 0); // Destroy segment
			img_ctx->shmInfo.shmid = -1;
		}
	} else 
	{
		if (img_ctx->xImage->data != NULL) 
		{
			free(img_ctx->xImage->data);
		}
	}

	img_ctx->xImage->data = NULL;

	XDestroyImage(img_ctx->xImage);

	img_ctx->xImage = NULL;

	if (img_ctx->sharedPixmap != None) 
	{
		XFreePixmap(img_ctx->display, img_ctx->sharedPixmap);
		img_ctx->sharedPixmap = None;
	}

	if (img_ctx->display != NULL) 
	{
		XFreeGC(img_ctx->display, img_ctx->gc);
		img_ctx->display = NULL;
	}

	return 0;
}

int image_put (image_context img_ctx, int srcX, int srcY, int dstX, int dstY,
				int width, int height) {

	if (img_ctx.xImage == NULL) return (-1);

	if (width < 0) width = image_width(img_ctx);
	if (height < 0) height = image_height(img_ctx);

	if (img_ctx.isShared) 
	{
		XShmPutImage(img_ctx.display, img_ctx.window, img_ctx.gc, img_ctx.xImage,
						srcX, srcY, dstX, dstY, width, height, False);
	} else 
	{
		XPutImage(img_ctx.display, img_ctx.window, img_ctx.gc, img_ctx.xImage, 
					srcX, srcY, dstX, dstY, width, height);
	}

	return 0;
}

int image_width(image_context img_ctx)  
{
	return ((img_ctx.xImage != NULL) ? img_ctx.xImage->width : 0);
}

int image_height(image_context img_ctx) 
{
	return ((img_ctx.xImage != NULL) ? img_ctx.xImage->height : 0);
}

int main (int argc, char ** argv) 
{
	image_context        img_ctx;
	uint8_t              *videoFrame;
	
	int                 MaxImageWidth        = 640;
	int                 MaxImageHeight       = 480;
	int                 width                = 640;
	int                 height               = 480;
	int                 index;
	int                 c;
	pix_fmt             pixel_format = PIX_FMT_YUV420P;
	
	for (;;) 
	{
		c = getopt_long (argc, argv, short_options, long_options, &index);

		if (-1 == c)
			break; //no more arguments

		switch (c) 
		{
			case 0: // getopt_long() flag
				break;

			case 'p':
				pixel_format = (pix_fmt) atoi(optarg);
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
					printf("\nError: Window size not supported\n");
					exit(EXIT_FAILURE);
				}
				break;

			case 'h':
				usage (stdout, argc, argv);
				exit (EXIT_SUCCESS);

			default:
				usage (stderr, argc, argv);
				exit (EXIT_FAILURE);
		}
	}
   
//****************** configure window and display  ****************************
	//try to open the display
	if ((img_ctx.display = XOpenDisplay(NULL)) == NULL) 
	{ 
		printf("Error: fallo XOpenDisplay() \n");
		exit(1);
	}
	
	//get default display number
	img_ctx.screenNumber = DefaultScreen(img_ctx.display);
	//associate screen with the default display
	img_ctx.screen = XScreenOfDisplay(img_ctx.display, img_ctx.screenNumber);

	//create the window
	img_ctx.window = XCreateSimpleWindow(
		img_ctx.display,
		RootWindowOfScreen(img_ctx.screen),
		0, // x
		0, // y
		width, // width
		height, // height
		0,                          // border width
		BlackPixelOfScreen(img_ctx.screen), // border
		BlackPixelOfScreen(img_ctx.screen)  // background
	);

	img_ctx.xImage = NULL;//xImage is not allocated yet
	
	if (image_create(&img_ctx, MaxImageWidth, MaxImageHeight, True, False) < 0)
	{
		printf("Error: image_create() failed\n");
		exit(1);
	}
	
	XMapRaised(img_ctx.display, img_ctx.window);
	
	XStoreName(img_ctx.display, img_ctx.window, "Viewer by Aquiles Yanez C.");
	
	XGetWindowAttributes(img_ctx.display, img_ctx.window, &(img_ctx.windowAttributes));
/******************* Show info about display, window and image ************************/

	fprintf(stderr,"\nDisplay:\n");
	fprintf(stderr,"Image byte order = %s\n", ByteOrderName(ImageByteOrder(img_ctx.display)));
	fprintf(stderr,"Bitmap unit      = %i\n", BitmapUnit(img_ctx.display));
	fprintf(stderr,"Bitmap bit order = %s\n", ByteOrderName(BitmapBitOrder(img_ctx.display)));
	fprintf(stderr,"Bitmap pad       = %i\n", BitmapPad(img_ctx.display));

	fprintf(stderr,"\nWindow:\n");
	fprintf(stderr,"Depth            = %i\n", img_ctx.windowAttributes.depth);
	//fprintf(stderr,"Visual ID        = 0x%02x\n", img_ctx.windowAttributes.visual->visualid);
	//fprintf(stderr,"Visual class     = %s\n", 
	//				VisualClassName(img_ctx.windowAttributes.visual->c_class));
	fprintf(stderr,"Red mask         = 0x%08lx\n", img_ctx.windowAttributes.visual->red_mask);
	fprintf(stderr,"Green mask       = 0x%08lx\n", img_ctx.windowAttributes.visual->green_mask);
	fprintf(stderr,"Blue mask        = 0x%08lx\n", img_ctx.windowAttributes.visual->blue_mask);
	fprintf(stderr,"Bits per R/G/B   = %i\n", img_ctx.windowAttributes.visual->bits_per_rgb);

	fprintf(stderr,"Image byte order = %s\n", ByteOrderName((img_ctx.xImage)->byte_order));
	fprintf(stderr,"Bitmap unit      = %i\n", img_ctx.xImage->bitmap_unit);
	fprintf(stderr,"Bitmap bit order = %s\n", ByteOrderName(img_ctx.xImage->bitmap_bit_order));
	fprintf(stderr,"Bitmap pad       = %i\n", img_ctx.xImage->bitmap_pad);
	fprintf(stderr,"Depth            = %i\n", img_ctx.xImage->depth);
	fprintf(stderr,"Red mask         = 0x%08lx\n", img_ctx.xImage->red_mask);
	fprintf(stderr,"Green mask       = 0x%08lx\n", img_ctx.xImage->green_mask);
	fprintf(stderr,"Blue mask        = 0x%08lx\n", img_ctx.xImage->blue_mask);
	fprintf(stderr,"Bits per pixel   = %i\n", img_ctx.xImage->bits_per_pixel);
	fprintf(stderr,"Bytes per line   = %i\n", img_ctx.xImage->bytes_per_line);
	fprintf(stderr,"IsShared         = %s\n", img_ctx.isShared ? "True" : "False");
	//fprintf(stderr,"HasSharedPixmap  = %s\n", img_ctx.HasSharedPixmap() ? "True" : "False");    
	
/***************************************************************************/
    videoFrame = (uint8_t*) malloc (width*height*4);
    
    while(1)
    {
        switch(pixel_format) 
		{
			case PIX_FMT_YUV420P:
				process_image_yuv420 (videoFrame, img_ctx, width, height);
			break;
			case PIX_FMT_RGB565:
				process_image_rgb565 (videoFrame, img_ctx, width, height);
			break;
			case PIX_FMT_RGB32:
				process_image_rgb32 (videoFrame, img_ctx, width, height);
			break;
			case PIX_FMT_YUYV:
			    process_image_yuv422 (videoFrame, img_ctx, width, height);
			break;
			default:
				printf("\n\nError: Pixel format NO supported\n\n");
				usage (stderr, argc, argv);
				exit (EXIT_FAILURE);
	    }
    }
    exit (1);
}
