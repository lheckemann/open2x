/*
 *  drivers/sound/mp2520f-audio.h -- audio interface for the MP2520F
 *  Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 *  Kane Ahn < hbahn@dignsys.com >
 *  hhsong < hhsong@dignsys.com >
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

/*
 * Buffer Management
 */

typedef struct {
	int size;		/* buffer size */
	char *start;		/* points to actual buffer */
	dma_addr_t dma_addr;	/* physical buffer address */
	struct semaphore sem;	/* down before touching the buffer */
	int master;		/* owner for buffer allocation, contain size when true */
	struct audio_stream_s *stream;	/* owning stream */
} audio_buf_t;

typedef struct audio_stream_s {
	char *name;		/* stream identifier */
	audio_buf_t *buffers;	/* pointer to audio buffer structures */
	audio_buf_t *buf;	/* current buffer used by read/write */
	u_int buf_idx;		/* index for the pointer above... */
	u_int fragsize;		/* fragment i.e. buffer size */
	u_int nbfrags;		/* nbr of fragments i.e. buffers */
	u_int channels;		/* audio channels 1:mono, 2:stereo */
	int bytecount;		/* nbr of processed bytes */
	int fragcount;		/* nbr of fragment transitions */
	u_int rate;		/* audio rate */
	dmach_t dma_ch;		/* DMA channel ID */
	wait_queue_head_t wq;   /* for poll */
	int mapped:1;		/* mmap()'ed buffers */
	int active:1;		/* actually in progress */
	int stopped:1;		/* might be active but stopped */
} audio_stream_t;

/*
 * State structure for one instance
 */

typedef struct {
	audio_stream_t *output_stream;
	audio_stream_t *input_stream;
	int dev_dsp;		/* audio device handle */
	char *output_id;
	char *input_id;
	int rd_ref:1;		/* open reference for recording */
	int wr_ref:1;		/* open reference for playback */
	void *data;
	void (*hw_init)(void *);
	void (*hw_shutdown)(void *);
	int (*client_ioctl)(struct inode *, struct file *, uint, ulong);
	struct pm_dev *pm_dev;
	struct semaphore sem;	/* to protect against races in attach() */
} audio_state_t;

/*
 * Functions exported by this module
 */
extern int mp2520f_audio_attach( struct inode *inode, struct file *file,
				audio_state_t *state);
