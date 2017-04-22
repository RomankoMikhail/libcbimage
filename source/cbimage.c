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

#include <cbimage.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>


int cbimage_inverse(cbimage_t *image)
{
	size_t i;
	assert(image != NULL);
	
	for(i = 0; i < image->height * image->width * ((image->bpp) >> 3); i++)
	{
		image->data[i] = 0xff - image->data[i];
	}
	return 0;
}





int cbimage_mirror(cbimage_t *image, int mirror)
{
	size_t i,t;
	
	assert(image != NULL);
	if(mirror & CBIMAGE_MIRROR_HORIZONTALY)
	{
		for(i = 0; i < image->height; i++)
		{
			for(t = 0; t < (image->width >> 1); t++)
			{
				uint8_t color[3];	
				
				color[0] = image->data[CBIMAGE_INDEX(image, t, i)];
				color[1] = image->data[CBIMAGE_INDEX(image, t, i) + 1];
				color[2] = image->data[CBIMAGE_INDEX(image, t, i) + 2];
				
				image->data[CBIMAGE_INDEX(image, t, i)] = image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i)];
				image->data[CBIMAGE_INDEX(image, t, i) + 1] = image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i) + 1];
				image->data[CBIMAGE_INDEX(image, t, i) + 2] = image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i) + 2];
				
				image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i)] = color[0];
				image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i) + 1] = color[1];
				image->data[CBIMAGE_INDEX(image,(image->width - t - 1), i) + 2] = color[2];

			}
		}
	} 
	
	if(mirror & CBIMAGE_MIRROR_VERTICALY)
	{
		for(t = 0; t < image->width; t++)
		{
			for(i = 0; i < (image->height >> 1); i++)
			{
				uint8_t color[3];
				
				color[0] = image->data[CBIMAGE_INDEX(image, t, i)];
				color[1] = image->data[CBIMAGE_INDEX(image, t, i) + 1];
				color[2] = image->data[CBIMAGE_INDEX(image, t, i) + 2];
				
				image->data[CBIMAGE_INDEX(image, t, i)] = image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1))];
				image->data[CBIMAGE_INDEX(image, t, i) + 1] = image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1)) + 1];
				image->data[CBIMAGE_INDEX(image, t, i) + 2] = image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1)) + 2];
				
				image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1))] = color[0];
				image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1)) + 1] = color[1];
				image->data[CBIMAGE_INDEX(image, t, (image->height - i - 1)) + 2] = color[2];

			}
		}
	}
	return 1;
}





int cbimage_rotate(cbimage_t *image, int angle)
{
	assert(image != NULL);
	if(angle == CBIMAGE_180_DEG || angle == CBIMAGE_M180_DEG) 
	{
		cbimage_mirror(image,  CBIMAGE_MIRROR_VERTICALY | CBIMAGE_MIRROR_HORIZONTALY);
	} 
	else 
	{
		uint8_t *new_image = calloc(1, sizeof(uint8_t) * image->width * image->height * image->bpp);
		size_t x,y;
		
		for(x = 0; x < image->width; x++)
		{
			for(y = 0; y < image->height; y++)
			{
				new_image[CBIMAGE_INDEX_W(image->height, image->bpp, y, x)] = image->data[CBIMAGE_INDEX(image, x, y)];
				new_image[CBIMAGE_INDEX_W(image->height, image->bpp, y, x) + 1] = image->data[CBIMAGE_INDEX(image, x, y) + 1];
				new_image[CBIMAGE_INDEX_W(image->height, image->bpp, y, x) + 2] = image->data[CBIMAGE_INDEX(image, x, y) + 2];
			}
		}
		
		free(image->data);
		image->data = new_image;
		image->width = y;
		image->height = x;
		
		if(angle == CBIMAGE_90_DEG || angle == CBIMAGE_M240_DEG)
		{
			cbimage_mirror(image, CBIMAGE_MIRROR_HORIZONTALY);
		} else {
			cbimage_mirror(image, CBIMAGE_MIRROR_VERTICALY);
		}
	}
	return 1;
}





cbimage_t *cbimage_bond(int bond_type, int num_images, ...) {
	cbimage_t **images = calloc(num_images, sizeof(cbimage_t*));
	size_t i = 0, width_sum = 0, height_sum = 0, width_max = 0, height_max = 0;
	va_list args;
	va_start(args, num_images);
	
	while(i < num_images)
	{
		images[i] = va_arg(args, cbimage_t *);
		
		if(images[i]->width > width_max)
			width_max = images[i]->width;
			
		if(images[i]->height > height_max)
			height_max = images[i]->height;
			
		width_sum += images[i]->width;
		height_sum += images[i]->height;
			
		i++;
	} 
	
	va_end(args);
	
	cbimage_t *out;
	
	if(bond_type == CBIMAGE_BOND_HORIZONTAL) {
		out = cbimage_create(width_sum, height_max, CBIMAGE_24BPP, CBIMAGE_RGB);
		width_sum = 0;
		for(i = 0; i < num_images; i++) {
			cbimage_insert(out, images[i], width_sum, 0);
			width_sum += images[i]->width;
		}
	} else {
		out = cbimage_create(width_max, height_sum, CBIMAGE_24BPP, CBIMAGE_RGB);
		height_sum = 0;
		for(i = 0; i < num_images; i++) {
			cbimage_insert(out, images[i], 0, height_sum);
			height_sum += images[i]->height;
		}
	}
	free(images);
	return out;
}
	







int cbimage_free(cbimage_t *image)
{
	assert(image != NULL);
	assert(image->data != NULL);
	
	free(image->data);
	image->data = NULL;
	return 1;
}





cbimage_t *cbimage_create(int width, int height, int bpp, int type)
{
	cbimage_t *new_image = calloc(1, sizeof(cbimage_t));
	assert(new_image != NULL);

	new_image->height = height;
	new_image->width = width;
	new_image->bpp = bpp;
	new_image->type = type;
	new_image->data = calloc(height * width * (bpp >> 3), sizeof(uint8_t));

	assert(new_image->data != NULL);
	return new_image;
}





void cbimage_insert(cbimage_t *dst, cbimage_t *src, int x, int y) {
	size_t ix, iy;

	for(ix = 0; ix < src->width; ix++) 
	{
		for(iy = 0; iy < src->height; iy++)
		{
			if(((ix + x) < dst->width) && ((iy + y) < dst->height) && ((iy + y) >= 0) && ((iy + y) >= 0))
			{
				dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y))] = src->data[CBIMAGE_INDEX(src, ix, iy)];
				dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y)) + 1] = src->data[CBIMAGE_INDEX(src, ix, iy) + 1];
				dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y)) + 2] = src->data[CBIMAGE_INDEX(src, ix, iy) + 2]; 
			}
		}
	}
}


