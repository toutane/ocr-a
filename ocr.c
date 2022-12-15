#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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
			if (image->pixels[y * image->width + x] < 100) {
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
			if (image->pixels[y * image->width + x] < 100) {
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
			if (pixel_value < 100) { /* test if grayscale value is under 100 */
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
		if (pixel_value < 100) /* update height */
			height = y - min_y;

		/* for each row, update width */
		min_x = x;
		while (x < image->width && image->pixels[y * image->width + x] < 100)
			x++;
		tmp = (x - min_x);
		x = min_x;

		/* check if there are new black pixels before min_x */
		x--;
		while (x >= 0 && image->pixels[y * image->width + x] < 100) {
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
int get_space_width(Image *image, int *digit_width, int *left_x, int *bottom_y, int *top_y, int *x_zero) {
	//int x = *left_x + *digit_width - 1;
	int x = *x_zero;
	int y = *top_y; /* precaution */
	int space_width = 0;
	while (x++ < image->width && image->pixels[y * image->width + x] > 100) {
		space_width++;
	}

	int tmp = 0;

	/* return the minimum width between two digits on the all height */
	for (size_t y = *bottom_y; y < *top_y; y++) {
		tmp = 0;
		//x = *left_x + *digit_width - 1;
		x = *x_zero;
		while (x++ < image->width && image->pixels[y * image->width + x] > 100)
			tmp++;
		if (tmp < space_width)
			space_width = tmp;
	}

	return space_width;
}

/* save debug image in a png file */
void save_debug_image(char *image_path, Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *digit_height, int *space_width) {
	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
	   as expected by OpenGL for textures */
	SDL_Surface *surface;

	/* create surface */
	surface = SDL_CreateRGBSurface(0, image->width, image->height, 32, 0, 0, 0, 0);
	/* handle error */
	if(surface == NULL) {
		fprintf(stderr, "save_debug_image: createRGBSurface failed: %s\n", SDL_GetError());
		return;
	}

	int x_zero = *left_x + *digit_width - 1;
	*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x_zero);

	Uint32 *pixels = surface->pixels;
	int cur_w = *left_x;
	int do_space = 0;

	/* fill pixels */
	for (int y = 0; y < image->height; y++) {
		cur_w = 0;
		for (int x = 0; x < image->width; x++) {
			int pixel = image->pixels[y * image->width + x];
			int r, g, b;
			r = g = b = pixel;
			/* print lines only if in the frame*/
			if (x <= *right_x && x >= *left_x && y <= *top_y && y >= *bottom_y) {
				/* print border lines */
				if (x == *left_x || x == *right_x || y == *bottom_y || y == *top_y) {
					cur_w = 1;
					b = 255;
					g = r = 0;
				}
				if (do_space) {
					/* print left line */
					if (cur_w == *space_width + 1) {
						cur_w = 0;
						do_space = 0;
						r = 255;
						g = b = 0;
					}
				} else {
					/* print rigth line */
					if (cur_w == *digit_width) {
						cur_w = 0;
						do_space = 1;
						r = 255;
						g = b = 0;

						/* recalculate space width */
						*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x);
						//*space_width = 3;
						/* print only one line */
						//if (y == *bottom_y + 2)
						//printf("space width: %i\n", *space_width);
					}
				}
			}
			pixels[y * image->width + x] = SDL_MapRGB(surface->format, r, g, b);
			if (x >= *left_x)
				cur_w++;
		}
	}

	/* save image as png */
	if (strcmp(image_path, "test") == 0)
		IMG_SavePNG(surface, "./debug.png");
	else {
		char path[100] = {} ;
		image_path[strlen(image_path) - 4] = 0;
		strcat(path, image_path);
		strcat(path, "_debug.png");
		printf("save_debug_image: save to %s\n", path);
		IMG_SavePNG(surface, path);
	}

}

char *ocr(Image *image, char *image_path) {
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

	int x_zero = left_x + digit_width - 1;
	int space_width = get_space_width(image, &digit_width, &left_x, &bottom_y, &top_y, &x_zero);

	/* save debug image in debug.png */
	save_debug_image(image_path, image, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &digit_height, &space_width);

	/* handle if height is too small or too big*/
	if (digit_height < 16 || digit_height > 512) {
		fprintf(stderr, "ocr: height is too small or too big\n");
		return 0;
	}


	return "123";
}
