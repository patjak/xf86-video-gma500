/**************************************************************************
 *
 * Copyright (c) (2005-2007) Imagination Technologies Limited.
 * Copyright (c) 2007, Intel Corporation.
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
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA..
 *
 **************************************************************************/

#ifndef _PSB_REG_H_
#define _PSB_REG_H_

/*
 * 2D defs.
 */

/*
 * 2D Slave Port Data : Block Header's Object Type
 */

#define	PSB_2D_CLIP_BH			(0x00000000)
#define	PSB_2D_PAT_BH			(0x10000000)
#define	PSB_2D_CTRL_BH			(0x20000000)
#define	PSB_2D_SRC_OFF_BH		(0x30000000)
#define	PSB_2D_MASK_OFF_BH		(0x40000000)
#define	PSB_2D_RESERVED1_BH		(0x50000000)
#define	PSB_2D_RESERVED2_BH		(0x60000000)
#define	PSB_2D_FENCE_BH			(0x70000000)
#define	PSB_2D_BLIT_BH			(0x80000000)
#define	PSB_2D_SRC_SURF_BH		(0x90000000)
#define	PSB_2D_DST_SURF_BH		(0xA0000000)
#define	PSB_2D_PAT_SURF_BH		(0xB0000000)
#define	PSB_2D_SRC_PAL_BH		(0xC0000000)
#define	PSB_2D_PAT_PAL_BH		(0xD0000000)
#define	PSB_2D_MASK_SURF_BH		(0xE0000000)
#define	PSB_2D_FLUSH_BH			(0xF0000000)

/*
 * Clip Definition block (PSB_2D_CLIP_BH)
 */
#define PSB_2D_CLIPCOUNT_MAX		(1)
#define PSB_2D_CLIPCOUNT_MASK		(0x00000000)
#define PSB_2D_CLIPCOUNT_CLRMASK	(0xFFFFFFFF)
#define PSB_2D_CLIPCOUNT_SHIFT		(0)
/* clip rectangle min & max */
#define PSB_2D_CLIP_XMAX_MASK		(0x00FFF000)
#define PSB_2D_CLIP_XMAX_CLRMASK	(0xFF000FFF)
#define PSB_2D_CLIP_XMAX_SHIFT		(12)
#define PSB_2D_CLIP_XMIN_MASK		(0x00000FFF)
#define PSB_2D_CLIP_XMIN_CLRMASK	(0x00FFF000)
#define PSB_2D_CLIP_XMIN_SHIFT		(0)
/* clip rectangle offset */
#define PSB_2D_CLIP_YMAX_MASK		(0x00FFF000)
#define PSB_2D_CLIP_YMAX_CLRMASK	(0xFF000FFF)
#define PSB_2D_CLIP_YMAX_SHIFT		(12)
#define PSB_2D_CLIP_YMIN_MASK		(0x00000FFF)
#define PSB_2D_CLIP_YMIN_CLRMASK	(0x00FFF000)
#define PSB_2D_CLIP_YMIN_SHIFT		(0)

/*
 * Pattern Control (PSB_2D_PAT_BH)
 */
#define PSB_2D_PAT_HEIGHT_MASK		(0x0000001F)
#define PSB_2D_PAT_HEIGHT_SHIFT		(0)
#define PSB_2D_PAT_WIDTH_MASK		(0x000003E0)
#define PSB_2D_PAT_WIDTH_SHIFT		(5)
#define PSB_2D_PAT_YSTART_MASK		(0x00007C00)
#define PSB_2D_PAT_YSTART_SHIFT		(10)
#define PSB_2D_PAT_XSTART_MASK		(0x000F8000)
#define PSB_2D_PAT_XSTART_SHIFT		(15)

/*
 * 2D Control block (PSB_2D_CTRL_BH)
 */
/* Present Flags */
#define PSB_2D_SRCCK_CTRL		(0x00000001)
#define PSB_2D_DSTCK_CTRL		(0x00000002)
#define PSB_2D_ALPHA_CTRL		(0x00000004)
/* Colour Key Colour (SRC/DST)*/
#define PSB_2D_CK_COL_MASK		(0xFFFFFFFF)
#define PSB_2D_CK_COL_CLRMASK		(0x00000000)
#define PSB_2D_CK_COL_SHIFT		(0)
/* Colour Key Mask (SRC/DST)*/
#define PSB_2D_CK_MASK_MASK		(0xFFFFFFFF)
#define PSB_2D_CK_MASK_CLRMASK		(0x00000000)
#define PSB_2D_CK_MASK_SHIFT		(0)
/* Alpha Control (Alpha/RGB)*/
#define PSB_2D_GBLALPHA_MASK		(0x000FF000)
#define PSB_2D_GBLALPHA_CLRMASK		(0xFFF00FFF)
#define PSB_2D_GBLALPHA_SHIFT		(12)
#define PSB_2D_SRCALPHA_OP_MASK		(0x00700000)
#define PSB_2D_SRCALPHA_OP_CLRMASK	(0xFF8FFFFF)
#define PSB_2D_SRCALPHA_OP_SHIFT	(20)
#define PSB_2D_SRCALPHA_OP_ONE		(0x00000000)
#define PSB_2D_SRCALPHA_OP_SRC		(0x00100000)
#define PSB_2D_SRCALPHA_OP_DST		(0x00200000)
#define PSB_2D_SRCALPHA_OP_SG		(0x00300000)
#define PSB_2D_SRCALPHA_OP_DG		(0x00400000)
#define PSB_2D_SRCALPHA_OP_GBL		(0x00500000)
#define PSB_2D_SRCALPHA_OP_ZERO		(0x00600000)
#define PSB_2D_SRCALPHA_INVERT		(0x00800000)
#define PSB_2D_SRCALPHA_INVERT_CLR	(0xFF7FFFFF)
#define PSB_2D_DSTALPHA_OP_MASK		(0x07000000)
#define PSB_2D_DSTALPHA_OP_CLRMASK	(0xF8FFFFFF)
#define PSB_2D_DSTALPHA_OP_SHIFT	(24)
#define PSB_2D_DSTALPHA_OP_ONE		(0x00000000)
#define PSB_2D_DSTALPHA_OP_SRC		(0x01000000)
#define PSB_2D_DSTALPHA_OP_DST		(0x02000000)
#define PSB_2D_DSTALPHA_OP_SG		(0x03000000)
#define PSB_2D_DSTALPHA_OP_DG		(0x04000000)
#define PSB_2D_DSTALPHA_OP_GBL		(0x05000000)
#define PSB_2D_DSTALPHA_OP_ZERO		(0x06000000)
#define PSB_2D_DSTALPHA_INVERT		(0x08000000)
#define PSB_2D_DSTALPHA_INVERT_CLR	(0xF7FFFFFF)

#define PSB_2D_PRE_MULTIPLICATION_ENABLE	(0x10000000)
#define PSB_2D_PRE_MULTIPLICATION_CLRMASK	(0xEFFFFFFF)
#define PSB_2D_ZERO_SOURCE_ALPHA_ENABLE		(0x20000000)
#define PSB_2D_ZERO_SOURCE_ALPHA_CLRMASK	(0xDFFFFFFF)

/*
 *Source Offset (PSB_2D_SRC_OFF_BH)
 */
#define PSB_2D_SRCOFF_XSTART_MASK	((0x00000FFF) << 12)
#define PSB_2D_SRCOFF_XSTART_SHIFT	(12)
#define PSB_2D_SRCOFF_YSTART_MASK	(0x00000FFF)
#define PSB_2D_SRCOFF_YSTART_SHIFT	(0)

/*
 * Mask Offset (PSB_2D_MASK_OFF_BH)
 */
#define PSB_2D_MASKOFF_XSTART_MASK	((0x00000FFF) << 12)
#define PSB_2D_MASKOFF_XSTART_SHIFT	(12)
#define PSB_2D_MASKOFF_YSTART_MASK	(0x00000FFF)
#define PSB_2D_MASKOFF_YSTART_SHIFT	(0)

/*
 * 2D Fence (see PSB_2D_FENCE_BH): bits 0:27 are ignored
 */

/*
 *Blit Rectangle (PSB_2D_BLIT_BH)
 */

#define PSB_2D_ROT_MASK			(3 << 25)
#define PSB_2D_ROT_CLRMASK		(~PSB_2D_ROT_MASK)
#define PSB_2D_ROT_NONE			(0 << 25)
#define PSB_2D_ROT_90DEGS		(1 << 25)
#define PSB_2D_ROT_180DEGS		(2 << 25)
#define PSB_2D_ROT_270DEGS		(3 << 25)

#define PSB_2D_COPYORDER_MASK		(3 << 23)
#define PSB_2D_COPYORDER_CLRMASK	(~PSB_2D_COPYORDER_MASK)
#define PSB_2D_COPYORDER_TL2BR		(0 << 23)
#define PSB_2D_COPYORDER_BR2TL		(1 << 23)
#define PSB_2D_COPYORDER_TR2BL		(2 << 23)
#define PSB_2D_COPYORDER_BL2TR		(3 << 23)

#define PSB_2D_DSTCK_CLRMASK		(0xFF9FFFFF)
#define PSB_2D_DSTCK_DISABLE		(0x00000000)
#define PSB_2D_DSTCK_PASS		(0x00200000)
#define PSB_2D_DSTCK_REJECT		(0x00400000)

#define PSB_2D_SRCCK_CLRMASK		(0xFFE7FFFF)
#define PSB_2D_SRCCK_DISABLE		(0x00000000)
#define PSB_2D_SRCCK_PASS		(0x00080000)
#define PSB_2D_SRCCK_REJECT		(0x00100000)

#define PSB_2D_CLIP_ENABLE		(0x00040000)

#define PSB_2D_ALPHA_ENABLE		(0x00020000)

#define PSB_2D_PAT_CLRMASK		(0xFFFEFFFF)
#define PSB_2D_PAT_MASK			(0x00010000)
#define PSB_2D_USE_PAT			(0x00010000)
#define PSB_2D_USE_FILL			(0x00000000)
/*
 * Tungsten Graphics note on rop codes: If rop A and rop B are
 * identical, the mask surface will not be read and need not be
 * set up.
 */

#define PSB_2D_ROP3B_MASK		(0x0000FF00)
#define PSB_2D_ROP3B_CLRMASK		(0xFFFF00FF)
#define PSB_2D_ROP3B_SHIFT		(8)
/* rop code A */
#define PSB_2D_ROP3A_MASK		(0x000000FF)
#define PSB_2D_ROP3A_CLRMASK		(0xFFFFFF00)
#define PSB_2D_ROP3A_SHIFT		(0)

#define PSB_2D_ROP4_MASK		(0x0000FFFF)
/*
 *	DWORD0:	(Only pass if Pattern control == Use Fill Colour)
 *	Fill Colour RGBA8888
 */
#define PSB_2D_FILLCOLOUR_MASK		(0xFFFFFFFF)
#define PSB_2D_FILLCOLOUR_SHIFT		(0)
/*
 *	DWORD1: (Always Present)
 *	X Start (Dest)
 *	Y Start (Dest)
 */
#define PSB_2D_DST_XSTART_MASK		(0x00FFF000)
#define PSB_2D_DST_XSTART_CLRMASK	(0xFF000FFF)
#define PSB_2D_DST_XSTART_SHIFT		(12)
#define PSB_2D_DST_YSTART_MASK		(0x00000FFF)
#define PSB_2D_DST_YSTART_CLRMASK	(0xFFFFF000)
#define PSB_2D_DST_YSTART_SHIFT		(0)
/*
 *	DWORD2: (Always Present)
 *	X Size (Dest)
 *	Y Size (Dest)
 */
#define PSB_2D_DST_XSIZE_MASK		(0x00FFF000)
#define PSB_2D_DST_XSIZE_CLRMASK	(0xFF000FFF)
#define PSB_2D_DST_XSIZE_SHIFT		(12)
#define PSB_2D_DST_YSIZE_MASK		(0x00000FFF)
#define PSB_2D_DST_YSIZE_CLRMASK	(0xFFFFF000)
#define PSB_2D_DST_YSIZE_SHIFT		(0)

/*
 * Source Surface (PSB_2D_SRC_SURF_BH)
 */
/*
 * WORD 0
 */

#define PSB_2D_SRC_FORMAT_MASK		(0x00078000)
#define PSB_2D_SRC_1_PAL		(0x00000000)
#define PSB_2D_SRC_2_PAL		(0x00008000)
#define PSB_2D_SRC_4_PAL		(0x00010000)
#define PSB_2D_SRC_8_PAL		(0x00018000)
#define PSB_2D_SRC_8_ALPHA		(0x00020000)
#define PSB_2D_SRC_4_ALPHA		(0x00028000)
#define PSB_2D_SRC_332RGB		(0x00030000)
#define PSB_2D_SRC_4444ARGB		(0x00038000)
#define PSB_2D_SRC_555RGB		(0x00040000)
#define PSB_2D_SRC_1555ARGB		(0x00048000)
#define PSB_2D_SRC_565RGB		(0x00050000)
#define PSB_2D_SRC_0888ARGB		(0x00058000)
#define PSB_2D_SRC_8888ARGB		(0x00060000)
#define PSB_2D_SRC_8888UYVY		(0x00068000)
#define PSB_2D_SRC_RESERVED		(0x00070000)
#define PSB_2D_SRC_1555ARGB_LOOKUP	(0x00078000)


#define PSB_2D_SRC_STRIDE_MASK		(0x00007FFF)
#define PSB_2D_SRC_STRIDE_CLRMASK	(0xFFFF8000)
#define PSB_2D_SRC_STRIDE_SHIFT		(0)
/*
 *  WORD 1 - Base Address
 */
#define PSB_2D_SRC_ADDR_MASK		(0x0FFFFFFC)
#define PSB_2D_SRC_ADDR_CLRMASK		(0x00000003)
#define PSB_2D_SRC_ADDR_SHIFT		(2)
#define PSB_2D_SRC_ADDR_ALIGNSHIFT	(2)

/*
 * Pattern Surface (PSB_2D_PAT_SURF_BH)
 */
/*
 *  WORD 0
 */

#define PSB_2D_PAT_FORMAT_MASK		(0x00078000)
#define PSB_2D_PAT_1_PAL		(0x00000000)
#define PSB_2D_PAT_2_PAL		(0x00008000)
#define PSB_2D_PAT_4_PAL		(0x00010000)
#define PSB_2D_PAT_8_PAL		(0x00018000)
#define PSB_2D_PAT_8_ALPHA		(0x00020000)
#define PSB_2D_PAT_4_ALPHA		(0x00028000)
#define PSB_2D_PAT_332RGB		(0x00030000)
#define PSB_2D_PAT_4444ARGB		(0x00038000)
#define PSB_2D_PAT_555RGB		(0x00040000)
#define PSB_2D_PAT_1555ARGB		(0x00048000)
#define PSB_2D_PAT_565RGB		(0x00050000)
#define PSB_2D_PAT_0888ARGB		(0x00058000)
#define PSB_2D_PAT_8888ARGB		(0x00060000)

#define PSB_2D_PAT_STRIDE_MASK		(0x00007FFF)
#define PSB_2D_PAT_STRIDE_CLRMASK	(0xFFFF8000)
#define PSB_2D_PAT_STRIDE_SHIFT		(0)
/*
 *  WORD 1 - Base Address
 */
#define PSB_2D_PAT_ADDR_MASK		(0x0FFFFFFC)
#define PSB_2D_PAT_ADDR_CLRMASK		(0x00000003)
#define PSB_2D_PAT_ADDR_SHIFT		(2)
#define PSB_2D_PAT_ADDR_ALIGNSHIFT	(2)

/*
 * Destination Surface (PSB_2D_DST_SURF_BH)
 */
/*
 * WORD 0
 */

#define PSB_2D_DST_FORMAT_MASK		(0x00078000)
#define PSB_2D_DST_332RGB		(0x00030000)
#define PSB_2D_DST_4444ARGB		(0x00038000)
#define PSB_2D_DST_555RGB		(0x00040000)
#define PSB_2D_DST_1555ARGB		(0x00048000)
#define PSB_2D_DST_565RGB		(0x00050000)
#define PSB_2D_DST_0888ARGB		(0x00058000)
#define PSB_2D_DST_8888ARGB		(0x00060000)
#define PSB_2D_DST_8888AYUV		(0x00070000)

#define PSB_2D_DST_STRIDE_MASK		(0x00007FFF)
#define PSB_2D_DST_STRIDE_CLRMASK	(0xFFFF8000)
#define PSB_2D_DST_STRIDE_SHIFT		(0)
/*
 * WORD 1 - Base Address
 */
#define PSB_2D_DST_ADDR_MASK		(0x0FFFFFFC)
#define PSB_2D_DST_ADDR_CLRMASK		(0x00000003)
#define PSB_2D_DST_ADDR_SHIFT		(2)
#define PSB_2D_DST_ADDR_ALIGNSHIFT	(2)

/*
 * Mask Surface (PSB_2D_MASK_SURF_BH)
 */
/*
 * WORD 0
 */
#define PSB_2D_MASK_STRIDE_MASK		(0x00007FFF)
#define PSB_2D_MASK_STRIDE_CLRMASK	(0xFFFF8000)
#define PSB_2D_MASK_STRIDE_SHIFT	(0)
/*
 *  WORD 1 - Base Address
 */
#define PSB_2D_MASK_ADDR_MASK		(0x0FFFFFFC)
#define PSB_2D_MASK_ADDR_CLRMASK	(0x00000003)
#define PSB_2D_MASK_ADDR_SHIFT		(2)
#define PSB_2D_MASK_ADDR_ALIGNSHIFT	(2)

/*
 * Source Palette (PSB_2D_SRC_PAL_BH)
 */

#define PSB_2D_SRCPAL_ADDR_SHIFT	(0)
#define PSB_2D_SRCPAL_ADDR_CLRMASK	(0xF0000007)
#define PSB_2D_SRCPAL_ADDR_MASK		(0x0FFFFFF8)
#define PSB_2D_SRCPAL_BYTEALIGN		(1024)

/*
 * Pattern Palette (PSB_2D_PAT_PAL_BH)
 */

#define PSB_2D_PATPAL_ADDR_SHIFT	(0)
#define PSB_2D_PATPAL_ADDR_CLRMASK	(0xF0000007)
#define PSB_2D_PATPAL_ADDR_MASK		(0x0FFFFFF8)
#define PSB_2D_PATPAL_BYTEALIGN		(1024)

/*
 * Rop3 Codes (2 LS bytes)
 */

#define PSB_2D_ROP3_SRCCOPY		(0xCCCC)
#define PSB_2D_ROP3_PATCOPY		(0xF0F0)
#define PSB_2D_ROP3_WHITENESS		(0xFFFF)
#define PSB_2D_ROP3_BLACKNESS		(0x0000)
#define PSB_2D_ROP3_SRC			(0xCC)
#define PSB_2D_ROP3_PAT			(0xF0)
#define PSB_2D_ROP3_DST			(0xAA)

static const int pvr_copy_rop[] = {
	0x00, 0x88, 0x44, 0xCC, 0x22, 0xAA, 0x66, 0xEE,
	0x11, 0x99, 0x55, 0xDD, 0x33, 0xBB, 0x77, 0xFF
};

static const int pvr_pattern_rop[] = {
	0x00, 0xA0, 0x50, 0xF0, 0x0A, 0xAA, 0x5A, 0xFA, 0x05,
	0xA5, 0x55, 0xF5, 0x0F, 0xAF, 0x5F, 0xFF
};

extern int pvr_copy_rop_pm(int xrop);
extern int pvr_pat_rop_pm(int xrop);
extern uint32_t pvr_bpp_to_format(int bpp, int is_dst);
extern uint32_t pvr_copy(uint32_t *buffer, int rop,
		uint32_t src_handle, uint32_t src_offset, uint32_t src_stride, uint32_t src_format,
		uint32_t dst_handle, uint32_t dst_offset, uint32_t dst_stride, uint32_t dst_format,
		uint16_t src_x, uint16_t src_y,
		uint16_t dst_x, uint16_t dst_y,
		uint16_t size_x, uint16_t size_y,
		uint32_t direction);


#endif
