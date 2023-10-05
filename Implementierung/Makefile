# constants
CC=gcc
FLAGS=-std=gnu11 -O2
DEBUG_FLAGS=-Wall -Wextra -Wpedantic -Wstrict-aliasing -fstrict-aliasing -g
FILES=main.c bitmap.c util.c bmp_rle.c bmp_rle_V1.c bmp_rle_V2.c bmp_rle_encode_V3.c 
OUT=bmpRle
# recipes
.PHONY: all clean
all: bmpRle
bmpRle: ${FILES}
	$(CC) $(FLAGS) -o ${OUT} $^
debug: ${FILES}
	$(CC) ${DEBUG_FLAGS} -o ${OUT} $^
clean:
	rm -f ${OUT}