/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 */

#ifndef _G2PLAYER_H_
#    define _G2PLAYER_H_

#    include <SDL.h>
#    include <SDL_thread.h>
#    include "libmpdemux/stream.h"
#    include "libmpdemux/demuxer.h"
#    include "libmpdemux/stheader.h"
#    include "libmpcodecs/dec_audio.h"
#    include "libmpcodecs/dec_video.h"
#    include "libmpcodecs/mp_image.h"
#    include "libmpcodecs/vf.h"
#    include "mmsp2_if.h"
#    define GVLIB 1
#    include "gvlib_export.h"

#    define SDL_AUDIO_BUFFER_SIZE 4096

typedef struct vs_t
{
    int             si;
    SDL_Thread     *audio_tid;
    SDL_Thread     *video_tid;
    SDL_Thread     *demux_tid;
    int             aq;
    int             vq;
    int             tr;
    int             vr;
    SDL_mutex      *drm;
    SDL_cond       *drc;
    float           sv;
    float           dsat;  
    float           by;
    float           rtc_timer;
    float           first_rtc_time; 
    int             width,
                    height;
    demux_stream_t *dmuxst_audio;   
    demux_stream_t *dmuxst_video;   
    sh_audio_t     *audio_sh;   
    sh_video_t     *video_sh;   
    ao_functions_t *current_ao; 
    int             se;     
    float           sn;       
    float           dss;   
    int             sp; 
    int             dems;   
    SDL_mutex      *sm;      
    SDL_mutex       *vsm;    
    SDL_cond        *vsc;
    int vs;
    SDL_mutex *arm;           
    SDL_cond  *arc;
    int ar;
    int c;
} VS;


SDL_mutex      *disp_mutex;
SDL_cond       *disp_cond;


#endif                          
