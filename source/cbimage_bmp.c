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

#include <cbimage.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/** 
 * \brief BMP header container structure.
 * 
 * Basic structure for conaining basic bmp header parametrs such as
 * 	- Width and Height
 * 	- Bits Per Pixel
 * 	- Pointer where pixel array begins
 * 	- Valid flag
 * 
 * \warning This structure provides only *BASIC* handling of the bmp file.
 * \warning This structure ment to be used *ONLY* internaly.
 */
typedef struct
{
	uint32_t	width;
	uint32_t	height;
	uint16_t	bpp;
	off_t 		pointer_data;
	int				valid;
} cbmp_header;


/** 
 * \brief Gets file size in bytes.
 * 
 * Basic function for getting size of currently opened file by its FILE* handle
 * 
 * \param handle File handle
 * \returns file size in bytes
 * 
 * \warning This function ment to be used *ONLY* internaly.
 */
off_t get_file_size(FILE *handle)
{
	assert(handle != NULL);
	
	off_t cur = ftell(handle), end;
	fseek(handle, 0, SEEK_END);
	end = ftell(handle);
	fseek(handle, cur, SEEK_SET);
	
	return end;
}



/** 
 * \brief Gets BMP header and returns it as structure
 * 
 * Function that retrives important information from the BMP file
 * 	- Width and Height
 * 	- Bits Per Pixel
 * 	- Pointer where pixel array begins
 * 
 * \param *handle pass succsesfully opened file handle
 * \return Returns BMP header
 * 
 * \warning This function ment to be used *ONLY* internaly.
 */
cbmp_header cbmp_get_info(FILE *handle)
{
	cbmp_header info = {0};
	
	assert(handle != NULL);
	
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
	
	if((info.bpp != CBIMAGE_24BPP) && (info.bpp != CBIMAGE_32BPP) && (info.bpp != CBIMAGE_1BPP)) {
		return info;
	}
	
	info.valid = 1;
	fseek(handle, cur_position, SEEK_SET);
	return info;
}



/** 
 * \brief Creates semi-valid BMP header
 * 
 * This function creates semi-valid BMP header using given parameters such as:
 * 	- Width and Height
 * 	- Bits Per Pixel
 * 
 * \param form[54] - byte array that represents BMP header with DIB
 * \param info - image, from it we gets several attributes such as width and height 
 * \param bpp - in what Bits Per Pixel format we need to save our image
 * 
 * \warning This function ment to be used *ONLY* internaly.
 */
void cbmp_form_info(uint8_t form[54], cbimage_t info, int bpp)
{
	assert(form != NULL);
	off_t bmp_row = (((bpp * info.width + 31) >> 5) << 2);
	size_t bmp_data = bmp_row * info.height;
	
	memset(form, 0, 54);
	
	strncpy((char*)form,"BM",2);
	*((uint32_t*)&form[2]) = bmp_data + 54;
	*((uint32_t*)&form[10]) = 54;
	*((uint32_t*)&form[14]) = 40;
	*((uint32_t*)&form[18]) = info.width;
	*((uint32_t*)&form[22]) = info.height;
	*((uint16_t*)&form[26]) = 1;
	*((uint16_t*)&form[28]) = bpp;
	*((int32_t*)&form[38])	= 64;
	*((int32_t*)&form[42])	= 64;
}



/** 
 * This function reads BMP file and tries to load it into the memory.
 */
cbimage_t *cbimage_load_bmp(char *filename) 
{
	/* Optimize in futher implementations
	 * 
	 * uint8_t	bmp_buffer[LIBCBMP_BUFFER];
	 * */
	size_t 	current_row, current_col;
	off_t		bmp_padding;
	
	cbimage_t			*loaded_image;
	cbmp_header 	header;
	
	FILE *handle;
	
	assert(filename != NULL);
	
	handle = fopen(filename, "rb");
	if(!handle)
	{
		fprintf(stderr,"[ERROR] file \"%s\": ",filename);
		perror("");
		return NULL;
	}
	
	header = cbmp_get_info(handle);
	if(!header.valid) {
		fprintf(stderr,"[ERROR] file \"%s\": not valid or unsupported\n",filename);
		fclose(handle);
		return NULL;
	}
	
	bmp_padding = (((header.bpp * header.width + 31) >> 5) << 2) - (header.width * (header.bpp >> 3));
	
	fseek(handle, header.pointer_data, SEEK_SET);
	
	loaded_image = cbimage_create(header.width, header.height, CBIMAGE_RGB);
	
	for(current_row = 0; current_row < header.height; current_row++)
	{
		for(current_col = 0; current_col < header.width; current_col++)
		{
			uint8_t color[4];
			
			
			switch(header.bpp)
			{
				case CBIMAGE_1BPP:
					fread(color, sizeof(uint8_t), 1, handle);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = (color[0] >> 7) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = (color[0] >> 7) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = (color[0] >> 7) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 6) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 6) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 6) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 5) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 5) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 5) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 4) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 4) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 4) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 3) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 3) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 3) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 2) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 2) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 2) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0] >> 1) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0] >> 1) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0] >> 1) & 0x1) ? (0xFFFF) : (0x0);
					current_col++;
					if(!(current_col < header.width))
						break;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = ((color[0]) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = ((color[0]) & 0x1) ? (0xFFFF) : (0x0);
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = ((color[0]) & 0x1) ? (0xFFFF) : (0x0);
					break;
				case CBIMAGE_24BPP:
					fread(color, sizeof(uint8_t), 3, handle);
					
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = color[0] << 8;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = color[1] << 8;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = color[2] << 8;
					fseek(handle, bmp_padding, SEEK_CUR);
					break;
				case CBIMAGE_32BPP:
					fread(color, sizeof(uint8_t), 4, handle);
					
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].a = color[0] << 8;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].b = color[1] << 8;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].g = color[2] << 8;
					loaded_image->data[(header.height - current_row - 1) * header.width + current_col].r = color[3] << 8;
					fseek(handle, bmp_padding, SEEK_CUR);
					break;
				default:
					fprintf(stderr, "[CRITICAL ERROR] Unsupported BMP format during reading! THIS IS A BUG!\n");
					exit(-1);
					break;
			}
			
		}
		
	}
	
	fclose(handle);
	
	return loaded_image;
}




/** 
 * This function save image from the memory to the disk. You may specify 
 * BPP option to choose in what format you want to save image.
 */
int cbimage_save_bmp(char *filename, cbimage_t image, int bpp)
{
	FILE 		*handle;
	uint8_t header[54];
	uint8_t zeros[3] = {0};
	off_t 	bmp_padding = (((bpp * image.width + 31) >> 5) << 2) - (image.width * (bpp >> 3));
	size_t	current_row, current_col;
	uint64_t mono_color = 0;
	uint8_t write_mono = 0, mono_wroten = 0;
	
	if((bpp != CBIMAGE_24BPP) && (bpp != CBIMAGE_32BPP)) {
		fprintf(stderr,"[ERROR] bpp format %d is not supported!\n", bpp);
		return -1;
	}
	
	handle = fopen(filename, "wb");
	
	if(!handle)
	{
		fprintf(stderr,"[ERROR] file \"%s\": ",filename);
		perror("");
		return -1;
	}
	
	cbmp_form_info(header, image, bpp);
	
	fwrite(header,sizeof(uint8_t), 54, handle);
	for(current_row = 0; current_row < image.height; current_row++)
	{
		for(current_col = 0; current_col < image.width; current_col++)
		{
			mono_wroten = 0;
			write_mono = 0;
			switch(bpp) 
			{
				case CBIMAGE_24BPP:
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].b >> 8,handle);
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].g >> 8,handle);
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].r >> 8,handle);
					fwrite(zeros,sizeof(uint8_t), bmp_padding, handle);
					break;
				case CBIMAGE_32BPP:
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].a >> 8,handle);
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].b >> 8,handle);
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].g >> 8,handle);
					fputc(image.data[(image.height - current_row - 1) * image.width + current_col].r >> 8,handle);
					fwrite(zeros,sizeof(uint8_t), bmp_padding, handle);
					break;
				default:
					fprintf(stderr, "[CRITICAL ERROR] Unsupported BMP format during writing! THIS IS A BUG!\n");
					exit(-1);
					break;
			}
			
		}
		
	}
	fclose(handle);
	return 0;
}


