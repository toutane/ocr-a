#include <stdio.h>

/* include ocr.h where Image is declared */
#include "ocr.h"

/* return left, right x and top, bottom y */
void get_corners_coordinates(Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y) {
	/* init left_x and bottom_y */
	int found_first = 0;
	int y = 0, x = 0;
	while (!found_first && ++y < image->height) {
		x = 0;
		while (!found_first && ++x < image->width) {
			if (image->pixels[y * image->width + x] < 150) {
				*left_x = x;
				*bottom_y = y;
				found_first = 1;
				break;
			}
		}
	}

	/* update coordinates */
	for (size_t y = 0; y < image->height; y++) {
		for (size_t x = 0; x < image->width; x++) {
			if (image->pixels[y * image->width + x] < 150) {
				if (x < *left_x)
					*left_x = x;
				if (x > *right_x)
					*right_x = x;
				if (y < *bottom_y)
					*bottom_y = y;
				if (y > *top_y)
					*top_y = y;
			}
		}
	}
	printf("get_corners_coordinates: left_x: %i, right_x: %i, bottom_y, %i, top_y %i\n", *left_x, *right_x, *bottom_y, *top_y);
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

/* rerturn the width of the space between two digits */
int get_space_width(Image *image, int *digit_width, int *left_x, int *bottom_y) {
	int x = *left_x + *digit_width - 1, y = *bottom_y + 2; /* precaution */
	int space_width = 0;

	while (x++ < image->width && image->pixels[y * image->width + x] > 150)
		space_width++;

	return space_width;
}

/* print to sdtout information of an image */
void print_debug_image(Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *digit_height, int *space_width) {
	/* declare variables */
	int width = image->width;
	int height = image->height;
	unsigned *pixels = image->pixels;
	int cur_w = *left_x;
	int do_space = 0;

	printf("digit width: %i\n", *digit_width);
	printf("space width: %i\n", *space_width);
	printf("\n");
	for (size_t y = 0; y < height; y++) {
		printf("%-*zu", 4, y);
		cur_w = 0;
		for (size_t x = 0; x < width; x++) {
			if (x == *left_x || x == *right_x || y == *bottom_y || y == *top_y) {
				cur_w = 1;
				printf("\033[0;31m");
			}
			if (do_space) {
				if (cur_w == *space_width + 1) {
					cur_w = 0;
					do_space = 0;
					printf("\033[0;31m");
				}
			} else {
				if (cur_w == *digit_width) {
					cur_w = 0;
					do_space = 1;
					printf("\033[0;31m");
				}
			}

			if (pixels[y * width + x] > 150)
				printf(".");
			else
				printf("x");
			if (x >= *left_x)
				cur_w++;
			printf("\033[0m");
		}	
		printf("\n");
	}
	printf("\n");
	printf("print_image: width: %i, height: %i\n", width, height);
	printf("\n");
}

char *ocr(Image *image) {
	/* declare variables */
	int img_width = image->width;
	int img_height = image->height;
	unsigned *img_pixels = image->pixels;

	/* get coordinates of each corners */
	int left_x = 0, right_x = 0, top_y = 0, bottom_y = 0;
	get_corners_coordinates(image, &left_x, &right_x, &top_y, &bottom_y);

	/* get the width and the height of a digit
	 * each digit has the same size
	 * the height is at least 16 pixels and at most 512 pixels */
	int digit_width, digit_height;
	get_digit_size(image, &digit_width, &digit_height);

	int space_width = get_space_width(image, &digit_width, &left_x, &bottom_y);

	/* print debug image to sdtout */
	print_debug_image(image, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &digit_height, &space_width);

	/* handle if height is too small or too big*/
	if (digit_height < 16 || digit_height > 512) {
		fprintf(stderr, "ocr: height is too small or too big\n");
		return 0;
	}


	return "123";
}
