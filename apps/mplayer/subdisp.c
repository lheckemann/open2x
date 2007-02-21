/*
 *  - godori <ghcstop>, www.aesop-embedded.org
 *
 *    => Created. Jan, 2005
 */

#include <SDL.h>
#include "gvlib_export.h"


#include "subreader.h"
#include "fbdisp/gfxdev.h"
#include "fbdisp/fbs.h"
#include "subdisp.h"


extern subtitle *vo_sub;
extern int      subtitle_changed;

extern int g_FBStatus;

void framebuffer_on(int fbnum)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;

    if (fbnum == 0)
    {

        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_ON;
        LEN(pdmsg) = 0;
        if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("1 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

        g_FBStatus = 0x01;
    }
    else if (fbnum == 1)
    {

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

        memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
        pdmsg = &dummymsg;
        MSG(pdmsg) = MMSP2_FB_RGB_OFF;
        LEN(pdmsg) = 0;
        if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
        {
            printf("FBMMSP2CTRL error\n");
        }
        printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));

        g_FBStatus &= ~0x01;
    }
    else if (fbnum == 1)
    {

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


    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_OFF;
    LEN(pdmsg) = 0;
    if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));


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

int switch_fb1_to_fb0(void)
{
    int             err;
    Msgdummy        dummymsg,
                   *pdmsg;


    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
    pdmsg = &dummymsg;
    MSG(pdmsg) = MMSP2_FB_RGB_ON;
    LEN(pdmsg) = 0;
    if ((err = ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) < 0))
    {
        printf("FBMMSP2CTRL error\n");
    }
    printf("0 FBMMSP2CTRL rt msg = %d, size = %d\n", MSG(pdmsg), LEN(pdmsg));


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


#ifdef GDEBUG
extern FILE    *dbg;
	#define gprintf(x...) fprintf(dbg, x)
#else
	#define gprintf(x...)
#endif

#if 0

int sub_disp(void)
{
    if ((quit_event == 1))
        return 1;

    if (vo_sub)
    {
        int             l;

        for (l = 0; l < vo_sub->lines; l++)
        {
            printf("%s\n", vo_sub->text[l]);
        }
        printf("\n");
        subtitle_changed = 0;
    }
    else
    {

        printf("clear ==============================\n");
        subtitle_changed = 0;
    }
}

#else

int sub_disp(void)
{


    if ((quit_event == 1))
        return 1;

    if(vo_sub)
    {
        int             l;
        int             x = 0;
        int             y = 240-60;



        gfx_draw_sized_rect(psd, 0, 240-60, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));

        for(l=0; l<vo_sub->lines; l++)
        {
            gfx_set_fg(psd, RGB2PIXEL(10, 10, 10));
            gfx_set_fc(psd, WHITE);


        	x = (320-(strlen(vo_sub->text[l])*7))/2;
            gfx_draw_mtext(psd, x, y, vo_sub->text[l]);
            y += 20;
        }

        subtitle_changed = 0;
    }
    else
    {


        gfx_draw_sized_rect(psd, 0, 240-60, psd->planew, psd->planeh, RGB2PIXEL(0xCC,0xCC,0xCC));

        subtitle_changed = 0;
    }

}

#endif

