#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* include ocr.h where Image and get_space_with are declared */
#include "ocr.h"

/* save debug image in a png file */
void save_debug_image(int colored, char *image_path, Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *digit_height, int *space_width) {
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

	//int x_zero = *left_x + *digit_width - 1;
	int x_zero = *left_x + *digit_width + 1;
	//int x_zero = *left_x;
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
			/* print colored lines only if colored arg is to 1 */
			if (colored == 1) {
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
							int p = x;
							p++;
							*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &p); /* TODO: DEBUG THAT LINE */
							//printf("sp: %i\n", *space_width);
						}
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

/* save digit image as png */
void save_digit_image(int colored, unsigned *pixels, int *digit_width, int *digit_height, int index, char *image_path) {
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

	/* calculate the thickness of digits to print debug lines (orange) */
	int thick = *digit_height / 8;
	if (thick < 3)
		thick = 3;

	/* calculate the y of the middle of digits */
	int mid_y = *digit_height / 2;

	/* set surface pixels */
	unsigned *surface_pixels = surface->pixels;
	for (int y = 0; y < *digit_height; y++) {
		for (int x = 0; x < *digit_width; x++) {
			int pixel = pixels[y * *digit_width + x];
			int r, g, b;
			r = g = b = pixel;
			/* print colored lines only if colored arg is equal to 1 */
			if (colored == 1) {
				/* draw horizontal delimitation zone lines */
				if (y == thick || y == *digit_height - thick || y == mid_y + thick / 2 || y == mid_y - thick / 2) {
					r = 255; /* orange */
					g = 153;
					b = 0;
				}
				/* draw vertical delimitation zone lines */
				if (x == thick || x == *digit_width - thick) {
					g = 255; /* green */
					r = b = 0;
				}
			}
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
