//[*]----------------------------------------------------------------------------------------------------[*]
#include "UnicodeFont.h"
#include "UnicodeFontCode.h"
#include "UnicodeFontSize.h"
//[*]----------------------------------------------------------------------------------------------------[*]
int GetFont(unsigned short code, unsigned short *data, int *heigth, int *width)
{
	unsigned int i = 0;
	unsigned int pos = (unsigned int)(code-0x0020);
	int ret = 0;
	
	if(code < 0x0020)
	{
		code 	= 0x0020;
		ret 	= -1;
	}
	
	*width 	= (int)UniFont_Width[pos];
	*heigth	= (int)12;
	
	pos = pos * 12;
	for(i=0; i<12; i++)
		data[i] = Unicode_Font[pos+i];
		
	return ret;
}
