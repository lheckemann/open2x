/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created.
 *
 */

#include <iconv.h>

//senquack
#define _GNU_SOURCE
#define __USE_GNU

#include <string.h>
#include "FontDisplay.h"
#include "UnicodeFont.h"

int transcode(const char *from_code, const char *to_code, const char *src, char *target, int len)
{
	iconv_t cd 		= 0;
	char 	*from	= NULL;
	char 	*to		= NULL;
	int 	flen	= 0; 
	int 	tlen	= 0;
	size_t 	result 	= 0;
	
	
	const char 	*errhand = (char*)strchrnul(to_code, '/');
	int 		nslash 	 = 2;
	char 		*newp	 = NULL;
	char 		*cp		 = NULL;
	
	if(*errhand == '/')
	{
		--nslash;
		errhand = (char *)strchrnul(errhand, '/');
	
		if(*errhand == '/')
		{
			--nslash;
			++errhand;
		}
	}
	
	newp 	= (char *)alloca(errhand - to_code + nslash + 6 + 1);

	//senquack
//	cp 		= (char *)mempcpy(newp, to_code, errhand - to_code);
	cp 		= mempcpy((void *)newp, (const void *)to_code, (int)(errhand - to_code));

	while(nslash-- > 0)
		*cp++ = '/';
	
	memcpy(cp, "IGNORE", sizeof ("IGNORE"));
	to_code = newp;
	
	from 	= (char *)src;
	to 		= target;
	
	
	cd 		= iconv_open(to_code, from_code);
	flen 	= len;
	tlen 	= len*2;


	
	result 	= iconv((void*)cd, &from, (size_t*)&flen, &to, (size_t*)&tlen);
	
	*to='\0';
	
	
	iconv_close(cd);
	
	return len-tlen;
}

void SetPixel(SDL_Surface *pScreen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint8 *ubuff8;
	Uint16 *ubuff16; 
	Uint32 *ubuff32;
	Uint32 color;

	Uint8 c1;
	Uint8 c2;
	Uint8 c3;

	color = SDL_MapRGB(pScreen->format, r, g, b );

	
	switch(pScreen->format->BytesPerPixel) 
	{
		case 1: 
			ubuff8 = (Uint8*)pScreen->pixels;
			ubuff8 += (y * pScreen->pitch) + x; 
			*ubuff8 = (Uint8)color;
			break;

		case 2:
			ubuff8 = (Uint8*)pScreen->pixels;
			ubuff8 += (y * pScreen->pitch) + (x*2);
			ubuff16 = (Uint16*)ubuff8;
			*ubuff16 = (Uint16)color; 
			break;  

		case 3:
			ubuff8 = (Uint8*)pScreen->pixels;
			ubuff8 += (y * pScreen->pitch) + (x*3);


			if(SDL_BYTEORDER == SDL_LIL_ENDIAN) 
			{
				c1 = (color & 0xFF0000) >> 16;
				c2 = (color & 0x00FF00) >> 8;
				c3 = (color & 0x0000FF);
			} 
			else 
			{
				c3 = (color & 0xFF0000) >> 16;
				c2 = (color & 0x00FF00) >> 8;
				c1 = (color & 0x0000FF);	
			}

			ubuff8[0] = c3;
			ubuff8[1] = c2;
			ubuff8[2] = c1;
			break;

		case 4:
			ubuff8 = (Uint8*)pScreen->pixels;
			ubuff8 = (Uint8*)((y*pScreen->pitch) + (x*4));
			ubuff32 = (Uint32*)ubuff8;
			*ubuff32 = color;
			break;

		default:
			fprintf(stderr, "Error: Unknown bitdepth!\n");
	}
}

int DrawFont(SDL_Surface *pScreen, int x, int y, unsigned short code, Uint8 r, Uint8 g, Uint8 b)
{
	unsigned short fontdata[12];
	int height = 0, width = 0;
	int cx = 0;
	int cy = 0;

	GetFont(code, fontdata, &height, &width);
	
	for(cx=0; cx<width; cx++)
	{
		for(cy=0; cy<height; cy++)
		{
			if(fontdata[cy] & (0x8000 >> cx))	
			{
				SetPixel(pScreen, cx + x, cy + y, r, g, b);
			}
		}
	}
	
	return width;
}

void DrawText(SDL_Surface *pScreen, int x, int y, unsigned short *code, int lencode, Uint8 r, Uint8 g, Uint8 b)
{
	int i = 0;
	int pos = 0;
	
	
	if(lencode == -1)	
	{
		for(i=0; code[i]; i++)
			pos += DrawFont(pScreen, x+pos, y, code[i], r, g, b);
	}
	
	else
	{
		for(i=0; i<lencode; i++)
			pos += DrawFont(pScreen, x+pos, y, code[i], r, g, b);
	}
}

void DrawTextOut(SDL_Surface *pScreen, int x, int y, int scale, unsigned short *code, Uint8 r, Uint8 g, Uint8 b)
{
	int i 		= 0;
	int pos		= 0;
	int cnt 	= 0;
	int total 	= 0;
	int height 	= 0;
	int width 	= 0;
	unsigned short fontdata[12] = { 0, };

	
	for(i=0; code[i]; i++)
	{
		GetFont(code[i], fontdata, &height, &width);		
		total += width;
		if(total > scale-18)	
			if(cnt == 0)
				cnt = i-1;								
	}
	
	if(total < scale)
	{
		for(i=0; code[i]; i++)
			pos += DrawFont(pScreen, x+pos, y, code[i], r, g, b);
	}
	else
	{
		for(i=0; i<cnt; i++)
			pos += DrawFont(pScreen, x+pos, y, code[i], r, g, b);
		pos += DrawFont(pScreen, x+pos, y, '.', r, g, b);
		pos += DrawFont(pScreen, x+pos, y, '.', r, g, b);
		pos += DrawFont(pScreen, x+pos, y, '.', r, g, b);
	}
}


unsigned short* OnUTF8ToUnicode(const char *src, int last)
{
	int len = 0;	
	unsigned short *dest = NULL;

	
	len	= (strlen(src)+1)*2;
	dest = (unsigned short*)malloc(len);
	memset((char*)dest, 0, len);			
	
	
	transcode("UTF-8", "UNICODELITTLE", src, (char*)dest, strlen(src)-last);
	
	return dest;
}
