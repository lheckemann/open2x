#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>

#include "arm_colorconv.h"

typedef uint32_t CARD32;
typedef uint16_t CARD16;

#define TEST_WIDTH 400
#define TEST_HEIGHT 240
#define TEST_LOOPCOUNT 2511

/**
 * Convertion from YV12 to YUV420, a reference implementation taken from xomap 
 * xserver sources (omap_video.c by Daniel Stone)
 */
void yv12_to_yuv420_xomap(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w)
{
    int j;
    CARD32 *d2;
    CARD32 *sy = (CARD32 *)y;
    CARD16 *sc = (CARD16 *)chroma;
    CARD16 *d1 = (CARD16 *)dst;
    w >>= 2;

    for (j = 0; j < w; j++) {
        if (((unsigned long) d1) & 3) {
            /* Luma 1, chroma 1. */
            *d1++ = (*sy & 0x000000ff) | ((*sc & 0x00ff) << 8);
            /* Chroma 2, luma 2. */
            *d1++ = ((*sc & 0xff00) >> 8) | (*sy & 0x0000ff00);
        }
        else {
            d2 = (CARD32 *) d1;
            /* Luma 1, chroma 1, chroma 2, luma 2. */
            *d2++ = (*sy & 0x000000ff) | (*sc << 8) |
                    ((*sy & 0x0000ff00) << 16);
            d1 = (CARD16 *) d2;
        }
        /* Luma 4, luma 3. */
        *d1++ = ((*sy & 0xff000000) >> 24) | ((*sy & 0x00ff0000) >> 8);
        sy++;
        sc++;
    }
}

/**
 * Convertion from YV12 to YUV420 taken from xomap sources (with a 32-bit write branch removed)
 */
void yv12_to_yuv420_xomap_nobranch(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w)
{
    int j;
    CARD32 *d2;
    CARD32 *sy = (CARD32 *)y;
    CARD16 *sc = (CARD16 *)chroma;
    CARD16 *d1 = (CARD16 *)dst;
    w >>= 2;

    for (j = 0; j < w; j++) {
        /* Luma 1, chroma 1. */
        *d1++ = (*sy & 0x000000ff) | ((*sc & 0x00ff) << 8);
        /* Chroma 2, luma 2. */
        *d1++ = ((*sc & 0xff00) >> 8) | (*sy & 0x0000ff00);
        /* Luma 4, luma 3. */
        *d1++ = ((*sy & 0xff000000) >> 24) | ((*sy & 0x00ff0000) >> 8);
        sy++;
        sc++;
    }
}

uint8_t __attribute__ ((aligned(32))) y_src_buffer[TEST_HEIGHT][TEST_WIDTH + 128];
uint8_t __attribute__ ((aligned(32))) u_src_buffer[TEST_HEIGHT][TEST_WIDTH / 2 + 128];
uint8_t __attribute__ ((aligned(32))) v_src_buffer[TEST_HEIGHT][TEST_WIDTH / 2 + 128];
uint8_t __attribute__ ((aligned(32))) out_buffer[TEST_HEIGHT][TEST_WIDTH * 2];

#define TIME_CLOCKS_PER_SEC 1000000

static int64_t gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

void do_test_yuv420(void (*f)(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w), const char *testname)
{
    int64_t before, after;
    double time_in_seconds, convspeed;
    int i, j, k;
    before = gettime();
    for (j = 0; j < TEST_LOOPCOUNT; j++) {
        for (i = 0; i < TEST_HEIGHT; i++) {
            f((uint16_t *)out_buffer[i], (uint32_t *)y_src_buffer[i], (uint16_t *)u_src_buffer[i], TEST_WIDTH);
        }
    }
    after = gettime();
    time_in_seconds = (double)(after - before) / (double)TIME_CLOCKS_PER_SEC;
    convspeed = (double)TEST_LOOPCOUNT * (double)(TEST_WIDTH * TEST_HEIGHT) / time_in_seconds / 1000000.;
    printf("test: '%s', time=%.3lfs, speed=%.3lfMP/s, memwritespeed=%.3lfMB/s\n", testname, 
        time_in_seconds, convspeed, convspeed * 3. / 2.);
}

void do_test_yuv422(void (*f)(uint32_t *dst, const uint16_t *y, const uint8_t *u, const uint8_t *v, int w), const char *testname)
{
    int64_t before, after;
    double time_in_seconds, convspeed;
    int i, j;
    before = gettime();
    for (j = 0; j < TEST_LOOPCOUNT; j++) {
        for (i = 0; i < TEST_HEIGHT; i++) {
            f((uint32_t *)out_buffer[i], (uint16_t *)y_src_buffer[i], u_src_buffer[i], v_src_buffer[i], TEST_WIDTH);
        }
    }
    after = gettime();
    time_in_seconds = (double)(after - before) / (double)TIME_CLOCKS_PER_SEC;
    convspeed = (double)TEST_LOOPCOUNT * (double)(TEST_WIDTH * TEST_HEIGHT) / time_in_seconds / 1000000.;
    printf("test: '%s', time=%.3lfs, speed=%.3lfMP/s, memwritespeed=%.3lfMB/s\n", testname, 
        time_in_seconds, convspeed, convspeed * 2.);
}

void do_test_correctness_yuv420(
    void (*f1)(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w),
    void (*f2)(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w),
    const char *testname1,
    const char *testname2)
{
    int i, w, shift;
    
    srand((int)gettime());
    for (i = 0; i < sizeof(y_src_buffer); i++) *((uint8_t *)y_src_buffer + i) = (uint8_t)rand();
    for (i = 0; i < sizeof(u_src_buffer); i++) *((uint8_t *)u_src_buffer + i) = (uint8_t)rand();
    for (i = 0; i < sizeof(v_src_buffer); i++) *((uint8_t *)v_src_buffer + i) = (uint8_t)rand();
/*
    for (i = 0; i < sizeof(y_src_buffer); i++) *((uint8_t *)y_src_buffer + i) = i & 0x7F;
    for (i = 0; i < sizeof(u_src_buffer); i++) *((uint8_t *)u_src_buffer + i) = (i & 0x7F) | 0x80;
*/
    for (i = 0; i < TEST_HEIGHT; i++) {
        memset(out_buffer[0], 0, TEST_WIDTH * 2);
        memset(out_buffer[1], 0, TEST_WIDTH * 2);
        shift = rand() % 8;
        w = rand() % (TEST_WIDTH - shift * 4);
        f1((uint16_t *)out_buffer[0] + shift, (uint32_t *)y_src_buffer[i], (uint16_t *)u_src_buffer[i], w);
        f2((uint16_t *)out_buffer[1] + shift, (uint32_t *)y_src_buffer[i], (uint16_t *)u_src_buffer[i], w);
        if (memcmp(out_buffer[0], out_buffer[1], TEST_WIDTH * 2) != 0) {
            printf("%s:\n", testname1);
            for (i = 0; i < TEST_WIDTH * 2; i++) printf("%02X ", out_buffer[0][i]);
            printf("\n%s:\n", testname2);
            for (i = 0; i < TEST_WIDTH * 2; i++) printf("%02X ", out_buffer[1][i]);
            printf("\nerror, '%s' and '%s' produce different results for width %d\n", testname1, testname2, w);
            return;
        }
    }
}

#define DO_TEST_CORRECTNESS_YUV420(f1, f2) do_test_correctness_yuv420(f1, f2, #f1, #f2)

#define DO_TEST_YUV420(f) do_test_yuv420(f, #f)
#define DO_TEST_YUV422(f) do_test_yuv422(f, #f)

void yv12_to_yuv420_line_arm_(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w)
{
    yv12_to_yuv420_line_arm(dst, (uint8_t *)y, (uint8_t *)chroma, w);
}

void yv12_to_yuv420_line_armv5_(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w)
{
    yv12_to_yuv420_line_armv5(dst, (uint8_t *)y, (uint8_t *)chroma, w);
}

void yv12_to_yuv420_line_armv6_(uint16_t *dst, const uint32_t *y, const uint16_t *chroma, int w)
{
    yv12_to_yuv420_line_armv6(dst, (uint16_t *)y, (uint8_t *)chroma, w);
}

int main()
{
    int i;
    for (i = 0; i < sizeof(y_src_buffer); i++) *((uint8_t *)y_src_buffer + i) = 0;
    for (i = 0; i < sizeof(u_src_buffer); i++) *((uint8_t *)u_src_buffer + i) = 0;
    for (i = 0; i < sizeof(v_src_buffer); i++) *((uint8_t *)v_src_buffer + i) = 0;
    DO_TEST_CORRECTNESS_YUV420(yv12_to_yuv420_xomap, yv12_to_yuv420_line_arm_);
    DO_TEST_CORRECTNESS_YUV420(yv12_to_yuv420_xomap, yv12_to_yuv420_line_armv5_);
    DO_TEST_CORRECTNESS_YUV420(yv12_to_yuv420_xomap, yv12_to_yuv420_line_armv6_);
    DO_TEST_YUV420(yv12_to_yuv420_xomap);
    DO_TEST_YUV420(yv12_to_yuv420_xomap_nobranch);
    DO_TEST_YUV420(yv12_to_yuv420_line_arm_);
    DO_TEST_YUV420(yv12_to_yuv420_line_armv5_);
    DO_TEST_YUV420(yv12_to_yuv420_line_armv6_);
}
