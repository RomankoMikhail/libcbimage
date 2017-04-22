/*
 * MIT License
 * Copyright (c) 2017 Romanko Mikhail
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LIB_C_BASIC_IMAGE_HEADER
#define LIB_C_BASIC_IMAGE_HEADER

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#define CBIMAGE_INDEX(image, x, y) (((y) * (image->width) * (image->bpp >> 3)) + ((x) * (image->bpp >> 3)))
#define CBIMAGE_INDEX_W(width, bpp, x, y) ((y) * width * (bpp >> 3) + (x) * (bpp >> 3))

enum {
	CBIMAGE_8BPP = 8,
	CBIMAGE_16BPP = 16,
	CBIMAGE_24BPP = 24,
	CBIMAGE_32BPP = 32
};

enum {
	CBIMAGE_MIRROR_HORIZONTALY = 1,
	CBIMAGE_MIRROR_VERTICALY = 2
};

enum {
	CBIMAGE_MONOCHROME = 0,
	CBIMAGE_RGB,
	CBIMAGE_ALPHA
};

enum {
	CBIMAGE_90_DEG = 0,
	CBIMAGE_180_DEG = 1,
	CBIMAGE_240_DEG = 2,
	CBIMAGE_M90_DEG = 2,
	CBIMAGE_M180_DEG = 1,
	CBIMAGE_M240_DEG = 0
};

enum {
	CBIMAGE_BOND_HORIZONTAL = 0,
	CBIMAGE_BOND_VERTICAL
};

typedef struct {
	uint8_t *data;
	size_t height, width;
	int bpp, type;
} cbimage_t;


extern cbimage_t *cbimage_load_bmp(char *filename);
extern int cbimage_save_bmp(char *filename, cbimage_t image);

extern int cbimage_inverse(cbimage_t *image);
extern int cbimage_mirror(cbimage_t *image, int mirror);
extern int cbimage_rotate(cbimage_t *image, int angle);
extern int cbimage_free(cbimage_t *image);

extern cbimage_t *cbimage_create(int width, int height, int bpp, int type);
extern void cbimage_insert(cbimage_t *dst, cbimage_t *src, int x, int y);
extern cbimage_t *cbimage_bond(int bond_type, int images, ...);

#endif /* LIB_C_BASIC_IMAGE_HEADER */
