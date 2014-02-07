/**************************************************************************
 * Copyright (c) 2014, Patrik Jakobsson <patrik.r.jakobsson@gmail.com>
 * Copyright (c) 2007-2011, Intel Corporation.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Intel funded Tungsten Graphics (http://www.tungstengraphics.com) to
 * develop this driver.
 *
 **************************************************************************/

#include <xf86.h>
#include "pvr_2d.h"

uint32_t pvr_copy_direction(int xdir, int ydir)
{
	if (xdir < 0)
		return (ydir < 0) ? PSB_2D_COPYORDER_BR2TL :
				    PSB_2D_COPYORDER_TR2BL;
	else
		return (ydir < 0) ? PSB_2D_COPYORDER_BL2TR :
				    PSB_2D_COPYORDER_TL2BR;
}

/*
 * Pattern as planemask.
 */
int pvr_copy_rop_pm(int xrop)
{
	return (pvr_copy_rop[xrop] & PSB_2D_ROP3_PAT) |
		(PSB_2D_ROP3_DST & ~PSB_2D_ROP3_PAT);
}

/*
 * Source as planemask.
 */
int pvr_pat_rop_pm(int xrop)
{
	return (pvr_copy_rop[xrop] & PSB_2D_ROP3_SRC) |
		(PSB_2D_ROP3_DST & ~PSB_2D_ROP3_SRC);
}

uint32_t pvr_bpp_to_format(int bpp, int is_dst)
{
	switch (bpp) {
	case 8:
		return (is_dst ? PSB_2D_DST_332RGB : PSB_2D_SRC_332RGB);
	case 16:
		return (is_dst ? PSB_2D_DST_555RGB : PSB_2D_SRC_555RGB);
	case 24:
		return (is_dst ? PSB_2D_DST_0888ARGB : PSB_2D_SRC_0888ARGB);
	case 32:
		return (is_dst ? PSB_2D_DST_8888ARGB : PSB_2D_SRC_8888ARGB);
	default:
		return 0;
	}
}

uint32_t pvr_copy(uint32_t *buffer, int rop,
		uint32_t src_handle, uint32_t src_offset, uint32_t src_stride, uint32_t src_format,
		uint32_t dst_handle, uint32_t dst_offset, uint32_t dst_stride, uint32_t dst_format,
		uint16_t src_x, uint16_t src_y,
		uint16_t dst_x, uint16_t dst_y,
		uint16_t size_x, uint16_t size_y,
		uint32_t direction)
{
	uint32_t blit_cmd;
	uint32_t *buf;

	buf = buffer;

	if (direction == PSB_2D_COPYORDER_BR2TL ||
	    direction == PSB_2D_COPYORDER_TR2BL) {
		src_x += size_x - 1;
		dst_x += size_x - 1;
	}
	if (direction == PSB_2D_COPYORDER_BR2TL ||
	    direction == PSB_2D_COPYORDER_BL2TR) {
		src_y += size_y - 1;
		dst_y += size_y - 1;
	}

	blit_cmd =
	    PSB_2D_BLIT_BH |
	    PSB_2D_ROT_NONE |
	    direction |
	    PSB_2D_DSTCK_DISABLE |
	    PSB_2D_SRCCK_DISABLE |
	    PSB_2D_USE_PAT | // PSB_2D_ROP3_SRCCOPY;
	    ((rop << PSB_2D_ROP3B_SHIFT) & PSB_2D_ROP3B_MASK) |
	    ((rop << PSB_2D_ROP3A_SHIFT) & PSB_2D_ROP3A_MASK);

	*buf++ = PSB_2D_DST_SURF_BH | dst_format |
		 (dst_stride << PSB_2D_DST_STRIDE_SHIFT);
	*buf++ = dst_handle;
	*buf++ = dst_offset;

	*buf++ = PSB_2D_SRC_SURF_BH | src_format |
		 (src_stride << PSB_2D_SRC_STRIDE_SHIFT);
	*buf++ = src_handle;
	*buf++ = src_offset;

	*buf++ = PSB_2D_SRC_OFF_BH |
		(src_x << PSB_2D_SRCOFF_XSTART_SHIFT) |
		(src_y << PSB_2D_SRCOFF_YSTART_SHIFT);

	*buf++ = blit_cmd;
	*buf++ = (dst_x << PSB_2D_DST_XSTART_SHIFT) |
		 (dst_y << PSB_2D_DST_YSTART_SHIFT);

	*buf++ = (size_x << PSB_2D_DST_XSIZE_SHIFT) |
		 (size_y << PSB_2D_DST_YSIZE_SHIFT);

	*buf++ = PSB_2D_FENCE_BH;
	*buf++ = PSB_2D_FLUSH_BH;

	return (buf - buffer);
}

