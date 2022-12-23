#pragma once

#include "ocr.h"

/* save debug image in a png file */
void save_debug_image(int colored, char *image_path, Image *image, int *left_x, int *right_x, int *top_y, int *bottom_y, int *digit_width, int *digit_height, int *space_width);

/* save digit image as png */
void save_digit_image(int colored, unsigned *pixels, int *digit_width, int *digit_height, int index, char *image_path);
