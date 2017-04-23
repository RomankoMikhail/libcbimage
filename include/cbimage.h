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

/** @file */ 

#ifndef LIB_C_BASIC_IMAGE_HEADER
#define LIB_C_BASIC_IMAGE_HEADER

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#define CBIMAGE_INDEX(image, x, y) (((y) * (image->width) * (image->bpp >> 3)) + ((x) * (image->bpp >> 3)))
#define CBIMAGE_INDEX_W(width, bpp, x, y) ((y) * width * (bpp >> 3) + (x) * (bpp >> 3))

enum {
	CBIMAGE_1BPP = 1,
	CBIMAGE_2BPP = 2,
	CBIMAGE_4BPP = 4,
	CBIMAGE_8BPP = 8,
	CBIMAGE_16BPP = 16,
	CBIMAGE_24BPP = 24,
	CBIMAGE_32BPP = 32
};

enum {
	CBIMAGE_INVERSE_WITHOUT_ALPHA = 0,
	CBIMAGE_INVERSE_ALL = 1
};

enum {
	CBIMAGE_MIRROR_HORIZONTALY = 1,
	CBIMAGE_MIRROR_VERTICALY = 2
};

enum {
	CBIMAGE_MONOCHROME = 0,
	CBIMAGE_RGB,
	CBIMAGE_RGBA
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
	uint16_t r, g, b, a;
} cbpixel_t;

typedef struct {
	cbpixel_t *data;
	size_t height, width;
	int type;
} cbimage_t;


/** 
 * \brief Reads BMP file a loads image into the memory
 * 
 * \warning Function cannot load 2, 4, 8 and 16 bit BMP files.
 * \warning Function cannot read neither color table or ICC profiles.
 * \bug Function missrepresent 1 bit BMP files and reades it as white and black, even if color table set for different colors
 * 
 * \param filename filename of the BMP file that ment to be readed
 * \return a newly loaded image or NULL if somthing goes wrong
 */
extern cbimage_t *cbimage_load_bmp(char *filename);

/** 
 * \brief Saves image into BMP file
 * 
 * \warning Function cannot save 1, 2, 4, 8 and 16 bit BMP files.
 * 
 * \param filename the filename of the file to which you want to save the image
 * \param image image that you want to save
 * \param bpp - specifies Bits Per Pixel for the output file (CBIMAGE_24BPP for classic RGB BMP or CBIMAGE_32BPP for RGBA) 
 * \return Returns 0 if succsesfull or -1 if failed
 */
extern int cbimage_save_bmp(char *filename, cbimage_t image, int bpp);

/** 
 * \brief Inverse colors of the image
 * 
 * \param image The image above which will be manipulated.
 * \param type Type of the inversion:
 * 	- CBIMAGE_INVERSE_WITHOUT_ALPHA - for classical inversion - without alpha channel
 * 	- CBIMAGE_INVERSE_ALL - for inversion with alpha channel
 */
extern void cbimage_inverse(cbimage_t *image, int type);

/** 
 * \brief Mirrors the image
 * 
 * \param image image that you want to save
 * \param bpp - specifies Bits Per Pixel for the output file (CBIMAGE_24BPP for classic RGB BMP or CBIMAGE_32BPP for RGBA) 
 */
extern void cbimage_mirror(cbimage_t *image, int mirror);

/** 
 * \brief Rotates image
 * 
 * \param image - image that you want to rotate
 * \param angle - on what angle you want to rotate the image. Choose between:
 * 	- CBIMAGE_90_DEG,
 *	- CBIMAGE_180_DEG,
 *	- CBIMAGE_240_DEG,
 *	- CBIMAGE_M90_DEG,
 *	- CBIMAGE_M180_DEG,
 *	- CBIMAGE_M240_DEG.
 * \return Returns 0 if succsesfull or -1 if failed
 */
extern int cbimage_rotate(cbimage_t *image, int angle);

/** 
 * \brief Free memory used by image
 * 
 * \param image - pointer to an image 
 * \return This function allways returns 0, except times when you try to pass NULL pointer (in this case, there will be programm termination by assert)
 */
extern int cbimage_free(cbimage_t *image);


/** 
 * \brief Creates blank image
 * 
 * \param width - width of the new image
 * \param height - height of the new image
 * \param type - specifies image type with following options:
 * 	- CBIMAGE_MONOCHROME - for monochrome image
 * 	- CBIMAGE_RGB - for RGB image
 * 	- CBIMAGE_RGBA - for RGB image with alpha channel
 * \return Returns new image or NULL if error occures.
 */
extern cbimage_t *cbimage_create(int width, int height, int type);

/** 
 * \brief Overlay one image over another (PERMANENTLY)
 * 
 * 
 * \param dst - image that you want overlay with other image
 * \param src - image that will overlay *dst* image
 * \param x - x coordinate of src image over dst
 * \param y - y coordinate of src image over dst
 */
extern void cbimage_insert(cbimage_t *dst, cbimage_t *src, int x, int y);

/** 
 * \brief Creates new image from the given one
 * 
 * \param bond_type - specify a bond type (Vertical or Horizontal)
 * \param images - how many images you want to bond
 * \param ... - pass as arguments images that you want to bond
 * \return Returns new image or NULL if error occures.
 */
extern cbimage_t *cbimage_bond(int bond_type, int images, ...);


#endif /* LIB_C_BASIC_IMAGE_HEADER */
