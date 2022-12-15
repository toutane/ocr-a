#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

/* rerturn the width of the space between two digits 
 * x_zero is the x coordinate before space (the last x of the first digit) */
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

/* count digits */
int count_digits(Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *digit_height, int *space_width) {
	int count = 0;

	int x_zero = *left_x + *digit_width - 1;
	*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x_zero);

	int cur_w = *left_x;
	int do_space = 0;

	/* loop through image */
	for (int y = 0; y < image->height; y++) {
		cur_w = 0;
		for (int x = 0; x < image->width; x++) {
			if (x <= *right_x && x >= *left_x && y <= *top_y && y >= *bottom_y) {
				if (x == *left_x || x == *right_x || y == *bottom_y || y == *top_y)
					cur_w = 1;
				if (do_space) {
					if (cur_w == *space_width + 1) {
						cur_w = 0;
						do_space = 0;
					}
				} else if (cur_w == *digit_width) {
					cur_w = 0;
					do_space = 1;
					*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x);
					if (y == *bottom_y + 5) /* choose one y arbitrary */
						count++;
				}
			}
			if (x >= *left_x)
				cur_w++;
		}
	}
	count++;

	printf("count_digits: %i digits in the image\n", count);
	printf("\n");

	return count;
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

	/* calculate the thickness of digits */
	int thick = *digit_height / 8;
	if (thick < 3)
		thick = 3;

	int cur_w = *left_x;
	int do_space = 0;

	/* calculate the y of the middle of digits */
	int mid_y = *bottom_y + *digit_height / 2;

	/* color pixels */
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
					/* print horizontal edges */
					if (y == *bottom_y + thick || y == *top_y - thick || y == mid_y + thick / 2 || y == mid_y - thick / 2) {
						r = 255; /* orange */
						g = 153;
						b = 0;
					}
					/* print vertical edges */
					if (cur_w == thick || cur_w == *digit_width - thick) {
						g = 255;
						r = b = 0;
					}
					/* print rigth line */
					if (cur_w == *digit_width) {
						cur_w = 0;
						do_space = 1;
						r = 255;
						g = b = 0;

						/* recalcul space width */
						*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x);
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
		printf("\n");
		IMG_SavePNG(surface, path);
	}

}

/* fill the ith digit array with pixel from original image image */
void fill_digit_array(Image *image, unsigned **digits_image_array, int index, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *space_width) {
	/* calcul the width of all space before the current_digit */
	int x_inf = *left_x;

	int sp = *space_width;
	int x = *left_x;
	for (size_t i = 0; i < index; i++) {
		x += *digit_width + 1;
		sp = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x);
		x += sp;
	}
	x_inf = x;
	//printf("fill_digit_array: for index %i, x_inf: %i\n", index, x_inf);

	int x_sup = x_inf + *digit_width;

	unsigned original_value;
	unsigned *array = NULL;

	/* loop through the original image */
	for (int y = *bottom_y; y < *top_y + 1; y++) {
		for (int x = x_inf; x < x_sup; x++) {
			array = *(digits_image_array + index);
			original_value = image->pixels[y * image->width + x];
			array[(y - *bottom_y) * *digit_width + (x - x_inf)] = original_value;
		}
	}
}

/* save digit image as png */
void save_digit_image(unsigned *pixels, int *digit_width, int *digit_height, int index, char *image_path) {
	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
	   as expected by OpenGL for textures */
	SDL_Surface *surface;

	/* create surface */
	surface = SDL_CreateRGBSurface(0, *digit_width, *digit_height, 32, 0, 0, 0, 0);
	/* handle error */
	if(surface == NULL) {
		fprintf(stderr, "save_debug_image: createRGBSurface failed: %s\n", SDL_GetError());
		return;
	}

	/* set surface pixels */
	unsigned *surface_pixels = surface->pixels;
	for (int y = 0; y < *digit_height; y++) {
		for (int x = 0; x < *digit_width; x++) {
			int pixel = pixels[y * *digit_width + x];
			int r, g, b;
			r = g = b = pixel;
			surface_pixels[y * *digit_width + x] = SDL_MapRGB(surface->format, r, g, b);
		}
	}

	/* convert index int to string */
	char str[5] = {};
	sprintf(str, "%i", index);
	/* create save path */
	char path[100] = {};
	strcat(path, image_path);
	strcat(path, "/");
	strcat(path, str);
	strcat(path, ".png");
	/* save as png */
	IMG_SavePNG(surface, path);
}

/* main function */
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
	/* handle if height is too small or too big*/
	if (digit_height < 16 || digit_height > 512) {
		fprintf(stderr, "ocr: height is too small or too big\n");
		return 0;
	}

	/* get the width of the first space between two digits */
	int x_zero = left_x + digit_width - 1;
	int space_width = get_space_width(image, &digit_width, &left_x, &bottom_y, &top_y, &x_zero);

	/* count digits */
	int number = count_digits(image, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &digit_height, &space_width);

	/* allocate memory for n digits (n array of width * height) */
	unsigned **digits_image_array = (unsigned **)malloc(number * sizeof(unsigned *));
	for (size_t i = 0; i < number; i++)
		*(digits_image_array + i) = (unsigned *)malloc(digit_width * digit_height * sizeof(unsigned));

	/* copy image path for folder of save name */
	char copy_image_path[100] = {};
	strcpy(copy_image_path, image_path);
	copy_image_path[strlen(copy_image_path) - 4] = 0;

	/* create directory if not exists, to put digit images */
	struct stat st = {0};
	if (stat(copy_image_path, &st) == -1)
		mkdir(copy_image_path, 0700);
	else
		printf("%s directory already exits\n\n", copy_image_path);

	/* fill each digits image array and save it as a png
	 * the function takes the index of the digit to do */
	for (size_t i = 0; i < number; i++) {
		/* copy pixels */
		fill_digit_array(image, digits_image_array, i, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &space_width);
		/* save digit as png */
		save_digit_image(*(digits_image_array + i), &digit_width, &digit_height, i, copy_image_path);
	}

	/* save debug image in debug.png */
	save_debug_image(image_path, image, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &digit_height, &space_width);

	/* free array for each digits */
	for (size_t i = 0; i < number; i++)
		free(*(digits_image_array + i));

	/* free image digits array */
	free(digits_image_array);

	return "123";
}
