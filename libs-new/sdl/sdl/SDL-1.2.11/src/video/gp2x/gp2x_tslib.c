/*
 * tslib-1.0 based touchscreen event handler
 *
 * Altered to work without dynamic loading and fixed modules,
 * i.e. all code in this file, modules supported are:
 *   module_raw ucb1x00
 *   module pthres
 *   module variance
 *   module dejitter
 *
 * Specifically re-written for HWSDL on the GP2X F-200
 * changes in functions to remove un-needed stuff like reading >1 value
 * at a time (SDL only ever reads 1)
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "tslib.h"

typedef struct ts_sample TS_SAMPLE;

typedef struct tslib_vars {
  const char *name;
  void *data;
  int (*fn)(char *str, void *data);
} TSLIB_VARS;

int tslib_parse_vars(const TSLIB_VARS *, int, const char *);

#define SQR(x) ((x) * (x))

/******************************
 * tslib open/close
 ******************************/

TSDEV *ts_open(const char *name, int nonblock)
{
  TSDEV *ts;
  int flags = O_RDONLY;

  if (nonblock)
    flags |= O_NONBLOCK;

  ts = malloc(sizeof(TSDEV));
  if (ts) {
    memset(ts, 0, sizeof(TSDEV));

    ts->fd = open(name, flags);
    if (ts->fd == -1) {
      free(ts);
      ts = NULL;
    }
  }

  return ts;
}



int ts_close(TSDEV *ts)
{
  if (ts)
  {
	  if (ts->fd != -1)
	  {
  			close(ts->fd);
			free(ts);
			return 0;
	  }
  }

  return -1;
}

inline int ts_fd(TSDEV *ts)
{
  return ts->fd;
}


/******************************
 * ucb1x100-raw
 ******************************/

typedef struct ucb1x00_ts_event {
  unsigned short pressure;
  unsigned short x;
  unsigned short y;
  unsigned short pad;
} UCB1X00_TS_EVENT;

//senquack - Originially, I had made this read multiple samples but it turns out the driver
//for the touchscreen is set up to give only one sample per read, so had to go back
//to this optimized version.  It should be noted that the device file only ever provides
//8 bytes of data, basically 3shorts: x,y,and pressure, and the old code tried to read
//data that was never provided.
static inline int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
	UCB1X00_TS_EVENT ucb1x00_evt;

	//	senquack -trying to solve touchscreen bug
	if (read(ts->fd, &ucb1x00_evt, sizeof(UCB1X00_TS_EVENT)) == sizeof(UCB1X00_TS_EVENT))
	{
		samp->x = ucb1x00_evt.x;
		samp->y = ucb1x00_evt.y;
		samp->pressure = ucb1x00_evt.pressure;
		return 1;
	} 
	else
	{
		return -1;
	}

}


/******************************
 * pthres
 ******************************/

#define MINPRESSURE 35000 // Accepting pressures below this on the GP2X F200 leads to
									// cursor jumping on taps.
static inline int pthres_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
  int ret;
  static int xsave = 0, ysave = 0;
  static int press = 0;

  ret = ucb1x00_read(ts, samp,1);
  if (ret >= 0)
  {
		if (samp->pressure < MINPRESSURE)
		{
			if (press != 0) {
				press = 0;
				samp->pressure = 0;
				samp->x = xsave;
				samp->y = ysave;
				ret = 1;
			} else {
				ret = 0;
			}
		} else {
			press = 1;
			xsave = samp->x;
			ysave = samp->y;
			ret = 1;
		}
  }

  return ret;
}

/******************************
 * variance
 ******************************/

typedef struct tslib_variance {
  int       delta;
  TS_SAMPLE last;
  TS_SAMPLE noise;
  unsigned int flags;
#define VAR_PENDOWN     0x00000001
#define VAR_LASTVALID   0x00000002
#define VAR_NOISEVALID  0x00000004
#define VAR_SUBMITNOISE 0x00000008
} TSLIB_VARIANCE;

TSLIB_VARIANCE variance_dev;

void variance_init()
{
  memset(&variance_dev, 0, sizeof(variance_dev));
  variance_dev.flags = 0;
  variance_dev.delta = 30 * 30;
}

//senquack - added parameter, flush_history, that gets assigned to 1 if a fast movement is
//			detected.. this is so the averaging algorithm will flush its history if 
//			enough of them are detected
static inline int variance_read(TSDEV *ts, TS_SAMPLE *samp, int nr, int *flush_history)
{
	struct ts_sample cur;
	int count = 0, dist;

	while (count < nr) {
		if (variance_dev.flags & VAR_SUBMITNOISE) {
			cur = variance_dev.noise;
			variance_dev.flags &= ~VAR_SUBMITNOISE;
		} else {
			if (pthres_read(ts, &cur, 1) < 1)
				return count;
		}

		if (cur.pressure == 0) {
			/* Flush the queue immediately when the pen is just
			 * released, otherwise the previous layer will
			 * get the pen up notification too late. This 
			 * will happen if info->next->ops->read() blocks.
			 */
			if (variance_dev.flags & VAR_PENDOWN) {
				variance_dev.flags |= VAR_SUBMITNOISE;
				variance_dev.noise = cur;
			}
			/* Reset the state machine on pen up events. */
			variance_dev.flags &= ~(VAR_PENDOWN | VAR_NOISEVALID | VAR_LASTVALID);
			goto acceptsample;
		} else
			variance_dev.flags |= VAR_PENDOWN;

		if (!(variance_dev.flags & VAR_LASTVALID)) {
			variance_dev.last = cur;
			variance_dev.flags |= VAR_LASTVALID;
			continue;
		}

		if (variance_dev.flags & VAR_PENDOWN) {
			/* Compute the distance between last sample and current */
			dist = SQR (cur.x - variance_dev.last.x) +
			       SQR (cur.y - variance_dev.last.y);

			if (dist > variance_dev.delta) {
				/* Do we suspect the previous sample was a noise? */
				if (variance_dev.flags & VAR_NOISEVALID) {
					/* Two "noises": it's just a quick pen movement */
					samp [count++] = variance_dev.last = variance_dev.noise;
					variance_dev.flags = (variance_dev.flags & ~VAR_NOISEVALID) |
						VAR_SUBMITNOISE;

					//senquack - flush sample history in averaging module
					*flush_history = 1;
				} else
					variance_dev.flags |= VAR_NOISEVALID;

				/* The pen jumped too far, maybe it's a noise ... */
				variance_dev.noise = cur;
				continue;
			} else
				variance_dev.flags &= ~VAR_NOISEVALID;
		}

acceptsample:
//#ifdef DEBUG
//		fprintf(stderr,"VARIANCE----------------> %d %d %d\n",
//			var->last.x, var->last.y, var->last.pressure);
//#endif
		samp [count++] = variance_dev.last;
		variance_dev.last = cur;
	}

	return count;
}

typedef struct {
	int x; 
	int y;
} XYHIST;

#define XYHISTLENGTH 15
XYHIST average_xyhist[XYHISTLENGTH];

//senquack - New de-jitterer that can handle high noise level of the GP2X:
static inline int average_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{

	int ret = 0;
	int newx, newy;

	int flush_history = 0; // variance module will tell us if we are getting fast movements
	static int flush_count = 0; // when we get enough fast movements, flush the history
	static int xyhist_full = 0; //senquack - when we have a full four samples, this is 1 
	static int xyhist_counter = 0; // senquack - how many entires in history?
	static int xyhist_index = 0;	//xyhist_index - 1 is index to most-recent sample in array

	ret = variance_read(ts, samp, nr, &flush_history);
	flush_count += flush_history;
	struct ts_sample *s;
	s = samp + (ret - 1); // this module can handle more than one sample, but the gp2x
								// touchscreen driver only ever provides 1 sample each read so
								// it's not really necessary but not really inefficient either way
	int i;

	for( i = ret; i > 0; i--, s--) {
		if ((s->pressure == 0) || (flush_count > 7)) {
			// when pen is lifted, or a fast movement is detected, flush stored history
			xyhist_full = 0;
			xyhist_counter = 0;
			xyhist_index = 0;
			flush_count = 0;
		} else {
			average_xyhist[xyhist_index].x = s->x;
			average_xyhist[xyhist_index].y = s->y;
			
			xyhist_counter++;
			if (xyhist_counter == XYHISTLENGTH) {
				xyhist_full = 1;
			}

			xyhist_index++;
			if (xyhist_index == XYHISTLENGTH) {
				xyhist_index = 0; // point back at beginning since array is full
			}

			if (xyhist_full) {
				//we have a full sample history, we can average this sample with the others

				int j = xyhist_index - 1; // point j to most recent entry in history
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}

				// 15-sample weighted average, pyramid, provides great smoothing for precise
				// operations like menus and drawing

				// sample 1 has weight of * 1
				newx = average_xyhist[j].x;
				newy = average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		
				
				// sample 2 has weight of * 2
				newx += average_xyhist[j].x << 1;
				newy += average_xyhist[j].y << 1;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 3 has weight of * 3
				newx += (average_xyhist[j].x << 1) + average_xyhist[j].x;
				newy += (average_xyhist[j].y << 1) + average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 4 has weight of * 4
				newx += average_xyhist[j].x << 2;
				newy += average_xyhist[j].y << 2;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 5 has weight of * 5
				newx += (average_xyhist[j].x << 2) + average_xyhist[j].x;
				newy += (average_xyhist[j].y << 2) + average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 6 has weight of * 6
				newx += (average_xyhist[j].x << 2) + (average_xyhist[j].x << 1);
				newy += (average_xyhist[j].y << 2) + (average_xyhist[j].y << 1);

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 7 has weight of * 7
				newx += (average_xyhist[j].x << 3) - average_xyhist[j].x;
				newy += (average_xyhist[j].y << 3) - average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 8, middle sample,  has weight of * 8
				newx += (average_xyhist[j].x << 3);
				newy += (average_xyhist[j].y << 3);

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 9 has weight of * 7
				newx += (average_xyhist[j].x << 3) - average_xyhist[j].x;
				newy += (average_xyhist[j].y << 3) - average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 10 has weight of * 6
				newx += (average_xyhist[j].x << 2) + (average_xyhist[j].x << 1);
				newy += (average_xyhist[j].y << 2) + (average_xyhist[j].y << 1);

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 11 has weight of * 5
				newx += (average_xyhist[j].x << 2) + average_xyhist[j].x;
				newy += (average_xyhist[j].y << 2) + average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 12 has weight of * 4
				newx += average_xyhist[j].x << 2;
				newy += average_xyhist[j].y << 2;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		
			
				// sample 13 has weight of * 3
				newx += (average_xyhist[j].x << 1) + average_xyhist[j].x;
				newy += (average_xyhist[j].y << 1) + average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 14 has weight of * 2
				newx += average_xyhist[j].x << 1;
				newy += average_xyhist[j].y << 1;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 15 has weight of * 1
				newx += average_xyhist[j].x;
				newy += average_xyhist[j].y;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		
				
				// divide results by 64 to provide average
				samp->x = newx >> 6;
				samp->y = newy >> 6;
			}
		}
	}

  return ret;
}

/******************************
 * linear
 ******************************/

typedef struct tslib_linear {
  int a[7];
} TSLIB_LINEAR;

TSLIB_LINEAR linear_dev;

void linear_init()
{
  struct stat sbuf;
  int pcal_fd, ret = 0;
  char pcalbuf[200];
  int index;
  char *tokptr;
  char *calfile=NULL;
  char *defaultcalfile="/etc/pointercal";

  /* set defaults to typical GP2X values */
  linear_dev.a[0] = 6203;
  linear_dev.a[1] = 0;
  linear_dev.a[2] = -1501397;
  linear_dev.a[3] = 0;
  linear_dev.a[4] = -4200;
  linear_dev.a[5] = 16132680;
  linear_dev.a[6] = 65536;

	TSLIB_LINEAR tmp_linear_dev;
	memset(pcalbuf, 0, 200);
	int got_good_values = 1;	// only when this remains 1 throughout reading of /etc/pointercal will values be accepted into driver

	if ((calfile = getenv("TSLIB_CALIBFILE")) == NULL) calfile = defaultcalfile;
	if (stat(calfile, &sbuf) == 0) {
		pcal_fd = open(calfile, O_RDONLY);
		if (pcal_fd != -1) {
			int bytes_read = read(pcal_fd, pcalbuf, 200);
			if (bytes_read > 0) { 
				tokptr = strtok(pcalbuf, " ");
				if (tokptr && (isdigit(*tokptr) || (*tokptr == '-'))) {
					tmp_linear_dev.a[0] = atoi(tokptr);
					index = 1;
					while ((index < 7) && got_good_values) {
						tokptr = strtok(NULL, " ");
						if (tokptr && (isdigit(*tokptr) || (*tokptr == '-'))) {
							tmp_linear_dev.a[index] = atoi(tokptr);
							index++;
						} else {
							got_good_values = 0;
						}
					}
				} else {
					got_good_values = 0;
				}
			} else {
				got_good_values = 0;
			}
			
			close(pcal_fd);

		} else {
			got_good_values = 0;
		}

		if (got_good_values) {
			linear_dev = tmp_linear_dev;
			fprintf(stderr, "Successfully loaded saved touchscreen calibration values.\n");
		} else {
			fprintf(stderr, "Error loading saved touchscreen calibration, using default values\n");
		}
  }
}

static inline int linear_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
	int ret;

	ret = average_read(ts, samp, nr);

	if (ret > 0) {
			 if (linear_dev.a[6] == 65536) {
				samp->x = (linear_dev.a[2] +
				 linear_dev.a[0] * samp->x) >> 16;
				samp->y = (linear_dev.a[5] +
				 linear_dev.a[4] * samp->y) >> 16;
			 } else {
				samp->x = (linear_dev.a[2] +
				 linear_dev.a[0] * samp->x) / linear_dev.a[6];
				samp->y = (linear_dev.a[5] +
				 linear_dev.a[4] * samp->y) / linear_dev.a[6];
			 }
		}

	return ret;
}

/******************************
 * ts_read
 ******************************/

int ts_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
	return(linear_read(ts, samp, nr));
}


int ts_config(TSDEV *ts)
{
	linear_init();		// read in touchscreen calibration file for proper interpretation of samples
	variance_init();	// initialize variance module
	return 0;
}

