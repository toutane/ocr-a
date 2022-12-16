#pragma once

/* define the type image */
typedef struct {
   int width, height;
   unsigned *pixels;
} Image;

/* define ocr function prototype */
void ocr(Image *image, char *image_path);

/* return the width of the space between two digits 
 * x_zero is the x coordinate before space (the last x of the first digit) */
int get_space_width(Image *image, int *digit_width, int *left_x, int *bottom_y, int *top_y, int *x_zero);
