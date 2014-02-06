#ifndef _GMA_UXA_H
#define _GMA_UXA_H

Bool gma_uxa_init(gma500Ptr gma, ScreenPtr screen);

void gma_set_surface(PixmapPtr pixmap, struct gma_bo *bo);
struct gma_bo *gma_get_surface(PixmapPtr pixmap);

#endif
