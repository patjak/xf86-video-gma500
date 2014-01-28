#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xorgVersion.h"

#include "gma_driver.h"
#include "gma_uxa.h"
#include "libgma.h"

static Bool
gma_uxa_check_solid(DrawablePtr drawable, int alu, Pixel planemask)
{
	return FALSE;
}

static Bool
gma_uxa_prepare_solid(PixmapPtr pixmap, int alu, Pixel planemask, Pixel fb)
{
	return FALSE;
}

static void
gma_uxa_solid(PixmapPtr pixmap, int x1, int y1, int x2, int y2)
{
}

static void
gma_uxa_done_solid(PixmapPtr pixmap)
{
}

static Bool
gma_uxa_check_copy(PixmapPtr source, PixmapPtr dest, int alu, Pixel planemask)
{
	return FALSE;
}

static Bool
gma_uxa_prepare_copy(PixmapPtr source, PixmapPtr dest, int xdir, int ydir,
		     int alu, Pixel planemask)
{
	return FALSE;
}

static void
gma_uxa_copy(PixmapPtr dest, int src_x, int src_y, int dest_x, int dest_y,
	     int width, int height)
{
}

static void
gma_uxa_done_copy(PixmapPtr dest)
{
}

Bool gma_uxa_init(gma500Ptr gma, ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn (screen);
	gma->uxa = uxa_driver_alloc();
	if (gma->uxa == NULL)
		return FALSE;

	gma->uxa->uxa_major = 1;
	gma->uxa->uxa_minor = 0;

	/* Solid */
	gma->uxa->check_solid = gma_uxa_check_solid;
	gma->uxa->prepare_solid = gma_uxa_prepare_solid;
	gma->uxa->solid = gma_uxa_solid;
	gma->uxa->done_solid = gma_uxa_done_solid;

	/* Copy */
	gma->uxa->check_copy = gma_uxa_check_copy;
	gma->uxa->prepare_copy = gma_uxa_prepare_copy;
	gma->uxa->copy = gma_uxa_copy;
	gma->uxa->done_copy = gma_uxa_done_copy;

	if (!uxa_driver_init(screen, gma->uxa)) {
		xf86DrvMsg (scrn->scrnIndex, X_ERROR,
			    "UXA initialization failed\n");
		return FALSE;
	}

	return TRUE;
}
