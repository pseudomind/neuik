/*******************************************************************************
 * Copyright (c) 2014-2020, Michael Leimon <leimon@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#include <SDL.h>

#include "NEUIK_error.h"
#include "NEUIK_Plot2D.h"
#include "NEUIK_Plot2D_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_Render32_SimpleLineToMask
 *
 *  Description:   Renders a 1-4 pixel wide X-Y scatter line plot to a maskMap.
 *                 This version handles rendering of 32bit floating point data.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Plot2D_Render32_SimpleLineToMask(
    NEUIK_Plot2D          * plot2d,
    NEUIK_PlotData        * data,
    neuik_PlotDataConfig  * dataCfg,
    int                     thickness,
    int                     maskW,
    int                     maskH,
    int                     ticZoneW,
    int                     ticZoneH,
    int                     ticZoneOffsetX,
    int                     ticZoneOffsetY,
    neuik_MaskMap        ** lineMask)
{
    unsigned int  uCtr       = 0;
    int           firstPt    = TRUE;
    int           isVert     = FALSE;
    int           lastPtOut  = FALSE;
    int           maskPtX    = 0;
    int           maskPtY    = 0;
    int           maskPtX1   = 0;
    int           maskPtX2   = 0;
    int           maskPtY1   = 0;
    int           maskPtY2   = 0;
    float         dX_32      = 0.0; /* delta in X value between two points */
    float         dY_32      = 0.0; /* delta in Y value between two points */
    float         dXmax_32   = 0.0; /* max delta in X value between two points */
    float         dYmax_32   = 0.0; /* max delta in Y value between two points */
    float         lst_ptX_32 = 0.0; /* value of preceding (last) point */
    float         lst_ptY_32 = 0.0; /* value of preceding (last) point */
    float         m_32       = 0.0; /* slope between two points */
    float         ptX_32     = 0.0;
    float         ptY_32     = 0.0;
    double        xRangeMin  = 0.0;
    double        xRangeMax  = 0.0;
    double        yRangeMin  = 0.0;
    double        yRangeMax  = 0.0;
    double        pxDeltaX   = 0.0; /* x-axis width represented by one pixel */
    double        pxDeltaY   = 0.0; /* y-axis height represented by one pixel */
    NEUIK_Plot  * plot       = NULL;
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_Plot2D_Render32_SimpleLineToMask";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `plot2d` is not of Plot2D class.",                           // [1]
        "Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.",     // [2]
        "Output Argument `lineMask` is NULL.",                                 // [3]
        "Failure in `neuik_MakeMaskMap()`.",                                   // [4]
        "Failure in `neuik_MaskAll()`.",                                       // [5]
        "Failure in `neuik_MaskMap_UnmaskUnboundedPoint()`.",                  // [6]
        "Failure in `neuik_MaskMap_UnmaskLine()`.",                            // [7]
        "Argument `data` has an unsupported value for precision.",             // [8]
        "Argument `thickness` has an invalid value (values `1-4` are valid).", // [9]
        "Failure in `neuik_MaskMap_UnmaskUnboundedLine()`.",                   // [10]
    };

    if (!neuik_Object_IsClass(plot2d, neuik__Class_Plot2D))
    {
        eNum = 1;
        goto out;
    }

    if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 2;
        goto out;
    }
    if (lineMask == NULL)
    {
        eNum = 3;
        goto out;
    }
    if (data->precision != 32)
    {
        eNum = 8;
        goto out;
    }
    if (thickness < 1 || thickness > 4)
    {
        eNum = 9;
        goto out;
    }

    xRangeMin = plot->x_range_min;
    xRangeMax = plot->x_range_max;
    yRangeMin = plot->y_range_min;
    yRangeMax = plot->y_range_max;

    if (neuik_MakeMaskMap(lineMask, maskW, maskH))
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* There are built-in methods for getting easy access to collapsed        */
    /* unmasked regions; leverage this by setting pixels not encompassed as   */
    /* masked.                                                                */
    /*------------------------------------------------------------------------*/
    if (neuik_MaskMap_MaskAll(*lineMask))
    {
        eNum = 5;
        goto out;
    }

    pxDeltaX = (xRangeMax - xRangeMin)/((double)(ticZoneW));
    pxDeltaY = (yRangeMax - yRangeMin)/((double)(ticZoneH));

    /*------------------------------------------------------------------------*/
    /* Iterate through the points in the PlotData set.                        */
    /*------------------------------------------------------------------------*/
    for (uCtr = 0; uCtr < data->nPoints; uCtr++)
    {
        isVert = FALSE;
        ptX_32 = data->data_32[uCtr*2];
        ptY_32 = data->data_32[(uCtr*2)+1];

        maskPtX1 = maskPtX2;
        maskPtY1 = maskPtY2;

        if (!firstPt)
        {
            if (!lastPtOut)
            {
                /*------------------------------------------------------------*/
                /* The preceding point was within the currently displayed     */
                /* region for this plot.                                      */
                /*------------------------------------------------------------*/
                dXmax_32 = ptX_32 - lst_ptX_32;
                dYmax_32 = ptY_32 - lst_ptY_32;
                dX_32 = dXmax_32;
                dY_32 = dYmax_32;
                if (dX_32 == 0.0)
                {
                    isVert = TRUE;
                }
                if (!isVert)
                {
                    m_32 = dY_32/dX_32; /* slope */
                }

                /*------------------------------------------------------------*/
                /* Check if this point is outside of the displayed region     */
                /* if so, limit the effective range for a drawn line.         */
                /*------------------------------------------------------------*/
                if ((double)(ptX_32) < xRangeMin ||
                    (double)(ptY_32) < yRangeMin ||
                    (double)(ptX_32) > xRangeMax ||
                    (double)(ptY_32) > yRangeMax)
                {
                    /*--------------------------------------------------------*/
                    /* This data point lies outside of the currently          */
                    /* displayed region for this plot; A partial line should  */
                    /* be drawn between this point and the last.              */
                    /*--------------------------------------------------------*/
                    lastPtOut = TRUE;

                    /*--------------------------------------------------------*/
                    /* Restrict the effective delta (for drawing lines to the */
                    /* region of supported values.                            */
                    /*--------------------------------------------------------*/
                    if (!isVert)
                    {
                        if ((double)(ptX_32) > xRangeMax)
                        {
                            dX_32 = (float)(xRangeMax) - lst_ptX_32;
                            dY_32 = m_32*dX_32 + lst_ptY_32;
                        }
                    }
                    if ((double)(ptY_32) < yRangeMin)
                    {
                        dY_32 = (float)(yRangeMin) - lst_ptY_32;
                        if (!isVert)
                        {
                            dX_32 = dY_32/m_32;
                        }
                    }
                    if ((double)(ptY_32) > yRangeMax)
                    {
                        dY_32 = (float)(yRangeMax) - lst_ptY_32;
                        if (!isVert)
                        {
                            dX_32 = dY_32/m_32;
                        }
                    }
                }

                maskPtX2 = (int)(
                    ((double)(lst_ptX_32 + dX_32) - xRangeMin)/pxDeltaX);
                maskPtY2 = (ticZoneH-1) - (int)(
                    ((double)(lst_ptY_32 + dY_32) - yRangeMin)/pxDeltaY);

                /*------------------------------------------------------------*/
                /* Prevent the line from drawing outside the mask by a single */
                /* pixel.                                                     */
                /*------------------------------------------------------------*/
                if (maskPtX2 == ticZoneW)
                {
                    if (isVert)
                    {
                        dX_32 = 0.0;
                    }
                    else 
                    {
                        if (dX_32 >= 0)
                        {
                            dX_32 -= (float)(pxDeltaX);
                        }
                        else
                        {
                            dX_32 += (float)(pxDeltaX);
                        }
                        dY_32 = m_32*dX_32 + lst_ptY_32;
                    }

                    maskPtX2 = (int)((
                        (double)(lst_ptX_32 + dX_32) - xRangeMin)/pxDeltaX);
                }
                if (maskPtY2 < 0)
                {
                    if (isVert)
                    {
                        dX_32 = 0.0;
                    }
                    else
                    {
                        if (dY_32 >= 0)
                        {
                            dY_32 -= (float)(pxDeltaY);
                        }
                        else
                        {
                            dY_32 += (float)(pxDeltaY);
                        }
                        dX_32 = dY_32/m_32 + lst_ptX_32;
                    }

                    maskPtY2 = (ticZoneH-1) - (int)((
                        (double)(lst_ptY_32 + dY_32) - yRangeMin)/pxDeltaY);
                }

                switch (thickness)
                {
                case 1:
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    break;
                case 2:
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    break;
                case 3:
                    /*--------------------------------------------------------*/
                    /* Top y-axis row (of three).                             */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY - 1,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY - 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY - 1,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY - 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY - 1,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY - 1))
                    {
                        eNum = 7;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Middle y-axis row (of three).                          */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Bottom y-axis row (of three).                          */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    break;
                case 4:
                    /*--------------------------------------------------------*/
                    /* Top y-axis row (first of four).                        */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY - 1,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY - 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY + 2,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY + 2))
                    {
                        eNum = 7;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Upper-middle y-axis row (second of four).              */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 2, 
                        maskPtY1 + ticZoneOffsetY,
                        maskPtX2 + ticZoneOffsetX + 2, 
                        maskPtY2 + ticZoneOffsetY))
                    {
                        eNum = 7;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Lower-middle y-axis row (third of four).               */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX - 1, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX - 1, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 2, 
                        maskPtY1 + ticZoneOffsetY + 1,
                        maskPtX2 + ticZoneOffsetX + 2, 
                        maskPtY2 + ticZoneOffsetY + 1))
                    {
                        eNum = 7;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Bottom y-axis row (of four).                           */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX, 
                        maskPtY1 + ticZoneOffsetY + 2,
                        maskPtX2 + ticZoneOffsetX, 
                        maskPtY2 + ticZoneOffsetY + 2))
                    {
                        eNum = 7;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskLine(*lineMask,
                        maskPtX1 + ticZoneOffsetX + 1, 
                        maskPtY1 + ticZoneOffsetY + 2,
                        maskPtX2 + ticZoneOffsetX + 1, 
                        maskPtY2 + ticZoneOffsetY + 2))
                    {
                        eNum = 7;
                        goto out;
                    }
                    break;
                }
            }
            else
            {
                /*------------------------------------------------------------*/
                /* The preceding point was outside of the currently displayed */
                /* region for this plot.                                      */
                /*------------------------------------------------------------*/
                if (((double)(ptX_32) < xRangeMin && 
                        (double)(lst_ptX_32) < xRangeMin) ||
                    ((double)(ptX_32) > xRangeMax && 
                        (double)(lst_ptX_32) > xRangeMax) ||
                    ((double)(ptY_32) < yRangeMin && 
                        (double)(lst_ptY_32) < yRangeMin) ||
                    ((double)(ptY_32) > yRangeMax && 
                        (double)(lst_ptY_32) > yRangeMax))
                {
                    /*--------------------------------------------------------*/
                    /* This line segment lies completely outside of the       */
                    /* currently displayed region for this plot.              */
                    /*--------------------------------------------------------*/
                    lastPtOut = TRUE;
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* While the previous point (and potentially this point)  */
                    /* are outside of the displayed region; it is possible    */
                    /* that part of the adjoining line segment could be       */
                    /* visible.                                               */
                    /*--------------------------------------------------------*/

                    if ((double)(ptX_32) < xRangeMin ||
                        (double)(ptY_32) < yRangeMin ||
                        (double)(ptX_32) > xRangeMax ||
                        (double)(ptY_32) > yRangeMax)
                    {
                        /*----------------------------------------------------*/
                        /* This data point also lies outside of the currently */
                        /* displayed region for this plot.                    */
                        /* However, a partial line segment (may) be drawn     */
                        /* between this point and the last.                   */
                        /*----------------------------------------------------*/
                        lastPtOut = TRUE;
                    }
                    else
                    {
                        /*----------------------------------------------------*/
                        /* This point is finally within bounds; at least one  */
                        /* and potentially more points should be unmasked.    */
                        /*----------------------------------------------------*/
                        lastPtOut = FALSE;
                    }

                    dXmax_32 = ptX_32 - lst_ptX_32;
                    dYmax_32 = ptY_32 - lst_ptY_32;
                    dX_32 = dXmax_32;
                    dY_32 = dYmax_32;
                    if (dX_32 == 0.0)
                    {
                        isVert = TRUE;
                    }
                    if (!isVert)
                    {
                        m_32 = dY_32/dX_32; /* slope */
                    }

                    /*--------------------------------------------------------*/
                    /* Restrict the effective delta (for drawing lines to the */
                    /* region of supported values.                            */
                    /*--------------------------------------------------------*/
                    if ((double)(lst_ptX_32) < xRangeMin)
                    {
                        lst_ptY_32 = 
                            lst_ptY_32 + m_32*(xRangeMin - lst_ptX_32);
                        lst_ptX_32 = xRangeMin;
                        maskPtY1   = (ticZoneH-1) - (int)((
                            (double)(lst_ptY_32) - yRangeMin)/pxDeltaY);
                    }
                    if ((double)(lst_ptY_32) < yRangeMin)
                    {
                        if (!isVert)
                        {
                            lst_ptX_32 = 
                                lst_ptX_32 + (yRangeMin - lst_ptY_32)/m_32;
                        }
                        lst_ptY_32 = yRangeMin;
                        maskPtY1   = ticZoneH-1;
                    }
                    if ((double)(lst_ptY_32) > yRangeMax)
                    {
                        maskPtY1   = 0;
                        if (!isVert)
                        {
                            lst_ptX_32 = 
                                lst_ptX_32 + (yRangeMax - lst_ptY_32)/m_32;
                        }
                        lst_ptY_32 = yRangeMax;
                    }
                    if ((double)(ptY_32) > yRangeMax)
                    {
                        dY_32 = yRangeMax - (float)(lst_ptY_32);
                    }
                    else
                    {
                        dY_32 = ptY_32 - (float)(lst_ptY_32);
                    }
                    if (!isVert)
                    {
                        dX_32 = dY_32/m_32;
                    }

                    maskPtX1 = (int)((
                        (double)(lst_ptX_32) - xRangeMin)/pxDeltaX);

                    maskPtX2 = (int)((
                        (double)(lst_ptX_32 + dX_32) - xRangeMin)/pxDeltaX);
                    maskPtY2 = (ticZoneH-1) - (int)((
                        (double)(lst_ptY_32 + dY_32) - yRangeMin)/pxDeltaY);


                    /*--------------------------------------------------------*/
                    /* Prevent the line from drawing outside the mask.        */
                    /*--------------------------------------------------------*/
                    if (maskPtX2 > ticZoneW)
                    {
                        if (dX_32 >= 0)
                        {
                            dX_32 -= (float)(maskPtX2 - ticZoneW)*
                                    (float)(pxDeltaX);
                        }
                        else
                        {
                            dX_32 += (float)(maskPtX2 - ticZoneW)*
                                    (float)(pxDeltaX);
                        }
                        dY_32 = m_32*dX_32 + lst_ptY_32;

                        maskPtX2 = (int)(((double)
                            (lst_ptX_32 + dX_32) - xRangeMin)/pxDeltaX);
                    }
                    if (maskPtY2 < 0)
                    {
                        if (dY_32 >= 0)
                        {
                            dY_32 += (float)(maskPtY2)*(float)(pxDeltaY);
                        }
                        else
                        {
                            dY_32 -= (float)(maskPtY2)*(float)(pxDeltaY);
                        }
                        dX_32 = dY_32/m_32 + lst_ptX_32;

                        maskPtY2 = (ticZoneH-1) - (int)(((double)
                            (lst_ptY_32 + dY_32) - yRangeMin)/pxDeltaY);
                    }

                    switch (thickness)
                    {
                    case 1:
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        break;
                    case 2:
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        break;
                    case 3:
                        /*----------------------------------------------------*/
                        /* Top y-axis row (of three).                         */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY - 1,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY - 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY - 1,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY - 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY - 1,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY - 1))
                        {
                            eNum = 10;
                            goto out;
                        }

                        /*----------------------------------------------------*/
                        /* Middle y-axis row (of three).                      */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }

                        /*----------------------------------------------------*/
                        /* Bottom y-axis row (of three).                      */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        break;
                    case 4:
                        /*----------------------------------------------------*/
                        /* Top y-axis row (first of four).                    */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY - 1,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY - 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY + 2,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY + 2))
                        {
                            eNum = 10;
                            goto out;
                        }

                        /*----------------------------------------------------*/
                        /* Upper-middle y-axis row (second of four).          */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 2, 
                            maskPtY1 + ticZoneOffsetY,
                            maskPtX2 + ticZoneOffsetX + 2, 
                            maskPtY2 + ticZoneOffsetY))
                        {
                            eNum = 10;
                            goto out;
                        }

                        /*----------------------------------------------------*/
                        /* Lower-middle y-axis row (third of four).           */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX - 1, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX - 1, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 2, 
                            maskPtY1 + ticZoneOffsetY + 1,
                            maskPtX2 + ticZoneOffsetX + 2, 
                            maskPtY2 + ticZoneOffsetY + 1))
                        {
                            eNum = 10;
                            goto out;
                        }

                        /*----------------------------------------------------*/
                        /* Bottom y-axis row (of four).                       */
                        /*----------------------------------------------------*/
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX, 
                            maskPtY1 + ticZoneOffsetY + 2,
                            maskPtX2 + ticZoneOffsetX, 
                            maskPtY2 + ticZoneOffsetY + 2))
                        {
                            eNum = 10;
                            goto out;
                        }
                        if (neuik_MaskMap_UnmaskUnboundedLine(*lineMask,
                            maskPtX1 + ticZoneOffsetX + 1, 
                            maskPtY1 + ticZoneOffsetY + 2,
                            maskPtX2 + ticZoneOffsetX + 1, 
                            maskPtY2 + ticZoneOffsetY + 2))
                        {
                            eNum = 10;
                            goto out;
                        }
                        break;
                    }
                }
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* This is how the first data point should be handled.            */
            /*----------------------------------------------------------------*/
            if ((double)(ptX_32) < xRangeMin ||
                (double)(ptY_32) < yRangeMin ||
                (double)(ptX_32) > xRangeMax ||
                (double)(ptY_32) > yRangeMax)
            {
                /*------------------------------------------------------------*/
                /* This data point lies outside of the currently displayed    */
                /* region for this plot.                                      */
                /*------------------------------------------------------------*/
                lastPtOut = TRUE;
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Unmask a single point.                                     */
                /*------------------------------------------------------------*/
                maskPtX = (int)(((double)(ptX_32) - xRangeMin)/(pxDeltaX));
                maskPtY = (ticZoneH-1) - 
                    (int)(((double)(ptY_32) - yRangeMin)/(pxDeltaY));
                maskPtX2 = maskPtX;
                maskPtY2 = maskPtY;

                switch (thickness)
                {
                case 1:
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    break;
                case 2:
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    break;
                case 3:
                    /*--------------------------------------------------------*/
                    /* Top y-axis row (of three).                             */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY - 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY - 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY - 1))
                    {
                        eNum = 6;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Middle y-axis row (of three).                          */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Bottom y-axis row (of three).                          */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    break;
                case 4:
                    /*--------------------------------------------------------*/
                    /* Top y-axis row (first of four).                        */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY - 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY - 1))
                    {
                        eNum = 6;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Upper-middle y-axis row (second of four).              */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 2, 
                        maskPtY + ticZoneOffsetY))
                    {
                        eNum = 6;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Lower-middle y-axis row (third of four).               */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX - 1, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 2, 
                        maskPtY + ticZoneOffsetY + 1))
                    {
                        eNum = 6;
                        goto out;
                    }

                    /*--------------------------------------------------------*/
                    /* Bottom y-axis row (of four).                           */
                    /*--------------------------------------------------------*/
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX, 
                        maskPtY + ticZoneOffsetY + 2))
                    {
                        eNum = 6;
                        goto out;
                    }
                    if (neuik_MaskMap_UnmaskUnboundedPoint(*lineMask, 
                        maskPtX + ticZoneOffsetX + 1, 
                        maskPtY + ticZoneOffsetY + 2))
                    {
                        eNum = 6;
                        goto out;
                    }
                    break;
                }
            }
            firstPt = FALSE;
        }
        lst_ptX_32 = ptX_32;
        lst_ptY_32 = ptY_32;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

