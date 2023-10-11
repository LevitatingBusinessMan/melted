#include <linux/fb.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int fbfd;
	if ((fbfd = open("/dev/fb0", O_RDWR)) == -1) {
		perror("Error opening framebuffer device");
	}
	printf("Opened as descripter %d\n", fbfd);

	struct fb_var_screeninfo v_info;

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &v_info) == -1) {
		perror("Error reading fixed screen info");
		exit(1);
	}

	printf("Resolution %dx%d, %d bpp\n", v_info.xres, v_info.yres, v_info.bits_per_pixel);

	char bytes_per_pixel = v_info.bits_per_pixel / 8;
	size_t map_size = v_info.xres * v_info.yres * bytes_per_pixel;

	// Memory mapped framebuffer
	unsigned char* fb_map = mmap(
		0,
		map_size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fbfd,
		0
	);

	printf("Mmap at 0x%x\n", fb_map);

	const int rowsize = v_info.xres * bytes_per_pixel;
	printf("mapsize: %d\n", map_size);

	// Move the pixels down in a melting pattern
	for (int i=0;;i++) {
		for (short col = 0; col < rowsize; col += bytes_per_pixel) {
			int depth = i * ((i + col - col % 5) % 3);
			for (short row = v_info.yres - depth -1; row >= 0; row --) {
				int offset = row * rowsize + col;
				for (int b=0; b < bytes_per_pixel; b++) {
					*(fb_map + offset + rowsize * depth + b) = *(fb_map + offset + b);
				}
			}
		}
		usleep(200000);
	}

	// Decent melted effect
	// for (short col = 0; col < rowsize; col += bytes_per_pixel) {
	// 	int depth = (col - col % 5) % 4;
	// 	for (short row = v_info.yres - depth - 1; row >= 0; row --) {
	// 		int offset = row * rowsize + col;
	// 		for (int i=0; i < bytes_per_pixel; i++) {
	// 			*(fb_map + offset + rowsize * depth + i) = *(fb_map + offset + i);
	// 		}
	// 	}
	// }

	// Dissect colors
	// for (short col = 0; col < rowsize; col++) {
	// 	int depth = col % 4 * 5;
	// 	for (short row = v_info.yres - depth - 1; row >= 0; row --) {
	// 		int offset = row * rowsize + col;
	// 		*(fb_map + offset + rowsize * depth) = *(fb_map + offset);
	// 	}
	// }

	// Shift everything 200 pixels down
	// for (size_t offset = map_size - rowsize*200 -1; offset > 0; offset--) {
	// 	*(fb_map + offset + rowsize*200) = *(fb_map + offset);
	// }

	munmap(fb_map, map_size);
	close(fbfd);
	return 0;
}


