/*              
 *  All right reserved.
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. 2005.01.04
 */
#ifndef __FBS_H
#define __FBS_H

//==============================================================================

#define FBMMSP2CTRL             0x4619 

#define MMSP2_RGB0_NUM            0
#define MMSP2_RGB1_NUM            1
#define MMSP2_RGB2_NUM            2
#define MMSP2_RGB3_NUM            3
#define MMSP2_RGB4_NUM            4
#define MMSP2_OSD_NUM             5
#define MMSP2_VIDA_NUM
#define MMSP2_VIDB_NUM            6


typedef struct Ioctl_Msg_Hdr_t
{
    u_short message;
    u_short plane;   
    u_short msgleng;
} Msghdr;

typedef struct Ioctl_Dummy_t
{
    u_short message;
    u_short msgleng;
    u_char  msgdata[32];
} Msgdummy, *PMsgdummy;


#define MSG(msg) (((Msghdr *)msg)->message)
#define PLANE(msg) (((Msghdr *)msg)->plane)
#define LEN(msg) (((Msghdr *)msg)->msgleng)

#define MSGOK  1000
#define MSGERR 1001


#define MMSP2_FB_RGB_ON             1
#define MMSP2_FB_RGB_OFF            0
#define MMSP2_FB_GET_RECT_INFO      2
#define MMSP2_FB_SET_RECT_INFO      3
#define MMSP2_FB_RGB_COLOR_KEY		4

// from GP2X linux kerenel, drivers/video/mmsp2fb.h
#define MMSP2_FB0_TV_LCD_CHECK		10
#define MMSP2_FB0_GET_TV_MODE		13

typedef struct Ioctl_GetRect_t
{
    Msghdr mhdr;
    u_short xoff;
    u_short yoff;
    u_short w;
    u_short h;
} MsgGetRect, *PMsgGetRect;
//==============================================================================


#endif

