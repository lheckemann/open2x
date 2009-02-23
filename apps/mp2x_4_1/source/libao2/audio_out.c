#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "audio_out.h"
#include "afmt.h"

#include "mp_msg.h"
#include "help_mp.h"

// there are some globals:
ao_data_t ao_data={0,0,0,0,OUTBURST,-1,0};
char *ao_subdevice = NULL;

#ifdef USE_OSS_AUDIO
extern ao_functions_t audio_out_oss;
#endif

ao_functions_t* audio_out_drivers[] =
{
// vo-related:   will fail unless you also do -vo mpegpes/dxr2
	// &audio_out_mpegpes, // ghcstop delete
#ifdef USE_OSS_AUDIO
        &audio_out_oss,
#endif
	NULL
};

void list_audio_out(){
      int i=0;
      mp_msg(MSGT_AO, MSGL_INFO, MSGTR_AvailableAudioOutputDrivers);
      while (audio_out_drivers[i]) {
        const ao_info_t *info = audio_out_drivers[i++]->info;
	printf("\t%s\t%s\n", info->short_name, info->name);
      }
      printf("\n");
}

ao_functions_t* init_best_audio_out(char** ao_list,int use_plugin,int rate,int channels,int format,int flags){
    int i;
    // first try the preferred drivers, with their optional subdevice param:
    if(ao_list && ao_list[0])
      while(ao_list[0][0]){
        char* ao=ao_list[0];
        int ao_len;
        if (strncmp(ao, "alsa9", 5) == 0 || strncmp(ao, "alsa1x", 6) == 0) {
          mp_msg(MSGT_AO, MSGL_FATAL, MSGTR_AO_ALSA9_1x_Removed);
          exit_player(NULL);
        }
        if (ao_subdevice) {
          free(ao_subdevice);
          ao_subdevice = NULL;
        }
	ao_subdevice=strchr(ao,':');
	if(ao_subdevice){
	    ao_len = ao_subdevice - ao;
	    ao_subdevice = strdup(&ao[ao_len + 1]);
	}
	else
	    ao_len = strlen(ao);
	for(i=0;audio_out_drivers[i];i++){
	    ao_functions_t* audio_out=audio_out_drivers[i];
	    if(!strncmp(audio_out->info->short_name,ao,ao_len)){

 
//	    printf("init_best_audio_out: ao = %s, audio_out_short_name = %s\n", ao, audio_out->info->short_name,ao);// ghcstop
	    
	    #if 0 // ghcstop delete
		// name matches, try it
		if(use_plugin){
		    audio_out_plugin.control(AOCONTROL_SET_PLUGIN_DRIVER,audio_out);
		    audio_out=&audio_out_plugin;
		}
            #endif		
		if(audio_out->init(rate,channels,format,flags))
		    return audio_out; // success!
	    }
	}
        // continue...
	++ao_list;
	if(!(ao_list[0])) return NULL; // do NOT fallback to others
      }
    if (ao_subdevice) {
      free(ao_subdevice);
      ao_subdevice = NULL;
    }
    // now try the rest...
    for(i=0;audio_out_drivers[i];i++){
	ao_functions_t* audio_out=audio_out_drivers[i];
	// ghcstop
	if(audio_out->init(rate,channels,format,flags))
	    return audio_out; // success!
    }
    return NULL;
}

