/*
 * Copyright 2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * Copyright 2011 Dave Airlie
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Original Author: Alan Hourihane <alanh@tungstengraphics.com>
 * Rewrite: Dave Airlie <airlied@redhat.com> 
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"
#include "xf86Pci.h"
#include "mipointer.h"
#include "micmap.h"
#include <X11/extensions/randr.h>
#include "fb.h"
#include "edid.h"
#include "xf86i2c.h"
#include "xf86Crtc.h"
#include "miscstruct.h"
#include "dixstruct.h"
#include "shadow.h"
#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include <xorg-server.h>
#ifdef XSERVER_PLATFORM_BUS
#include "xf86platformBus.h"
#endif
#if XSERVER_LIBPCIACCESS
#include <pciaccess.h>
#endif

#include "compat-api.h"
#include "gma_driver.h"

static void AdjustFrame(ADJUST_FRAME_ARGS_DECL);
static Bool CloseScreen(CLOSE_SCREEN_ARGS_DECL);
static Bool EnterVT(VT_FUNC_ARGS_DECL);
static void Identify(int flags);
static const OptionInfoRec *AvailableOptions(int chipid, int busid);
static ModeStatus ValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose,
			    int flags);
static void FreeScreen(FREE_SCREEN_ARGS_DECL);
static void LeaveVT(VT_FUNC_ARGS_DECL);
static Bool SwitchMode(SWITCH_MODE_ARGS_DECL);
static Bool ScreenInit(SCREEN_INIT_ARGS_DECL);
static Bool PreInit(ScrnInfoPtr pScrn, int flags);

static Bool Probe(DriverPtr drv, int flags);
static Bool gma_pci_probe(DriverPtr driver,
			 int entity_num, struct pci_device *device,
			 intptr_t match_data);
static Bool gma_driver_func(ScrnInfoPtr scrn, xorgDriverFuncOp op,
			   void *data);

#ifdef XSERVER_LIBPCIACCESS
static const struct pci_id_match gma_device_match[] = {
    {
	PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
	0x00030000, 0x00ff0000, 0
    },

    { 0, 0, 0 },
};
#endif

#ifdef XSERVER_PLATFORM_BUS
static Bool gma_platform_probe(DriverPtr driver,
                          int entity_num, int flags, struct xf86_platform_device *device,
			  intptr_t match_data);
#endif

_X_EXPORT DriverRec gma500 = {
    1,
    "gma500",
    Identify,
    Probe,
    AvailableOptions,
    NULL,
    0,
    gma_driver_func,
    gma_device_match,
    gma_pci_probe,
#ifdef XSERVER_PLATFORM_BUS
    gma_platform_probe,
#endif
};

static SymTabRec Chipsets[] = {
    {0, "kms" },
    {-1, NULL}
};

typedef enum
{
    OPTION_SW_CURSOR,
    OPTION_DEVICE_PATH,
    OPTION_SHADOW_FB,
} gma500Opts;

static const OptionInfoRec Options[] = {
    {OPTION_SW_CURSOR, "SWcursor", OPTV_BOOLEAN, {0}, FALSE},
    {OPTION_DEVICE_PATH, "kmsdev", OPTV_STRING, {0}, FALSE },
    {OPTION_SHADOW_FB, "ShadowFB", OPTV_BOOLEAN, {0}, FALSE },
    {-1, NULL, OPTV_NONE, {0}, FALSE}
};

int gma500EntityIndex = -1;

static MODULESETUPPROTO(Setup);

static XF86ModuleVersionInfo VersRec = {
    "gma500",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
    ABI_CLASS_VIDEODRV,
    ABI_VIDEODRV_VERSION,
    MOD_CLASS_VIDEODRV,
    {0, 0, 0, 0}
};

_X_EXPORT XF86ModuleData gma500ModuleData = { &VersRec, Setup, NULL };

static pointer
Setup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = 0;

    /* This module should be loaded only once, but check to be sure.
     */
    if (!setupDone) {
	setupDone = 1;
	xf86AddDriver(&gma500, module, HaveDriverFuncs);

	/*
	 * The return value must be non-NULL on success even though there
	 * is no TearDownProc.
	 */
	return (pointer) 1;
    } else {
	if (errmaj)
	    *errmaj = LDR_ONCEONLY;
	return NULL;
    }
}

static void
Identify(int flags)
{
    xf86PrintChipsets("gma500", "Driver for gma500 kernel driver",
		      Chipsets);
}

static int open_hw(const char *dev)
{
    int fd;
    if (dev)
	fd = open(dev, O_RDWR, 0);
    else {
	dev = getenv("KMSDEVICE");
	if ((NULL == dev) || ((fd = open(dev, O_RDWR, 0)) == -1)) {
	    dev = "/dev/dri/card0";
	    fd = open(dev,O_RDWR, 0);
	}
    }
    if (fd == -1)
	xf86DrvMsg(-1, X_ERROR,"open %s: %s\n", dev, strerror(errno));

    return fd;
}

static int check_outputs(int fd)
{
    drmModeResPtr res = drmModeGetResources(fd);
    int ret;

    if (!res)
        return FALSE;
    ret = res->count_connectors > 0;
    drmModeFreeResources(res);
    return ret;
}

static Bool probe_hw(const char *dev)
{
    int fd = open_hw(dev);
    if (fd != -1) {
        int ret = check_outputs(fd);
        close(fd);
        return ret;
    }
    return FALSE;
}

static char *
gma_DRICreatePCIBusID(const struct pci_device *dev)
{
    char *busID;

    if (asprintf(&busID, "pci:%04x:%02x:%02x.%d",
                 dev->domain, dev->bus, dev->dev, dev->func) == -1)
        return NULL;

    return busID;
}


static Bool probe_hw_pci(const char *dev, struct pci_device *pdev)
{
    int ret = FALSE, fd = open_hw(dev);
    char *id, *devid;
    drmSetVersion sv;

    if (fd == -1)
	return FALSE;

    sv.drm_di_major = 1;
    sv.drm_di_minor = 4;
    sv.drm_dd_major = -1;
    sv.drm_dd_minor = -1;
    if (drmSetInterfaceVersion(fd, &sv)) {
        close(fd);
        return FALSE;
    }


    id = drmGetBusid(fd);
    devid = gma_DRICreatePCIBusID(pdev);
    close(fd);

    if (id && devid && !strcmp(id, devid))
        ret = check_outputs(fd);

    free(id);
    free(devid);
    return ret;
}
static const OptionInfoRec *
AvailableOptions(int chipid, int busid)
{
    return Options;
}

static Bool
gma_driver_func(ScrnInfoPtr scrn, xorgDriverFuncOp op, void *data)
{
    xorgHWFlags *flag;
    
    switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
	    flag = (CARD32 *)data;
	    (*flag) = 0;
	    return TRUE;
	default:
	    return FALSE;
    }
}

#if XSERVER_LIBPCIACCESS
static Bool
gma_pci_probe(DriverPtr driver,
	     int entity_num, struct pci_device *dev, intptr_t match_data)
{
    ScrnInfoPtr scrn = NULL;

    scrn = xf86ConfigPciEntity(scrn, 0, entity_num, NULL,
			       NULL, NULL, NULL, NULL, NULL);
    if (scrn) {
	const char *devpath;
	GDevPtr devSection = xf86GetDevFromEntity(scrn->entityList[0],
						  scrn->entityInstanceList[0]);

	devpath = xf86FindOptionValue(devSection->options, "kmsdev");
	if (probe_hw_pci(devpath, dev)) {
	    scrn->driverVersion = 1;
	    scrn->driverName = "gma500";
	    scrn->name = "gma500";
	    scrn->Probe = NULL;
	    scrn->PreInit = PreInit;
	    scrn->ScreenInit = ScreenInit;
	    scrn->SwitchMode = SwitchMode;
	    scrn->AdjustFrame = AdjustFrame;
	    scrn->EnterVT = EnterVT;
	    scrn->LeaveVT = LeaveVT;
	    scrn->FreeScreen = FreeScreen;
	    scrn->ValidMode = ValidMode;

	    xf86DrvMsg(scrn->scrnIndex, X_CONFIG,
		       "claimed PCI slot %d@%d:%d:%d\n", 
		       dev->bus, dev->domain, dev->dev, dev->func);
	    xf86DrvMsg(scrn->scrnIndex, X_INFO,
		       "using %s\n", devpath ? devpath : "default device");
	} else
	    scrn = NULL;
    }
    return scrn != NULL;
}
#endif

#ifdef XSERVER_PLATFORM_BUS
static Bool
gma_platform_probe(DriverPtr driver,
              int entity_num, int flags, struct xf86_platform_device *dev, intptr_t match_data)
{
    ScrnInfoPtr scrn = NULL;
    const char *path = xf86_get_platform_device_attrib(dev, ODEV_ATTRIB_PATH);
    int scr_flags = 0;

    if (flags & PLATFORM_PROBE_GPU_SCREEN)
            scr_flags = XF86_ALLOCATE_GPU_SCREEN;

    if (probe_hw(path)) {
        scrn = xf86AllocateScreen(driver, scr_flags);
        xf86AddEntityToScreen(scrn, entity_num);

        scrn->driverName = "gma500";
        scrn->name = "gma500";
        scrn->PreInit = PreInit;
        scrn->ScreenInit = ScreenInit;
        scrn->SwitchMode = SwitchMode;
        scrn->AdjustFrame = AdjustFrame;
        scrn->EnterVT = EnterVT;
        scrn->LeaveVT = LeaveVT;
        scrn->FreeScreen = FreeScreen;
        scrn->ValidMode = ValidMode;
        xf86DrvMsg(scrn->scrnIndex, X_INFO,
                   "using drv %s\n", path ? path : "default device");
    }

    return scrn != NULL;
}
#endif

static Bool
Probe(DriverPtr drv, int flags)
{
    int i, numDevSections;
    GDevPtr *devSections;
    Bool foundScreen = FALSE;
    const char *dev;
    ScrnInfoPtr scrn = NULL;

    /* For now, just bail out for PROBE_DETECT. */
    if (flags & PROBE_DETECT)
	return FALSE;

    /*
     * Find the config file Device sections that match this
     * driver, and return if there are none.
     */
    if ((numDevSections = xf86MatchDevice("gma500", &devSections)) <= 0) {
	return FALSE;
    }

    for (i = 0; i < numDevSections; i++) {

	dev = xf86FindOptionValue(devSections[i]->options,"kmsdev");
	if (probe_hw(dev)) {
	    int entity;
	    entity = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE);
	    scrn = xf86ConfigFbEntity(scrn, 0, entity,
				  NULL, NULL, NULL, NULL);
	}

	if (scrn) {
	    foundScreen = TRUE;
	    scrn->driverVersion = 1;
	    scrn->driverName = "gma500";
	    scrn->name = "gma500";
	    scrn->Probe = Probe;
	    scrn->PreInit = PreInit;
	    scrn->ScreenInit = ScreenInit;
	    scrn->SwitchMode = SwitchMode;
	    scrn->AdjustFrame = AdjustFrame;
	    scrn->EnterVT = EnterVT;
	    scrn->LeaveVT = LeaveVT;
	    scrn->FreeScreen = FreeScreen;
	    scrn->ValidMode = ValidMode;

	    xf86DrvMsg(scrn->scrnIndex, X_INFO,
			   "using %s\n", dev ? dev : "default device");
	}
    }

    free(devSections);

    return foundScreen;
}

static Bool
GetRec(ScrnInfoPtr pScrn)
{
    if (pScrn->driverPrivate)
	return TRUE;

    pScrn->driverPrivate = xnfcalloc(sizeof(gma500Rec), 1);

    return TRUE;
}

static int dispatch_dirty_region(ScrnInfoPtr scrn,
				 PixmapPtr pixmap,
				 DamagePtr damage,
				 int fb_id)
{
    gma500Ptr gma = gma500PTR(scrn);
    RegionPtr dirty = DamageRegion(damage);
    unsigned num_cliprects = REGION_NUM_RECTS(dirty);

    if (num_cliprects) {
	drmModeClip *clip = malloc(num_cliprects * sizeof(drmModeClip));
	BoxPtr rect = REGION_RECTS(dirty);
	int i, ret;
	    
	if (!clip)
	    return -ENOMEM;

	/* XXX no need for copy? */
	for (i = 0; i < num_cliprects; i++, rect++) {
	    clip[i].x1 = rect->x1;
	    clip[i].y1 = rect->y1;
	    clip[i].x2 = rect->x2;
	    clip[i].y2 = rect->y2;
	}

	/* TODO query connector property to see if this is needed */
	ret = drmModeDirtyFB(gma->fd, fb_id, clip, num_cliprects);
	free(clip);
	DamageEmpty(damage);
	if (ret) {
	    if (ret == -EINVAL)
		return ret;
	}
    }
    return 0;
}

static void dispatch_dirty(ScreenPtr pScreen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
    gma500Ptr gma = gma500PTR(scrn);
    PixmapPtr pixmap = pScreen->GetScreenPixmap(pScreen);
    int fb_id = gma->drmmode.fb_id;
    int ret;

    ret = dispatch_dirty_region(scrn, pixmap, gma->damage, fb_id);
    if (ret == -EINVAL || ret == -ENOSYS) {
	gma->dirty_enabled = FALSE;
	DamageUnregister(&pScreen->GetScreenPixmap(pScreen)->drawable, gma->damage);
	DamageDestroy(gma->damage);
	gma->damage = NULL;
	xf86DrvMsg(scrn->scrnIndex, X_INFO, "Disabling kernel dirty updates, not required.\n");
	return;
    }
}

#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
static void dispatch_dirty_crtc(ScrnInfoPtr scrn, xf86CrtcPtr crtc)
{
    gma500Ptr gma = gma500PTR(scrn);
    PixmapPtr pixmap = crtc->randr_crtc->scanout_pixmap;
    gmaPixmapPrivPtr ppriv = gmaGetPixmapPriv(&gma->drmmode, pixmap);
    drmmode_crtc_private_ptr drmmode_crtc = crtc->driver_private;
    DamagePtr damage = drmmode_crtc->slave_damage;
    int fb_id = ppriv->fb_id;
    int ret;

    ret = dispatch_dirty_region(scrn, pixmap, damage, fb_id);
    if (ret) {

    }
}

static void dispatch_slave_dirty(ScreenPtr pScreen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(pScreen);
    xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    int c;

    for (c = 0; c < xf86_config->num_crtc; c++) {
	xf86CrtcPtr crtc = xf86_config->crtc[c];

	if (!crtc->randr_crtc)
	    continue;
	if (!crtc->randr_crtc->scanout_pixmap)
	    continue;

	dispatch_dirty_crtc(scrn, crtc);
    }
}
#endif

static void gmaBlockHandler(BLOCKHANDLER_ARGS_DECL)
{
    SCREEN_PTR(arg);
    gma500Ptr gma = gma500PTR(xf86ScreenToScrn(pScreen));

    pScreen->BlockHandler = gma->BlockHandler;
    pScreen->BlockHandler(BLOCKHANDLER_ARGS);
    pScreen->BlockHandler = gmaBlockHandler;
#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
    if (pScreen->isGPU)
        dispatch_slave_dirty(pScreen);
    else
#endif
    if (gma->dirty_enabled)
        dispatch_dirty(pScreen);
}

static void
FreeRec(ScrnInfoPtr pScrn)
{
    gma500Ptr gma;

    if (!pScrn)
        return;

    gma = gma500PTR(pScrn);
    if (!gma)
        return;
    pScrn->driverPrivate = NULL;

    if (gma->fd > 0) {
        int ret;

        if (gma->pEnt->location.type == BUS_PCI)
            ret = drmClose(gma->fd);
        else
            ret = close(gma->fd);
        (void) ret;
    }
    free(gma->Options);
    free(gma);

}

static Bool
PreInit(ScrnInfoPtr pScrn, int flags)
{
    gma500Ptr gma;
    rgb defaultWeight = { 0, 0, 0 };
    EntityInfoPtr pEnt;
    EntPtr gmaEnt = NULL;
    char *BusID = NULL;
    const char *devicename;
    Bool prefer_shadow = TRUE;
    uint64_t value = 0;
    int ret;
    int bppflags;
    int defaultdepth, defaultbpp;

    if (pScrn->numEntities != 1)
	return FALSE;

    pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

    if (flags & PROBE_DETECT) {
	return FALSE;
    }

    /* Allocate driverPrivate */
    if (!GetRec(pScrn))
	return FALSE;

    gma = gma500PTR(pScrn);
    gma->SaveGeneration = -1;
    gma->pEnt = pEnt;

    pScrn->displayWidth = 640;	       /* default it */

    /* Allocate an entity private if necessary */
    if (xf86IsEntityShared(pScrn->entityList[0])) {
	gmaEnt = xf86GetEntityPrivate(pScrn->entityList[0],
				     gma500EntityIndex)->ptr;
	gma->entityPrivate = gmaEnt;
    } else
	gma->entityPrivate = NULL;

    if (xf86IsEntityShared(pScrn->entityList[0])) {
	if (xf86IsPrimInitDone(pScrn->entityList[0])) {
	    /* do something */
	} else {
	    xf86SetPrimInitDone(pScrn->entityList[0]);
	}
    }

    pScrn->monitor = pScrn->confScreen->monitor;
    pScrn->progClock = TRUE;
    pScrn->rgbBits = 8;

#if XSERVER_PLATFORM_BUS
    if (pEnt->location.type == BUS_PLATFORM) {
        char *path = xf86_get_platform_device_attrib(pEnt->location.id.plat, ODEV_ATTRIB_PATH);
        gma->fd = open_hw(path);
    }
    else 
#endif
    if (pEnt->location.type == BUS_PCI) {
        gma->PciInfo = xf86GetPciInfoForEntity(gma->pEnt->index);
        if (gma->PciInfo) {
            BusID = malloc(64);
            sprintf(BusID, "PCI:%d:%d:%d",
#if XSERVER_LIBPCIACCESS
                    ((gma->PciInfo->domain << 8) | gma->PciInfo->bus),
                    gma->PciInfo->dev, gma->PciInfo->func
#else
                    ((pciConfigPtr) gma->PciInfo->thisCard)->busnum,
                    ((pciConfigPtr) gma->PciInfo->thisCard)->devnum,
                    ((pciConfigPtr) gma->PciInfo->thisCard)->funcnum
#endif
                );
        }
        gma->fd = drmOpen(NULL, BusID);
    } else {
        devicename = xf86FindOptionValue(gma->pEnt->device->options, "kmsdev");
        gma->fd = open_hw(devicename);
    }
    if (gma->fd < 0)
	return FALSE;

    gma->drmmode.fd = gma->fd;

#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
    pScrn->capabilities = 0;
#ifdef DRM_CAP_PRIME
    ret = drmGetCap(gma->fd, DRM_CAP_PRIME, &value);
    if (ret == 0) {
        if (value & DRM_PRIME_CAP_IMPORT)
            pScrn->capabilities |= RR_Capability_SinkOutput;
    }
#endif
#endif
    drmmode_get_default_bpp(pScrn, &gma->drmmode, &defaultdepth, &defaultbpp);
    if (defaultdepth == 24 && defaultbpp == 24)
	    bppflags = SupportConvert32to24 | Support24bppFb;
    else
	    bppflags = PreferConvert24to32 | SupportConvert24to32 | Support32bppFb;
    
    if (!xf86SetDepthBpp
	(pScrn, defaultdepth, defaultdepth, defaultbpp, bppflags))
	return FALSE;

    switch (pScrn->depth) {
    case 15:
    case 16:
    case 24:
	break;
    default:
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Given depth (%d) is not supported by the driver\n",
		   pScrn->depth);
	return FALSE;
    }
    xf86PrintDepthBpp(pScrn);

    /* Process the options */
    xf86CollectOptions(pScrn, NULL);
    if (!(gma->Options = malloc(sizeof(Options))))
	return FALSE;
    memcpy(gma->Options, Options, sizeof(Options));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, gma->Options);

    if (!xf86SetWeight(pScrn, defaultWeight, defaultWeight))
	return FALSE;
    if (!xf86SetDefaultVisual(pScrn, -1))
	return FALSE;

    if (xf86ReturnOptValBool(gma->Options, OPTION_SW_CURSOR, FALSE)) {
	gma->drmmode.sw_cursor = TRUE;
    }

    ret = drmGetCap(gma->fd, DRM_CAP_DUMB_PREFER_SHADOW, &value);
    if (!ret) {
	prefer_shadow = !!value;
    }

    gma->drmmode.shadow_enable = xf86ReturnOptValBool(gma->Options, OPTION_SHADOW_FB, prefer_shadow);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "ShadowFB: preferred %s, enabled %s\n", prefer_shadow ? "YES" : "NO", gma->drmmode.shadow_enable ? "YES" : "NO");
    if (drmmode_pre_init(pScrn, &gma->drmmode, pScrn->bitsPerPixel / 8) == FALSE) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "KMS setup failed\n");
	goto fail;
    }

    /*
     * If the driver can do gamma correction, it should call xf86SetGamma() here.
     */
    {
	Gamma zeros = { 0.0, 0.0, 0.0 };

	if (!xf86SetGamma(pScrn, zeros)) {
	    return FALSE;
	}
    }

    if (pScrn->modes == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No modes.\n");
	return FALSE;
    }

    pScrn->currentMode = pScrn->modes;

    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

    /* Load the required sub modules */
    if (!xf86LoadSubModule(pScrn, "fb")) {
	return FALSE;
    }

    if (gma->drmmode.shadow_enable) {
	if (!xf86LoadSubModule(pScrn, "shadow")) {
	    return FALSE;
	}
    }

    return TRUE;
    fail:
    return FALSE;
}

static void *
gmaShadowWindow(ScreenPtr screen, CARD32 row, CARD32 offset, int mode,
	       CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(screen);
    gma500Ptr gma = gma500PTR(pScrn);
    int stride;

    stride = (pScrn->displayWidth * pScrn->bitsPerPixel) / 8;
    *size = stride;

    return ((uint8_t *)gma->drmmode.front_bo->ptr + row * stride + offset);
}

static Bool
CreateScreenResources(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    gma500Ptr gma = gma500PTR(pScrn);
    PixmapPtr rootPixmap;
    Bool ret;
    void *pixels;
    pScreen->CreateScreenResources = gma->createScreenResources;
    ret = pScreen->CreateScreenResources(pScreen);
    pScreen->CreateScreenResources = CreateScreenResources;

    if (!drmmode_set_desired_modes(pScrn, &gma->drmmode))
      return FALSE;

    drmmode_uevent_init(pScrn, &gma->drmmode);

    if (!gma->SWCursor)
        drmmode_map_cursor_bos(pScrn, &gma->drmmode);
    pixels = drmmode_map_front_bo(&gma->drmmode);
    if (!pixels)
	return FALSE;

    rootPixmap = pScreen->GetScreenPixmap(pScreen);

    if (gma->drmmode.shadow_enable)
	pixels = gma->drmmode.shadow_fb;
    
    if (!pScreen->ModifyPixmapHeader(rootPixmap, -1, -1, -1, -1, -1, pixels))
	FatalError("Couldn't adjust screen pixmap\n");

    if (gma->drmmode.shadow_enable) {
	if (!shadowAdd(pScreen, rootPixmap, shadowUpdatePackedWeak(),
		       gmaShadowWindow, 0, 0))
	    return FALSE;
    }

    gma->damage = DamageCreate(NULL, NULL, DamageReportNone, TRUE,
                              pScreen, rootPixmap);

    if (gma->damage) {
	DamageRegister(&rootPixmap->drawable, gma->damage);
	gma->dirty_enabled = TRUE;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Damage tracking initialized\n");
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Failed to create screen damage record\n");
	return FALSE;
    }
    return ret;
}

static Bool
gmaShadowInit(ScreenPtr pScreen)
{
    if (!shadowSetup(pScreen)) {
	return FALSE;
    }
    return TRUE;
}

#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
static Bool
gmaSetSharedPixmapBacking(PixmapPtr ppix, void *fd_handle)
{
    ScreenPtr screen = ppix->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    gma500Ptr gma = gma500PTR(scrn);
    Bool ret;
    int size = ppix->devKind * ppix->drawable.height;
    int ihandle = (int)(long)fd_handle;

    ret = drmmode_SetSlaveBO(ppix, &gma->drmmode, ihandle, ppix->devKind, size);
    if (ret == FALSE)
	    return ret;

    return TRUE;
}
#endif

static Bool
ScreenInit(SCREEN_INIT_ARGS_DECL)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    gma500Ptr gma = gma500PTR(pScrn);
    VisualPtr visual;
    int ret;

    pScrn->pScreen = pScreen;

    ret = drmSetMaster(gma->fd);
    if (ret) {
        ErrorF("Unable to set master\n");
        return FALSE;
    }
      
    /* HW dependent - FIXME */
    pScrn->displayWidth = pScrn->virtualX;
    if (!drmmode_create_initial_bos(pScrn, &gma->drmmode))
	return FALSE;

    if (gma->drmmode.shadow_enable) {
	gma->drmmode.shadow_fb = calloc(1, pScrn->displayWidth * pScrn->virtualY *
			       ((pScrn->bitsPerPixel + 7) >> 3));
	if (!gma->drmmode.shadow_fb)
	    gma->drmmode.shadow_enable = FALSE;
    }	
    
    miClearVisualTypes();

    if (!miSetVisualTypes(pScrn->depth,
			  miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits, pScrn->defaultVisual))
	return FALSE;

    if (!miSetPixmapDepths())
	return FALSE;

#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
    if (!dixRegisterScreenSpecificPrivateKey(pScreen, &gma->drmmode.pixmapPrivateKeyRec,
        PRIVATE_PIXMAP, sizeof(gmaPixmapPrivRec))) { 
	return FALSE;
    }
#endif

    pScrn->memPhysBase = 0;
    pScrn->fbOffset = 0;

    if (!fbScreenInit(pScreen, NULL,
		      pScrn->virtualX, pScrn->virtualY,
		      pScrn->xDpi, pScrn->yDpi,
		      pScrn->displayWidth, pScrn->bitsPerPixel))
	return FALSE;

    if (pScrn->bitsPerPixel > 8) {
	/* Fixup RGB ordering */
	visual = pScreen->visuals + pScreen->numVisuals;
	while (--visual >= pScreen->visuals) {
	    if ((visual->class | DynamicClass) == DirectColor) {
		visual->offsetRed = pScrn->offset.red;
		visual->offsetGreen = pScrn->offset.green;
		visual->offsetBlue = pScrn->offset.blue;
		visual->redMask = pScrn->mask.red;
		visual->greenMask = pScrn->mask.green;
		visual->blueMask = pScrn->mask.blue;
	    }
	}
    }

    fbPictureInit(pScreen, NULL, 0);

    if (gma->drmmode.shadow_enable && !gmaShadowInit(pScreen)) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "shadow fb init failed\n");
	return FALSE;
    }
  
    gma->createScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = CreateScreenResources;

    xf86SetBlackWhitePixels(pScreen);

    xf86SetBackingStore(pScreen);
    xf86SetSilkenMouse(pScreen);
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    /* Need to extend HWcursor support to handle mask interleave */
    if (!gma->drmmode.sw_cursor)
	xf86_cursors_init(pScreen, 64, 64,
			  HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64 |
			  HARDWARE_CURSOR_ARGB);

    /* Must force it before EnterVT, so we are in control of VT and
     * later memory should be bound when allocating, e.g rotate_mem */
    pScrn->vtSema = TRUE;

    pScreen->SaveScreen = xf86SaveScreen;
    gma->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = CloseScreen;

    gma->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = gmaBlockHandler;

#ifdef MODESETTING_OUTPUT_SLAVE_SUPPORT
    pScreen->SetSharedPixmapBacking = gmaSetSharedPixmapBacking;
#endif

    if (!xf86CrtcScreenInit(pScreen))
	return FALSE;

    if (!miCreateDefColormap(pScreen))
	return FALSE;

    xf86DPMSInit(pScreen, xf86DPMSSet, 0);

    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

    return EnterVT(VT_FUNC_ARGS);
}

static void
AdjustFrame(ADJUST_FRAME_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    gma500Ptr gma = gma500PTR(pScrn);

    drmmode_adjust_frame(pScrn, &gma->drmmode, x, y);
}

static void
FreeScreen(FREE_SCREEN_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    FreeRec(pScrn);
}

static void
LeaveVT(VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    gma500Ptr gma = gma500PTR(pScrn);
    xf86_hide_cursors(pScrn);

    pScrn->vtSema = FALSE;

    drmDropMaster(gma->fd);
}

/*
 * This gets called when gaining control of the VT, and from ScreenInit().
 */
static Bool
EnterVT(VT_FUNC_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);
    gma500Ptr gma = gma500PTR(pScrn);

    pScrn->vtSema = TRUE;

    if (drmSetMaster(gma->fd)) {
        xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "drmSetMaster failed: %s\n",
                   strerror(errno));
    }

    if (!drmmode_set_desired_modes(pScrn, &gma->drmmode))
	return FALSE;

    return TRUE;
}

static Bool
SwitchMode(SWITCH_MODE_ARGS_DECL)
{
    SCRN_INFO_PTR(arg);

    return xf86SetSingleMode(pScrn, mode, RR_Rotate_0);
}

static Bool
CloseScreen(CLOSE_SCREEN_ARGS_DECL)
{
    ScrnInfoPtr pScrn = xf86ScreenToScrn(pScreen);
    gma500Ptr gma = gma500PTR(pScrn);

    if (gma->damage) {
	DamageUnregister(&pScreen->GetScreenPixmap(pScreen)->drawable, gma->damage);
	DamageDestroy(gma->damage);
	gma->damage = NULL;
    }

    if (gma->drmmode.shadow_enable) {
	shadowRemove(pScreen, pScreen->GetScreenPixmap(pScreen));
	free(gma->drmmode.shadow_fb);
	gma->drmmode.shadow_fb = NULL;
    }
    drmmode_uevent_fini(pScrn, &gma->drmmode);

    drmmode_free_bos(pScrn, &gma->drmmode);

    if (pScrn->vtSema) {
        LeaveVT(VT_FUNC_ARGS);
    }

    pScreen->CreateScreenResources = gma->createScreenResources;
    pScreen->BlockHandler = gma->BlockHandler;

    pScrn->vtSema = FALSE;
    pScreen->CloseScreen = gma->CloseScreen;
    return (*pScreen->CloseScreen) (CLOSE_SCREEN_ARGS);
}

static ModeStatus
ValidMode(SCRN_ARG_TYPE arg, DisplayModePtr mode, Bool verbose, int flags)
{
    return MODE_OK;
}
