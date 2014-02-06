#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "xorgVersion.h"

#include "gma_driver.h"
#include "gma_uxa.h"
#include "libgma.h"

#if HAS_DEVPRIVATEKEYREC
DevPrivateKeyRec uxa_pixmap_index;
#else
int uxa_pixmap_index;
#endif

void gma_set_surface(PixmapPtr pixmap, struct gma_bo *bo)
{
	dixSetPrivate(&pixmap->devPrivates, &uxa_pixmap_index, bo);
}

struct gma_bo *gma_get_surface(PixmapPtr pixmap)
{
#if HAS_DEVPRIVATEKEYREC
	return dixGetPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#else
	return dixLookupPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#endif
}

static void
gma_uxa_set_screen_pixmap(PixmapPtr pixmap)
{
	pixmap->drawable.pScreen->devPrivate = pixmap;
}

static PixmapPtr
gma_uxa_create_pixmap(ScreenPtr screen, int w, int h, int depth, unsigned usage)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	gma500Ptr gma = gma500PTR(scrn);
	struct gma_bo *bo;
	PixmapPtr pixmap;

	if (uxa_swapped_out(screen)) {
		goto fallback;
	}

	if (!w || !h) {
		goto fallback;
	}

	bo = gma_bo_create_surface(gma->fd, w, h, depth, GMA_BO_BLIT, 0);

	if (bo) {
		pixmap = fbCreatePixmap(screen, 0, 0, depth, usage);
		screen->ModifyPixmapHeader(pixmap, w, h, -1, -1, -1, NULL);
		pixmap->devKind = bo->pitch;
		gma_set_surface(pixmap, bo);
	} else {
fallback:
		/* Software fallback */
		pixmap = fbCreatePixmap(screen, w, h, depth, usage);
	}

	return pixmap;
}

static Bool
gma_uxa_destroy_pixmap(PixmapPtr pixmap)
{
	ScreenPtr screen = pixmap->drawable.pScreen;
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	gma500Ptr gma = gma500PTR(scrn);
	struct gma_bo *bo;

	/* Only remove bo if we're the last user */
	if (pixmap->refcnt == 1) {
		bo = gma_get_surface(pixmap);

		if (bo) {
			gma_set_surface(pixmap, NULL);
			gma_bo_destroy(gma->fd, bo);
		}
	}

	fbDestroyPixmap(pixmap);

	return TRUE;
}

static Bool
gma_uxa_pixmap_is_offscreen(PixmapPtr pixmap)
{
	if (gma_get_surface(pixmap) == NULL)
		return FALSE;
	else
		return TRUE;
}

static Bool
gma_uxa_put_image(PixmapPtr pixmap, int x, int y, int w, int h, char *src,
		  int src_pitch)
{
	return FALSE;
}

static Bool
gma_uxa_get_image(PixmapPtr pixmap, int x, int y, int w, int h, char *dst,
		  int dst_pitch)
{
	return FALSE;
}

static Bool
gma_uxa_prepare_access(PixmapPtr pixmap, RegionPtr region, uxa_access_t access)
{
	ScreenPtr screen = pixmap->drawable.pScreen;
	ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
	gma500Ptr gma = gma500PTR(scrn);
	struct gma_bo *bo;
	int ret;

	bo = gma_get_surface(pixmap);

	if (!bo)
		return TRUE;

	if (!bo->ptr) {
		ret = gma_bo_mmap(gma->fd, bo);
		if (ret)
			return FALSE;
	}

	pixmap->devPrivate.ptr = bo->ptr;
	pixmap->devKind = bo->pitch;

	return TRUE;
}

static void
gma_uxa_finish_access(PixmapPtr pixmap)
{
}

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

static Bool
gma_uxa_check_composite(int op, PicturePtr pSrcPict, PicturePtr pMaskPict,
			PicturePtr pDstPict, int w, int h)
{
	return FALSE;
}

static Bool
gma_uxa_check_composite_target(PixmapPtr pixmap)
{
	return FALSE;
}

static Bool
gma_uxa_prepare_composite(int op, PicturePtr pSrcPict, PicturePtr pMaskPict,
			  PicturePtr pDstPict, PixmapPtr pSrc, PixmapPtr pMask,
			  PixmapPtr pDst)
{
	return FALSE;
}

Bool gma_uxa_init(gma500Ptr gma, ScreenPtr screen)
{
	ScrnInfoPtr scrn = xf86ScreenToScrn (screen);
	gma->uxa = uxa_driver_alloc();
	if (gma->uxa == NULL)
		return FALSE;

#if HAS_DIXREGISTERPRIVATEKEY
	if (!dixRegisterPrivateKey (&uxa_pixmap_index, PRIVATE_PIXMAP, 0))
		return FALSE;
#else
	if (!dixRequestPrivate (&uxa_pixmap_index, 0))
		return FALSE;
#endif

	gma->uxa->uxa_major = 1;
	gma->uxa->uxa_minor = 0;

	/* Access */
	gma->uxa->prepare_access = gma_uxa_prepare_access;
	gma->uxa->finish_access = gma_uxa_finish_access;
	gma->uxa->pixmap_is_offscreen = gma_uxa_pixmap_is_offscreen;

	/* Migration */
	gma->uxa->put_image = gma_uxa_put_image;
	gma->uxa->get_image = gma_uxa_get_image;

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

	/* Composite */
	gma->uxa->check_composite = gma_uxa_check_composite;
	gma->uxa->check_composite_target = gma_uxa_check_composite_target;
	gma->uxa->prepare_composite = gma_uxa_prepare_composite;
	gma->uxa->composite = NULL;
	gma->uxa->done_composite = NULL;

	screen->SetScreenPixmap = gma_uxa_set_screen_pixmap;
	screen->CreatePixmap = gma_uxa_create_pixmap;
	screen->DestroyPixmap = gma_uxa_destroy_pixmap;

	if (!uxa_driver_init(screen, gma->uxa)) {
		xf86DrvMsg (scrn->scrnIndex, X_ERROR,
			    "UXA initialization failed\n");
		return FALSE;
	}

	return TRUE;
}
