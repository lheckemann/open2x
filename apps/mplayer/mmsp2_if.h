/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */

#ifndef _MMSP2_IF_H_
#    define _MMSP2_IF_H_

#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>
#    include <unistd.h>
#    include <sys/types.h>
#    include <sys/mman.h>
#    include <fcntl.h>
#    include <sys/ioctl.h>
#    include <sys/stat.h>
#    include <time.h>           

#    include <signal.h>



#    include "dualcpu.h"
#    include "vpp.h"
#	 include "cx25874.h"

#    include "osdep/timer.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;




#    if 0

#        define MAX_DISP_Q  2   

typedef struct FDC_Disp_Q_t
{
    Qlist           link;       
    double          vpts;
    unsigned short  yoff;
    unsigned short  uoff;
    unsigned short  voff;
} Dispq        , *PDispq;

void            DISPQ_q_init(void);
PDispq          get_DISPQ_from_qpool(void);
void            free_DISPQ_to_qpool(PDispq pdispq);




#        define MAX_FRAME_Q  4

typedef struct FRAME_Q_t
{
    Qlist           link;       
    unsigned char  *pbuf;
} FRAME        , *PFRAME;

void            FRAME_q_init(void);
PFRAME          get_FRAME_from_qpool(void);
void            free_FRAME_to_qpool(PFRAME pframe);


#    endif



inline int      put_vpts_readq(float pts);
inline float    get_vpts_readq(void);
void            init_vpts_readq(void);

inline int      put_nft_readq(float pts);   
inline float    get_nft_readq(void);
void            init_nft_readq(void);



#    define CODEC_BUFF_ADDR 0x02000000


#    define PATH_DEV_DUALCPU              "/dev/dualcpu"
#    define PATH_DEV_DUALCPU_WITH_DEVFS   "/dev/misc/dualcpu"
                                                            
#    define PATH_DEV_VPP                  "/dev/vpp"
#    define PATH_DEV_VPP_WITH_DEVFS       "/dev/misc/vpp"

#    define PATH_DEV_CX25874                  "/dev/cx25874"
#    define PATH_DEV_CX25874_WITH_DEVFS       "/dev/misc/cx25874"



#    define MAX_WIDTH  1024
#    define MAX_HEIGHT 512
#    define MP4D_CODEC_INFO_SIZE 10


#if defined(DTK3)
	#define LCD_WIDTH 	800
	#define LCD_HEIGHT 	600
#elif defined(DTK4)
	#define LCD_WIDTH 	640
	#define LCD_HEIGHT 	480
#elif defined(WDMS)
	#define LCD_WIDTH 	320
	#define LCD_HEIGHT 	240
#endif

#define MONITOR_WIDTH 	800
#define MONITOR_HEIGHT 	600


#define TV_NTSC_ENC_WIDTH 	670		
#define TV_NTSC_ENC_HEIGHT 	(232*2)
#define TV_PAL_ENC_WIDTH 	670		
#define TV_PAL_ENC_HEIGHT 	(280*2)

#    ifndef mmioFOURCC
#        define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (unsigned int)(char)(ch0) | ( (unsigned int)(char)(ch1) << 8 ) |  \
        ( (unsigned int)(char)(ch2) << 16 ) | ( (unsigned int)(char)(ch3) << 24 ) )
#    endif


#if 0 
unsigned char  *in_buf[4];      
#else
unsigned char  *dbuf;           
#endif
MP4D_DISPLAY_PARAM mp4d_disp;   

struct mp4d_codec_type_t
{
    unsigned int    format;
    short           number;
};


int             mmsp2_940_interface_buffer_init(void);
int             mmsp2_setup_940_interface_buffer(int width, int height, unsigned int vformat_fourcc);
int             open_dualcpu_vpp(void);
void            close_dualcpu_vpp_unmap(void);
inline int      read_mp4d_940_status(MP4D_DISPLAY_PARAM * mp4d_940);
int             set_FDC_YUVB_plane(int width, int height);


#    if 0
inline int      run_940_decoder(int frame_index);
#    else
inline int      run_940_decoder(void);

#    endif



inline void     run_FDC_and_display(unsigned short yoffset, unsigned short cboffset, unsigned short croffset);

int             run_940_decoder_onlyrun(void);




#endif                          
