/* 
 * Original decode routine from Mplayer < www.mplayerhq.hu >             
 *
 *  - www.aesop-embedded.org
 *             godori <ghcstop>
 *
 *    => 2005.01.04 remake main decode routine for mmsp2 HW 
 *
 *    => DIGNSYS Inc. < www.dignsys.com > maintaining from April 2005
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#include "version.h"
#include "config.h"

#include "mp_msg.h"

#define HELP_MP_DEFINE_STATIC
#include "help_mp.h"

#include "libao2/audio_out.h"

#include "codec-cfg.h"

#include "osdep/getch2.h"
#include "osdep/timer.h"


#ifdef HAVE_MATROSKA
#    include "libmpdemux/matroska.h"
#endif

#include <math.h>
#include "g2player.h"
#include "rtc_1024_table.h"
#include "glock.h"
#include "get_path.c"
#include "mixer.h"


#define dsprintf(x...)
#define dfprintf(x...)
#define gprintf(x...)

struct mmsp2_rtc_info
{
    unsigned long   sec_count;
    unsigned short  msec_count;
};

#define PATH_DEV_RTC                  "/dev/rtc"
#define PATH_DEV_RTC_WITH_DEVFS       "/dev/misc/rtc"   

#define IOCTL_MMSP2_READ_RTC    _IOR('p', 0x13, struct mmsp2_rtc_info)

#undef USE_VPTS_Q               
#define USE_VPTS_Q              


#define PT_NEXT_ENTRY 	1
#define PT_PREV_ENTRY 	-1
#define PT_NEXT_SRC 	2
#define PT_PREV_SRC 	-2
#define PT_UP_NEXT 		3
#define PT_UP_PREV 		-3



#define INITED_VO 		1
#define INITED_AO 		2
#define INITED_GUI 		4
#define INITED_GETCH2 	8
#define INITED_SPUDEC 	32
#define INITED_STREAM 	64
#define INITED_INPUT    128
#define INITED_VOBSUB  	256
#define INITED_DEMUXER 	512
#define INITED_ACODEC  	1024
#define INITED_VCODEC  	2048
#define INITED_ALL 		0xFFFF
void ivs(VS * is);

static int video_thread(void *arg);
static int audio_thread(void *arg);


inline int mmsp2_mp4_video_decode(VS * is, unsigned char *buf, int data_length);
inline int pre_process(VS * is, unsigned short yoffset, unsigned short cboffset, unsigned short croffset, int display);
inline int post_process(VS * is);
void dss(void); 
VS      vs;         

static int      rtc_fd = 0;
static struct mmsp2_rtc_info rtcinfo;


int verbose 	= 0;
int identify 	= 0;

SDL_mutex      *apm;

float 	nat = 0;
int		ftr = 0;   
float	time_frame = 0; 

int		delay_corrected = 1;

static int      drop_frame_cnt = 0; 
static float    default_max_pts_correction = -1; 
static float    max_pts_correction = 0;
static float    c_total = 0;
float           AV_delay = 0;   

static int      total_frame_cnt = 0;
static int      nframes = 0;    
float           rel_seek_secs = 0;
int             abs_seek_pos = 0;

static 			SyncLock slock;

#ifdef USE_SUB
	#include "subreader.h"
#endif

char          **sub_name = NULL;
float           sub_delay = 0;
float           sub_fps = 0;
int             sub_auto = 1;
char           *vobsub_name = NULL;


int             subcc_enabled = 0;
int             suboverlap_enabled = 1;


#ifdef USE_SUB
sub_data       *set_of_subtitles[MAX_SUBTITLE_FILES];
int             set_of_sub_size = 0;
int             set_of_sub_pos = -1;
float           sub_last_pts = -303;

sub_data       *subdata = NULL; 
char           *mem_ptr;        
#endif

int             global_sub_size = 0;  
int             global_sub_pos = -1;  

#define SUB_SOURCE_SUBS 0
#define SUB_SOURCE_VOBSUB 1
#define SUB_SOURCE_DEMUX 2
#define SUB_SOURCES 3

int             global_sub_indices[SUB_SOURCES];
int             global_sub_quiet_osd_hack = 0;

subtitle       *vo_sub = NULL;
subtitle       *vo_sub2 = NULL;
int             subtitle_changed = 0;
SDL_Event       subchange;

void            select_subtitle(void);

static char    *stream_dump_name = "stream.dump"; 
int             stream_dump_type = 0;
int             eof = 1;

float           playback_speed = 1.0;   
float           audio_delay = 0.0;


static char    *seek_to_sec = NULL;
static off_t    seek_to_byte = 0;
static off_t    step_sec = 0;
static int      loop_times = -1;
static int      loop_seek = 0;


char          **audio_codec_list = NULL; 
char          **video_codec_list = NULL; 
char          **audio_fm_list = NULL;  
char          **video_fm_list = NULL;  


int             audio_id 		= -1;
int             video_id 		= -1;
int             dvdsub_id 		= -1;
int             vobsub_id 		= -1;
char           	*audio_lang 	= NULL;
char           	*dvdsub_lang 	= NULL;
static char    	*spudec_ifo 	= NULL;
char           	*filename 		= NULL;    
int             forced_subs_only = 0;



int             cache_size = -1;

#ifdef USE_STREAM_CACHE
	extern int cache_fill_status;
#else
	#define cache_fill_status 0
#endif

static int      softsleep = 0;

float           force_fps = 0;
static int      force_srate = 0;
static int      audio_output_format = 0;


char          **video_driver_list = NULL;
#if 0
    char          **audio_driver_list = NULL;
#else
    char           *audio_driver_list[1] = { "oss" };
#endif

static stream_t   *stream 		= NULL;
static demuxer_t  *demuxer 		= NULL;
static sh_audio_t *sh_audio 	= NULL;
static sh_video_t *sh_video 	= NULL;
ao_functions_t    *audio_out 	= NULL;


static demux_stream_t *d_audio = NULL;  
static demux_stream_t *d_video = NULL;
static demux_stream_t *d_dvdsub = NULL;

char           *current_module = NULL;  


static unsigned int inited_flags = 0;
extern int      audio_output_channels;
#include "typed.h"

extern int		NowTick			;
extern int 		TotalPlayTime	;
extern float	CurrentPlayTime	;
extern int 		g_Kbps			;
extern int 		g_Khz			;
extern char		*g_pTagTitle 	;
extern int 		g_stero			;
extern int 		g_repeate		;
extern int 		g_shuf			;
extern Equlize 	g_eq			;

extern float g_fResumePlayTime;
extern bool g_bResumePlay;
extern SDL_Event *resume_open_event;

int pause_flag 			= 0;			
int g_Timesec			= 0;
extern int GetMenuStatus();
extern bool	bMenuStatus;
extern g_NotSupported;

static mixer_t 	  mixer;
int leftVol		= 70;

void hwEq_change(int eq_flag)
{
	mixer_hwEq(&mixer, eq_flag); 
}

void reset_player_variable(void)
{
    exit_media_play();

	pause_flag 			= 0;			
	g_Timesec			= 0;                                      
	TotalPlayTime		= 0 ;                                     
	CurrentPlayTime		= 0 ;                                     	
	g_Kbps				= 0 ;                                     	
	g_Khz				= 0 ;                                     	
	g_Timesec			= 0;
	g_NotSupported		= false;
	
    memset((char *) &vs, 0x00, sizeof(VS)); 
    
    rtc_fd = 0;
    
    verbose 	= 0;
    identify 	= 0;
    
    
    nat 		= 0;
    ftr 	= 0;
    time_frame 				= 0;
    
    delay_corrected = 1;
    
    drop_frame_cnt 				= 0; 
    default_max_pts_correction 	= -1;
    max_pts_correction 			= 0; 
    c_total 	= 0;
    AV_delay 	= 0;   
    
    total_frame_cnt = 0;
    nframes 		= 0;    
    
    rel_seek_secs 	= 0;
    abs_seek_pos 	= 0;
    
    sub_name 	= NULL;
    sub_delay 	= 0;
    sub_fps 	= 0;
    sub_auto 	= 1;
    vobsub_name = NULL;
    
    subcc_enabled 		= 0;
    suboverlap_enabled 	= 1;
    
    #ifdef USE_SUB
    set_of_sub_size = 0;
    set_of_sub_pos 	= -1;
    sub_last_pts 	= -303;
    
    subdata = NULL;
    #endif
    
    global_sub_size = 0;
    global_sub_pos = -1;
    
    global_sub_quiet_osd_hack = 0;
    
    vo_sub = NULL;
    subtitle_changed = 0;
    stream_dump_type = 0;

    eof = 1;
    
    playback_speed 	= 1.0;
    audio_delay 	= 0.0;
    
    seek_to_sec 	= NULL;
    seek_to_byte 	= 0;
    step_sec 		= 0;
    loop_times 		= -1;
    loop_seek 		= 0;
    
    audio_id 	= -1;
    video_id 	= -1;
    dvdsub_id 	= -1;
    vobsub_id 	= -1;
    audio_lang 	= NULL;
    dvdsub_lang = NULL;
    spudec_ifo 	= NULL;
    filename 	= NULL;    
    forced_subs_only = 0;
    
    cache_size = -1;
    softsleep = 0;
    
    force_fps 	= 0;
    force_srate = 0;
    audio_output_format = 0;
    
    stream 		= NULL;
    demuxer 	= NULL;
    sh_audio 	= NULL;
    sh_video 	= NULL;
    audio_out 	= NULL;
    
    d_audio 	= NULL;  
    d_video 	= NULL;
    d_dvdsub 	= NULL;
    
    current_module = NULL;  
    
    inited_flags = 0;
    
    set_lock_value( &slock, SFULL ); 
}

static void uninit_player(unsigned int mask)
{
    mask = inited_flags & mask;

    if(mask & INITED_ACODEC)
    {
        inited_flags &= ~INITED_ACODEC;
        current_module = "uninit_acodec";

        if(sh_audio)	uninit_audio(sh_audio);
        sh_audio = NULL;
    }

    if(mask & INITED_DEMUXER)
    {
        inited_flags &= ~INITED_DEMUXER;
        current_module = "free_demuxer";

        if(demuxer)
        {
            stream = demuxer->stream;
            free_demuxer(demuxer);
        }
        demuxer = NULL;
    }

    if(mask & INITED_STREAM)
    {
        inited_flags &= ~INITED_STREAM;
        current_module = "uninit_stream";

        if(stream)	free_stream(stream);
        stream = NULL;
    }

    if(mask & INITED_AO)
    {
        inited_flags &= ~INITED_AO;
        current_module = "uninit_ao";


#ifdef CVS_040109
        audio_out->uninit();
#else
        audio_out->uninit(1);   
#endif
        audio_out = NULL;
    }

    sh_video = NULL;

#ifdef USE_SUB
    if(set_of_sub_size > 0)
    {
        int             i;

        current_module = "sub_free";
        for (i = 0; i < set_of_sub_size; ++i)
            sub_free(set_of_subtitles[i]);

        set_of_sub_size = 0;
        vo_sub = NULL;
        subdata = NULL;
    }
#endif
    current_module = NULL;
}

static void exit_player_with_rc(char *how, int rc)
{
    if(vs.audio_tid)
    {
        SDL_WaitThread(vs.audio_tid, NULL);
    }

    if(vs.video_tid)
    {
		if(vs.vr)	
		{
	        printf("video thread wait\n");
	        SDL_WaitThread(vs.video_tid, NULL);
		}
        printf("mmsp2 engine shutdown...\n");
        close_dualcpu_vpp_unmap();  
    }
    
    if(vs.demux_tid)
    {
        printf("demux thread wait\n");
        SDL_WaitThread(vs.demux_tid, NULL);
    }

    if(apm)
        SDL_DestroyMutex(apm);

    if(vs.drm)
    {
        SDL_DestroyMutex(vs.drm);
        SDL_DestroyCond(vs.drc);
    }

    if(vs.sm)
    {
        SDL_DestroyMutex(vs.sm);
        SDL_DestroyMutex(vs.vsm);
        SDL_DestroyCond(vs.vsc);

        SDL_DestroyMutex(vs.arm);
        SDL_DestroyCond(vs.arc);
    }

    if(rtc_fd > 0)
    {
        printf("rtc close\n");
        close(rtc_fd);
    }

    uninit_player(INITED_ALL);

    current_module = "exit_player";
}

void exit_media_play(void)
{
    exit_player_with_rc(MSGTR_Exit_eof, 0);
}

void exit_player(char *how)
{
    exit_player_with_rc(how, 1);
}

static int libmpdemux_was_interrupted(int eof)
{
#if 0                           
    mp_cmd_t       *cmd;

    if ((cmd = mp_input_get_cmd(0, 0)) != NULL)
    {
        switch (cmd->id)
        {
        case MP_CMD_QUIT:
            exit_player_with_rc(MSGTR_Exit_quit, 0);
        case MP_CMD_PLAY_TREE_STEP:
            eof = (cmd->args[0].v.i > 0) ? PT_NEXT_ENTRY : PT_PREV_ENTRY;
            break;
        case MP_CMD_PLAY_TREE_UP_STEP:
            {
                eof = (cmd->args[0].v.i > 0) ? PT_UP_NEXT : PT_UP_PREV;
            }
            break;
        case MP_CMD_PLAY_ALT_SRC_STEP:
            {
                eof = (cmd->args[0].v.i > 0) ? PT_NEXT_SRC : PT_PREV_SRC;
            }
            break;
        }
        mp_cmd_free(cmd);
    }
#endif
    return eof;
}


int sub_source()
{
    int             source = -1;
    int             top = -1;
    int             i;

    for (i = 0; i < SUB_SOURCES; i++)
    {
        int             j = global_sub_indices[i];

        if ((j >= 0) && (j > top) && (global_sub_pos >= j))
        {
            source = i;
            top = j;
        }
    }
    return source;
}

#ifdef USE_SUB

void add_subtitles(char *filename, float fps, int silent)
{
    sub_data       *subd;

    if (filename == NULL)
    {
        return;
    }

    subd = sub_read_file(filename, fps);
    if (!subd && !silent)
        mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_CantLoadSub, filename);
    if (subd == NULL || set_of_sub_size >= MAX_SUBTITLE_FILES)
        return;
    set_of_subtitles[set_of_sub_size] = subd;
    ++set_of_sub_size;
    mp_msg(MSGT_FIXME, MSGL_FIXME, MSGTR_AddedSubtitleFile, set_of_sub_size, filename);
}


void update_set_of_subtitles() 	
{
    int             i;

    if (set_of_sub_size > 0 && subdata == NULL)
    {                           
        for (i = set_of_sub_pos + 1; i < set_of_sub_size; ++i)
            set_of_subtitles[i - 1] = set_of_subtitles[i];
        set_of_subtitles[set_of_sub_size - 1] = NULL;
        --set_of_sub_size;
        if (set_of_sub_size > 0)
            subdata = set_of_subtitles[set_of_sub_pos = 0];
    }
    else if (set_of_sub_size > 0 && subdata != NULL)
    {                           
        set_of_subtitles[set_of_sub_pos] = subdata;
    }
    else if (set_of_sub_size <= 0 && subdata != NULL)
    {                           
        set_of_subtitles[set_of_sub_pos = set_of_sub_size] = subdata;
        ++set_of_sub_size;
    }
}

#endif

int init_etc_setting(void)
{
#ifdef GDEBUG
    dbg = fopen("debug.txt", "wt");
    if(dbg == NULL)
    {
        printf("debug file open error\n");
        exit(1);
    }
#endif

    srand((int) time(NULL));
    gprintf("d1\n");
    mp_msg_init();
    mp_msg_set_level(MSGL_INFO);    
    gprintf("d2\n");
    all_codec_register();       
    bin_lock_init( &slock, SFULL ); 

    return 1;
}

void close_etc_setting(void)
{
    lock_destroy(&slock);       
    
#ifdef GDEBUG
    printf("debug.txt close()\n");
    fclose(dbg);
#endif

}

int init_media_play(char* ifile)
{
    int             file_format = DEMUXER_TYPE_UNKNOWN;
    int             i;
    int             gui_no_filename = 0;

    filename = ifile;

    if(access(filename, F_OK) != 0)
    {
        printf("cannot access: %s\n", filename);
        return -1;
    }

    gprintf("d3\n");

    if(!filename)
    {
        mp_msg(MSGT_CPLAYER, MSGL_INFO, help_text);
        exit(0);
    }

    mp_msg_set_level(verbose + MSGL_STATUS);
    current_module = NULL;
play_next_file:
	sub_fps = 0;
    global_sub_size = 0;
    {
        int i = 0;
        for(i = 0; i < SUB_SOURCES; i++)
            global_sub_indices[i] = -1;
    }
    global_sub_quiet_osd_hack = 1;

    if(filename)
        mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_Playing, filename);

    stream 	= NULL;
    demuxer = NULL;
    
    if(d_audio){d_audio = NULL;}
    if(d_video){d_video = NULL;}
    
    sh_audio = NULL;
    sh_video = NULL;

    gprintf("d5\n");
    stream = open_stream(filename, 0, &file_format);
    
    
    if(!stream)                
    {
#if 0                           
        eof = libmpdemux_was_interrupted(PT_NEXT_ENTRY);
#else
        libmpdemux_was_interrupted(PT_NEXT_ENTRY);
#endif
        goto goto_next_file;
    }
    inited_flags |= INITED_STREAM;
    stream->start_pos += seek_to_byte;
    gprintf("d6\n");
    demuxer = demux_open(stream, file_format, audio_id, video_id, dvdsub_id, filename);
    if(!demuxer)
    {
        goto goto_next_file;
    }


    inited_flags |= INITED_DEMUXER;
    d_audio 	= demuxer->audio;
    d_video 	= demuxer->video;
    d_dvdsub 	= demuxer->sub;
    sh_audio = d_audio->sh;
    sh_video = d_video->sh;

    gprintf("d7\n");
    
    if(sh_video)
    {
        if (!video_read_properties(sh_video))
        {
            mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_CannotReadVideoProperties);
			sh_video = d_video->sh = NULL;
        }
        else
        {
#if 1
            mp_msg(MSGT_CPLAYER, MSGL_V, "[V] filefmt:%d  fourcc:0x%X  size:%dx%d  fps:%5.2f  ftime:=%6.4f\n",
                   demuxer->file_format, sh_video->format, sh_video->disp_w, sh_video->disp_h, sh_video->fps, sh_video->frametime);
            
#endif
            if (force_fps)
            {
                sh_video->fps = force_fps;
                sh_video->frametime = 1.0f / sh_video->fps;
            }

            if (!sh_video->fps && !force_fps)
            {
                mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_FPSnotspecified);
                sh_video = d_video->sh = NULL;
            }
        }
    }                           

    fflush(stdout);

    
    if(!sh_video && !sh_audio)
    {
        mp_msg(MSGT_CPLAYER, MSGL_FATAL, MSGTR_NoStreamFound);
        goto goto_next_file;    
    }

    gprintf("d8\n");
    demux_info_print(demuxer);
    gprintf("d9\n");

#if 1
#ifdef USE_SUB
    
    if(sh_video)
    {
        if(sub_name)           
        {
            for(i = 0; sub_name[i]!=NULL; ++i)
                add_subtitles(sub_name[i], sh_video->fps, 0);
        }

        
        if(sub_auto)           
        {
            char *psub = get_path("sub/");
            char **tmp  = sub_filenames((psub ? psub : ""), filename);
            char **tmp2 = tmp;

            free(psub);         
            
            while(*tmp2)
                add_subtitles(*tmp2++, sh_video->fps, 0);

            free(tmp);
            if(set_of_sub_size == 0)
            {
                add_subtitles(mem_ptr = get_path("default.sub"), sh_video->fps, 1);
                free(mem_ptr);  
            }

            if(set_of_sub_size > 0)
				add_subtitles(NULL, sh_video->fps, 1);
        }

        if(set_of_sub_size > 0)
        {
			
			global_sub_indices[SUB_SOURCE_SUBS] = global_sub_size;  
			global_sub_size += set_of_sub_size;
        }
    }
#endif

    gprintf("d10\n");
    if(global_sub_size)
    {
        if(vobsub_id >= 0)
        {
            global_sub_pos = global_sub_indices[SUB_SOURCE_VOBSUB] + vobsub_id;
        }
        else if(dvdsub_id >= 0 && global_sub_indices[SUB_SOURCE_DEMUX] >= 0)
        {
            global_sub_pos = global_sub_indices[SUB_SOURCE_DEMUX] + dvdsub_id;
        }
        else if(global_sub_indices[SUB_SOURCE_SUBS] >= 0)
        {
            global_sub_pos = global_sub_indices[SUB_SOURCE_SUBS];
        }
        else
        {
            global_sub_pos = -1;
        }
        
        global_sub_pos--;
    }

    select_subtitle();
    
#endif    
    
    gprintf("d11\n");

    if(d_audio)		vs.dmuxst_audio = d_audio;
    if(d_video)		vs.dmuxst_video = d_video;
    if(sh_audio)	vs.audio_sh = sh_audio;

#ifdef WMV_TRACE

    sh_video = NULL;
    vs.video_sh = sh_video;
#else
    if(sh_video)	vs.video_sh = sh_video;
#endif

    gprintf("d12\n");

    if(sh_audio)
    {
        mp_msg(MSGT_CPLAYER, MSGL_INFO, "========================================================================\n");
        if(!init_best_audio_codec(sh_audio, audio_codec_list, audio_fm_list))
        {
            printf("\n\ninit_best_audio_codec load fail\n\n");
            sh_audio = d_audio->sh = NULL;  
            
            goto mmsp2out;
            
        }
        else
            inited_flags |= INITED_ACODEC;

        mp_msg(MSGT_CPLAYER, MSGL_INFO, "========================================================================\n");
    }

    
    identify = 0;               
    
    if(identify)
    {
        mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_FILENAME=%s\n", filename);
        if (sh_video)
        {
            if (sh_video->format >= 0x20202020)
                mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_FORMAT=%.4s\n", &sh_video->format);
            else
                mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_FORMAT=0x%08X\n", sh_video->format);

            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_BITRATE=%d\n", sh_video->i_bps * 8);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_WIDTH=%d\n", sh_video->disp_w);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_HEIGHT=%d\n", sh_video->disp_h);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_FPS=%5.3f\n", sh_video->fps);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_VIDEO_ASPECT=%1.4f\n", sh_video->aspect);
        }

        if(sh_audio)
        {
            if (sh_audio->codec)
                mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_CODEC=%s\n", sh_audio->codec->name);

            if (sh_audio->format >= 0x20202020)	mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_FORMAT=%.4s\n"	, &sh_audio->format	);
            else								mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_FORMAT=%d\n"	, sh_audio->format	);

            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_BITRATE=%d\n", sh_audio->i_bps * 8);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_RATE=%d\n", sh_audio->samplerate);
            mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_AUDIO_NCH=%d\n", sh_audio->channels);
        }
        mp_msg(MSGT_GLOBAL, MSGL_INFO, "ID_LENGTH=%ld\n", demuxer_get_time_length(demuxer));
    }

    
    if(!sh_video)
        goto main;

main:
    fflush(stdout);

    gprintf("d8\n");
    {
        if (sh_audio)
        {
            ao_data.samplerate = force_srate ? force_srate : sh_audio->samplerate * playback_speed;
            ao_data.channels = audio_output_channels ? audio_output_channels : sh_audio->channels;
            ao_data.format = audio_output_format ? audio_output_format : sh_audio->sample_format;
#if 1
            
            if (!preinit_audio_filters(sh_audio,
                                       (int) (sh_audio->samplerate * playback_speed), sh_audio->channels, sh_audio->sample_format, sh_audio->samplesize,
                                       &ao_data.samplerate, &ao_data.channels, &ao_data.format, audio_out_format_bits(ao_data.format) / 8))
            {
                mp_msg(MSGT_CPLAYER, MSGL_ERR, "Error at audio filter chain pre-init!\n");
            }
            else
            {
#    if 1                       
                
                mp_msg(MSGT_CPLAYER, MSGL_INFO, "AF_pre: %dHz %dch %s\n", ao_data.samplerate, ao_data.channels, audio_out_format_name(ao_data.format));
#    endif
            }
#endif

            if (!(audio_out = init_best_audio_out(audio_driver_list, 0, 
                                                  force_srate ? force_srate : ao_data.samplerate,
                                                  audio_output_channels ? audio_output_channels : ao_data.channels, audio_output_format ? audio_output_format : ao_data.format, 0)))
            {
                
                mp_msg(MSGT_CPLAYER, MSGL_ERR, MSGTR_CannotInitAO);
                uninit_player(INITED_ACODEC);   

                sh_audio = d_audio->sh = NULL;  
            }
            else
            {
     			mixer.audio_out = audio_out;		
        		mixer_setvolume(&mixer, leftVol, leftVol);	
        		hwEq_change(0);		
            	
                inited_flags |= INITED_AO;
#if 0           
                
                mp_msg(MSGT_CPLAYER, MSGL_INFO, "AO: [%s] %dHz %dch %s (%d bps)\n",
                       audio_out->info->short_name, ao_data.samplerate, ao_data.channels, audio_out_format_name(ao_data.format), audio_out_format_bits(ao_data.format) / 8);

                mp_msg(MSGT_CPLAYER, MSGL_V, "AO: Description: %s\nAO: Author: %s\n", audio_out->info->name, audio_out->info->author);

                if (strlen(audio_out->info->comment) > 0)
                    mp_msg(MSGT_CPLAYER, MSGL_V, "AO: Comment: %s\n", audio_out->info->comment);
#endif

                
#if 1
                current_module = "af_init";
                if (!init_audio_filters(sh_audio, (int) (sh_audio->samplerate * playback_speed), sh_audio->channels, sh_audio->sample_format, sh_audio->samplesize, ao_data.samplerate, ao_data.channels, ao_data.format, audio_out_format_bits(ao_data.format) / 8,    
                                        ao_data.outburst * 4, ao_data.buffersize))
                {
                    mp_msg(MSGT_CPLAYER, MSGL_ERR, "Couldn't find matching filter / ao format!\n");
                }
#endif
            }                   
        }                       

        current_module = "av_init";
        if(sh_video)
            sh_video->timer = 0;

        
        if(sh_audio)
        {
            sh_audio->delay = -audio_delay; 
        }


        
        if (!sh_audio)
        {
            mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_NoSound);
            mp_msg(MSGT_CPLAYER, MSGL_V, "Freeing %d unused audio chunks\n", d_audio->packs);
            ds_free_packs(d_audio); 
            d_audio->id = -2;   
            
        }

        if (!sh_video)
        {
            mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_Video_NoVideo);
            mp_msg(MSGT_CPLAYER, MSGL_V, "Freeing %d unused video chunks\n", d_video->packs);
            ds_free_packs(d_video); 
            d_video->id = -2;
        }

        
        if (!sh_video && !sh_audio)
            goto goto_next_file;

        if (force_fps && sh_video)
        {
            sh_video->frametime = 1.0f / sh_video->fps;
            mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_FPSforced, sh_video->fps, sh_video->frametime);
        }

        if (loop_times > 1)
            loop_times--;
        else if (loop_times == 1)
            loop_times = -1;

        mp_msg(MSGT_CPLAYER, MSGL_INFO, MSGTR_StartPlaying);
        fflush(stdout);
        InitTimer();
#ifdef USE_DVDNAV
        if (stream->type == STREAMTYPE_DVDNAV)
        {
            dvdnav_stream_fullstart((dvdnav_priv_t *) stream->priv);
        }
#endif

        
        

#if 0
        play_n_frames = play_n_frames_mf;
        if (play_n_frames == 0)
        {
            eof = PT_NEXT_ENTRY;
            goto goto_next_file;
        }
#endif
        if (audio_out)
            vs.current_ao = audio_out;
        if (sh_video)
        {
            if (mmsp2_940_interface_buffer_init() < 0)
            {
                printf("interface buffer init error\n");
                goto mmsp2out;
            }

            if (open_dualcpu_vpp() < 0)
            {
                printf("dualcpu vpp device open error\n");
                goto mmsp2out;
            }
            printf("width = %d, height = %d, comp: %u\n", sh_video->disp_w, sh_video->disp_h, sh_video->format);

            if(mmsp2_setup_940_interface_buffer(sh_video->disp_w, sh_video->disp_h, sh_video->format) < 0)
            {
                printf("buffer setup failed\n");
                goto mmsp2out;
            }
            {
                int             real_width,
                                real_height;
                real_width = ((sh_video->disp_w >> 4) << 4);
                real_height = ((sh_video->disp_h >> 4) << 4);
                set_FDC_YUVB_plane(real_width, real_height);
            }
        }
        
        rtc_fd = open(PATH_DEV_RTC, O_RDWR);
        if (rtc_fd < 0)
        {
            rtc_fd = open(PATH_DEV_RTC_WITH_DEVFS, O_RDWR);
            if (rtc_fd < 0)
            {
                printf("RTC device open error\n");
                goto mmsp2out;
            }
        }

        printf("RTC open ==\n");
        printf("================================================\n");
        printf("  RTC Test Test\n");
        printf("================================================\n");

        ioctl(rtc_fd, IOCTL_MMSP2_READ_RTC, &rtcinfo);
        printf("RTCTCNT = %u, RTCSTCNT = %u\n", rtcinfo.sec_count, rtcinfo.msec_count);

        ivs(&vs);  
    }                           
    
    return 1;                   


mmsp2out:
    close_dualcpu_vpp_unmap();  
	g_NotSupported = true;

goto_next_file:              
    uninit_player(INITED_ALL - (INITED_GUI + INITED_INPUT + 0));    

#if 0                           
    if (use_gui || playtree_iter != NULL)
    {
        eof = 0;
        goto play_next_file;
    }
#endif

    exit_player_with_rc(MSGTR_Exit_eof, 0);
    return -1;                  

}

void ivs(VS * is)
{
    is->tr = 0;
    is->drm = SDL_CreateMutex();
    is->drc = SDL_CreateCond();
    is->se = 0;
    is->sn = 0.0;
    is->c = 0;
    apm = SDL_CreateMutex();

    if( is->video_sh && is->audio_sh ) 	is->vr = 0;
    else						        is->vr = 1; 

    is->aq = 0;
    is->vq = 0;

    is->si = 1;

    
    is->dss 	= 0.0;
    is->sp = 0;
    is->dems 	= 0;

    is->vs 	= 0;
    is->ar 	= 0;

    is->sm 	= SDL_CreateMutex();
    
    is->vsm = SDL_CreateMutex();
    is->vsc	= SDL_CreateCond();

    is->arm	= SDL_CreateMutex();
    is->arc	= SDL_CreateCond();
    

    if (is->video_sh)	is->video_tid = SDL_CreateThread(video_thread, is); 
    if (is->audio_sh)	is->audio_tid = SDL_CreateThread(audio_thread, is); 
}

static int audio_thread(void *arg)
{
    VS     *is = arg;
    SDL_Event       event;
    int             ret;
    unsigned int    t;
    double          tt;
    int             playsize;
	char tmp[256];
	int TimeSec;
	int  LengthInSec;
    float           spts;
    float           in_bufpts,
                    device_delay,
                    obuf_delay;

	
	if( sh_audio  ) 
	{
		LengthInSec=demuxer_get_time_length(demuxer);
		TotalPlayTime = LengthInSec;
		
		memset( tmp, 0x00, sizeof(tmp) );
		snprintf( tmp,sizeof( tmp ),"%02d:%02d:%02d", LengthInSec/3600, LengthInSec/60%60, LengthInSec%60 );
		
		printf("\n==============================\n");
		printf("Total Playing Time: %s, %d, %d\n", tmp, TotalPlayTime, LengthInSec);
		printf("==============================\n\n");
	
	}

    is->audio_sh->delay = 0.0;
    while (1)
    {
		while(pause_flag)	SDL_Delay(1);	

        if (quit_event == 1)
            goto break_signal;
        
        SDL_LockMutex(is->sm);
        if( is->se == 1 )
        {
            if( is->sp == 1 ) 
            {
                SDL_UnlockMutex(is->sm);
                continue;
            }
            is->dems      = 1;
            is->sp    = 1;
            is->tr = 0; 
            
            if( sh_video)
            {
                is->vr = 0;
                SDL_LockMutex(is->vsm);
                while (is->vs == 0 )  
                    SDL_CondWait(is->vsc, is->vsm);
                SDL_UnlockMutex(is->vsm);
            }
            dss();
        }
        SDL_UnlockMutex(is->sm);


        playsize = audio_out->get_space();

#if 0
        if (!playsize && !(is->video_sh))   
#else
        if (!playsize)          
#endif
        {
            SDL_Delay(10);      
            continue;
        }
        
        if (playsize > MAX_OUTBURST)
            playsize = MAX_OUTBURST;    

        if (is->dmuxst_audio->eof == 1)
        {
            if (audio_out->get_delay() > 0.0)
            {
                SDL_Delay(50);
                continue;
            }
            else
                goto quit;
        }
        
        while (is->audio_sh->a_out_buffer_len < playsize && !is->dmuxst_audio->eof)
        {
			while(pause_flag)	SDL_Delay(1);	

            if (quit_event == 1)
                goto break_signal;

            ret = decode_audio(is->audio_sh, &is->audio_sh->a_out_buffer[is->audio_sh->a_out_buffer_len],
                               playsize - is->audio_sh->a_out_buffer_len, is->audio_sh->a_out_buffer_size - is->audio_sh->a_out_buffer_len);
            if (ret <= 0)
            {
                goto quit;      
            }
            is->audio_sh->a_out_buffer_len += ret;
        }
        if (playsize > is->audio_sh->a_out_buffer_len)
            playsize = is->audio_sh->a_out_buffer_len;
        
        
        if( is->tr == 0 )
        {
            if( sh_video )
            {
                SDL_LockMutex(is->drm);
                is->tr = 1;
                SDL_CondSignal(is->drc);
                SDL_UnlockMutex(is->drm);
            }
            else
                is->tr = 1;

            if( is->se == 1 )
            {
                if( sh_video )
                {
                    SDL_LockMutex(is->arm);
                    while (is->ar == 0 )  
                        SDL_CondWait(is->arc, is->arm);
                    SDL_UnlockMutex(is->arm);
                }

                SDL_LockMutex(is->sm);
                is->se = 0;
                is->sp = 0;
                is->vs = 0;
                is->ar = 0;
                is->dss = 0;
                SDL_UnlockMutex(is->sm);
            }
            
        }   

        SDL_LockMutex(apm);
        playsize = audio_out->play(is->audio_sh->a_out_buffer, playsize, 0);
        
		while(pause_flag)	SDL_Delay(1);	
        
        if (playsize > 0)
        {
            is->audio_sh->a_out_buffer_len -= playsize;
            memmove(is->audio_sh->a_out_buffer, &is->audio_sh->a_out_buffer[playsize], is->audio_sh->a_out_buffer_len);
            is->audio_sh->delay += (float) playsize / ((float) ((ao_data.bps && sh_audio->afilter) ? ao_data.bps : sh_audio->o_bps));
            is->by = (float) is->audio_sh->a_buffer_len / (float) is->audio_sh->o_bps;
            is->dsat = (float) is->dmuxst_audio->pts;
            is->dsat += (float)((ds_tell_pts(is->dmuxst_audio) - is->audio_sh->a_in_buffer_len) / (float)(is->audio_sh->i_bps*8));		
        }
        SDL_UnlockMutex(apm);

		g_Timesec = (int)is->audio_sh->delay;
		CurrentPlayTime = is->dmuxst_audio->pts;
		
    }

break_signal:
quit:

    is->aq = 1;
    if (break_signal_sent == 0)
    {
        break_signal_sent = 1;
        event.type = EVENT_QUIT_PLAY;   
        SDL_PushEvent(&event);
    }

    return 0;
}


static int video_thread(void *arg)
{
    VS     *is = arg;

    int             adec;       
    int             dframe = 0;
    int             in_size = -1;
	float           nat_sum = 0;
    SDL_Event       event;
    unsigned char  *start = NULL;
    
    init_readq();
    
    while (1)
    {
		while(pause_flag)	SDL_Delay(1);	

        if (quit_event == 1)
        {
            goto break_signal;
        }

        if(is->vr == 0)
        {
            if(is->se == 1)
            {
                SDL_LockMutex(is->vsm);
                is->vs = 1;
                SDL_CondSignal(is->vsc);
                SDL_UnlockMutex(is->vsm);
            }

            SDL_LockMutex(is->drm);
            while (is->tr == 0)  
                SDL_CondWait(is->drc, is->drm);
            is->vr = 1;    
            SDL_UnlockMutex(is->drm);
            if( is->se == 1 )
            {
                SDL_LockMutex(is->arm);
                is->ar = 1;
                SDL_CondSignal(is->arc);
                SDL_UnlockMutex(is->arm);
            }
        }

        in_size = video_read_frame(is->video_sh, &nat, &start, force_fps);
        if(in_size < 0) goto quit;
		nat_sum += nat;
		if(in_size == 0) continue; // for 120fps

		put_readq(is->video_sh->pts, nat_sum);
		nat_sum = 0;

		if(in_size > 8) {
	        dframe = mmsp2_mp4_video_decode(is, start, in_size);
			if(dframe > 0) post_process(is);
		}

		if(g_fResumePlayTime != 0.0f && g_bResumePlay == true)
		{
		    resume_open_event->type = EVENT_RESUME_SEEK;
		    SDL_PushEvent(resume_open_event);
		}
		if(pause_flag) PauseTimer(true);
    }                           

break_signal:
quit:
    is->vq = 1;

    if (break_signal_sent == 0)
    {
        break_signal_sent = 1;
        
        event.type = EVENT_QUIT_PLAY;
        SDL_PushEvent(&event);
    }
    
    return 0;
}


inline int mmsp2_mp4_video_decode(VS * is, unsigned char *buf, int data_length)
{
    int fn = 0;
    int run_fault_count = 0;    

    memcpy(dbuf, buf, data_length);
    memset(dbuf + data_length, 0, 2048);    
    do
    {
        if (run_940_decoder() < 0)
        {
            return -1;
        }

        if (read_mp4d_940_status(&mp4d_disp) < 0)
        {
            return -1;
        }
        
        if (mp4d_disp.Command == MP_CMD_DISPLAY)
        {
            if (mp4d_disp.display_frames != 0)
            {
                fn++;
                pre_process(is, mp4d_disp.luma_offset_addr, mp4d_disp.cb_offset_addr, mp4d_disp.cr_offset_addr, 1);
            }
            if(mp4d_disp.display_frames == 2)
            {
                fn++;
				post_process(is);
				pre_process(is, mp4d_disp.remain_frame_luma_offset_addr, mp4d_disp.remain_frame_cb_offset_addr, mp4d_disp.remain_frame_cr_offset_addr, 1);
            }
        }
        else if (mp4d_disp.Command == MP_CMD_FAIL)
        {
            fn = 0;
            goto exit_routine;
        }

    }
    while (mp4d_disp.has_bframe);   

exit_routine:

    return fn;    
}

inline int pre_process(VS * is, unsigned short yoffset, unsigned short cboffset, unsigned short croffset, int display)
{
    float           delay,
                    d;
    float           min;
    float           d_time;
    float           frame_time;

	get_readq(&is->sv, &is->sn);

	frame_time = is->sn;
    sh_video->timer += frame_time;
    time_frame += frame_time;   

    {
        ftr = 0;   
        time_frame -= GetRelativeTime();    

        if (sh_audio && !d_audio->eof)
        {
            SDL_LockMutex(apm);
            if (sh_audio)
            {
                sh_audio->delay -= frame_time;
                d_time = sh_audio->delay;
            }
            delay = audio_out->get_delay();
            SDL_UnlockMutex(apm);
            time_frame = delay - sh_audio->delay;
            
            if (delay > 0.25)   
                delay = 0.25;
            else if (delay < 0.10)  
                delay = 0.10;

            if (time_frame > delay * 0.6)
            {
                ftr = 1;
                time_frame = delay * 0.5;   
            }
        }
        else                    
        {
            if (time_frame < -3 * frame_time || time_frame > 3 * frame_time)
                time_frame = 0;
        }
    }                           
    if (time_frame > 0.001)
    {
        min = 0.005;
        while (time_frame > min)
        {
            if (time_frame <= 0.020)
                usec_sleep(0);  
            else
                usec_sleep(1000000 * (time_frame - 0.020));
            time_frame -= GetRelativeTime();
        }
    }
    
    if (display)
        run_FDC_and_display(yoffset, cboffset, croffset);

}

inline int post_process(VS * is)
{
    float           a_pts = 0,
        b;
    float           v_pts = 0;
    float           delay;
    float           x = 0.0;

    if (sh_audio)
    {
        SDL_LockMutex(apm);
        a_pts = is->dsat;
        b = is->by;
        delay = b + audio_out->get_delay();
        SDL_UnlockMutex(apm);

        v_pts = is->sv ?  is->sv : is->video_sh->pts; 

        AV_delay = (a_pts - delay - audio_delay) - v_pts;

        if (AV_delay > 0.5 && drop_frame_cnt > 50)
        {
            printf("too\n");
        }

        x = AV_delay * 0.1f;
        if(x < -max_pts_correction)		x = -max_pts_correction;
        else if(x > max_pts_correction)	x = max_pts_correction;

        if (default_max_pts_correction >= 0)
            max_pts_correction = default_max_pts_correction;
        else
        {
            
            max_pts_correction = sh_video->frametime * 0.10;    
        }
        if (!ftr)  
        {
            SDL_LockMutex(apm);
            sh_audio->delay += x;
            SDL_UnlockMutex(apm);

            c_total += x;
        }

        nframes++;
    }
    else                        
    {
    }                           

#if 1
#    ifdef USE_SUB
    
    if (subdata && v_pts > 0)
    {
        float           pts = v_pts;

        if (sub_fps == 0)
            sub_fps = sh_video->fps;
        
        if (pts > sub_last_pts || pts < sub_last_pts - 1.0)
        {
            find_sub(subdata, (pts + sub_delay) * (subdata->sub_uses_time ? 100. : sub_fps));
            
            sub_last_pts = pts;
            if (subtitle_changed == 1)
            {
				subtitle_changed = 0;
                subchange.type = EVENT_SUBTITLE_CHANGE; 
                SDL_PushEvent(&subchange);
            }
        }
        current_module = NULL;
    }
	#endif
#endif
    return 1;
}

void set_audio_delay(ADelay fast_slow)
{
    lock_take(&slock);          
    if (fast_slow == AD_PLUS_100MS)
    {
        SDL_LockMutex(apm);
        vs.audio_sh->delay += (-0.100); 
        audio_delay += (-0.100);
        SDL_UnlockMutex(apm);
    }
    else if (fast_slow == AD_MINUS_100MS)
    {
        SDL_LockMutex(apm);
        vs.audio_sh->delay += (0.100);  
        audio_delay += (0.100);
        SDL_UnlockMutex(apm);
    }
    lock_give(&slock);
}




int movie_seek(float seeksec, MSeek how)
{
    if (vs.si == 0)
    {
        return -1;
    }

    SDL_LockMutex(vs.sm);
    if (vs.se == 1)
    {
        SDL_UnlockMutex(vs.sm);
        return -1;
    }
    vs.dss = seeksec;
    if( how == RELATIVE_SEEK_BY_SECOND )
    {
        abs_seek_pos = 0;
    }
    else if( how == ABSOLUTE_SEEK_BY_POSITION )
    {
        abs_seek_pos = 3;
    }
    
    vs.se = 1;
    SDL_UnlockMutex(vs.sm);

    return 1;
}

void dss(void)
{
    if( abs_seek_pos == 0 )
    {
        rel_seek_secs += vs.dss; 
    }
    else if( abs_seek_pos == 3 )
    {
        rel_seek_secs = 0.01 * vs.dss;
    }

    if (demux_seek(demuxer, rel_seek_secs, abs_seek_pos))
    {
        if (sh_video)
        {
            sh_video->pts = d_video->pts;
        }


        fflush(stdout);

        if (sh_audio)
        {
            audio_out->reset(); 
        }

        if (sh_video)
        {
            c_total = 0;
            max_pts_correction = 0.1;
        }
    }                           

   
    if (sh_video)
    {
        init_readq();
        put_readq(sh_video->pts, vs.sn);

        ftr = 0;
    }

    rel_seek_secs = 0;
    abs_seek_pos = 0;

    vs.dems = 0;
}
void select_subtitle(void)
{
    if(global_sub_size)
    {
        int             source = -1;

        global_sub_pos++;

        if(global_sub_pos >= global_sub_size)	global_sub_pos = -1;
        if(global_sub_pos >= 0)	source = sub_source();

        printf("subtitles: %d subs, (v@%d s@%d d@%d), @%d, source @%d\n",
               global_sub_size, global_sub_indices[SUB_SOURCE_VOBSUB], global_sub_indices[SUB_SOURCE_SUBS], global_sub_indices[SUB_SOURCE_DEMUX], global_sub_pos, source);

#ifdef USE_SUB
        set_of_sub_pos = -1;
        subdata = NULL;
        vo_sub = NULL;
#endif

        vobsub_id = -1;
        dvdsub_id = -1;

        if(d_dvdsub)	d_dvdsub->id = -1;

        if (source == SUB_SOURCE_SUBS)
        {
            set_of_sub_pos = global_sub_pos - global_sub_indices[SUB_SOURCE_SUBS];
            subdata = set_of_subtitles[set_of_sub_pos];
            
            if (stream_dump_type == 3)	list_sub_file(subdata);
            if (stream_dump_type == 4)	dump_mpsub(subdata, sh_video->fps);
            if (stream_dump_type == 6)	dump_srt(subdata, sh_video->fps);
            if (stream_dump_type == 7)	dump_microdvd(subdata, sh_video->fps);
            if (stream_dump_type == 8)	dump_jacosub(subdata, sh_video->fps);
            if (stream_dump_type == 9)	dump_sami(subdata, sh_video->fps);
        }
#if 0
        else
        {
            if (!global_sub_quiet_osd_hack)
                osd_show_vobsub_changed = sh_video->fps;
#    ifdef USE_SUB
            vo_osd_changed(OSDTYPE_SUBTITLE);
#    endif
        }
#endif
        
        global_sub_quiet_osd_hack = 0;
    }

    return;
}

void volume_change(bool up_flag)
{
	int vol = 0;
	if(up_flag)
	{
		leftVol	 += 10;
		if(leftVol > 100)	leftVol = 100;
	}
	else
	{
		leftVol	 -= 10;
		if(leftVol < 0)	leftVol = 0;
	}
	
	mixer_setvolume(&mixer, leftVol, leftVol); 	
	
	if(leftVol < 14)		vol = 0;
	else if(leftVol < 14*2)	vol = 1;
	else if(leftVol < 14*3)	vol = 2;
	else if(leftVol < 14*4)	vol = 3;
	else if(leftVol < 14*5)	vol = 4;
	else if(leftVol < 14*6)	vol = 5;
	else if(leftVol < 14*7)	vol = 6;
	else 					vol = 7;

	if(bMenuStatus)	OnDraw_Volume(vol);
}
