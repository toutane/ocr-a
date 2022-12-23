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

/* include save_debug_image and save_digit_image */
#include "utils.h"

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

/* return the width of the space between two digits 
 * x_zero is the x coordinate before space (the last x of the first digit) */
int get_space_width(Image *image, int *digit_width, int *left_x, int *bottom_y, int *top_y, int *x_zero) {
	//int x = *left_x + *digit_width - 1;
	int x = *x_zero - 1;
	int y = *top_y; /* precaution */
	int space_width = 0;
	while (x < image->width && image->pixels[y * image->width + x] > 100) {
		space_width++;
		x++;
	}

	int tmp = 0;

	/* return the minimum width between two digits on the all height */
	for (size_t y = *bottom_y; y < *top_y; y++) {
		tmp = 0;
		//x = *left_x + *digit_width - 1;
		//x = *x_zero;
		x = *x_zero - 1;
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

	//int x_zero = *left_x + *digit_width - 1;
	int x_zero = *left_x;
	//printf("count_digits: x_zero: %i\n", x_zero);
	*space_width = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x_zero);
	//printf("count_digits: space_width: %i\n", *space_width);

	int cur_w = *left_x;
	int do_space = 0;

	/* loop through image */
	/* TODO: REDO THIS LOOP */
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
					if (y == *bottom_y + 5)
						count++;
				}
			}
			if (x >= *left_x)
				cur_w++;
		}
	}
	//count++;

	printf("count_digits: %i digits in the image\n", count);
	printf("\n");

	return count;
}


/* fill the ith digit array with pixel from original image image */
void fill_digit_array(Image *image, unsigned **digits_image_array, int index, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *space_width) {
	/* calcul the width of all space before the current_digit */
	int x_inf = *left_x;

	int sp = *space_width;
	int x = *left_x;
	for (size_t i = 0; i < index; i++) {
		x += *digit_width + 1;
		//x += *digit_width;
		sp = get_space_width(image, digit_width, left_x, bottom_y, top_y, &x);
		x += sp;
	}
	x_inf = x;
	//printf("left_x: %i\n", *left_x);
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

/* deduct number from a array of pixels representing one digit and return it
 * based on the percentage of black pixels in each zones */
int deduct_number(unsigned *pixels, int *digit_width, int *digit_height) {
	/* calculate the thickness of digits */
	int thick = *digit_height / 8;
	if (thick < 3)
		thick = 3;

	/* calculate the y of the middle of digits */
	int mid_y = *digit_height / 2;

	/* declare zone array and initialize it to 0
	 * if zone is filled, 1 at the index zone, 0 else 
	 * corners are not take in count */
	int zones[7] = {};
	int zones_pixels[7] = {};
//	printf("thick: %i, digit_width: %i, digit_height: %i, mid_y: %i\n", thick, *digit_width, *digit_height, mid_y);

	int value;
	//	/* TODO: refactore with coherant <, > or <=, >= */
	/* loop through pixels array and fill zones_pixels */
	for (int y = 0; y < *digit_height; y++)
		for (int x = 0; x < *digit_width; x++) {
			value = pixels[y * *digit_width + x];
			if (value > 100)
				value = 1;
			else
				value = 0;
			//if (y <= thick && x >= thick && x <= *digit_width - thick) /* in zone 1 */
			if (y < thick && x >= thick && x <= *digit_width - thick) /* in zone 1 */
				zones_pixels[0] += value;
			else if (y > thick && y <= mid_y - thick / 2 && x >= *digit_width - thick) /* in zone 2 */
				zones_pixels[1] += value;
			//else if (y > mid_y + thick / 2 && y < *digit_height - thick && x >= *digit_width - thick) /* in zone 3 */
			else if (y > mid_y + thick / 2 && y < *digit_height - thick && x > *digit_width - thick) /* in zone 3 */
				zones_pixels[2] += value;
			else if (y > *digit_height - thick && x >= thick && x <= *digit_width - thick) /* in zone 4 */
				zones_pixels[3] += value;
			//else if (y > mid_y + thick / 2 && y < *digit_height - thick && x <= thick) /* in zone 5 */
			else if (y > mid_y + thick / 2 && y < *digit_height - thick && x < (thick - 1)) /* in zone 5 */
				zones_pixels[4] += value;
			else if (y > mid_y - thick / 2 && y <= mid_y + thick / 2 && x > thick && x <= *digit_width - thick) /* in zone 6 */
				zones_pixels[5] += value;
			else if (y > thick && y <= mid_y - thick / 2 && x < (thick - 1)) /* in zone 7 */
				zones_pixels[6] += value;
		}

	int average = 0;
	/* calculate average zone pixels value */
	for (size_t i = 0; i < 7; i++) {
		average += zones_pixels[i];
		printf("deduct_number: zone: %zu, value: %i\n", i + 1, zones_pixels[i]);
	}
	average = average / 7;
	printf("deduct_number: average: %i\n", average);
	printf("\n");

	/* for each zones, determines if is under average. If so, put zones array to 1 */
	for (size_t i = 0; i < 7; i++)
		if (zones_pixels[i] <= average) {
			printf("deduct_number: zone: %zu is filled\n", i + 1);
			zones[i] = 1;
		}

	int res = -1;
	/* according to the actived zones, deduct which digit it is */
	if (zones[0] && zones[1] && zones[2] && zones[3] && zones[4] && zones[6]) /* 0 */
		res = 0;
	else if (zones[0] && zones[1] && zones[3] && zones[4] && zones[5]) /* 2 */
		res = 2;
	else if (zones[0] && zones[1] && zones[2] && zones[3] && zones[5]) /* 3 */
		res = 3;
	else if (zones[2] && zones[5] && zones[6]) /* 4 */
		res = 4;
	else if (zones[2] && zones[3] && zones[4] && zones[6]) /* 6 */
		res = 6;
	else if (zones[0] && zones[2] && zones[3] && zones[4] && zones[5]) /* 8 */
		res = 8;
	else if (zones[0] && zones[2] && zones[3] && zones[5]) /* 5 */
		res = 5;
	else if (zones[0] && zones[1] && zones[2] && zones[6]) /* 9 */
		res = 9;
	else if (zones[0] && zones[2] && zones[3]) /* 1 */
		res = 1;
	else if ((zones[0] && zones[1] && zones[5]) || (zones[0] && zones[1])) /* 7 */
		res = 7;
	else /* not recognized */
		res = -1;

	return res;
}

/* main function */
void ocr(Image *image, char *image_path) {
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
		return;
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

	/* fill each digits image array 
	 * the function takes the index of the digit to do */
	for (size_t i = 0; i < number; i++)
		fill_digit_array(image, digits_image_array, i, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &space_width);


	char image_folder_path[100] = {};
	/* if not in debug mode, save each image in the image/image_path folder */
	if (strcmp(image_path, "test") != 0) {
		/* copy image path for folder of save name */
		strcpy(image_folder_path, image_path);
		image_folder_path[strlen(image_folder_path) - 4] = 0;
	}
	else {
		image_folder_path[0] = 'd';
		image_folder_path[1] = 'e';
		image_folder_path[2] = 'b';
		image_folder_path[3] = 'u';
		image_folder_path[4] = 'g';
		image_folder_path[5] = 0;
	}

	/* create directory if not exists, to put digit images */
	struct stat st = {0};
	if (stat(image_folder_path, &st) == -1)
		mkdir(image_folder_path, 0700);
	else
		printf("%s directory already exits\n\n", image_folder_path);

	/* declare colored variable: is equal to 1 print colored lines on debug images */
	int colored = 1;

	/* fill each digits image array and save it as a png
	 * the function takes the index of the digit to do */
	for (size_t i = 0; i < number; i++)
		save_digit_image(colored, *(digits_image_array + i), &digit_width, &digit_height, i, image_folder_path);

	/* declare and set to 0 string of digits which is the return value */
	char res[100] = {};

	/* for each digits of the original image
	 * count zones filled with black pixel 
	 * and deduce from that the actual number */
	int digit = -1;
	for (size_t i = 0; i < number; i++) {
		printf("deduct digit number: %zu\n", i);
		digit = deduct_number(*(digits_image_array + i), &digit_width, &digit_height);
		printf("\n");
		char str[2] = {};
		if (digit == -1)
			str[0] = '?';
		else {
			/* convert index int to string */
			sprintf(str, "%i", digit);
		}
		/* create save path */
		strcat(res, str);
	}

	/* save debug image in debug.png */
	save_debug_image(colored, image_path, image, &left_x, &right_x, &top_y, &bottom_y, &digit_width, &digit_height, &space_width);

	/* free array for each digits */
	for (size_t i = 0; i < number; i++)
		free(*(digits_image_array + i));

	/* free image digits array */
	free(digits_image_array);

	printf("ocr: res: %s\n", res);
}
