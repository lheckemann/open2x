/*              
 *  Original code routine from Mplayer < www.mplayerhq.hu >             
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. Jan, 2005
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */

#ifndef __MPLAYER_SUBREADER_H
#    define __MPLAYER_SUBREADER_H

#    include <stdio.h>

extern int      suboverlap_enabled;
extern int      sub_no_text_pp; 
extern int      sub_match_fuzziness;


#    define SUB_INVALID   -1
#    define SUB_MICRODVD  0
#    define SUB_SUBRIP    1
#    define SUB_SUBVIEWER 2
#    define SUB_SAMI      3
#    define SUB_VPLAYER   4
#    define SUB_RT        5
#    define SUB_SSA       6
#    define SUB_PJS       7
#    define SUB_MPSUB     8
#    define SUB_AQTITLE   9
#    define SUB_SUBVIEWER2 10
#    define SUB_SUBRIP09 11
#    define SUB_JACOSUB  12
#    define SUB_MPL2     13


extern int      sub_format;

#    define MAX_SUBTITLE_FILES 128

#    define SUB_MAX_TEXT 10
#    define SUB_ALIGNMENT_BOTTOMLEFT       1
#    define SUB_ALIGNMENT_BOTTOMCENTER     2
#    define SUB_ALIGNMENT_BOTTOMRIGHT      3
#    define SUB_ALIGNMENT_MIDDLELEFT       4
#    define SUB_ALIGNMENT_MIDDLECENTER     5
#    define SUB_ALIGNMENT_MIDDLERIGHT      6
#    define SUB_ALIGNMENT_TOPLEFT          7
#    define SUB_ALIGNMENT_TOPCENTER        8
#    define SUB_ALIGNMENT_TOPRIGHT         9

typedef struct
{

    int             lines;

    unsigned long   start;
    unsigned long   end;

    char           *text[SUB_MAX_TEXT];
    unsigned char   alignment;
} subtitle;

typedef struct
{
    subtitle       *subtitles;
    char           *filename;
    int             sub_uses_time;
    int             sub_num;    
    int             sub_errs;
} sub_data;

#    ifdef  USE_FRIBIDI
extern char    *fribidi_charset;
extern int      flip_hebrew;
extern int      fribidi_flip_commas;
#    endif

sub_data       *sub_read_file(char *filename, float pts);
subtitle       *subcp_recode1(subtitle * sub);



void            subcp_open(FILE * enca_fd); 
void            subcp_close(void);  

#    ifdef HAVE_ENCA
void           	*guess_cp(FILE * enca_fd, char *preferred_language, char *fallback);
#    endif
char          	**sub_filenames(char *path, char *fname);
void            list_sub_file(sub_data * subd);
void            dump_srt(sub_data * subd, float fps);
void            dump_mpsub(sub_data * subd, float fps);
void            dump_microdvd(sub_data * subd, float fps);
void            dump_jacosub(sub_data * subd, float fps);
void            dump_sami(sub_data * subd, float fps);
void            sub_free(sub_data * subd);
inline void     find_sub(sub_data * subd, int key); 
void            step_sub(sub_data * subd, float pts, int movement);
#endif
