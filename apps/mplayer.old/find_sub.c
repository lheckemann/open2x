/*              
 *  Original code routine from Mplayer < www.mplayerhq.hu >             
 *
 *  - godori <ghcstop>, www.aesop-embedded.org
 *    => Modified. Jan, 2005
 */

#if 0
#    ifdef USE_SUB
  
if (subdata && sh_video->pts > 0)
{
    float           pts = sh_video->pts;

    if (sub_fps == 0)
        sub_fps = sh_video->fps;
    current_module = "find_sub";
    
    if (pts > sub_last_pts || pts < sub_last_pts - 1.0)
    {
        find_sub(subdata, (pts + sub_delay) * (subdata->sub_uses_time ? 100. : sub_fps));
        
        sub_last_pts = pts;
    }
    current_module = NULL;
}
#    endif
#endif

#include "config.h"
#ifdef USE_OSD
#    include <stdio.h>
#    include "subreader.h"
#    include "mp_msg.h"
#    include "help_mp.h"

#    ifdef DEBUG
#        define dprintf(x...) printf(x)
#    else
#        define dprintf(x...)
#    endif

#    ifdef GDEBUG
extern FILE    *dbg;

#        define gprintf(x...) fprintf(dbg, x)
#    else
#        define gprintf(x...)
#    endif

static int      current_sub = 0;


static int      nosub_range_start = -1;
static int      nosub_range_end = -1;


extern float    sub_delay;
extern float    sub_fps;

extern subtitle *vo_sub;        
extern int      subtitle_changed;   

void
step_sub(sub_data * subd, float pts, int movement)
{
    subtitle       *subs;
    int             key;

    if (subd == NULL)
        return;
    subs = subd->subtitles;
    key = (pts + sub_delay) * (subd->sub_uses_time ? 100 : sub_fps);

    
#    if 0
    vo_osd_changed(OSDTYPE_SUBTITLE);
#    else
    subtitle_changed = 1;
#    endif


    if (movement > 0 && key < subs[current_sub].start)
        movement--;
    if (movement < 0 && key >= subs[current_sub].end)
        movement++;

    
    if (current_sub + movement < 0)
        movement = 0 - current_sub;
    if (current_sub + movement >= subd->sub_num)
        movement = subd->sub_num - current_sub - 1;

    current_sub += movement;
    sub_delay = subs[current_sub].start / (subd->sub_uses_time ? 100 : sub_fps) - pts;
}


inline void
find_sub(sub_data * subd, int key)
{
    subtitle       *subs;
    int             i,
                    j;

    if (!subd || subd->sub_num == 0)
        return;
    subs = subd->subtitles;

    
    if (vo_sub)                 
    {
        gprintf("ye vo_sub: ");
        if (key >= vo_sub->start && key <= vo_sub->end)
        {
            gprintf("no change\n");
            return;             
        }
    }
    else                        
    {
        
        gprintf("no vo_sub: ");
        
        if (key > nosub_range_start && key < nosub_range_end)
        {
            gprintf("no change\n");
            return;             
        }
    }

    gprintf("change\n");
    
#    if 0
    vo_osd_changed(OSDTYPE_SUBTITLE);
#    else
    subtitle_changed = 1;
#    endif
    
    if (key <= 0)
    {
        vo_sub = NULL;          
        return;
    }
    
    if (current_sub >= 0 && current_sub + 1 < subd->sub_num)
    {
        
        if (key > subs[current_sub].end && key < subs[current_sub + 1].start)
        {
            
            nosub_range_start = subs[current_sub].end;
            nosub_range_end = subs[current_sub + 1].start;
            gprintf("1 NULL set\n");    
            vo_sub = NULL;
            return;
        }
        ++current_sub;
        vo_sub = &subs[current_sub];
        gprintf("1 vo_sub[%d]->text = %s ", current_sub, vo_sub->text[0]);
        
        if (key >= vo_sub->start && key <= vo_sub->end)
        {
            gprintf("return \n");
            return;             
        }
        
        gprintf("no return \n");
    }
    
    i = 0;
    j = subd->sub_num - 1;
    
    while (j >= i)
    {
        current_sub = (i + j + 1) / 2;
        vo_sub = &subs[current_sub];
        if (key < vo_sub->start)
        {
            j = current_sub - 1;
        }
        else if (key > vo_sub->end)
        {
            i = current_sub + 1;
        }
        else
        {
            gprintf("2 vo_sub[%d]->text = %s\n", current_sub, vo_sub->text[0]);
            return;             
        }
    }
    
    if (key < vo_sub->start)
    {
        if (current_sub <= 0)
        {
            
            nosub_range_start = key - 1;    
            nosub_range_end = vo_sub->start;
            gprintf("2 NULL set: FIRST...  key=%d  end=%d  \n", key, vo_sub->start);
            vo_sub = NULL;
            return;
        }
        --current_sub;

        if (key > subs[current_sub].end && key < subs[current_sub + 1].start)
        {
            
            nosub_range_start = subs[current_sub].end;
            nosub_range_end = subs[current_sub + 1].start;
            gprintf("3 NULL set: No sub... 1 \n");
            vo_sub = NULL;
            return;
        }
        gprintf("HEH????  ");
    }
    else
    {
        if (key <= vo_sub->end)
        {
            gprintf("JAJJ!  ");
        }
        else if (current_sub + 1 >= subd->sub_num)
        {
            
            nosub_range_start = vo_sub->end;
            nosub_range_end = 0x7FFFFFFF;   
            gprintf("3 NULL set: END!?\n");
            vo_sub = NULL;
            return;
        }
        else if (key > subs[current_sub].end && key < subs[current_sub + 1].start)
        {
            
            nosub_range_start = subs[current_sub].end;
            nosub_range_end = subs[current_sub + 1].start;
            gprintf("4 NULL set: No sub... 2 \n");
            vo_sub = NULL;
            return;
        }
    }

    printf("SUB ERROR:  %d  ?  %d --- %d  [%d]  \n", key, (int) vo_sub->start, (int) vo_sub->end, current_sub);

    vo_sub = NULL;              
}

#endif
