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

typedef struct tsdev {
  int fd;
} TSDEV;

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
  int ret;

  ret = close(ts->fd);
  free(ts);

  return ret;
}

int ts_fd(TSDEV *ts)
{
  return ts->fd;
}


/******************************
 * ucb1x100-raw
 ******************************/

//DKS - I am having problems reading the device, it is only returning 8 bytes
//typedef struct ucb1x00_ts_event {
//  unsigned short pressure;
//  unsigned short x;
//  unsigned short y;
//  unsigned short pad;
//  struct timeval stamp;
//} UCB1X00_TS_EVENT;
typedef struct ucb1x00_ts_event {
  unsigned short pressure;
  unsigned short x;
  unsigned short y;
  unsigned short pad;
} UCB1X00_TS_EVENT;


//DKS
//static int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp)
//{
//  UCB1X00_TS_EVENT ucb1x00_evt;
//  int ret;
//
//  ret = read(ts->fd, &ucb1x00_evt, sizeof(ucb1x00_evt));
//  if (ret > 0) {
//    samp->x = ucb1x00_evt.x;
//    samp->y = ucb1x00_evt.y;
//    samp->pressure = ucb1x00_evt.pressure;
//    samp->tv.tv_usec = ucb1x00_evt.stamp.tv_usec;
//    samp->tv.tv_sec = ucb1x00_evt.stamp.tv_sec;
//    ret = 1;
//  } else
//    ret = -1;
//
//  return ret;
//}
//DKS - known working copy, latest one before change to read multiple samples:
//static int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp)
//{
//  UCB1X00_TS_EVENT ucb1x00_evt;
//  int ret;
//
//  ret = read(ts->fd, &ucb1x00_evt, sizeof(ucb1x00_evt));
//  if (ret > 0) {
//    samp->x = ucb1x00_evt.x;
//    samp->y = ucb1x00_evt.y;
//    samp->pressure = ucb1x00_evt.pressure;
//
//	 //DKS
//#ifdef DKS_DEBUG
//	 if (samp->pressure > 0) {
//		 if (samp->pressure < tslib_minpressure) {
//			 tslib_minpressure = samp->pressure;
//		 } 
//		 if (samp->pressure > tslib_maxpressure) {
//			 tslib_maxpressure = samp->pressure;
//		 }
//	 }
//#endif
//
//	 //DKS - we're never going to need this
////    samp->tv.tv_usec = ucb1x00_evt.stamp.tv_usec;
////    samp->tv.tv_sec = ucb1x00_evt.stamp.tv_sec;
//    ret = 1;
//  } else
//    ret = -1;
//
//  return ret;
//}
//DKS - OK, hours of work only to find you cannot read more than one sample at a time
// out of the touchscreen device
//DKS - I am gonna disable this code for now, change it to a for loop and read more at a time
//static inline int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
//{
////	struct tsdev *ts = inf->dev;
////	struct ucb1x00_ts_event *ucb1x00_evt;
//	UCB1X00_TS_EVENT *ucb1x00_evt;
//	int ret;
////	int total = 0;
//	ucb1x00_evt = alloca(sizeof(UCB1X00_TS_EVENT) * nr);
//
//	tslib_debug5 = sizeof(UCB1X00_TS_EVENT) * nr; // DKS - got 16 here
//
//	ret = read(ts->fd, ucb1x00_evt, sizeof(UCB1X00_TS_EVENT) * nr); 
//
//	//DKS - debugging
////	tslib_debug = ret; // this gave me 8
//	tslib_debug = sizeof(UCB1X00_TS_EVENT); // this gave me 16
//	tslib_debug4 = ret; // GOT 8 here	  
//
//	int numread = 0;
//	if(ret > 0) {
//		numread = ret / sizeof(UCB1X00_TS_EVENT);
//
//		tslib_debug2 = numread;
//
//		while(ret >= (int)sizeof(UCB1X00_TS_EVENT)) {
//			samp->x = ucb1x00_evt->x;
//			samp->y = ucb1x00_evt->y;
//			samp->pressure = ucb1x00_evt->pressure;
////#ifdef DEBUG
////        fprintf(stderr,"RAW---------------------------> %d %d %d\n",samp->x,samp->y,samp->pressure);
////#endif /*DEBUG*/
//		  //DKS - won't need these
////			samp->tv.tv_usec = ucb1x00_evt->stamp.tv_usec;
////			samp->tv.tv_sec = ucb1x00_evt->stamp.tv_sec;
//			samp++;
//			ucb1x00_evt++;
//			ret -= sizeof(UCB1X00_TS_EVENT);
//		}
//	} else {
//		return -1;
//	}
//
//	ret = numread;
//	return ret;
//}
//DKS - Originially, I had made this read multiple samples but it turns out the driver
//for the touchscreen is set up to give only one sample per read, so had to go back
//to this optimized version.  It should be noted that the device file only ever provides
//8 bytes of data, basically 3shorts: x,y,and pressure, and the old code tried to read
//data that was never provided.
static inline int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
	UCB1X00_TS_EVENT ucb1x00_evt;

	if (read(ts->fd, &ucb1x00_evt, sizeof(UCB1X00_TS_EVENT)) == sizeof(UCB1X00_TS_EVENT)) {
			samp->x = ucb1x00_evt.x;
			samp->y = ucb1x00_evt.y;
			samp->pressure = ucb1x00_evt.pressure;
			return 1;
	} else {
		return -1;
	}

}


/******************************
 * pthres
 ******************************/

//DKS - don't need these
//typedef struct tslib_pthres {
//  unsigned int pmin;
//  unsigned int pmax;
//} TSLIB_PTHRES;
//
//TSLIB_PTHRES pthres_dev;
//
//int threshold_vars(char *str, void *data)
//{
//  unsigned long v;
//  int err = errno;
//
//  v = strtoul(str, NULL, 0);
//  if (v == ULONG_MAX && errno == ERANGE)
//    return -1;
//  errno = err;
//  switch ((int)data) {
//  case 0:
//    pthres_dev.pmin = v;
//    break;
//  case 1:
//    pthres_dev.pmax = v;
//    break;
//  default:
//    return -1;
//  }
//  return 0;
//}

//DKS - don't need this
//static const TSLIB_VARS pthres_vars[] = {
//  {"pmin", (void *)0, threshold_vars},
//  {"pmax", (void *)1, threshold_vars}
//};

//DKS - don't need this
//#define NR_PTHRES_VARS (sizeof(pthres_vars) / sizeof(pthres_vars[0]))


//DKS - no idea why we might still need this
//int pthres_init(const char *params)
//{
//  int ret = 0;
//
//  pthres_dev.pmin = 1;
//  pthres_dev.pmax = INT_MAX;
//  if (tslib_parse_vars(pthres_vars, NR_PTHRES_VARS, params)) {
//    ret = 1;
//  }
//
//  return ret;
//}

//DKS ok, there is a bad pointer dereference in TS_SAMPLE below, gonna copy and fix it.
//int pthres_read(TSDEV *ts, TS_SAMPLE *samp)
//{
//#ifdef DKS_DEBUG
//	//DKS - continuously update this to current global variable value, so programs can
//	// tweak it real-time
//  pthres_dev.pmin = tslib_pmin;
//#endif
//
//  int ret;
//  static int xsave = 0, ysave = 0;
//  static int press = 0;
//
//  ret = ucb1x00_read(ts, samp);
//  if (ret >= 0) {
//    TS_SAMPLE *s;
//
//    if (s->pressure < pthres_dev.pmin) {
//      if (press != 0) {
//	press = 0;
//	s->pressure = 0;
//	s->x = xsave;
//	s->y = ysave;
//	ret = 1;
//      } else
//	ret = 0;
//    } else {
//      if (s->pressure > pthres_dev.pmax)
//	ret = 0;
//      else {
//	press = 1;
//	xsave = s->x;
//	ysave = s->y;
//	ret = 1;
//      }
//    }
//  }
//
//  return ret;
//}
//int pthres_read(TSDEV *ts, TS_SAMPLE *samp)
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
//DKS - known working copy of multiple-sample version, but changing back to 1-sample for now
//#define MINPRESSURE 35000  //DKS - for the GP2X, reading values below this results in
//									// tapping inaccuracies
////static int
//static inline int
////pthres_read(struct tslib_module_info *info, struct ts_sample *samp, int nr)
//pthres_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
//{
////	struct tslib_pthres *p = (struct tslib_pthres *)info;
//	int ret;
//	static int xsave = 0, ysave = 0;
//	static int press = 0;
//
////	ret = info->next->ops->read(info->next, samp, nr);
//	ret = ucb1x00_read(ts, samp, nr); 
//	if (ret >= 0) {
//		int nr = 0, i;
//		struct ts_sample *s;
//
//		for (s = samp, i = 0; i < ret; i++, s++) {
////			if (s->pressure < p->pmin) {
//			if (s->pressure < MINPRESSURE) {
//				if (press != 0) {
//					/* release */
//					press = 0;
//					s->pressure = 0;
//					s->x = xsave;
//					s->y = ysave;
//				} else {
//					/* release with no press, outside bounds, dropping */
//					int left = ret - nr - 1;
//					if (left > 0) {
//						memmove(s, s + 1, left * sizeof(struct ts_sample));
//						s--;
//						continue;
//					}
//					break;
//				}
//			} else {
////DKS - on the GP2X F200, we don't worry about pressure maximums
////				if (s->pressure > p->pmax) {
////					/* pressure outside bounds, dropping */
////					int left = ret - nr - 1;
////					if (left > 0) {
////						memmove(s, s + 1, left * sizeof(struct ts_sample));
////						s--;
////						continue;
////					}
////					break;
////				}
//				/* press */
//				press = 1;
//				xsave = s->x;
//				ysave = s->y;
//			}
//			nr++;
//		}
//		return nr;
//	}
//	return ret;
//}

//DKS - no idea why we might need this still
//static int pthres_fini(struct tslib_module_info *info)
//{
//	free(info);
//	return 0;
//}


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

static int variance_limit(char *str, void *data)
{
  unsigned long v;
  int err = errno;

  v = strtoul(str, NULL, 0);
  if (v == ULONG_MAX && errno == ERANGE)
    return -1;
  errno = err;
  switch ((int)data) {
  case 1:
    variance_dev.delta = v;
    break;
  default:
    return -1;
  }
  return 0;
}

static const TSLIB_VARS variance_vars[] = {
  {"delta", (void *)1, variance_limit}
};

#define NR_VAR_VARS (sizeof(variance_vars) / sizeof(variance_vars[0]))

int variance_init(const char *params)
{
  int ret = 0;

  bzero(&variance_dev, sizeof(variance_dev));
  variance_dev.delta = 30;
  variance_dev.flags = 0;
  if (tslib_parse_vars(variance_vars, NR_VAR_VARS, params)) {
    ret = 1;
  }
//  variance_dev.delta = sqr(variance_dev.delta);
  variance_dev.delta = SQR(variance_dev.delta);

  return ret;
}

//DKS - added parameter, flush_history, that gets assigned to 1 if a fast movement is
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

					//DKS - flush sample history in averaging module
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


/******************************
 * dejitter
 ******************************/

//#define NR_SAMPHISTLEN 4
//static const unsigned char weight[NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
//  {
//	  //    DKS - experiment.. the last of each row is the number 2^x that we should divide by, 
//	  //    the first four numbers in each row should add up to 2^x
////    {5, 3, 0, 0, 3},
////    {8, 5, 3, 0, 4},
////    {6, 4, 3, 3, 4}
//    {1, 3, 2, 2, 3},
//    {2, 6, 4, 4, 4},
//    {6, 4, 3, 3, 4}
//  };
//
//typedef struct ts_hist {
//  int x;
//  int y;
//  unsigned int p;
//} TS_HIST;
//
//typedef struct tslib_dejitter {
//  int delta;
//  int x;
//  int y;
//  int down;
//  int nr;
//  int head;
//  TS_HIST hist[NR_SAMPHISTLEN];
//} TSLIB_DEJITTER;
//
//TSLIB_DEJITTER dejitter_dev;
//
//static int dejitter_limit(char *str, void *data)
//{
//  unsigned long v;
//  int err = errno;
//
//  v = strtoul(str, NULL, 0);
//  if (v == ULONG_MAX && errno == ERANGE)
//    return -1;
//  errno = err;
//  switch ((int)data) {
//  case 1:
//    dejitter_dev.delta = v;
//    break;
//  default:
//    return -1;
//  }
//  return 0;
//}
//
//static const TSLIB_VARS dejitter_vars[] = {
//  {"delta", (void *)1, dejitter_limit}
//};
//
//#define NR_DEJITTER_VARS (sizeof(dejitter_vars) / sizeof(dejitter_vars[0]))

//DKS - changing to use SQR macro, not that it matters here
//int dejitter_init(const char *params)
//{
//  int ret = 0;
//
//  bzero(&dejitter_dev, sizeof(dejitter_dev));
//  dejitter_dev.delta = 100;
//  dejitter_dev.head = 0;
//  if (tslib_parse_vars(dejitter_vars, NR_DEJITTER_VARS, params)) {
//    ret = 1;
//  }
//  dejitter_dev.delta = sqr(dejitter_dev.delta);
//
//  return ret;
//}
//int dejitter_init(const char *params)
//{
//  int ret = 0;
//
//  bzero(&dejitter_dev, sizeof(dejitter_dev));
//  dejitter_dev.delta = 100;
//  dejitter_dev.head = 0;
//  if (tslib_parse_vars(dejitter_vars, NR_DEJITTER_VARS, params)) {
//    ret = 1;
//  }
////  dejitter_dev.delta = sqr(dejitter_dev.delta);
//  dejitter_dev.delta = SQR(dejitter_dev.delta);
//
//  return ret;
//}

//static void average(TS_SAMPLE *samp)
//{
//  const unsigned char *w;
//  int sn = dejitter_dev.head;
//  int i, x = 0, y = 0;
//  unsigned int p = 0;
//
//  w = weight[dejitter_dev.nr - 2];
//
//  for (i = 0; i < dejitter_dev.nr; i++) {
//    x += dejitter_dev.hist[sn].x * w[i];
//    y += dejitter_dev.hist[sn].y * w[i];
//    p += dejitter_dev.hist[sn].p * w[i];
//    sn = (sn - 1) & (NR_SAMPHISTLEN - 1);
//  }
//
//  samp->x = x >> w[NR_SAMPHISTLEN];
//  samp->y = y >> w[NR_SAMPHISTLEN];
//  samp->pressure = p >> w[NR_SAMPHISTLEN];
//}
//
//static int dejitter_read(TSDEV *ts, TS_SAMPLE *samp)
//{
//#ifdef DKS_DEBUG
//  dejitter_dev.delta = tslib_dejitter;
//  if (tslib_skipdejitter) {
//  		return (variance_read(ts, samp));
//  }
//#endif
//  TS_SAMPLE *s;
//  int count = 0, ret;
//
//  ret = variance_read(ts, samp);
//  for (s = samp; ret > 0; s++, ret--) {
//    if (s->pressure == 0) {
//      dejitter_dev.nr = 0;
//      samp[count++] = *s;
//      continue;
//    }
//
//    if (dejitter_dev.nr) {
//      int prev = (dejitter_dev.head - 1) & (NR_SAMPHISTLEN - 1);
//      if (sqr(s->x - dejitter_dev.hist[prev].x) +
//	  sqr(s->y - dejitter_dev.hist[prev].y) > dejitter_dev.delta)
//	dejitter_dev.nr = 0;
//    }
//
//    dejitter_dev.hist[dejitter_dev.head].x = s->x;
//    dejitter_dev.hist[dejitter_dev.head].y = s->y;
//    dejitter_dev.hist[dejitter_dev.head].p = s->pressure;
//    if (dejitter_dev.nr < NR_SAMPHISTLEN)
//      dejitter_dev.nr++;
//
//    if (dejitter_dev.nr == 1)
//      samp[count] = *s;
//    else {
//      average(samp + count);
//      samp[count].tv = s->tv;
//    }
//    count++;
//    dejitter_dev.head = (dejitter_dev.head + 1) & (NR_SAMPHISTLEN - 1);
//  }
//
//  return count;
//}

//DKS - gonna do my own de-jitterer that can handle high noise level of the GP2X
typedef struct {
	int x; 
	int y;
} XYHIST;

//#define XYHISTLENGTH 15
//XYHIST average_xyhist[XYHISTLENGTH];
//
//static inline int average_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
//{
//
//	int ret = 0;
//	int newx, newy;
//
//	int flush_history = 0; // variance module will tell us if we are getting fast movements
//	static int flush_count = 0; // when we get enough fast movements, flush the history
//	static int xyhist_full = 0; //DKS - when we have a full four samples, this is 1 
//	static int xyhist_counter = 0; // DKS - how many entires in history?
//	static int xyhist_index = 0;	//xyhist_index - 1 is index to most-recent sample in array
//
//	ret = variance_read(ts, samp, nr, &flush_history);
//	flush_count += flush_history;
//	struct ts_sample *s;
//	s = samp + (ret - 1); // this module can handle more than one sample, but the gp2x
//								// touchscreen driver only ever provides 1 sample each read so
//								// it's not really necessary but not really inefficient either way
//	int i;
//
//	for( i = ret; i > 0; i--, s--) {
//		if ((s->pressure == 0) || (flush_count > 7)) {
//			// when pen is lifted, or a fast movement is detected, flush stored history
//			xyhist_full = 0;
//			xyhist_counter = 0;
//			xyhist_index = 0;
//			flush_count = 0;
//		} else {
//			average_xyhist[xyhist_index].x = s->x;
//			average_xyhist[xyhist_index].y = s->y;
//			
//			xyhist_counter++;
//			if (xyhist_counter == XYHISTLENGTH) {
//				xyhist_full = 1;
//			}
//
//			xyhist_index++;
//			if (xyhist_index == XYHISTLENGTH) {
//				xyhist_index = 0; // point back at beginning since array is full
//			}
//
//			if (xyhist_full) {
//				//we have a full sample history, we can average this sample with the others
//
//				int j = xyhist_index - 1; // point j to most recent entry in history
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}
//
//				// 15-sample weighted average, pyramid, provides great smoothing for precise
//				// operations like menus and drawing
//
//				// sample 1 has weight of * 1
//				newx = average_xyhist[j].x;
//				newy = average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//				
//				// sample 2 has weight of * 2
//				newx += average_xyhist[j].x << 1;
//				newy += average_xyhist[j].y << 1;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 3 has weight of * 3
//				newx += (average_xyhist[j].x << 1) + average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 1) + average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 4 has weight of * 4
//				newx += average_xyhist[j].x << 2;
//				newy += average_xyhist[j].y << 2;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 5 has weight of * 5
//				newx += (average_xyhist[j].x << 2) + average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 2) + average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 6 has weight of * 6
//				newx += (average_xyhist[j].x << 2) + (average_xyhist[j].x << 1);
//				newy += (average_xyhist[j].y << 2) + (average_xyhist[j].y << 1);
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 7 has weight of * 7
//				newx += (average_xyhist[j].x << 3) - average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 3) - average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 8, middle sample,  has weight of * 8
//				newx += (average_xyhist[j].x << 3);
//				newy += (average_xyhist[j].y << 3);
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 9 has weight of * 7
//				newx += (average_xyhist[j].x << 3) - average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 3) - average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 10 has weight of * 6
//				newx += (average_xyhist[j].x << 2) + (average_xyhist[j].x << 1);
//				newy += (average_xyhist[j].y << 2) + (average_xyhist[j].y << 1);
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 11 has weight of * 5
//				newx += (average_xyhist[j].x << 2) + average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 2) + average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 12 has weight of * 4
//				newx += average_xyhist[j].x << 2;
//				newy += average_xyhist[j].y << 2;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//			
//				// sample 13 has weight of * 3
//				newx += (average_xyhist[j].x << 1) + average_xyhist[j].x;
//				newy += (average_xyhist[j].y << 1) + average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 14 has weight of * 2
//				newx += average_xyhist[j].x << 1;
//				newy += average_xyhist[j].y << 1;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//
//				// sample 15 has weight of * 1
//				newx += average_xyhist[j].x;
//				newy += average_xyhist[j].y;
//
//				j--;
//				if (j < 0) {
//					j = XYHISTLENGTH - 1;
//				}		
//				
//				// divide results by 64 to provide average
//				samp->x = newx >> 6;
//				samp->y = newy >> 6;
//			}
//		}
//	}
//
//  return ret;
//}
#define XYHISTLENGTH 5
XYHIST average_xyhist[XYHISTLENGTH];

static inline int average_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{

	int ret = 0;
	int newx, newy;

	int flush_history = 0; // variance module will tell us if we are getting fast movements
	static int flush_count = 0; // when we get enough fast movements, flush the history
	static int xyhist_full = 0; //DKS - when we have a full four samples, this is 1 
	static int xyhist_counter = 0; // DKS - how many entires in history?
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

				// sample 1 has weight of * 15
				newx = average_xyhist[j].x * 15;
				newy = average_xyhist[j].y * 15;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		
				
				// sample 2 has weight of * 14
				newx += average_xyhist[j].x * 14;
				newy += average_xyhist[j].y * 14;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 3 has weight of * 13
				newx += average_xyhist[j].x * 13;
				newy += average_xyhist[j].y * 13;
				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 4 has weight of * 12
				newx += average_xyhist[j].x * 12;
				newy += average_xyhist[j].y * 12;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 5 has weight of * 11 
				newx += average_xyhist[j].x * 11;
				newy += average_xyhist[j].y * 11;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 6 has weight of * 10
				newx += average_xyhist[j].x * 10;
				newy += average_xyhist[j].y * 10;

				j--;
				if (j < 0) {
					j = XYHISTLENGTH - 1;
				}		

				// sample 7 has weight of * 9
				newx += average_xyhist[j].x * 9;
				newy += average_xyhist[j].y * 9;

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
				
				samp->x = newx / 120;
				samp->y = newy / 120;
			}
		}
	}

  return ret;
}

/******************************
 * linear
 ******************************/

typedef struct tslib_linear {
  int swap_xy;
  int a[7];
} TSLIB_LINEAR;

TSLIB_LINEAR linear_dev;

//DKS - don't need this anymore
//static int linear_xyswap(char *str, void *data)
//{
//  linear_dev.swap_xy = 1;
//  return 0;
//}

//DKS - don't need this anymore
//static const TSLIB_VARS linear_vars[] = {
//  {"xyswap", (void *)1, linear_xyswap}
//};
//#define NR_LINEAR_VARS (sizeof(linear_vars) / sizeof(linear_vars[0]))

int linear_init(const char *params)
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
  //  DKS
//  linear_dev.swap_xy = 0;

  if ((calfile = getenv("TSLIB_CALIBFILE")) == NULL) calfile = defaultcalfile;
  if (stat(calfile, &sbuf) == 0) {
    pcal_fd = open(calfile, O_RDONLY);
    read(pcal_fd, pcalbuf, 200);
    linear_dev.a[0] = atoi(strtok(pcalbuf, " "));
    index = 1;
    while (index < 7) {
      tokptr = strtok(NULL, " ");
      if (*tokptr != '\0') {
	linear_dev.a[index] = atoi(tokptr);
	index++;
      }
    }
    close(pcal_fd);
  }
  //  DKS
//  if (tslib_parse_vars(linear_vars, NR_LINEAR_VARS, params))
//    ret = -1;

  return ret;
}


//DKS - last known working copy, before conversion to multi-sample
//int linear_read(TSDEV *ts, TS_SAMPLE *samp)
//{
//  int ret;
//  int xtemp, ytemp;
//
//  //DKS - experimenting with my own filter
////  ret = dejitter_read(ts, samp);
//  ret = average_read(ts, samp);
//  //DKS - why not try averaging after linear instead of before?
////	ret = variance_read(ts, samp);
//  if (ret >= 0) {
//    xtemp = samp->x;
//    ytemp = samp->y;
//    if (linear_dev.a[6] == 65536) {
//      samp->x = (linear_dev.a[2] +
//		 linear_dev.a[0] * xtemp) >> 16;
//      samp->y = (linear_dev.a[5] +
//		 linear_dev.a[4] * ytemp) >> 16;
//    } else {
//      samp->x = (linear_dev.a[2] +
//		 linear_dev.a[0] * xtemp) / linear_dev.a[6];
//      samp->y = (linear_dev.a[5] +
//		 linear_dev.a[4] * ytemp) / linear_dev.a[6];
//    }
//
//	 //DKS - why was this enabled?  I hope this isn't related to the MK2 F200's swapping problem
//
////    if (linear_dev.swap_xy) {
////      int tmp = samp->x;
////      samp->x = samp->y;
////      samp->y = tmp;
////    }
//  }
//
//  return ret;
//}
static inline int linear_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
	int ret;

////  ret = dejitter_read(ts, samp);
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
  //  DKS - original line:
//  ret = linear_read(ts, samp);
	return(linear_read(ts, samp, nr));
}


/*
 * ts_config
 */

#define BUF_SIZE 512
static const char *DEFAULT_TSLIB_CONF[] = {
  "module pthres pmin=1",
  "module variance delta=30",
  //  //DKS
//  "module dejitter delta=100",
  "module linear",
  NULL
};
const char *sep = " \t";

char *ts_fgets(char *buf, int size, FILE *f, int line)
{
  char *ret;

  if (f)
    ret = fgets(buf, size, f);
  else {
    if (DEFAULT_TSLIB_CONF[line] == NULL)
      ret = NULL;
    else
      ret = strcpy(buf, DEFAULT_TSLIB_CONF[line]);
  }

  return ret;
}


int ts_config(TSDEV *ts)
{
  char buf[BUF_SIZE], *p;
  FILE *f;
  int line = 0;
  int ret = 0;

  char *conffile;

  if ((conffile = getenv("TSLIB_CONFFILE")) == NULL) {
    conffile = "/etc/ts.conf";
  }

  f = fopen(conffile, "r");

  buf[BUF_SIZE - 2] = '\0';
  while ((p = ts_fgets(buf, BUF_SIZE, f, line)) != NULL) {
    char *e;
    char *tok;
    char *module_name;

    line++;
    e = strchr(p, '\n');
    if (e)
      *e = '\0';

    if (buf[BUF_SIZE - 2] != '\0') {
      fprintf(stderr, "%s: line %d too long\n", conffile, line);
      break;
    }

    tok = strsep(&p, sep);
    if (p == NULL || *tok == '#')
      continue;

    if (strcasecmp(tok, "module") == 0) {
      module_name = strsep(&p, sep);
      ret = ts_load_module(ts, module_name, p);
    } else if (strcasecmp(tok, "module_raw") == 0) {
      module_name = strsep(&p, sep);
      ret = 0; /* ignore */
    } else {
      fprintf(stderr, "%s: Unrecognised option %s at line %d\n",
	      conffile, tok, line);
      break;
    }
    if (ret != 0) {
      fprintf(stderr, "Couldn't load module %s\n", module_name);
      break;
    }
  }

  if (f) fclose(f);
  
  return ret;
}


int ts_load_module(TSDEV *ts, const char *module, const char *p)
{
  int ret;


//  if (!strcmp(module, "pthres"))
//DKS - disabled this, no initialization needed
//    ret = pthres_init(p);
  if (!strcmp(module, "variance"))
    ret = variance_init(p);
//  else if (!strcmp(module, "dejitter"))
//  DKS - dejitter module disabled
//    ret = dejitter_init(p);
  else if (!strcmp(module, "linear"))
    ret = linear_init(p);
  else
    ret = 0;

  return ret;
}

char s_holder[1024];

int tslib_parse_vars(const TSLIB_VARS *vars, int nr, const char *str)
{
  char *s, *p;
  int ret = 0;

  if (!str)
    return 0;

  bzero(s_holder, 1024);
  strncpy(s_holder, str, strlen(str));
  s = s_holder;
  while ((p = strsep(&s, sep)) != NULL && ret == 0) {
    const TSLIB_VARS *v;
    char *eq;

    eq = strchr(p, '=');
    if (eq)
      *eq++ = '\0';

    for (v = vars; v < vars + nr; v++)
      if (strcasecmp(v->name, p) == 0) {
	ret = v->fn(eq, v->data);
	break;
      }
  }

  return ret;
}
