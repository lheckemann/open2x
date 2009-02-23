/*              
 *  DIGNSYS Inc. < www.dignsys.com >
 *  	Created, Developing from April 2005
 */

#include "drawcontrol.h"


#include "fbdisp/gfxdev.h"
#include "filelistview.h"
#include "UnicodeFont.h"
#include "typed.h"
#include "imgbinary.h"
#include "imgNumber.h"

extern int posCursor;
extern int bMenuStatus;
extern SViewInfomation 	infoView;					
extern SDirInfomation	infoDir;					
extern int leftVol;
extern int TotalPlayTime;
extern float CurrentPlayTime;
extern int sub_disp(void);


static int posicon[] =
{
	13, 44,	76,	106, 137, 169, 198,	230
};

void OnDraw_Progress(int x, int y, int per)
{
	int cx 	= 0;
	int cy 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;
	
	if(per <   0)	per = 0;
	if(per > 100)	per = 100;
	
	
	for(cy=0; cy<5; cy++)
	{
		for(cx=0; cx<(int)(per/100.0*230); cx++)
		{
			pos = (cy*314*3)+(cx*3);

			val = RGB2PIXEL(0xFF, 0xFF, 0xFF);
    		gfx_draw_pixel(psd, x+cx, y+cy, val);
	    }
	}

	for(cy=0; cy<5; cy++)
	{
		for(cx=(int)(per/100.0*230); cx<230; cx++)
		{
			pos = ((cy+y)*314*3)+((cx+x)*3);
			val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);

    		gfx_draw_pixel(psd, x+cx, y+cy, val);
	    }
	}
}

void OnDraw_Progress2(int x, int y, int per)
{
	int cx 	= 0;
	int cy 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;
	
	if(per <   0)	per = 0;
	if(per > 100)	per = 100;
	
	
	for(cy=0; cy<5; cy++)
	{
		for(cx=0; cx<(int)(per/100.0*230); cx++)
		{
			pos = (cy*314*3)+(cx*3);
			val = RGB2PIXEL(0xFF, 0xFF, 0xFF);
    		gfx_draw_pixel(psd, x+cx, y+cy, val);
	    }
	}

	
	for(cy=0; cy<5; cy++)
	{
		for(cx=(int)(per/100.0*230); cx<230; cx++)
		{
			pos = ((cy+y)*314*3)+((cx+x)*3);
			val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
    		gfx_draw_pixel(psd, x+cx, y+cy, val);
	    }
	}
}

void OnHide_LargeNumber()
{
	int x = 255;
	int y = 15;
	
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<59; cx++)
		{
			pos = ((cy+y)*4*3)+((cx+x)*3);
			val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
			gfx_draw_pixel(psd, cx+x+3, y+cy, val);
	    }
	}
}

void OnHide_LargeNumber2()
{
	int x = 255;
	int y = 15;
	
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = RGB2PIXEL(0xCC, 0xCC, 0xCC);
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<59; cx++)
		{
			pos = ((cy+y)*4*3)+((cx+x)*3);
			gfx_draw_pixel(psd, cx+x+3, y+cy, val);
	    }
	}
}

void OnHide_SmallNumber()
{
	int x = 255;
	int y = 0;
	
	int cx 	= 0;
	int cy 	= 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<59; cx++)
		{
			pos = ((cy+y)*4*3)+((cx+x)*3);
			val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
			gfx_draw_pixel(psd, cx+x+3, y+cy, val);
	    }
	}
}

void OnHide_SmallNumber2()
{
	int x = 255;
	int y = 0;
	
	int cx 	= 0;
	int cy 	= 0;
	int pos = 0;   
	MWPIXELVAL val = RGB2PIXEL(0xCC, 0xCC, 0xCC);
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<59; cx++)
		{
			pos = ((cy+y)*4*3)+((cx+x)*3);
			gfx_draw_pixel(psd, cx+x+3, y+cy, val);
	    }
	}
}

void OnDraw_SmallNumberText(int x, int y, int hour, int min)
{
	int i = 0;
	char szHour[10] 	= { 0,	};
	char szMinute[10] 	= { 0,	};
	
	
	memset(szHour	, 0, 10);
	memset(szMinute	, 0, 10);

	sprintf(szHour	, "%02d", hour	);
	sprintf(szMinute, "%02d", min	);

	for(i=strlen(szMinute)-1; i>=0; i--)
	{
		OnDraw_SmallNumber(x, y, (szMinute[i]-'0'));
		x-=6;
	}

	x+=3;
	OnDraw_SColon(x, y);
	x-=6;
	
	for(i=strlen(szHour)-1; i>=0; i--)
	{
		OnDraw_SmallNumber(x, y, (szHour[i]-'0'));
		x-=6;
	}
}

void OnDraw_LargeNumberText(int x, int y, int hour, int min)
{
	int i = 0;
	char szHour[10] 	= { 0,	};
	char szMinute[10] 	= { 0,	};
	
	
	memset(szHour	, 0, 10);
	memset(szMinute	, 0, 10);

	sprintf(szHour	, "%02d", hour	);
	sprintf(szMinute, "%02d", min	);

	for(i=strlen(szMinute)-1; i>=0; i--)
	{
		OnDraw_LargeNumber(x, y, (szMinute[i]-'0'));
		x-=10;
	}

	x+=6;
	OnDraw_LColon(x, y);
	x-=10;
	
	for(i=strlen(szHour)-1; i>=0; i--)
	{
		OnDraw_LargeNumber(x, y, (szHour[i]-'0'));
		x-=10;
	}
}

void OnDraw_LColon(int x, int y)
{
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<4; cx++)
		{
			pos = (cy*4*3)+(cx*3);
			if(!(img_LColon[pos+0] == 0xFF && img_LColon[pos+1] == 0x66 && img_LColon[pos+2] == 0xFF))
			{
				val = RGB2PIXEL(img_LColon[pos+0], img_LColon[pos+1], img_LColon[pos+2]);
    			gfx_draw_pixel(psd, x+cx, y+cy, val);
    		}
	    }
	}
}

void OnDraw_SColon(int x, int y)
{
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<9; cy++)
	{
		for(cx=0; cx<3; cx++)
		{
			pos = (cy*3*3)+(cx*3);
			if(!(img_SColon[pos+0] == 0xFF && img_SColon[pos+1] == 0x66 && img_SColon[pos+2] == 0xFF))
			{
				val = RGB2PIXEL(img_SColon[pos+0], img_SColon[pos+1], img_SColon[pos+2]);
    			gfx_draw_pixel(psd, x+cx, y+cy, val);
    		}
	    }
	}
}

void OnDraw_LargeNumber(int x, int y, int num)
{
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<14; cy++)
	{
		for(cx=0; cx<10; cx++)
		{
			pos = (cy*10*3)+(cx*3) + (num*30*14);
			if(!(img_LNumber[pos+0] == 0xFF && img_LNumber[pos+1] == 0x66 && img_LNumber[pos+2] == 0xFF))
			{
				val = RGB2PIXEL(img_LNumber[pos+0], img_LNumber[pos+1], img_LNumber[pos+2]);
    			gfx_draw_pixel(psd, x+cx, y+cy, val);
    		}
	    }
	}
}

void OnDraw_SmallNumber(int x, int y, int num)
{
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	for(cy=0; cy<9; cy++)
	{
		for(cx=0; cx<6; cx++)
		{
			pos = (cy*6*3)+(cx*3) + (num*6*3*9);
			if(!(img_SNumber[pos+0] == 0xFF && img_SNumber[pos+1] == 0x66 && img_SNumber[pos+2] == 0xFF))
			{
				val = RGB2PIXEL(img_SNumber[pos+0], img_SNumber[pos+1], img_SNumber[pos+2]);
    			gfx_draw_pixel(psd, x+cx, y+cy, val);
    		}
	    }
	}
}

void OnDraw_VolumeIcon(int x, int y, int vol, bool flag)
{
	int cx = 0;
	int cy = 0;
	int pos = 0;   
	MWPIXELVAL val = 0;
	
	if(flag == true)
	{
		for(cy=0; cy<16; cy++)
		{
			for(cx=0; cx<6; cx++)
			{
				pos = (cy*6*3)+(cx*3) + (vol*18*16);
				val = RGB2PIXEL(img_volume[pos+0], img_volume[pos+1], img_volume[pos+2]);
	    		gfx_draw_pixel(psd, x+cx, y+cy, val);
		    }
		}
	}
	else
	{
		for(cy=0; cy<16; cy++)
		{
			for(cx=0; cx<6; cx++)
			{
				pos = ((cy+y)*314*3)+((cx+x)*3);
				val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
	    		gfx_draw_pixel(psd, x+cx, y+cy, val);
		    }
		}
	}
}

void OnDraw_Volume(int vol)
{
	int i = 0;
	for(i=0; i<vol; i++)	OnDraw_VolumeIcon(271+(i*6), 34, i, true);
	for(i=vol; i<7; i++)	OnDraw_VolumeIcon(271+(i*6), 34, i, false);
}


void OnDraw_MoveMenu(int posOld, int pos)
{
	int cy = 26;

	OnDrawMenu_BackGorundButton(posOld);
	OnDrawMenu_Icon(posicon[pos], cy, pos, 1);	
}


void OnDrawMenu_BackGorundButton(int posOld)
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;

	for(y=26; y<22+25; y++)
	{
		for(x=posicon[posOld]-2; x<21+posicon[posOld]+4; x++)
		{
			pos = (y*314*3)+(x*3);
			val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
    		gfx_draw_pixel(psd, x+3, y, val);
	    }
	}
}


void OnDrawMenu_BackGorund()
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	
	MWPIXELVAL val = 0;
	
	for(y=0; y<67; y++)
	{
		for(x=0; x<314; x++)
		{
			pos = (y*314*3)+(x*3);

			if(backimage[pos+0] != 242 && backimage[pos+1] != 109  && backimage[pos+2] != 125)
			{
				val = RGB2PIXEL(backimage[pos+0], backimage[pos+1], backimage[pos+2]);
	    		gfx_draw_pixel(psd, x+3, y, val);
	    	}
	    }
	}
}


void OnDrawMenu_Icon(int cx, int cy, int icon, int status)
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;

	for(y=0; y<21; y++)
	{
		for(x=0; x<21; x++)
		{
			pos = (y*21*3)+(x*3)+(icon*21*21*3);
			val = RGB2PIXEL(img_button[pos+0], img_button[pos+1], img_button[pos+2]);
	    	gfx_draw_pixel(psd, x + cx, y + cy, val);
	    }
	}
}


int Menu_DrawFont(int x, int y, unsigned short code, Uint8 r, Uint8 g, Uint8 b)
{
	unsigned short fontdata[12];
	int height = 0, width = 0;
	int cx = 0, cy = 0;
	MWPIXELVAL val = 0;

	GetFont(code, fontdata, &height, &width);
	
	for(cx=0; cx<width; cx++)
	{
		for(cy=0; cy<height; cy++)
		{
			if(fontdata[cy] & (0x8000 >> cx))	
			{
				val = RGB2PIXEL(r, g, b);
	    		gfx_draw_pixel(psd, cx + x, cy + y, val);
			}
		}
	}
	
	return width;
}


void Menu_DrawText(int x, int y, unsigned short *code, int lencode, Uint8 r, Uint8 g, Uint8 b)
{
	int i 		= 0; 
	int pos 	= 0;
	
	
	if(lencode == -1)	
	{
		for(i=0; code[i]; i++)
		{
			pos += Menu_DrawFont(x+pos, y, code[i], r, g, b);
		}
	}
	
	else
	{
		for(i=0; i<lencode; i++)
			pos += Menu_DrawFont(x+pos, y, code[i], r, g, b);
	}
}


void Menu_DrawTextOut(int x, int y, int scale, unsigned short *code, Uint8 r, Uint8 g, Uint8 b)
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
	
	if(total<scale)
	{
		for(i=0; code[i]; i++)
			pos += Menu_DrawFont(x+pos, y, code[i], r, g, b);
	}
	else
	{
		for(i=0; i<cnt; i++)
			pos += Menu_DrawFont(x+pos, y, code[i], r, g, b);
		pos += Menu_DrawFont(x+pos, y, '.', r, g, b);
		pos += Menu_DrawFont(x+pos, y, '.', r, g, b);
		pos += Menu_DrawFont(x+pos, y, '.', r, g, b);
	}
}

extern int g_FBStatus;			


void ShowMenu()
{
	int cx 	= 13+3;	
	int cy 	= 26;
	int i  	= 0;
	int vol = 0;
   	unsigned short *dest = NULL;		

   	
  	
		switch_fb0_to_fb1();
   	

	
	OnDrawMenu_BackGorund();
	
	OnDrawMenu_Icon(posicon[posCursor], cy, posCursor, 1);	
	bMenuStatus = true;
	
	dest = OnUTF8ToUnicode(infoDir.pList[infoView.nPosition].szName, 4);
	Menu_DrawTextOut(20, 7, 280, dest, 0xFF, 0xFF, 0xFF);
	
	if(leftVol < 14)		vol = 0;
	else if(leftVol < 14*2)	vol = 1;
	else if(leftVol < 14*3)	vol = 2;
	else if(leftVol < 14*4)	vol = 3;
	else if(leftVol < 14*5)	vol = 4;
	else if(leftVol < 14*6)	vol = 5;
	else if(leftVol < 14*7)	vol = 6;
	else 					vol = 7;
	
	OnDraw_Volume(vol);  
	
	
	OnDraw_SmallNumberText(300,  4, ((int)CurrentPlayTime)/60, ((int)CurrentPlayTime)%60);
	OnDraw_LargeNumberText(300, 15, ((int)TotalPlayTime)/60, ((int)TotalPlayTime)%60);
	printf("TotalPlayTime %d\n", TotalPlayTime);
	
	
	
	if(dest)	free(dest);

	sub_disp();
}


void HideMenu()
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	
	MWPIXELVAL val = 0;
	
	for(y=0; y<67; y++)
	{
		for(x=0; x<314; x++)
		{
			pos = (y*314*3)+(x*3);
			val = RGB2PIXEL(0xCC, 0xCC, 0xCC);
	    	gfx_draw_pixel(psd, x+3, y, val);
	    }
	}

	bMenuStatus = false;
}


void ShowLoding()
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;
	int cx = (320-233)/2;
	int cy = (240- 98)/2;

	for(y=0; y<98; y++)
	{
		for(x=0; x<233; x++)
		{
			pos = (y*233*3)+(x*3);
			val = RGB2PIXEL(imgLoading[pos+0], imgLoading[pos+1], imgLoading[pos+2]);
	    	gfx_draw_pixel(psd, x + cx, y + cy, val);
	    }
	}
}


void HideLoding()
{
	int x 	= 0;
	int y 	= 0;
	int pos = 0;
	MWPIXELVAL val = 0;
	int cx = (320-233)/2;
	int cy = (240- 98)/2;

	for(y=0; y<98; y++)
	{
		for(x=0; x<233; x++)
		{
			pos = (y*233*3)+(x*3);
			val = RGB2PIXEL(0xCC, 0xCC, 0xCC);
	    	gfx_draw_pixel(psd, x + cx, y + cy, val);
	    }
	}
}
