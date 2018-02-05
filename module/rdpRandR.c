/*
Copyright 2011-2017 Jay Sorg

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

RandR draw calls

*/

#if defined(HAVE_CONFIG_H)
#include "config_ac.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this should be before all X11 .h files */
#include <xorg-server.h>
#include <xorgVersion.h>

/* all driver need this */
#include <xf86.h>
#include <xf86_OSproc.h>

#include <mipointer.h>
#include <fb.h>
#include <micmap.h>
#include <mi.h>

#include "rdp.h"
#include "rdpDraw.h"
#include "rdpReg.h"
#include "rdpMisc.h"
#include "rdpRandR.h"

static int g_panning = 0;

/******************************************************************************/
#define LOG_LEVEL 1
#define LLOGLN(_level, _args) \
    do { if (_level < LOG_LEVEL) { ErrorF _args ; ErrorF("\n"); } } while (0)

/******************************************************************************/
Bool
rdpRRRegisterSize(ScreenPtr pScreen, int width, int height)
{
    int mmwidth;
    int mmheight;
    RRScreenSizePtr pSize;
    ScrnInfoPtr pScrn;

    LLOGLN(0, ("rdpRRRegisterSize: width %d height %d", width, height));
    pScrn = xf86Screens[pScreen->myNum];
    mmwidth = PixelToMM(width, pScrn->xDpi);
    mmheight = PixelToMM(height, pScrn->yDpi);
    pSize = RRRegisterSize(pScreen, width, height, mmwidth, mmheight);
    /* Tell RandR what the current config is */
    RRSetCurrentConfig(pScreen, RR_Rotate_0, 0, pSize);
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRSetConfig(ScreenPtr pScreen, Rotation rotateKind, int rate,
               RRScreenSizePtr pSize)
{
    LLOGLN(0, ("rdpRRSetConfig:"));
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRGetInfo(ScreenPtr pScreen, Rotation *pRotations)
{
    LLOGLN(0, ("rdpRRGetInfo:"));
    *pRotations = RR_Rotate_0;
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRScreenSetSize(ScreenPtr pScreen, CARD16 width, CARD16 height,
                   CARD32 mmWidth, CARD32 mmHeight)
{
    WindowPtr root;
    PixmapPtr screenPixmap;
    BoxRec box;
    rdpPtr dev;

    LLOGLN(0, ("rdpRRScreenSetSize: width %d height %d mmWidth %d mmHeight %d",
           width, height, (int)mmWidth, (int)mmHeight));
    dev = rdpGetDevFromScreen(pScreen);
    root = rdpGetRootWindowPtr(pScreen);
    if ((width < 1) || (height < 1))
    {
        LLOGLN(10, ("  error width %d height %d", width, height));
        return FALSE;
    }
    dev->width = width;
    dev->height = height;
    dev->paddedWidthInBytes = PixmapBytePad(dev->width, dev->depth);
    dev->sizeInBytes = dev->paddedWidthInBytes * dev->height;
    pScreen->width = width;
    pScreen->height = height;
    pScreen->mmWidth = mmWidth;
    pScreen->mmHeight = mmHeight;
    screenPixmap = pScreen->GetScreenPixmap(pScreen);
    free(dev->pfbMemory_alloc);
    dev->pfbMemory_alloc = g_new0(uint8_t, dev->sizeInBytes + 16);
    dev->pfbMemory = (uint8_t *) RDPALIGN(dev->pfbMemory_alloc, 16);
    if (screenPixmap != 0)
    {
        pScreen->ModifyPixmapHeader(screenPixmap, width, height,
                                    -1, -1,
                                    dev->paddedWidthInBytes,
                                    dev->pfbMemory);
    }
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = width;
    box.y2 = height;
    rdpRegionInit(&root->winSize, &box, 1);
    rdpRegionInit(&root->borderSize, &box, 1);
    rdpRegionReset(&root->borderClip, &box);
    rdpRegionBreak(&root->clipList);
    root->drawable.width = width;
    root->drawable.height = height;
    ResizeChildrenWinSize(root, 0, 0, 0, 0);
    RRGetInfo(pScreen, 1);
    LLOGLN(0, ("  screen resized to %dx%d", pScreen->width, pScreen->height));
    RRScreenSizeNotify(pScreen);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1, 13, 0, 0, 0)
    xf86EnableDisableFBAccess(pScreen->myNum, FALSE);
    xf86EnableDisableFBAccess(pScreen->myNum, TRUE);
#else
    xf86EnableDisableFBAccess(xf86Screens[pScreen->myNum], FALSE);
    xf86EnableDisableFBAccess(xf86Screens[pScreen->myNum], TRUE);
#endif
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRCrtcSet(ScreenPtr pScreen, RRCrtcPtr crtc, RRModePtr mode,
             int x, int y, Rotation rotation, int numOutputs,
             RROutputPtr *outputs)
{
    LLOGLN(0, ("rdpRRCrtcSet:"));
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRCrtcSetGamma(ScreenPtr pScreen, RRCrtcPtr crtc)
{
    LLOGLN(0, ("rdpRRCrtcSetGamma:"));
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRCrtcGetGamma(ScreenPtr pScreen, RRCrtcPtr crtc)
{
    LLOGLN(0, ("rdpRRCrtcGetGamma: %p %p %p %p", crtc, crtc->gammaRed,
           crtc->gammaBlue, crtc->gammaGreen));
    return TRUE;
}

/******************************************************************************/
Bool
rdpRROutputSetProperty(ScreenPtr pScreen, RROutputPtr output, Atom property,
                       RRPropertyValuePtr value)
{
    LLOGLN(0, ("rdpRROutputSetProperty:"));
    return TRUE;
}

/******************************************************************************/
Bool
rdpRROutputValidateMode(ScreenPtr pScreen, RROutputPtr output,
                        RRModePtr mode)
{
    LLOGLN(0, ("rdpRROutputValidateMode:"));
    return TRUE;
}

/******************************************************************************/
void
rdpRRModeDestroy(ScreenPtr pScreen, RRModePtr mode)
{
    LLOGLN(0, ("rdpRRModeDestroy:"));
}

/******************************************************************************/
Bool
rdpRROutputGetProperty(ScreenPtr pScreen, RROutputPtr output, Atom property)
{
    LLOGLN(0, ("rdpRROutputGetProperty:"));
    return TRUE;
}

/******************************************************************************/
#if 0
static int
get_rect(rdpPtr dev, const char *name, BoxPtr rect)
{
    if (strcmp(name, "rdp0") == 0)
    {
        rect->x1 = dev->minfo[0].left; 
        rect->y1 = dev->minfo[0].top;
        rect->x2 = dev->minfo[0].right + 1;
        rect->y2 = dev->minfo[0].bottom + 1;
    }
    else if (strcmp(name, "rdp1") == 0)
    {
        rect->x1 = dev->minfo[1].left; 
        rect->y1 = dev->minfo[1].top;
        rect->x2 = dev->minfo[1].right + 1;
        rect->y2 = dev->minfo[1].bottom + 1;
    }
    else if (strcmp(name, "rdp2") == 0)
    {
        rect->x1 = dev->minfo[2].left; 
        rect->y1 = dev->minfo[2].top;
        rect->x2 = dev->minfo[2].right + 1;
        rect->y2 = dev->minfo[2].bottom + 1;
    }
    else if (strcmp(name, "rdp3") == 0)
    {
        rect->x1 = dev->minfo[3].left; 
        rect->y1 = dev->minfo[3].top;
        rect->x2 = dev->minfo[3].right + 1;
        rect->y2 = dev->minfo[3].bottom + 1;
    }
    return 0;
}
#endif

/******************************************************************************/
Bool
rdpRRGetPanning(ScreenPtr pScreen, RRCrtcPtr crtc, BoxPtr totalArea,
                BoxPtr trackingArea, INT16 *border)
{
    rdpPtr dev;
    BoxRec totalAreaRect;
    BoxRec trackingAreaRect;

    LLOGLN(10, ("rdpRRGetPanning: totalArea %p trackingArea %p border %p",
                totalArea, trackingArea, border));

    if (!g_panning)
    {
        return FALSE;
    }

    dev = rdpGetDevFromScreen(pScreen);

    totalAreaRect.x1 = 0;
    totalAreaRect.y1 = 0;
    totalAreaRect.x2 = dev->width;
    totalAreaRect.y2 = dev->height;

    trackingAreaRect.x1 = 0;
    trackingAreaRect.y1 = 0;
    trackingAreaRect.x2 = dev->width;
    trackingAreaRect.y2 = dev->height;

    if (totalArea != 0)
    {
        *totalArea = totalAreaRect;
    }

    if (trackingArea != 0)
    {
        *trackingArea = trackingAreaRect;
    }

    if (border != 0)
    {
        border[0] = 0;
        border[1] = 0;
        border[2] = 0;
        border[3] = 0;
    }
    return TRUE;
}

/******************************************************************************/
Bool
rdpRRSetPanning(ScreenPtr pScreen, RRCrtcPtr crtc, BoxPtr totalArea,
                BoxPtr trackingArea, INT16 *border)
{
    LLOGLN(0, ("rdpRRSetPanning:"));
    return TRUE;
}

/******************************************************************************/
static RROutputPtr
rdpRRAddOutput(rdpPtr dev, const char *aname, int x, int y, int width, int height)
{
    RRModePtr mode;
    RRCrtcPtr crtc;
    RROutputPtr output;
    xRRModeInfo modeInfo;
    char name[64];
    const int vfreq = 50;
    int i;

    sprintf (name, "%dx%d", width, height);
    memset (&modeInfo, 0, sizeof(modeInfo));
    modeInfo.width = width;
    modeInfo.height = height;
    modeInfo.hTotal = width;
    modeInfo.vTotal = height;
    modeInfo.dotClock = vfreq * width * height;
    modeInfo.nameLength = strlen(name);
    mode = RRModeGet(&modeInfo, name);
    if (mode == 0)
    {
        LLOGLN(0, ("rdpRRAddOutput: RRModeGet failed"));
        return 0;
    }

    crtc = RRCrtcCreate(dev->pScreen, NULL);
    if (crtc == 0)
    {
        LLOGLN(0, ("rdpRRAddOutput: RRCrtcCreate failed"));
        RRModeDestroy(mode);
        return 0;
    }
    /* Create and initialise (unused) gamma ramps */
    RRCrtcGammaSetSize (crtc, 256);
    for (i = 0 ; i < crtc->gammaSize; ++i)
    {
        unsigned short val = (0xffff * i) / (crtc->gammaSize - 1);
        crtc->gammaRed[i] = val;
        crtc->gammaGreen[i] = val;
        crtc->gammaBlue[i] = val;
    }

    output = RROutputCreate(dev->pScreen, aname, strlen(aname), NULL);
    if (output == 0)
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputCreate failed"));
        RRCrtcDestroy(crtc);
        RRModeDestroy(mode);
        return 0;
    }
    if (!RROutputSetClones(output, NULL, 0))
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputSetClones failed"));
    }
    if (!RROutputSetModes(output, &mode, 1, 0))
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputSetModes failed"));
    }
    if (!RROutputSetCrtcs(output, &crtc, 1))
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputSetCrtcs failed"));
    }
    if (!RROutputSetConnection(output, RR_Connected))
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputSetConnection failed"));
    }
    RRCrtcNotify(crtc, mode, x, y, RR_Rotate_0, NULL, 1, &output);

    dev->output[dev->extra_outputs] = output;
    dev->crtc[dev->extra_outputs] = crtc;
    dev->extra_outputs++;

    return output;
}

/******************************************************************************/
static RROutputPtr
rdpRRUpdateOutput(rdpPtr dev, int x, int y, int width, int height, int index)
{
    RRModePtr mode;
    RRCrtcPtr crtc;
    RROutputPtr output;
    xRRModeInfo modeInfo;
    char name[64];
    const int vfreq = 50;

    sprintf (name, "%dx%d", width, height);
    memset (&modeInfo, 0, sizeof(modeInfo));
    modeInfo.width = width;
    modeInfo.height = height;
    modeInfo.hTotal = width;
    modeInfo.vTotal = height;
    modeInfo.dotClock = vfreq * width * height;
    modeInfo.nameLength = strlen(name);
    mode = RRModeGet(&modeInfo, name);
    if (mode == 0)
    {
        LLOGLN(0, ("rdpRRAddOutput: RRModeGet failed"));
        return 0;
    }
    output = dev->output[index];
    if (!RROutputSetModes(output, &mode, 1, 0))
    {
        LLOGLN(0, ("rdpRRAddOutput: RROutputSetModes failed"));
    }
    crtc = dev->crtc[index];
    RRCrtcNotify(crtc, mode, x, y, RR_Rotate_0, NULL, 1, &output);
    RROutputChanged(output, 1);
    return output;
}

/******************************************************************************/
static void
RRSetPrimaryOutput(rrScrPrivPtr pScrPriv, RROutputPtr output)
{
    if (pScrPriv->primaryOutput == output)
    {
        return;
    }
    /* clear the old primary */
    if (pScrPriv->primaryOutput)
    {
        RROutputChanged(pScrPriv->primaryOutput, 0);
        pScrPriv->primaryOutput = NULL;
    }
    /* set the new primary */
    if (output)
    {
        pScrPriv->primaryOutput = output;
        RROutputChanged(output, 0);
    }
    pScrPriv->layoutChanged = TRUE;
}

/******************************************************************************/
int
rdpRRSetRdpOutputs(rdpPtr dev)
{
    rrScrPrivPtr pRRScrPriv;
    int index;
    int left;
    int top;
    int width;
    int height;
    char text[256];
    RROutputPtr output;

    pRRScrPriv = rrGetScrPriv(dev->pScreen);
    LLOGLN(0, ("rdpRRSetRdpOutputs: numCrtcs %d monitorCount %d",
           pRRScrPriv->numCrtcs, dev->monitorCount));
    if (dev->monitorCount <= 0)
    {
        left = 0;
        top = 0;
        width = dev->width;
        height = dev->height;
        if (pRRScrPriv->numCrtcs > 0)
        {
            /* update */
            LLOGLN(0, ("rdpRRSetRdpOutputs: update output %d "
                   "left %d top %d width %d height %d",
                   0, left, top, width, height));
            output = rdpRRUpdateOutput(dev,
                                       left, top, width, height, 0);
        }
        else
        {
            /* add */
            LLOGLN(0, ("rdpRRSetRdpOutputs: add output %d "
                   "left %d top %d width %d height %d",
                   0, left, top, width, height));
            snprintf(text, 255, "rdp%d", 0);
            output = rdpRRAddOutput(dev, text,
                                    left, top, width, height);
        }
        /* remove any entra */
        for (index = pRRScrPriv->numCrtcs; index > 1; index--)
        {
            RRCrtcDestroy(pRRScrPriv->crtcs[index - 1]);
        }
        for (index = pRRScrPriv->numOutputs; index > 1; index--)
        {
            RROutputDestroy(pRRScrPriv->outputs[index - 1]);
        }
    }
    else
    {
        for (index = 0; index < dev->monitorCount; index++)
        {
            left = dev->minfo[index].left;
            top = dev->minfo[index].top;
            width = dev->minfo[index].right - dev->minfo[index].left + 1;
            height = dev->minfo[index].bottom - dev->minfo[index].top + 1;
            if (index < pRRScrPriv->numCrtcs)
            {
                /* update */
                LLOGLN(0, ("rdpRRSetRdpOutputs: update output %d "
                       "left %d top %d width %d height %d",
                       index, left, top, width, height));
                output = rdpRRUpdateOutput(dev,
                                           left, top, width, height, index);
            }
            else
            {
                /* add */
                LLOGLN(0, ("rdpRRSetRdpOutputs: add output %d "
                       "left %d top %d width %d height %d",
                       index, left, top, width, height));
                snprintf(text, 255, "rdp%d", index);
                output = rdpRRAddOutput(dev, text,
                                        left, top, width, height);
            }
            if ((output != 0) && (dev->minfo[index].is_primary))
            {
                RRSetPrimaryOutput(pRRScrPriv, output);
            }
        }
        /* remove any entra */
        for (index = pRRScrPriv->numCrtcs; index > dev->monitorCount; index--)
        {
            RRCrtcDestroy(pRRScrPriv->crtcs[index - 1]);
        }
        for (index = pRRScrPriv->numOutputs; index > dev->monitorCount; index--)
        {
            RROutputDestroy(pRRScrPriv->outputs[index - 1]);
        }
    }
    return 0;
}

