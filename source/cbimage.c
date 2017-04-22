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

#include "../include/cbimage.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define LIBCBMP_BUFFER 1024


typedef struct
{
	uint32_t	width;
	uint32_t	height;
	uint16_t	bpp;
	off_t 		pointer_data;
	int				valid;
} lcbmp_header;




off_t get_file_size(FILE *handle)
{
	off_t cur = ftell(handle), end;
	fseek(handle, 0, SEEK_END);
	end = ftell(handle);
	fseek(handle, cur, SEEK_SET);
	
	return end;
}




lcbmp_header lcbmp_get_info(FILE *handle)
{
	lcbmp_header info = {0};
	
	off_t		file_size 		= get_file_size(handle);
	off_t 	cur_position 	= ftell(handle);
	
	uint8_t bmp_header[54];
	
	if(file_size < 54)
	{
		return info;
	}
		
	
	fseek(handle, 0, SEEK_SET);
	fread(bmp_header, 54, 1, handle);
	
	if(strncmp((char*)bmp_header, "BM", 2))
	{
		return info;
	}
	
	if(*((uint32_t*)&bmp_header[2]) != file_size)
	{
		return info;
	}
	
	info.pointer_data = *((uint32_t*)&bmp_header[10]);
	info.width = *((uint32_t*)&bmp_header[18]);
	info.height = *((uint32_t*)&bmp_header[22]);
	info.bpp = *((uint16_t*)&bmp_header[28]);
	
	if(info.bpp != CBIMAGE_24BPP) {
		return info;
	}
	
	info.valid = 1;
	fseek(handle, cur_position, SEEK_SET);
	return info;
}




void lcbmp_form_info(uint8_t form[54], cbimage_t info)
{	
	off_t bmp_row = (((info.bpp * info.width + 31) >> 5) << 2);
	size_t bmp_data = bmp_row * info.height;
	
	memset(form,0,54);
	
	strncpy((char*)form,"BM",2);
	*((uint32_t*)&form[2]) = bmp_data + 54;
	*((uint32_t*)&form[10]) = 54;
	*((uint32_t*)&form[14]) = 40;
	*((uint32_t*)&form[18]) = info.width;
	*((uint32_t*)&form[22]) = info.height;
	*((uint16_t*)&form[26]) = 1;
	*((uint16_t*)&form[28]) = info.bpp;
	
	*((uint32_t*)&form[30]) = 0;
	*((uint32_t*)&form[34]) = 0;
	*((int32_t*)&form[38]) = 64;
	*((int32_t*)&form[42]) = 64;
	*((uint32_t*)&form[46]) = 0;
	*((uint32_t*)&form[50]) = 0;
}





cbimage_t *cbimage_load_bmp(char *filename) 
{
	/* Optimize in futher implementations
	 * 
	 * uint8_t	bmp_buffer[LIBCBMP_BUFFER];
	 * */
	size_t 	current_row, current_col;
	off_t		bmp_padding;
	
	cbimage_t			*loaded_image;
	lcbmp_header 	header;
	
	FILE *handle;
	
	assert(filename != NULL);
	
	handle = fopen(filename, "rb");
	if(!handle)
	{
		fprintf(stderr,"[ERROR] file \"%s\": ",filename);
		perror("");
		return NULL;
	}
	
	header = lcbmp_get_info(handle);
	if(!header.valid) {
		fprintf(stderr,"[ERROR] file \"%s\": not valid\n",filename);
		fclose(handle);
		return NULL;
	}
	bmp_padding = (((header.bpp * header.width + 31) >> 5) << 2) - (header.width * (header.bpp >> 3));
	
	fseek(handle, header.pointer_data, SEEK_SET);
	
	loaded_image = calloc(1, sizeof(cbimage_t));
	loaded_image->data = calloc(header.width * header.height, sizeof(uint8_t) * 3);
	
	for(current_row = 0; current_row < header.height; current_row++)
	{
		for(current_col = 0; current_col < header.width; current_col++)
		{
			uint8_t color[3];
			fread(color, sizeof(uint8_t), 3, handle);
			
			loaded_image->data[(header.height - current_row - 1) * header.width * 3 + current_col*3 + 2] = color[0];
			loaded_image->data[(header.height - current_row - 1) * header.width * 3 + current_col*3 + 1] = color[1];
			loaded_image->data[(header.height - current_row - 1) * header.width * 3 + current_col*3] = color[2];
			
		}
		fseek(handle, bmp_padding, SEEK_CUR);
	}
	loaded_image->bpp = header.bpp;
	loaded_image->width = header.width;
	loaded_image->height = header.height;
	loaded_image->type = CBIMAGE_RGB;
	
	fclose(handle);
	
	return loaded_image;
}





int cbimage_save_bmp(char *filename, cbimage_t image)
{
	FILE 		*handle;
	uint8_t header[54];
	uint8_t zeros[3] = {0};
	off_t 	bmp_padding = (((image.bpp * image.width + 31) >> 5) << 2) - (image.width * (image.bpp >> 3));
	size_t	current_row, current_col;
	
	handle = fopen(filename, "wb");
	if(!handle)
	{
		fprintf(stderr,"[ERROR] file \"%s\": ",filename);
		perror("");
		return -1;
	}
	
	lcbmp_form_info(header, image);
	
	fwrite(header,sizeof(uint8_t), 54, handle);
	for(current_row = 0; current_row < image.height; current_row++)
	{
		for(current_col = 0; current_col < image.width; current_col++)
		{
			fputc(image.data[(image.height - current_row - 1) * image.width * 3 + current_col * 3 + 2],handle);
			fputc(image.data[(image.height - current_row - 1) * image.width * 3 + current_col * 3 + 1],handle);
			fputc(image.data[(image.height - current_row - 1) * image.width * 3 + current_col * 3],handle);
		}
		fwrite(zeros,sizeof(uint8_t), bmp_padding, handle);
	}
	fclose(handle);
	return 0;
}





int cbimage_inverse(cbimage_t *image)
{
	size_t i;
	assert(image != NULL);
	
	for(i = 0; i < image->height * image->width * ((image->bpp) >> 3); i++)
	{
		image->data[i] = 255 - image->data[i];
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
	new_image->height = height;
	new_image->width = width;
	new_image->bpp = bpp;
	new_image->type = type;
	new_image->data = calloc(height * width * (bpp >> 3), sizeof(uint8_t));
	return new_image;
}





void cbimage_insert(cbimage_t *dst, cbimage_t *src, int x, int y) {
	size_t ix, iy;
	
	for(ix = 0; ix < src->width; ix++) 
	{
		for(iy = 0; iy < src->height; iy++)
		{
			dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y))] = src->data[CBIMAGE_INDEX(src, ix, iy)];
			dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y)) + 1] = src->data[CBIMAGE_INDEX(src, ix, iy) + 1];
			dst->data[CBIMAGE_INDEX(dst, (ix + x), (iy + y)) + 2] = src->data[CBIMAGE_INDEX(src, ix, iy) + 2]; 
		}
	}
}


