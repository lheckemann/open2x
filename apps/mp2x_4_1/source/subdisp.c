/*********************************************************************************
 * Simple subtitle renderer for GP2X. (from TCPMP Subtitle Plugin)
 * Copyright (C) 2005 Kyeong-min, Kim. <nightknight>, www.mystictales.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *********************************************************************************/

#include <SDL.h>
#include "gvlib_export.h"
#include "config.h"
#include "subreader.h"
#include "fbdisp/gfxdev.h"
#include "fbdisp/fbs.h"
#include "subdisp.h"

#include "cx25874.h"

#define MLC_STL1_STX  0x28e2>>1
#define MLC_STL1_ENDX 0x28e4>>1
#define MLC_STL1_STY  0x28e6>>1
#define MLC_STL1_ENDY 0x28e8>>1

#define MLC_STL2_STX  0x28ea>>1
#define MLC_STL2_ENDX 0x28ec>>1
#define MLC_STL2_STY  0x28ee>>1
#define MLC_STL2_ENDY 0x28f0>>1

#define MLC_STL_HSC   0x2906>>1
#define MLC_STL_VSCL  0x2908>>1
#define MLC_STL_VSCH  0x290a>>1
#define MLC_STL_HW    0x290c>>1

extern unsigned int disp_mode;
extern int g_bTVMode;
extern int memfd;

extern subtitle *vo_sub;
extern int g_FBStatus;
unsigned short subtitle_buf[320*240];
extern int bMenuStatus;

extern int leftgap, rightgap, topgap, bottomgap, minbottom;
extern int font_red, font_green, font_blue;
extern int outline_red, outline_green, outline_blue;

void framebuffer_on(int fbnum)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;

    if (fbnum == 0)
    {
        // fb 0 on
        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_ON;
        LEN(pdmsg) = 0;

			 //senquack - SDL_videofd was some sort of retarded global GPH put in their crappy SDL:
			 //		We'll just open /dev/fb0 directly since that is where this ioctl should be going
			int SDL_videofd = open("/dev/fb0", O_RDWR);
			if (SDL_videofd == -1) {
				  fprintf(stderr, "Error opening /dev/fb0\n" );
				  exit(1);
			}

        if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

        g_FBStatus = 0x01;

		  //senquack
		  close(SDL_videofd);
    }
    else if (fbnum == 1)
    {
        // fb 1 on
        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_ON;
        LEN(pdmsg) = 0;
        if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));


	    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
	    pdmsg = &dummymsg;
	    MSG(pdmsg) = MMSP2_FB_RGB_COLOR_KEY;
	    LEN(pdmsg) = 0;
	    if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
	    {
	        printf("MMSP2_FB_RGB_COLOR_KEY error\n");
	    }
	    printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

	    gfx_draw_sized_rect(psd, 0, 0, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));

        g_FBStatus = 0x02;
    }
    else
        printf("invalid fbnum\n");
}

void framebuffer_off(int fbnum)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;

    if (fbnum == 0)
    {
        // fb 0 off
        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_OFF;
        LEN(pdmsg) = 0;

			 //senquack - SDL_videofd was some sort of retarded global GPH put in their crappy SDL:
			 //		We'll just open /dev/fb0 directly since that is where this ioctl should be going
			int SDL_videofd = open("/dev/fb0", O_RDWR);
			if (SDL_videofd == -1) {
				  fprintf(stderr, "Error opening /dev/fb0\n" );
				  exit(1);
			}

        if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

        g_FBStatus &= ~0x01;

		  //senquack
		  close(SDL_videofd);
    }
    else if (fbnum == 1)
    {
        // fb 1 off
        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_OFF;
        LEN(pdmsg) = 0;
        if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

        g_FBStatus &= ~0x02;
    }
    else
        printf("invalid fbnum\n");
}

int switch_fb0_to_fb1(void)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;

    // fb 0 off
    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_OFF;
    LEN(pdmsg) = 0;

	 //senquack - SDL_videofd was some sort of retarded global GPH put in their crappy SDL:
	 //		We'll just open /dev/fb0 directly since that is where this ioctl should be going
	int SDL_videofd = open("/dev/fb0", O_RDWR);
	if (SDL_videofd == -1) {
		  fprintf(stderr, "Error opening /dev/fb0\n" );
		  exit(1);
	}

    if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

	  //senquack
	  close(SDL_videofd);

    // fb 1 on
    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_ON;
    LEN(pdmsg) = 0;
    if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_COLOR_KEY;
    LEN(pdmsg) = 0;
    if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("MMSP2_FB_RGB_COLOR_KEY error\n");
    }
    printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

    gfx_draw_sized_rect(psd, 0, 0, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));

	if((disp_mode == DISPLAY_TV) && (g_bTVMode == 0) && (memfd > 0)) // NTSC TV Mode...
	{
		unsigned short *regs;
		regs = (unsigned short*)mmap(0, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0xC0000000);
		if (regs != NULL)
		{
			//regs[MLC_STL_HSC] = 512;
			regs[MLC_STL_VSCL] = regs[MLC_STL_HW];
			regs[MLC_STL_VSCH] = 0;
			//regs[MLC_STL2_STX] = 0;
			//regs[MLC_STL2_ENDX] = 719;
			regs[MLC_STL2_STY] = 0;
			regs[MLC_STL2_ENDY] = 239;
			munmap(regs, 0x10000);
		}
	}

	g_FBStatus = 0x02;
}
//[*]----------------------------------------------------------------------------------------------------[*]
int switch_fb1_to_fb0(void)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;

    // fb 0 on
    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_ON;
    LEN(pdmsg) = 0;

		 //senquack - SDL_videofd was some sort of retarded global GPH put in their crappy SDL:
		 //		We'll just open /dev/fb0 directly since that is where this ioctl should be going
		int SDL_videofd = open("/dev/fb0", O_RDWR);
		if (SDL_videofd == -1) {
			  fprintf(stderr, "Error opening /dev/fb0\n" );
			  exit(1);
		}

    if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

	  //senquack
	  close(SDL_videofd);

    // fb 1 off
    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_OFF;
    LEN(pdmsg) = 0;
    if ((err = ioctl(psd->fbfd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

	g_FBStatus = 0x01;
}

int _DrawFont(int x, int y, unsigned short code, unsigned short color, unsigned short back, int *width, int *height)
{
	unsigned short *buf;
	unsigned short fontdata[12];
	int cx = 0;
	int cy = 0;

	GetFont(code, fontdata, height, width);

	buf = subtitle_buf + (x + (y+1) * 320);
	for(cy=0; cy<*height; cy++)
	{
		for(cx=0; cx<*width; cx++)
		{
			if(fontdata[cy] & (0x8000 >> cx))
			{
				buf[cx] = color;
			}
		}
		buf += 320;
	}
	*height += 2;

	return *width;
}

int GetFontSize(unsigned short code, int *width, int *height)
{
	unsigned short fontdata[12];

	GetFont(code, fontdata, height, width);
	*height += 2;

	return *width;
}

static void BlitSubtitle(int x, int y, int width, int height)
{
	unsigned short *addr = psd->addr;
	unsigned short *buf = subtitle_buf;
	int linelen = psd->linelen;
	int dx, dy;
	if(!(addr != 0)) return;
	buf += x;
	if ((bMenuStatus)&&(y<67)) {
		buf += (67-y) * 320;
		y = 67;
	}
	addr += (x + psd->xoffset) + (y + psd->yoffset) * linelen;
	for (dy = 0;dy < height; dy++) {
		unsigned short *src = buf;
		unsigned short *dst = addr;
		for (dx = 0; dx < width; dx++, src++, dst++) {
			*dst = *src;
		}
		addr += linelen;
		buf += 320;
	}
}

Uint16 *U8toU16(Uint16 *unicode, Uint8 *utf8, int len)
{
	int i, j;
	Uint16 ch;

	for ( i=0, j=0; i < len; ++i, ++j ) {
		ch = ((const unsigned char *)utf8)[i];
		if ( ch >= 0xF0 ) {
			ch  =  (Uint16)(utf8[i]&0x07) << 18;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else
		if ( ch >= 0xE0 ) {
			ch  =  (Uint16)(utf8[i]&0x0F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else
		if ( ch >= 0xC0 ) {
			ch  =  (Uint16)(utf8[i]&0x1F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		}
		unicode[j] = ch;
	}
	unicode[j] = 0;

	return unicode;
}

int sub_disp(void)
{
	if ((quit_event == 1))
		return 1;

	if(vo_sub)
	{
		int l;
		int MaxWidth = 320 - leftgap - rightgap;
		int MaxHeight = 240 - topgap - bottomgap;
		int BeginWidth = leftgap;
		int BkColor = RGB2PIXEL(0xCC,0xCC,0xCC);
		int OutlineColor = RGB2PIXEL(outline_red, outline_green, outline_blue);
		int DrawColor = RGB2PIXEL(font_red, font_green, font_blue);
		int fc;
		int dw, dx, dy, cx, cy;
		int cont;
		int SubWidth = MaxWidth;
		int SubHeight = 0;
		unsigned short cont1[512];

		MaxWidth -= 2;MaxHeight -= 2; // for Outline
		BeginWidth++;
		dw = 0;dy = 1;fc = -1;cx = 0;cy = 0;

		if(OutlineColor == BkColor) OutlineColor++;
		if(DrawColor == BkColor) DrawColor++;
		if(DrawColor == OutlineColor) OutlineColor--;
		if(OutlineColor == -1) OutlineColor = 1;

		for(l=0; l<vo_sub->lines; l++)
		{ // Rendering Start
			int i = 0;
			int j;
			int nBegin;
			int FontHeight = 0;
			U8toU16(cont1, (Uint8 *)vo_sub->text[l], strlen(vo_sub->text[l]));

			if( cont1[i] == 0xFEFF) i++;
			nBegin = i;
			while (i < 512) {
				if( (cont1[i] >= 32) )
					GetFontSize(cont1[i], &cx, &cy);
				else
					cx = 0;
				if (cy > FontHeight)
					FontHeight = cy;
				if( (cont1[i] == 0) || (cont1[i] == 13) || ((dw + cx) > MaxWidth) ){
					unsigned short *buf = subtitle_buf;
					buf += dy * 320;
					for (j = 0;j < (FontHeight * 320);j++) {
						buf[j] = BkColor;
					}
					dx = ((MaxWidth - dw) >> 1) + BeginWidth;
					dw = cx;
					for (j = nBegin; j < i; j++) {
						if( (cont1[j] == 0) || (cont1[j] == 13) )
							break;
						if( (cont1[j] >= 32) ) {
							_DrawFont(dx, dy, cont1[j], DrawColor, BkColor, &cx, &cy);
							dx += cx;
						}
					}
					dy += FontHeight;
					if ( (cont1[i] == 0) || ((dy + FontHeight) > MaxHeight) )
						break;
					if ( (cont1[i] == 13) && (cont1[i+1] == 10) )
						i+=2;
					nBegin = i;
				} else {
					dw += cx;
					i++;
				}
			}
		} // Rendering End

		{
			unsigned short *buf = subtitle_buf;
			int j;
			for (j = 0;j < 320;j++) {
				buf[j] = BkColor;
			}
			buf += dy * 320;
			for (j = 0;j < 320;j++) {
				buf[j] = BkColor;
			}
		}

		SubHeight = dy + 1;

		{	// Draw Outline
			int x, y, i;
			unsigned short *src = subtitle_buf + 320 + leftgap + 1;
			const int OutLine[8] = {-321, -320, -319, -1, 1, 319, 320, 321};

			for (y=0;y<(SubHeight-2);y++) {
				for (x=0;x<(SubWidth-2);x++, src++) {
					if ((*src != OutlineColor) && (*src != BkColor)) {
						for(i=0;i<8;i++) {
							if(*(src + OutLine[i]) == BkColor)
								*(src + OutLine[i]) = OutlineColor;
						}
					}
				}
				src += rightgap + leftgap + 2;
			}
		}

		if(bMenuStatus)
			gfx_draw_sized_rect(psd, 0, 67, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));
		else
			gfx_draw_sized_rect(psd, 0, 0, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));
		BlitSubtitle(leftgap, 240 - ((SubHeight > minbottom ? SubHeight : minbottom) + bottomgap), SubWidth, SubHeight);
	}
	else
	{
		if(bMenuStatus)
			gfx_draw_sized_rect(psd, 0, 67, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));
		else
			gfx_draw_sized_rect(psd, 0, 0, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));
	}

}
