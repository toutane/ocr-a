#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* include ocr.h where Image and ocr are declared */
#include "ocr.h"

/* convert each pixel of a sdl surface to grayscale value and fill image_pixels array */
void to_grayscale(SDL_Surface* surface, unsigned image_pixels[]) {
	SDL_PixelFormat *fmt;
	Uint32 temp, pixel;
	Uint8 red, green, blue;
	unsigned grayscale;

	fmt = surface->format;
	SDL_LockSurface(surface);
	Uint32* surface_pixels = surface->pixels;

	/* fill pixels array with grayscale value */
	for (size_t y = 0; y < surface->h; y++) {
		for (size_t x = 0; x < surface->w; x++) {
			pixel = surface_pixels[y * surface->w + x];
			/* Extracting color components from a 32-bit color value */

			/* Get Red component */
			temp = pixel & fmt->Rmask;  /* Isolate red component */
			temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
			temp = temp << fmt->Rloss;  /* Expand to a full 8-bit number */
			red = (Uint8)temp;

			/* Get Green component */
			temp = pixel & fmt->Gmask;  /* Isolate green component */
			temp = temp >> fmt->Gshift; /* Shift it down to 8-bit */
			temp = temp << fmt->Gloss;  /* Expand to a full 8-bit number */
			green = (Uint8)temp;

			/* Get Blue component */
			temp = pixel & fmt->Bmask;  /* Isolate blue component */
			temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
			temp = temp << fmt->Bloss;  /* Expand to a full 8-bit number */
			blue = (Uint8)temp;

			grayscale = (red + green + blue) / 3;
			image_pixels[y * surface->w + x] = grayscale;
			//printf("Pixel Color -> R: %d,  G: %d,  B: %d,  V: %d\n", red, green, blue, grayscale);
		}
	}

	SDL_UnlockSurface(surface);

}

int main(int argc, char *argv[]) {
	FILE *fp;
	char *prog = argv[0]; /* program name for errors */

	if (argc == 1) { /* no args; do test */
		unsigned data[] = {
			0xff, 0xff, 0xff, 0xc0, 0xad, 0xad, 0xad, 0xad, 0xaf, 0xe8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xad, 0xad, 0xad, 0xad, 0xaf, 0xe8, 0xff, 0xff, 0xff,  
			0xff, 0xff, 0xda,  0x2,  0x0,  0x0,  0x0,  0x0,  0x0, 0x3c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xda,  0x2,  0x0,  0x0,  0x0,  0x0,  0x0, 0x3c, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0x28, 0x2e,  0xe,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0x28, 0x2e,  0xe,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd4,  0x0,  0x0, 0xdc, 0xff, 0x50,  0x0, 0x37, 0xff, 0xff, 0xff,
			0xff, 0xd3, 0x42,  0x0,  0x0, 0x3a, 0x44, 0x12,  0x0,  0xe, 0x81, 0xf3, 0xff, 0xff, 0xd3, 0x42,  0x0,  0x0, 0x3a, 0x44, 0x12,  0x0,  0xe, 0x81, 0xf3, 0xff,
			0xd9,  0xb,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x4f, 0xff, 0xd9,  0xb,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x4f, 0xff,
			0xb0,  0x0,  0xb, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x35,  0x0,  0xa, 0xf1, 0xb0,  0x0,  0xb, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x35,  0x0,  0xa, 0xf1,
			0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8, 0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8,
			0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8, 0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8,
			0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8, 0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8,
			0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8, 0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8,
			0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8, 0xad,  0x0, 0x2b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,  0x0,  0x0, 0xe8,
			0xb2,  0x0,  0x8, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x28,  0x0,  0xe, 0xf5, 0xb2,  0x0,  0x8, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x28,  0x0,  0xe, 0xf5,
			0xe8, 0x1e, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x6b, 0xff, 0xe8, 0x1e, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x6b, 0xff,
		};
		Image image = { 26, 18, data };	/* create image structure */

		ocr(&image, "test");	
	} else
		while (--argc > 0)
			if ((fp = fopen(*++argv, "r")) == NULL) {
				fprintf(stderr, "%s: can't open %s\n", prog, *argv);
				return 1;
			} else {
				printf("%s: open %s\n", prog, *argv);

				/* load image and create surface */
				SDL_Surface *surface = IMG_Load(*argv);

				/* handle error */
				if (surface == NULL) {
					fprintf(stderr, "%s: can't load %s\n", prog, *argv);
					fclose(fp);
					return 1;
				}

				/* allocate memory for pixels array */
				unsigned *pixels = malloc(surface->w * surface->h* sizeof(unsigned));

				/* convert image pixels to grayscale */
				to_grayscale(surface, pixels);

				/* create image structure */
				Image image = {surface->w, surface->h, pixels};

				/* free surface */
				SDL_FreeSurface(surface);

				/*
				char *res = ocr(&image, *argv);	
				if (res == 0)
					return 1;

				printf("%s: res: %s\n", prog, res);
				*/
				ocr(&image, *argv);	

				/* free pixels array */
				free(pixels);

				fclose(fp);
			}

	return 0;
}
