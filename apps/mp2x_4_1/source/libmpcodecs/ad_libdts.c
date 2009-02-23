#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include "config.h"

#include "mp_msg.h"
#include "help_mp.h"

#include "ad_internal.h"

//#include "cpudetect.h"

#include "../libaf/af_format.h"

#include "../libdts/dts.h"
//#include "../libdts/mm_accel.h"

static dts_state_t *dts_state;
static uint32_t dts_flags=2;

/** The output is multiplied by this var.  Used for volume control */
static sample_t dts_level = 1;
static int frame_length;

#include "bswap.h"

static ad_info_t info = 
{
	"DTS decoding with libdts",
	"libdts",
	"Gildas Bazin",
	"mp2x",
	""
};

LIBAD_EXTERN(libdts)

extern int audio_output_channels;

int dts_fillbuff(sh_audio_t *sh_audio){
	int length=0;
	int flags=0;
	int sample_rate=0;
	int bit_rate=0;

    sh_audio->a_in_buffer_len=0;
    /* sync frame:*/
	while(1){
		while(sh_audio->a_in_buffer_len<8){
			int c=demux_getc(sh_audio->ds);
			if(c<0) return -1; /* EOF*/
			sh_audio->a_in_buffer[sh_audio->a_in_buffer_len++]=c;
		}
		if(sh_audio->format!=0x2001) swab(sh_audio->a_in_buffer,sh_audio->a_in_buffer,8);
		length = dts_syncinfo (sh_audio->a_in_buffer, &flags, &sample_rate, &bit_rate, &frame_length);
		if(length>=7 && length<=3840) break; /* we're done.*/
		/* bad file => resync*/
		if(sh_audio->format!=0x2001) swab(sh_audio->a_in_buffer,sh_audio->a_in_buffer,8);
		memmove(sh_audio->a_in_buffer,sh_audio->a_in_buffer+1,7);
		--sh_audio->a_in_buffer_len;
	}
    mp_msg(MSGT_DECAUDIO,MSGL_DBG2,"dts: len=%d  flags=0x%X  %d Hz %d bit/s\n",length,flags,sample_rate,bit_rate);
    sh_audio->samplerate=sample_rate;
    sh_audio->i_bps=bit_rate/8;
    sh_audio->samplesize=sh_audio->sample_format==AF_FORMAT_F ? 4 : 2;
    demux_read_data(sh_audio->ds,sh_audio->a_in_buffer+8,length-8);
    if(sh_audio->format!=0x2001)
		swab(sh_audio->a_in_buffer+8,sh_audio->a_in_buffer+8,length-8);
/*
    if(crc16_block(sh_audio->a_in_buffer+2,length-2)!=0)
		mp_msg(MSGT_DECAUDIO,MSGL_STATUS,"dts: CRC check failed!  \n");
*/
    return length;
}

/* returns: number of available channels*/
static int dts_printinfo(sh_audio_t *sh_audio){
	int flags, sample_rate, bit_rate;
	char* mode="unknown";
	int channels=0;
	dts_syncinfo (sh_audio->a_in_buffer, &flags, &sample_rate, &bit_rate, &frame_length);
	switch(flags&DTS_CHANNEL_MASK){
		case DTS_CHANNEL: mode="channel"; channels=2; break;
		case DTS_MONO: mode="mono"; channels=1; break;
		case DTS_STEREO: mode="stereo"; channels=2; break;
		case DTS_3F: mode="3f";channels=3;break;
		case DTS_2F1R: mode="2f+1r";channels=3;break;
		case DTS_3F1R: mode="3f+1r";channels=4;break;
		case DTS_2F2R: mode="2f+2r";channels=4;break;
		case DTS_3F2R: mode="3f+2r";channels=5;break;
		case DTS_CHANNEL1: mode="channel1"; channels=2; break;
		case DTS_CHANNEL2: mode="channel2"; channels=2; break;
		case DTS_DOLBY: mode="dolby"; channels=2; break;
	}
	mp_msg(MSGT_DECAUDIO,MSGL_INFO,"AC3: %d.%d (%s%s)  %d Hz  %3.1f kbit/s\n",
	channels, (flags&DTS_LFE)?1:0,
	mode, (flags&DTS_LFE)?"+lfe":"",
	sample_rate, bit_rate*0.001f);
	return (flags&DTS_LFE) ? (channels+1) : channels;
}

static int preinit(sh_audio_t *sh)
{
	/* Dolby AC3 audio: */
	/* however many channels, 2 bytes in a word, 256 samples in a block, 6 blocks in a frame */
	if (sh->samplesize < 2) sh->samplesize = 2;
	sh->audio_out_minsize=audio_output_channels*sh->samplesize*256*6;
	sh->audio_in_minsize=3840;
	dts_level = 1.0;
	return 1;
}

static int init(sh_audio_t *sh_audio)
{
	uint32_t dts_accel=0;
	sample_t level=dts_level, bias=384;
	int flags=0;

	dts_state=dts_init (0);
	if (dts_state == NULL) {
		mp_msg(MSGT_DECAUDIO,MSGL_ERR,"DTS init failed\n");
		return 0;
	}
	if(dts_fillbuff(sh_audio)<0){
		mp_msg(MSGT_DECAUDIO,MSGL_ERR,"DTS sync failed\n");
		return 0;
	}

	/* 'dts cannot upmix' hotfix:*/
	dts_printinfo(sh_audio);
	sh_audio->channels=audio_output_channels;

	while(sh_audio->channels>0){
		switch(sh_audio->channels){
			case 1: dts_flags=DTS_MONO; break;
			case 2: dts_flags=DTS_STEREO; break;
			/*case 2: dts_flags=DTS_DOLBY; break;*/
			/*case 3: dts_flags=DTS_3F; break;*/
			case 3: dts_flags=DTS_2F1R; break;
			case 4: dts_flags=DTS_2F2R; break; /* 2+2*/
			case 5: dts_flags=DTS_3F2R; break;
			case 6: dts_flags=DTS_3F2R|DTS_LFE; break; /* 5.1*/
		}
		/* test:*/
		flags=dts_flags|DTS_ADJUST_LEVEL;
		mp_msg(MSGT_DECAUDIO,MSGL_V,"DTS flags before dts_frame: 0x%X\n",flags);
		if (dts_frame (dts_state, sh_audio->a_in_buffer, &flags, &level, bias)){
			mp_msg(MSGT_DECAUDIO,MSGL_ERR,"dts: error decoding frame -> nosound\n");
			return 0;
		}
		mp_msg(MSGT_DECAUDIO,MSGL_V,"DTS flags after dts_frame: 0x%X\n",flags);
		/* frame decoded, let's init resampler:*/
		channel_map = 0;

		if(dts_resample_init(flags,sh_audio->channels)) break;
		--sh_audio->channels; /* try to decrease no. of channels*/
	}

	if(sh_audio->channels<=0){
		mp_msg(MSGT_DECAUDIO,MSGL_ERR,"dts: no resampler. try different channel setup!\n");
		return 0;
	}
	return 1;
}

static void uninit(sh_audio_t *sh)
{
}

static int control(sh_audio_t *sh,int cmd,void* arg, ...)
{
	switch(cmd)
	{
		case ADCTRL_RESYNC_STREAM:
		case ADCTRL_SKIP_FRAME:
			dts_fillbuff(sh);
			return CONTROL_TRUE;
		case ADCTRL_QUERY_FORMAT:
			if (*(int*)arg == AF_FORMAT_SI)
				return CONTROL_TRUE;
			return CONTROL_FALSE;
	}
	return CONTROL_UNKNOWN;
}

static int channels_multi (int flags)
{
	if (flags & DTS_LFE) return 6;
	else if (flags & 1) return 5; /* center channel */
	else if ((flags & DTS_CHANNEL_MASK) == DTS_2F2R) return 4;
	else return 2;
}
extern void convert2s16_multi (convert_t * _f, int16_t * s16, int flags);

static int decode_audio(sh_audio_t *sh_audio,unsigned char *buf,int minlen,int maxlen)
{
	sample_t level, bias;
	int flags = dts_flags | DTS_ADJUST_LEVEL;
	int i,len = -1;

	if(!sh_audio->a_in_buffer_len) 
		if(dts_fillbuff(sh_audio)<0)
			return len; /* EOF */

	sh_audio->a_in_buffer_len=0;
	level = CONVERT_LEVEL;
	bias = CONVERT_BIAS;

	if (dts_frame (dts_state, sh_audio->a_in_buffer, &flags, &level, bias)){
		mp_msg(MSGT_DECAUDIO,MSGL_WARN,"dts: error decoding frame\n");
		return len;
	}

	len=0;
	for (i = 0; i < dts_blocks_num (state); i++)
	{
		int chans;

		if (dts_block (state)){
			mp_msg(MSGT_DECAUDIO,MSGL_WARN,"dts: error at resampling\n");
			break;
		}

		chans = channels_multi (flags);
		convert2s16_multi (dts_samples (state), (int16_t *)&buf[len], flags & (DTS_CHANNEL_MASK | DTS_LFE));

		len += 256 * sizeof (int16_t) * chans;
	}
/*	
	for (i = 0; i < 6; i++) {
		if (dts_block (dts_state)){
			mp_msg(MSGT_DECAUDIO,MSGL_WARN,"dts: error at resampling\n");
			break;
		}
		len+=2*dts_resample(dts_samples(dts_state),(int16_t *)&buf[len]);
	}
*/
	assert(len <= maxlen);
	return len;
}
