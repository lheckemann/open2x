/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 *
 */

#ifndef _GVLIB_EXPORT_H_
#define _GVLIB_EXPORT_H_
    
 
#define EVENT_MOVIE_PLAY   		(SDL_USEREVENT)
#define EVENT_QUIT_PLAY    		(SDL_USEREVENT+1)
#define EVENT_SUBTITLE_CHANGE 	(SDL_USEREVENT+2)
#define EVENT_FILE_OPEN			(SDL_USEREVENT+3)
#define EVENT_RESUME_SEEK		(SDL_USEREVENT+4)

typedef struct VideoFile_t 
{
char           filename[256];
} VideoFile;

extern int      quit_event,
                break_signal_sent;

int             init_etc_setting(void);
int            init_media_play(char* ifile);
void           exit_media_play(void);
void           close_etc_setting(void);

typedef enum 
{ 
AD_PLUS_100MS, 
AD_MINUS_100MS 
} ADelay;
void          set_audio_delay(ADelay fast_slow);

typedef enum 
{ 
RELATIVE_SEEK_BY_SECOND,   
ABSOLUTE_SEEK_BY_POSITION  
} MSeek;
int            movie_seek(float seeksec, MSeek how);
    
#endif
