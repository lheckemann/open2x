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

static int sqr(int x)
{
  return x*x;
}


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

typedef struct ucb1x00_ts_event {
  unsigned short pressure;
  unsigned short x;
  unsigned short y;
  unsigned short pad;
  struct timeval stamp;
} UCB1X00_TS_EVENT;


static int ucb1x00_read(TSDEV *ts, TS_SAMPLE *samp)
{
  UCB1X00_TS_EVENT ucb1x00_evt;
  int ret;

  ret = read(ts->fd, &ucb1x00_evt, sizeof(ucb1x00_evt));
  if (ret > 0) {
    samp->x = ucb1x00_evt.x;
    samp->y = ucb1x00_evt.y;
    samp->pressure = ucb1x00_evt.pressure;
    samp->tv.tv_usec = ucb1x00_evt.stamp.tv_usec;
    samp->tv.tv_sec = ucb1x00_evt.stamp.tv_sec;
    ret = 1;
  } else
    ret = -1;

  return ret;
}


/******************************
 * pthres
 ******************************/

typedef struct tslib_pthres {
  unsigned int pmin;
  unsigned int pmax;
} TSLIB_PTHRES;

TSLIB_PTHRES pthres_dev;

int threshold_vars(char *str, void *data)
{
  unsigned long v;
  int err = errno;

  v = strtoul(str, NULL, 0);
  if (v == ULONG_MAX && errno == ERANGE)
    return -1;
  errno = err;
  switch ((int)data) {
  case 0:
    pthres_dev.pmin = v;
    break;
  case 1:
    pthres_dev.pmax = v;
    break;
  default:
    return -1;
  }
  return 0;
}

static const TSLIB_VARS pthres_vars[] = {
  {"pmin", (void *)0, threshold_vars},
  {"pmax", (void *)1, threshold_vars}
};

#define NR_PTHRES_VARS (sizeof(pthres_vars) / sizeof(pthres_vars[0]))


int pthres_init(const char *params)
{
  int ret = 0;

  pthres_dev.pmin = 1;
  pthres_dev.pmax = INT_MAX;
  if (tslib_parse_vars(pthres_vars, NR_PTHRES_VARS, params)) {
    ret = 1;
  }

  return ret;
}


int pthres_read(TSDEV *ts, TS_SAMPLE *samp)
{
  int ret;
  static int xsave = 0, ysave = 0;
  static int press = 0;

  ret = ucb1x00_read(ts, samp);
  if (ret >= 0) {
    TS_SAMPLE *s;

    if (s->pressure < pthres_dev.pmin) {
      if (press != 0) {
	press = 0;
	s->pressure = 0;
	s->x = xsave;
	s->y = ysave;
	ret = 1;
      } else
	ret = 0;
    } else {
      if (s->pressure > pthres_dev.pmax)
	ret = 0;
      else {
	press = 1;
	xsave = s->x;
	ysave = s->y;
	ret = 1;
      }
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
  variance_dev.delta = sqr(variance_dev.delta);

  return ret;
}

static int variance_read(TSDEV *ts, TS_SAMPLE *samp)
{
  TS_SAMPLE cur;
  int count = 0, dist;

  while (count < 1) {
    if (variance_dev.flags & VAR_SUBMITNOISE) {
      cur = variance_dev.noise;
      variance_dev.flags &= ~VAR_SUBMITNOISE;
    } else {
      if (pthres_read(ts, &cur) < 1)
	return count;
    }
    
    if (cur.pressure == 0) {
      if (variance_dev.flags & VAR_PENDOWN) {
	variance_dev.flags |= VAR_SUBMITNOISE;
	variance_dev.noise = cur;
      }
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
      dist = sqr(cur.x - variance_dev.last.x) +
	     sqr(cur.y - variance_dev.last.y);

      if (dist > variance_dev.delta) {
	if (variance_dev.flags & VAR_NOISEVALID) {
	  samp[count++] = variance_dev.last = variance_dev.noise;
	  variance_dev.flags = (variance_dev.flags & ~VAR_NOISEVALID) |
	                       VAR_SUBMITNOISE;
	} else
	  variance_dev.flags |= VAR_NOISEVALID;

	variance_dev.noise = cur;
	continue;
      } else
	variance_dev.flags &= ~VAR_NOISEVALID;
    }
  acceptsample:
    samp[count++] = variance_dev.last;
    variance_dev.last = cur;
  }

  return count;
}


/******************************
 * dejitter
 ******************************/

#define NR_SAMPHISTLEN 4
static const unsigned char weight[NR_SAMPHISTLEN - 1][NR_SAMPHISTLEN + 1] =
  {
    {5, 3, 0, 0, 3},
    {8, 5, 3, 0, 4},
    {6, 4, 3, 3, 4}
  };

typedef struct ts_hist {
  int x;
  int y;
  unsigned int p;
} TS_HIST;

typedef struct tslib_dejitter {
  int delta;
  int x;
  int y;
  int down;
  int nr;
  int head;
  TS_HIST hist[NR_SAMPHISTLEN];
} TSLIB_DEJITTER;

TSLIB_DEJITTER dejitter_dev;

static int dejitter_limit(char *str, void *data)
{
  unsigned long v;
  int err = errno;

  v = strtoul(str, NULL, 0);
  if (v == ULONG_MAX && errno == ERANGE)
    return -1;
  errno = err;
  switch ((int)data) {
  case 1:
    dejitter_dev.delta = v;
    break;
  default:
    return -1;
  }
  return 0;
}

static const TSLIB_VARS dejitter_vars[] = {
  {"delta", (void *)1, dejitter_limit}
};

#define NR_DEJITTER_VARS (sizeof(dejitter_vars) / sizeof(dejitter_vars[0]))

int dejitter_init(const char *params)
{
  int ret = 0;

  bzero(&dejitter_dev, sizeof(dejitter_dev));
  dejitter_dev.delta = 100;
  dejitter_dev.head = 0;
  if (tslib_parse_vars(dejitter_vars, NR_DEJITTER_VARS, params)) {
    ret = 1;
  }
  dejitter_dev.delta = sqr(dejitter_dev.delta);

  return ret;
}

static void average(TS_SAMPLE *samp)
{
  const unsigned char *w;
  int sn = dejitter_dev.head;
  int i, x = 0, y = 0;
  unsigned int p = 0;

  w = weight[dejitter_dev.nr - 2];

  for (i = 0; i < dejitter_dev.nr; i++) {
    x += dejitter_dev.hist[sn].x * w[i];
    y += dejitter_dev.hist[sn].y * w[i];
    p += dejitter_dev.hist[sn].p * w[i];
    sn = (sn - 1) & (NR_SAMPHISTLEN - 1);
  }

  samp->x = x >> w[NR_SAMPHISTLEN];
  samp->y = y >> w[NR_SAMPHISTLEN];
  samp->pressure = p >> w[NR_SAMPHISTLEN];
}

static int dejitter_read(TSDEV *ts, TS_SAMPLE *samp)
{
  TS_SAMPLE *s;
  int count = 0, ret;

  ret = variance_read(ts, samp);
  for (s = samp; ret > 0; s++, ret--) {
    if (s->pressure == 0) {
      dejitter_dev.nr = 0;
      samp[count++] = *s;
      continue;
    }

    if (dejitter_dev.nr) {
      int prev = (dejitter_dev.head - 1) & (NR_SAMPHISTLEN - 1);
      if (sqr(s->x - dejitter_dev.hist[prev].x) +
	  sqr(s->y - dejitter_dev.hist[prev].y) > dejitter_dev.delta)
	dejitter_dev.nr = 0;
    }

    dejitter_dev.hist[dejitter_dev.head].x = s->x;
    dejitter_dev.hist[dejitter_dev.head].y = s->y;
    dejitter_dev.hist[dejitter_dev.head].p = s->pressure;
    if (dejitter_dev.nr < NR_SAMPHISTLEN)
      dejitter_dev.nr++;

    if (dejitter_dev.nr == 1)
      samp[count] = *s;
    else {
      average(samp + count);
      samp[count].tv = s->tv;
    }
    count++;
    dejitter_dev.head = (dejitter_dev.head + 1) & (NR_SAMPHISTLEN - 1);
  }

  return count;
}


/******************************
 * linear
 ******************************/

typedef struct tslib_linear {
  int swap_xy;
  int a[7];
} TSLIB_LINEAR;

TSLIB_LINEAR linear_dev;

static int linear_xyswap(char *str, void *data)
{
  linear_dev.swap_xy = 1;
  return 0;
}

static const TSLIB_VARS linear_vars[] = {
  {"xyswap", (void *)1, linear_xyswap}
};

#define NR_LINEAR_VARS (sizeof(linear_vars) / sizeof(linear_vars[0]))

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
  linear_dev.swap_xy = 0;

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
  if (tslib_parse_vars(linear_vars, NR_LINEAR_VARS, params))
    ret = -1;

  return ret;
}


int linear_read(TSDEV *ts, TS_SAMPLE *samp)
{
  int ret;
  int xtemp, ytemp;

  ret = dejitter_read(ts, samp);
  if (ret >= 0) {
    xtemp = samp->x;
    ytemp = samp->y;
    if (linear_dev.a[6] == 65536) {
      samp->x = (linear_dev.a[2] +
		 linear_dev.a[0] * xtemp) >> 16;
      samp->y = (linear_dev.a[5] +
		 linear_dev.a[4] * ytemp) >> 16;
    } else {
      samp->x = (linear_dev.a[2] +
		 linear_dev.a[0] * xtemp) / linear_dev.a[6];
      samp->y = (linear_dev.a[5] +
		 linear_dev.a[4] * ytemp) / linear_dev.a[6];
    }
    if (linear_dev.swap_xy) {
      int tmp = samp->x;
      samp->x = samp->y;
      samp->y = tmp;
    }
  }

  return ret;
}


/******************************
 * ts_read
 ******************************/

int ts_read(TSDEV *ts, TS_SAMPLE *samp, int nr)
{
  int ret;

  ret = linear_read(ts, samp);

  return ret;
}


/*
 * ts_config
 */

#define BUF_SIZE 512
static const char *DEFAULT_TSLIB_CONF[] = {
  "module pthres pmin=1",
  "module variance delta=30",
  "module dejitter delta=100",
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

  if (!strcmp(module, "pthres"))
    ret = pthres_init(p);
  else if (!strcmp(module, "variance"))
    ret = variance_init(p);
  else if (!strcmp(module, "dejitter"))
    ret = dejitter_init(p);
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
