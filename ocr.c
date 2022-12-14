#include <stdio.h>

/* include ocr.h where Image is declared */
#include "ocr.h"

/* print to sdtout information of an image */
void print_image(Image *image) {
	/* declare variables */
	int width = image->width;
	int height = image->height;
	unsigned *pixels = image->pixels;

	printf("\n");
	for (size_t y = 0; y < height; y++) {
		printf("%-*zu", 4, y);
		for (size_t x = 0; x < width; x++)
			if (pixels[y * width + x] > 150)
				printf(".");
			else
				printf("x");
		printf("\n");
	}
	printf("\n");
	printf("print_image: width: %i, height: %i\n", width, height);
	printf("\n");
}

/* return width and height of the digits of an image */
void get_digit_size(Image *image, int *digit_width, int *digit_height) {
	/* get the start of a digit beginning by top left corner */
	int found_start = 0, pixel_value = 0;
	int y = 0, x = 0;
	while (!found_start && ++y < image->height) {
		x = 0;
		while (!found_start && x < image->width) {
			pixel_value = image->pixels[y * image->width + x];
			if (pixel_value < 150) { /* test if grayscale value is under 150 */
				found_start = 1;
				break;
			}
			x++;
		}
	}
	printf("get_digit_size: found the start of a digit at x: %i, y: %i\n", x, y);
	printf("\n");

	/* one we get the start, go down and see if the width increase */
	int min_y = y - 1;
	int min_x = x;
	int height = 0, width = 0;
	int tmp;
	while (y < image->height) {
		pixel_value = image->pixels[y * image->width + x];
		if (pixel_value < 150) /* update height */
			height = y - min_y;

		/* for each row, update width */
		min_x = x;
		while (x < image->width && image->pixels[y * image->width + x] < 150)
			x++;
		tmp = (x - min_x);
		x = min_x;

		/* check if there are new black pixels before min_x */
		x--;
		while (x >= 0 && image->pixels[y * image->width + x] < 150) {
			tmp++;
			x--;
		}
		x = min_x;
		if (tmp  > width)
			width = tmp;

		y++;
	}
	printf("get_digit_size: height: %i, width: %i\n", height, width);
	printf("\n");

	/* return digit_height and digit_width */
	*digit_height = height;
	*digit_width = width;
}

char *ocr(Image *image) {
	/* declare variables */
	int img_width = image->width;
	int img_height = image->height;
	unsigned *img_pixels = image->pixels;

	/* print image */
	print_image(image);

	/* try to get the width and the height of a digit
	 * each digit has the same size
	 * the height is at least 16 pixels and at most 512 pixels */
	int digit_width, digit_height;
	get_digit_size(image, &digit_width, &digit_height);

	return "123";
}
