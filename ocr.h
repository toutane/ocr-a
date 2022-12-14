#pragma once

/* define the type image */
typedef struct {
   int width, height;
   unsigned *pixels;
} Image;

/* define ocr function prototype */
char *ocr(Image *image);
