
#ifndef __AUDIO_OUT_H
#define __AUDIO_OUT_H

typedef struct ao_info_s
{
        /* driver name ("Matrox Millennium G200/G400" */
        const char *name;
        /* short name (for config strings) ("mga") */
        const char *short_name;
        /* author ("Aaron Holtzman <aholtzma@ess.engr.uvic.ca>") */
        const char *author;
        /* any additional comments */
        const char *comment;
} ao_info_t;

/* interface towards mplayer and */
typedef struct ao_functions_s
{
	ao_info_t *info;
        int (*control)(int cmd,void *arg);
        int (*init)(int rate,int channels,int format,int flags);
        void (*uninit)(int immed);
        void (*reset)();
        int (*get_space)();
        int (*play)(void* data,int len,int flags);
        float (*get_delay)();
        void (*pause)();
        void (*resume)();
		void (*audio_uf_chk)();	// hhsond 051122 add
} ao_functions_t;

/* global data used by mplayer and plugins */
typedef struct ao_data_s
{
  int samplerate;
  int channels;
  int format;
  int bps;
  int outburst; 	  
  int buffersize;         
  int pts;
} ao_data_t;

extern char *ao_subdevice;
extern ao_data_t ao_data;

// prototypes
extern char *audio_out_format_name(int format);
extern int   audio_out_format_bits(int format);

void list_audio_out();
ao_functions_t* init_best_audio_out(char** ao_list,int use_plugin,int rate,int channels,int format,int flags);

// NULL terminated array of all drivers
extern ao_functions_t* audio_out_drivers[];

#define CONTROL_OK 1
#define CONTROL_TRUE 1
#define CONTROL_FALSE 0
#define CONTROL_UNKNOWN -1
#define CONTROL_ERROR -2
#define CONTROL_NA -3

#define AOCONTROL_SET_DEVICE 1
#define AOCONTROL_GET_DEVICE 2
#define AOCONTROL_QUERY_FORMAT 3 /* test for availabilty of a format */
#define AOCONTROL_GET_VOLUME 4
#define AOCONTROL_SET_VOLUME 5
#define AOCONTROL_SET_PLUGIN_DRIVER 6
#define AOCONTROL_SET_PLUGIN_LIST 7

#define AOCONTROL_HW_EQ		8	
#define SOUND_MIXER_EQ_HW	32
#define FLAT				0	
#define ROCK				1
#define POP					2
#define JAZZ				3
#define	CLASSICAL			4
#define DANCE				5	
#define	HEAVY 				6
#define DISCO				7
#define SOFT				8
#define LIVE_3D_ST			9
#define HALL				10		


typedef struct ao_control_vol_s {
	float left;
	float right;
} ao_control_vol_t;



#endif
