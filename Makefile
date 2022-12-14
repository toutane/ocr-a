all: main.c
	gcc `pkg-config --libs sdl2 SDL2_image` -o main main.c ocr.c
