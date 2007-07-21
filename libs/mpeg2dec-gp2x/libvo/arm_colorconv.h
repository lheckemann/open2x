/*
 * ARM assembly optimized color format conversion functions
 * (YV12 -> YUY2)
 *
 * Copyright (C) 2007 Siarhei Siamashka <ssvb@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef __ARM_COLORCONV_H__
#define __ARM_COLORCONV_H__

#include <stdint.h>

/**
 * Convert a line of pixels from YV12 to YUY2 color format
 * @param dst   - destination buffer for YUY2 pixel data, it should be 32-bit aligned
 * @param src_y - pointer to Y plane
 * @param src_u - pointer to U plane
 * @param src_v - pointer to V plane
 * @param w     - number of pixels to convert (should be multiple of 2)
 */
void yv12_to_yuy2_line_arm(uint32_t *dst, const uint16_t *src_y, const uint8_t *src_u, const uint8_t *src_v, int w);

#endif
