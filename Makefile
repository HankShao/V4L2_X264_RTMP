all: capturer_mmap capturer_read viewer

capturer_mmap: capturer_mmap.c
	gcc -O2  -o capturer_mmap capturer_mmap.c

capturer_read: capturer_read.c
	gcc -O2  -o capturer_read capturer_read.c
	
viewer: viewer.c
	gcc -O2  -L/usr/X11R6/lib -lX11 -lXext -o viewer viewer.c

clean:
	rm -f capturer_mmap
	rm -f capturer_read
	rm -f viewer
