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
#include <stdlib.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Fill.h"
#include "NEUIK_Label.h"
#include "NEUIK_Plot.h"
#include "NEUIK_Plot2D.h"
#include "NEUIK_Plot2D_internal.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;
extern int neuik__Report_Debug;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Plot2D(void ** vgPtr);
int neuik_Object_Free__Plot2D(void * vgPtr);

int neuik_Element_GetMinSize__Plot2D(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Plot2D(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Plot2D_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__Plot2D,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__Plot2D,

    /* CaptureEvent(): Determine if this element caputures a given event */
    NULL,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Plot2D_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__Plot2D,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__Plot2D,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Plot2D
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Plot2D()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_Plot2D";
    static char * errMsgs[]  = {"",                  // [0] no error
        "NEUIK library must be initialized first.",   // [1]
        "Failed to register `Plot2D` object class .", // [2]
    };

    if (!neuik__isInitialized)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Otherwise, register the object                                         */
    /*------------------------------------------------------------------------*/
    if (neuik_RegisterClass(
        "NEUIK_Plot2D",                                           // className
        "An plot element which displays data in two dimensions.", // classDescription
        neuik__Set_NEUIK,                                         // classSet
        neuik__Class_Plot,                                        // superClass
        &neuik_Plot2D_BaseFuncs,                                  // baseFuncs
        NULL,                                                     // classFuncs
        &neuik__Class_Plot2D))                                    // newClass
    {
        eNum = 2;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New__Plot2D
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Plot2D(
    void ** pltPtr)
{
    int                    eNum       = 0;
    int                    ctr        = 0;
    NEUIK_Plot           * plot       = NULL;
    NEUIK_Plot2D         * plot2d     = NULL;
    neuik_PlotDataConfig * dataCfg    = NULL;
    NEUIK_Element        * sClassPtr  = NULL;
    static char            funcName[] = "neuik_Object_New__Plot2D";
    static char          * errMsgs[]  = {"", // [0] no error
        "Output Argument `pltPtr` is NULL.",                               // [1]
        "Failure to allocate memory.",                                     // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",                        // [3]
        "Failure in function `neuik.NewElement`.",                         // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",               // [5]
        "Argument `pltPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
        "Failure in function `NEUIK_Container_AddElement()`.",             // [7]
        "Failure in function `NEUIK_NewCanvas()`.",                        // [8]
        "Failure in function `NEUIK_Element_Configure()`.",                // [9]
        "Failure in function `NEUIK_NewGridLayout()`.",                    // [10]
        "Failure in function `NEUIK_GridLayout_SetDimensions()`.",         // [11]
        "Failure in function `NEUIK_GridLayout_SetSpacing()`.",            // [12]
        "Failure in function `NEUIK_NewHGroup()`.",                        // [13]
        "Failure in function `NEUIK_NewVGroup()`.",                        // [14]
        "Failure in function `NEUIK_GridLayout_SetElementAt()`.",          // [15]
        "Failure in function `NEUIK_HGroup_SetHSpacing()`.",               // [16]
        "Failure in function `NEUIK_VGroup_SetVSpacing()`.",               // [17]
    };

    if (pltPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*pltPtr) = (NEUIK_Plot2D*) malloc(sizeof(NEUIK_Plot2D));
    plot2d = *pltPtr;
    if (plot2d == NULL)
    {
        eNum = 2;
        goto out;
    }
    /* Allocation successful */

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK,
            neuik__Class_Plot2D,
            NULL,
            &(plot2d->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the plot background layer.                                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewCanvas(&plot2d->drawing_background))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_Configure(plot2d->drawing_background, "FillAll", NULL))
    {
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the plot ticmark layer.                                         */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewGridLayout(&plot2d->drawing_ticmarks))
    {
        eNum = 10;
        goto out;
    }
    if (NEUIK_GridLayout_SetDimensions(plot2d->drawing_ticmarks, 2, 2))
    {
        eNum = 11;
        goto out;
    }
    if (NEUIK_Element_Configure(plot2d->drawing_ticmarks, "FillAll", NULL))
    {
        eNum = 9;
        goto out;
    }
    if (NEUIK_GridLayout_SetSpacing(plot2d->drawing_ticmarks, 0))
    {
        eNum = 12;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the Y-axis ticmarks.                                            */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewVGroup(&plot2d->drawing_y_axis_ticmarks))
    {
        eNum = 14;
        goto out;
    }
    if (NEUIK_VGroup_SetVSpacing(plot2d->drawing_y_axis_ticmarks, 0))
    {
        eNum = 17;
        goto out;
    }
    if (NEUIK_Element_Configure(plot2d->drawing_y_axis_ticmarks, "VFill", NULL))
    {
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the X-axis ticmarks.                                            */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewHGroup(&plot2d->drawing_x_axis_ticmarks))
    {
        eNum = 13;
        goto out;
    }
    if (NEUIK_HGroup_SetHSpacing(plot2d->drawing_x_axis_ticmarks, 0))
    {
        eNum = 16;
        goto out;
    }
    if (NEUIK_Element_Configure(plot2d->drawing_x_axis_ticmarks, "HFill", NULL))
    {
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the plot area ticmark drawing canvas.                           */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewCanvas(&plot2d->drawing_ticmarks_plot_area))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_Configure(
        plot2d->drawing_ticmarks_plot_area, "FillAll", NULL))
    {
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create the plotted values layer.                                       */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewCanvas(&plot2d->drawing_plotted_data))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_Configure(plot2d->drawing_plotted_data, "FillAll", NULL))
    {
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(plot2d->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Plot, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(plot2d, &neuik_Plot2D_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 6;
        goto out;
    }
    if (NEUIK_Container_AddElement(plot->drawing, plot2d->drawing_background))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Container_AddElement(plot->drawing, plot2d->drawing_ticmarks))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Container_AddElement(plot->drawing, plot2d->drawing_plotted_data))
    {
        eNum = 7;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Add the Y-Axis elements.                                               */
    /*------------------------------------------------------------------------*/
    if (NEUIK_GridLayout_SetElementAt(
        plot2d->drawing_ticmarks, 0, 0, plot2d->drawing_y_axis_ticmarks))
    {
        eNum = 15;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Add the X-Axis elements.                                               */
    /*------------------------------------------------------------------------*/
    if (NEUIK_GridLayout_SetElementAt(
        plot2d->drawing_ticmarks, 1, 1, plot2d->drawing_x_axis_ticmarks))
    {
        eNum = 15;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Add the plot area element.                                             */
    /*------------------------------------------------------------------------*/
    if (NEUIK_GridLayout_SetElementAt(
        plot2d->drawing_ticmarks, 1, 0, plot2d->drawing_ticmarks_plot_area))
    {
        eNum = 15;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate memory for tracking DataSets.                                 */
    /*------------------------------------------------------------------------*/
    plot->data_sets = malloc(5*sizeof(NEUIK_Object));
    if (plot->data_sets == NULL)
    {
        eNum = 2;
        goto out;
    }
    plot->data_configs = malloc(5*sizeof(neuik_PlotDataConfig));
    if (plot->data_configs == NULL)
    {
        eNum = 2;
        goto out;
    }
    plot->n_allocated = 5;
    plot->n_used = 0;

    for (ctr = plot->n_used; ctr < plot->n_allocated; ctr++)
    {
        dataCfg = &(plot->data_configs[ctr]);
        dataCfg->uniqueName = NULL;
        dataCfg->label      = NULL;
    }

    /*------------------------------------------------------------------------*/
    /* Set the initial states for the configurable parameters.                */
    /*------------------------------------------------------------------------*/
    plot2d->xAxisCfg.nTicmarks = UNDEFINED;
    plot2d->yAxisCfg.nTicmarks = UNDEFINED;
    /*------------------------------------------------------------------------*/
    plot2d->xAxisCfg.showGridlines = TRUE;
    plot2d->yAxisCfg.showGridlines = TRUE;
    /*------------------------------------------------------------------------*/
    plot2d->xAxisCfg.showTicLabels = TRUE;
    plot2d->yAxisCfg.showTicLabels = TRUE;

    plot2d->colorGridline.r = 130;
    plot2d->colorGridline.g = 130;
    plot2d->colorGridline.b = 130;
    plot2d->colorGridline.a = 255;
    /*------------------------------------------------------------------------*/
    plot2d->xAxisCfg.colorGridline.r = 175;
    plot2d->xAxisCfg.colorGridline.g = 175;
    plot2d->xAxisCfg.colorGridline.b = 175;
    plot2d->xAxisCfg.colorGridline.a = 255;
    /*------------------------------------------------------------------------*/
    plot2d->yAxisCfg.colorGridline.r = 175;
    plot2d->yAxisCfg.colorGridline.g = 175;
    plot2d->yAxisCfg.colorGridline.b = 175;
    plot2d->yAxisCfg.colorGridline.a = 255;

out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewPlot2D
 *
 *  Description:   Create and return a pointer to a new NEUIK_Plot2D.
 *
 *                 Wrapper function to neuik_Object_New__Plot2D.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewPlot2D(
    NEUIK_Plot2D ** pltPtr)
{
    return neuik_Object_New__Plot2D((void**)pltPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Plot2D(
    void  * pltPtr)
{
    int            eNum       = 0;    /* which error to report (if any) */
    NEUIK_Plot2D * plt        = NULL;
    static char    funcName[] = "neuik_Object_Free__Plot2D";
    static char  * errMsgs[]  = {"",                    // [0] no error
        "Argument `pltPtr` is NULL.",                   // [1]
        "Argument `pltPtr` is not of Container class.", // [2]
        "Failure in function `neuik_Object_Free`.",     // [3]
    };

    if (pltPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(pltPtr, neuik__Class_Plot2D))
    {
        eNum = 2;
        goto out;
    }
    plt = (NEUIK_Plot2D*)pltPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(plt->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    free(plt);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__Plot2D
 *
 *  Description:   Returns the rendered size of a given Plot2D.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Plot2D(
    NEUIK_Element   pltElem,
    RenderSize    * rSize)
{
    int            eNum       = 0;   /* which error to report (if any) */
    NEUIK_Plot2D * plt        = NULL;
    NEUIK_Plot   * plot       = NULL;
    static char    funcName[] = "neuik_Element_GetMinSize__Plot2D";
    static char  * errMsgs[]  = {"",                                        // [0] no error
        "Argument `pltElem` is not of Plot2D class.",                       // [1]
        "Argument `pltElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Failure in `neuik_Element_GetSize()`.",                            // [3]
    };

    /*------------------------------------------------------------------------*/
    /* Check for problems before proceding                                    */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(pltElem, neuik__Class_Plot2D))
    {
        eNum = 1;
        goto out;
    }
    plt = (NEUIK_Plot2D*)pltElem;

    if (neuik_Object_GetClassObject(plt, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 2;
        goto out;
    }

    if (neuik_Element_GetMinSize(plot->visual, rSize) != 0)
    {
        eNum = 3;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_RenderSimpleLineToMask
 *
 *  Description:   Renders a 1-4 pixel wide X-Y scatter line plot to a maskMap.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Plot2D_RenderSimpleLineToMask(
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
    NEUIK_Plot  * plot       = NULL;
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_Plot2D_RenderSimpleLineToMask";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `plot2d` is not of Plot2D class.",                           // [1]
        "Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.",     // [2]
        "Output Argument `lineMask` is NULL.",                                 // [3]
        "Argument `data` has an unsupported value for precision.",             // [4]
        "Argument `thickness` has an invalid value (values `1-4` are valid).", // [5]
        "Failure in `neuik_Plot2D_Render32_SimpleLineToMask()`.",              // [6]
        "Failure in `neuik_Plot2D_Render64_SimpleLineToMask()`.",              // [7]
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
    if (!(data->precision == 32 || data->precision == 64))
    {
        eNum = 4;
        goto out;
    }
    if (thickness < 1 || thickness > 4)
    {
        eNum = 5;
        goto out;
    }

    if (data->precision == 32)
    {
        if (neuik_Plot2D_Render32_SimpleLineToMask(
            plot2d, data, dataCfg, thickness, maskW, maskH, 
            ticZoneW, ticZoneH, ticZoneOffsetX, ticZoneOffsetY, lineMask))
        {
            eNum = 6;
            goto out;
        }
    }
    else if (data->precision == 64)
    {
        if (neuik_Plot2D_Render64_SimpleLineToMask(
            plot2d, data, dataCfg, thickness, maskW, maskH, 
            ticZoneW, ticZoneH, ticZoneOffsetX, ticZoneOffsetY, lineMask))
        {
            eNum = 7;
            goto out;
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__Plot2D
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Plot2D(
    NEUIK_Element   pltElem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    unsigned int           uCtr         = 0;
    int                    ctr          = 0;
    int                    eNum         = 0; /* which error to report (if any) */
    int                    lnThickness  = 0;
    int                    maskCtr      = 0; /* maskMap counter */
    int                    maskW        = 0;
    int                    maskH        = 0;
    int                    maskRegions  = 0; /* number of regions in maskMap */
    int                    pltOffsetX   = 0;
    int                    pltOffsetY   = 0;
    int                    tic_x_cl     = 0;
    int                    tic_xmin     = 0;
    int                    tic_xmax     = 0;
    int                    tic_y_cl     = 0;
    int                    tic_ymin     = 0;
    int                    tic_ymax     = 0;
    int                    ticMarkPos   = 0;
    int                    ticZoneW     = 0;
    int                    ticZoneH     = 0;
    const int            * regionY0;         /* Array of region Y0 values */
    const int            * regionYf;         /* Array of region Yf values */
    double                 tic_x_offset = 0.0;
    double                 tic_x_adj    = 0.0;
    double                 tic_y_offset = 0.0;
    double                 tic_y_adj    = 0.0;
    static NEUIK_Color     autoColors[] = {
        COLOR_PLOTLINE_01,
        COLOR_PLOTLINE_02,
        COLOR_PLOTLINE_03,
        COLOR_PLOTLINE_04,
        COLOR_PLOTLINE_05,
        COLOR_PLOTLINE_06,
        COLOR_PLOTLINE_07,
        COLOR_PLOTLINE_08,
        COLOR_PLOTLINE_09,
        COLOR_PLOTLINE_10,
        COLOR_PLOTLINE_11,
        COLOR_PLOTLINE_12,
    };

    RenderLoc              rl;
    RenderLoc              rlRel      = {0, 0}; /* renderloc relative to parent */
    RenderLoc              dwg_loc;
    RenderLoc              tic_loc;
    RenderLoc              ticPlotLoc;
    SDL_Rect               rect       = {0, 0, 0, 0};
    RenderSize             rs         = {0, 0};
    RenderSize             dwg_rs;
    RenderSize             tic_rs;
    NEUIK_Element          ticElem    = NULL;
    SDL_Renderer         * rend       = NULL;
    NEUIK_Plot           * plot       = NULL;
    NEUIK_ElementBase    * eBase      = NULL;
    NEUIK_ElementConfig  * eCfg       = NULL;
    NEUIK_Plot2D         * plt        = NULL;
    NEUIK_PlotData       * data       = NULL;
    neuik_PlotDataConfig * dataCfg    = NULL;
    NEUIK_Canvas         * dwg;               /* pointer to active drawing (don't free) */
    neuik_MaskMap        * maskMap    = NULL; /* FREE upon return */
    enum neuik_bgstyle     bgStyle;
    static char            funcName[] = "neuik_Element_Render__Plot2D";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `pltElem` is not of Plot2D class.",                       // [1]
        "Failure in `neuik_Element_GetCurrentBGStyle()`.",                  // [2]
        "Element_GetConfig returned NULL.",                                 // [3]
        "Element_GetMinSize Failed.",                                       // [4]
        "Failure in `neuik_Element_Render()`",                              // [5]
        "Invalid specified `rSize` (negative values).",                     // [6]
        "Failure in `neuik_MakeMaskMap()`",                                 // [7]
        "Argument `pltElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
        "Failure in neuik_Element_RedrawBackground().",                     // [9]
        "Failure in `neuik_Window_FillTranspMaskFromLoc()`",                // [10]
        "Failure in `NEUIK_Container_GetFirstElement()`",                   // [11]
        "Failure in `NEUIK_Container_GetLastElement()`",                    // [12]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                 // [13]
        "Failure in `neuik_Plot2D_RenderSimpleLineToMask()`.",              // [14]
        "Failure in `neuik_MaskMap_GetUnmaskedRegionsOnVLine()`.",          // [15]
    };

    if (!neuik_Object_IsClass(pltElem, neuik__Class_Plot2D))
    {
        eNum = 1;
        goto out;
    }
    plt = (NEUIK_Plot2D*)pltElem;

    if (neuik_Object_GetClassObject(pltElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 8;
        goto out;
    }
    if (neuik_Object_GetClassObject(pltElem, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 8;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 6;
        goto out;
    }

    eBase->eSt.rend = xRend;
    rend = eBase->eSt.rend;

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik_Element_GetCurrentBGStyle(pltElem, &bgStyle))
        {
            eNum = 2;
            goto out;
        }
        if (bgStyle != NEUIK_BGSTYLE_TRANSPARENT)
        {
            /*----------------------------------------------------------------*/
            /* Create a MaskMap an mark off the trasnparent pixels.           */
            /*----------------------------------------------------------------*/
            if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
            {
                eNum = 7;
                goto out;
            }

            rl = eBase->eSt.rLoc;
            if (neuik_Window_FillTranspMaskFromLoc(
                    eBase->eSt.window, maskMap, rl.x, rl.y))
            {
                eNum = 10;
                goto out;
            }

            if (neuik_Element_RedrawBackground(pltElem, rlMod, maskMap))
            {
                eNum = 9;
                goto out;
            }
        }
    }
    rl = eBase->eSt.rLoc;

    /*------------------------------------------------------------------------*/
    /* Render and place the currently active stack element                    */
    /*------------------------------------------------------------------------*/
    eCfg = neuik_Element_GetConfig(plot->visual);
    if (eCfg == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Start with the default calculated element size                         */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetMinSize(plot->visual, &rs))
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check for and apply if necessary Horizontal and Veritcal fill          */
    /*------------------------------------------------------------------------*/
    if (eCfg->HFill)
    {
        /* This element is configured to fill space horizontally */
        rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
    }
    if (eCfg->VFill)
    {
        /* This element is configured to fill space vertically */
        rs.h = rSize->h - (eCfg->PadTop + eCfg->PadBottom);
    }

    /*------------------------------------------------------------------------*/
    /* Update the stored location before rendering the element. This is       */
    /* necessary as the location of this object will propagate to its         */
    /* child objects.                                                         */
    /*------------------------------------------------------------------------*/
    switch (eCfg->HJustify)
    {
        case NEUIK_HJUSTIFY_LEFT:
            rect.x = eCfg->PadLeft;
            break;
        case NEUIK_HJUSTIFY_DEFAULT:
        case NEUIK_HJUSTIFY_CENTER:
            rect.x = rSize->w/2 - (rs.w/2);
            break;
        case NEUIK_HJUSTIFY_RIGHT:
            rect.x = rSize->w - (rs.w + eCfg->PadRight);
            break;
    }

    switch (eCfg->VJustify)
    {
        case NEUIK_VJUSTIFY_TOP:
            rect.y = eCfg->PadTop;
            break;
        case NEUIK_VJUSTIFY_DEFAULT:
        case NEUIK_VJUSTIFY_CENTER:
            rect.y = (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 - (rs.h/2);
            break;
        case NEUIK_VJUSTIFY_BOTTOM:
            rect.y = rSize->h - (rs.h + eCfg->PadBottom);
            break;
    }

    rect.w = rs.w;
    rect.h = rs.h;
    rl.x = (eBase->eSt.rLoc).x + rect.x;
    rl.y = (eBase->eSt.rLoc).y + rect.y;
    rlRel.x = rect.x;
    rlRel.y = rect.y;
    neuik_Element_StoreSizeAndLocation(plot->visual, rs, rl, rlRel);
    /*------------------------------------------------------------------------*/
    /* The following render operation will result in a calculated size for    */
    /* plot drawing area.                                                     */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_Render(plot->visual, &rs, rlMod, rend, TRUE))
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* At this point, the size of the plot drawing area should be known. Now  */
    /* we can do the actual drawing to the draw area and then re-render the   */
    /* superclass plot element.                                               */
    /*------------------------------------------------------------------------*/
    dwg = plt->drawing_background;
    if (neuik_Element_GetSizeAndLocation(dwg, &dwg_rs, &dwg_loc))
    {
        eNum = 13;
        goto out;
    }

    /*========================================================================*/
    /* Draw in the y-axis/x-axis tic marks.                                   */
    /*========================================================================*/
    dwg = plt->drawing_ticmarks_plot_area;
    if (neuik_Element_GetSizeAndLocation(dwg, &dwg_rs, &dwg_loc))
    {
        eNum = 13;
        goto out;
    }
    ticPlotLoc = dwg_loc;

    /*------------------------------------------------------------------------*/
    /* Get the size and location information for the x_min ticmark label.     */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Container_GetFirstElement(plt->drawing_x_axis_ticmarks, &ticElem))
    {
        eNum = 11;
        goto out;
    }
    if (neuik_Element_GetSizeAndLocation(ticElem, &tic_rs, &tic_loc))
    {
        eNum = 13;
        goto out;
    }
    tic_xmin = (tic_loc.x - dwg_loc.x) + (tic_rs.w/2);

    /*------------------------------------------------------------------------*/
    /* Get the size and location information for the x_max ticmark label.     */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Container_GetLastElement(plt->drawing_x_axis_ticmarks, &ticElem))
    {
        eNum = 12;
        goto out;
    }
    if (neuik_Element_GetSizeAndLocation(ticElem, &tic_rs, &tic_loc))
    {
        eNum = 13;
        goto out;
    }
    tic_xmax = (tic_loc.x - dwg_loc.x) + (tic_rs.w/2);

    /*------------------------------------------------------------------------*/
    /* Get the size and location information for the y_max ticmark label.     */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Container_GetFirstElement(plt->drawing_y_axis_ticmarks, &ticElem))
    {
        eNum = 11;
        goto out;
    }
    if (neuik_Element_GetSizeAndLocation(ticElem, &tic_rs, &tic_loc))
    {
        eNum = 13;
        goto out;
    }
    tic_ymax = (tic_loc.y - dwg_loc.y) + (tic_rs.h/2);


    /*------------------------------------------------------------------------*/
    /* Get the size and location information for the y_min ticmark label.     */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Container_GetLastElement(plt->drawing_y_axis_ticmarks, &ticElem))
    {
        eNum = 12;
        goto out;
    }
    if (neuik_Element_GetSizeAndLocation(ticElem, &tic_rs, &tic_loc))
    {
        eNum = 13;
        goto out;
    }
    tic_ymin = (tic_loc.y - dwg_loc.y) + (tic_rs.h/2);


    /*------------------------------------------------------------------------*/
    /* Start off with a clean slate.                                          */
    /*------------------------------------------------------------------------*/
    NEUIK_Canvas_Clear(dwg);

    /*------------------------------------------------------------------------*/
    /* Draw the inner ticmarks/gridlines first; afterwards, the outer         */
    /* gridlines will be drawn.                                               */
    /*------------------------------------------------------------------------*/
    if (plt->yAxisCfg.nTicmarks > 2)
    {
        /*--------------------------------------------------------------------*/
        /* One or more internal ticmarks was specified for this axis.         */
        /*--------------------------------------------------------------------*/
        NEUIK_Canvas_SetDrawColor(dwg, 
            plt->yAxisCfg.colorGridline.r, 
            plt->yAxisCfg.colorGridline.g, 
            plt->yAxisCfg.colorGridline.b,
            plt->yAxisCfg.colorGridline.a); /* dwg ticmark label color */

        tic_y_offset = (double)(tic_ymax);
        tic_y_adj    = ((double)(tic_ymin - tic_ymax))/
            ((double)(plt->yAxisCfg.nTicmarks - 1));

        ticMarkPos = 2;
        for (ctr = 1; ctr < plt->yAxisCfg.nTicmarks - 1; ctr++)
        {
            tic_y_offset += tic_y_adj;
            tic_y_cl = (int)(tic_y_offset);

            if (plt->yAxisCfg.showGridlines)
            {
                /*------------------------------------------------------------*/
                /* Draw a full width y-axis gridline.                         */
                /*------------------------------------------------------------*/
                NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_y_cl);
                NEUIK_Canvas_DrawLine(dwg, tic_xmax,     tic_y_cl);
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Draw a small ticmark along the y-axis.                     */
                /*------------------------------------------------------------*/
                NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_y_cl);
                NEUIK_Canvas_DrawLine(dwg, (tic_xmin+6), tic_y_cl);
            }

            ticMarkPos += 2;
        }
    }

    if (plt->xAxisCfg.nTicmarks > 2)
    {
        /*--------------------------------------------------------------------*/
        /* One or more internal ticmarks was specified for this axis.         */
        /*--------------------------------------------------------------------*/
        NEUIK_Canvas_SetDrawColor(dwg, 
            plt->xAxisCfg.colorGridline.r, 
            plt->xAxisCfg.colorGridline.g, 
            plt->xAxisCfg.colorGridline.b,
            plt->xAxisCfg.colorGridline.a); /* dwg ticmark label color */

        tic_x_offset = (double)(tic_xmin);
        tic_x_adj    = ((double)(tic_xmax - tic_xmin))/
            ((double)(plt->xAxisCfg.nTicmarks - 1));

        for (ctr = 1; ctr < plt->xAxisCfg.nTicmarks - 1; ctr++)
        {
            tic_x_offset += tic_x_adj;
            tic_x_cl = (int)(tic_x_offset);

            if (plt->xAxisCfg.showGridlines)
            {
                /*------------------------------------------------------------*/
                /* Draw a full width y-axis gridline.                         */
                /*------------------------------------------------------------*/
                NEUIK_Canvas_MoveTo(dwg,   tic_x_cl, tic_ymax);
                NEUIK_Canvas_DrawLine(dwg, tic_x_cl, tic_ymin + 5);
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Draw a small ticmark along the y-axis.                     */
                /*------------------------------------------------------------*/
                NEUIK_Canvas_MoveTo(dwg,   tic_x_cl, tic_ymin - 6);
                NEUIK_Canvas_DrawLine(dwg, tic_x_cl, tic_ymin + 5);
            }

            ticMarkPos += 2;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Draw the outer (bounding) X/Y axis gridlines.                          */
    /*------------------------------------------------------------------------*/
    NEUIK_Canvas_SetDrawColor(dwg, 
        plt->colorGridline.r, 
        plt->colorGridline.g, 
        plt->colorGridline.b,
        plt->colorGridline.a); /* dwg ticmark label color */

    /* draw y-axis vert line */
    NEUIK_Canvas_MoveTo(dwg,   tic_xmin,   tic_ymin);
    NEUIK_Canvas_DrawLine(dwg, tic_xmin,   tic_ymax);
    NEUIK_Canvas_MoveTo(dwg,   tic_xmin+1, tic_ymin);
    NEUIK_Canvas_DrawLine(dwg, tic_xmin+1, tic_ymax);

    /* draw y-axis horizontal ticmark line (y-min ticmark)*/
    NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_ymin);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax,     tic_ymin);
    NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_ymin-1);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax,     tic_ymin-1);

    /* draw y-axis horizontal ticmark line (y-max ticmark)*/
    NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax,     tic_ymax);
    NEUIK_Canvas_MoveTo(dwg,   (tic_xmin-5), tic_ymax+1);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax,     tic_ymax+1);

    /* draw x-axis horizontal line */
    NEUIK_Canvas_MoveTo(dwg,   tic_xmin, tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax, tic_ymax);

    /* draw x-axis horizontal ticmark line (x-min ticmark)*/
    NEUIK_Canvas_MoveTo(dwg,   tic_xmin,   tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmin,   tic_ymin + 5);
    NEUIK_Canvas_MoveTo(dwg,   tic_xmin+1, tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmin+1, tic_ymin + 5);

    /* draw x-axis horizontal ticmark line (x-max ticmark)*/
    NEUIK_Canvas_MoveTo(dwg,   tic_xmax,   tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax,   tic_ymin + 5);
    NEUIK_Canvas_MoveTo(dwg,   tic_xmax-1, tic_ymax);
    NEUIK_Canvas_DrawLine(dwg, tic_xmax-1, tic_ymin + 5);

    /*------------------------------------------------------------------------*/
    /* Fill the background with white and draw the outside border             */
    /*------------------------------------------------------------------------*/
    dwg = plt->drawing_background;
    if (neuik_Element_GetSizeAndLocation(dwg, &dwg_rs, &dwg_loc))
    {
        eNum = 13;
        goto out;
    }

    NEUIK_Canvas_Clear(dwg);
    NEUIK_Canvas_SetDrawColor(dwg, 255, 255, 255, 255); /* dwg bg color */
    NEUIK_Canvas_Fill(dwg);
    NEUIK_Canvas_SetDrawColor(dwg, 150, 150, 150, 255); /* dwg border color */
    NEUIK_Canvas_MoveTo(dwg, 0, 0);
    NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), 0);
    NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), (dwg_rs.h-1));
    NEUIK_Canvas_DrawLine(dwg, 0, (dwg_rs.h-1));
    NEUIK_Canvas_DrawLine(dwg, 0, 0);

    /*------------------------------------------------------------------------*/
    /* Now it is time for the contained PlotData sets to be rendered.         */
    /*------------------------------------------------------------------------*/
    dwg = plt->drawing_plotted_data;
    if (neuik_Element_GetSizeAndLocation(dwg, &dwg_rs, &dwg_loc))
    {
        eNum = 13;
        goto out;
    }

    NEUIK_Canvas_Clear(dwg);

    pltOffsetX = (ticPlotLoc.x - dwg_loc.x);
    pltOffsetY = (ticPlotLoc.y - dwg_loc.y);

    for (uCtr = 0; uCtr < plot->n_used; uCtr++)
    {
        data = (NEUIK_PlotData*)(plot->data_sets[uCtr]);
        if (!data->boundsSet) continue;

        dataCfg = &(plot->data_configs[uCtr]);

        /*--------------------------------------------------------------------*/
        /* Set the drawing line color.                                        */
        /*--------------------------------------------------------------------*/
        if (!dataCfg->lineColorSpecified)
        {
            /*----------------------------------------------------------------*/
            /* No color was specified; use one of the default colors.         */
            /*----------------------------------------------------------------*/
            if (uCtr < 11)
            {
                NEUIK_Canvas_SetDrawColor(dwg, 
                    autoColors[uCtr].r, 
                    autoColors[uCtr].g, 
                    autoColors[uCtr].b, 
                    autoColors[uCtr].a); /* dwg line color */
            }
            else
            {
                NEUIK_Canvas_SetDrawColor(dwg, 
                    autoColors[11].r, 
                    autoColors[11].g, 
                    autoColors[11].b, 
                    autoColors[11].a); /* dwg line color */
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* A specific color was specified; use that.                      */
            /*----------------------------------------------------------------*/
            NEUIK_Canvas_SetDrawColor(dwg, 
                dataCfg->lineColor.r, 
                dataCfg->lineColor.g, 
                dataCfg->lineColor.b, 
                dataCfg->lineColor.a); /* dwg line color */
        }

        if (maskMap != NULL)
        {
            neuik_Object_Free(maskMap);
            maskMap = NULL;
        }

        maskW = dwg_rs.w;
        maskH = dwg_rs.h; /* yMax value is at the top of the plot */

        lnThickness = 2;

        ticZoneW = tic_xmax - tic_xmin;
        ticZoneH = tic_ymin - tic_ymax; /* yMax value is at the top of the plot */

        if (neuik_Plot2D_RenderSimpleLineToMask(
            plt, data, dataCfg, lnThickness, maskW, maskH, 
            ticZoneW, ticZoneH, tic_xmin, tic_ymax, &maskMap))
        {
            eNum = 14;
            goto out;
        }

        for (ctr = 0; ctr < maskW; ctr++)
        {
            if (neuik_MaskMap_GetUnmaskedRegionsOnVLine(
                maskMap, ctr, &maskRegions, &regionY0, &regionYf))
            {
                eNum = 15;
                goto out;
            }

            for (maskCtr = 0; maskCtr < maskRegions; maskCtr++)
            {
                NEUIK_Canvas_MoveTo(dwg, 
                    pltOffsetX + ctr, pltOffsetY + regionY0[maskCtr]);

                if (regionY0[maskCtr] != regionYf[maskCtr])
                {
                    /*--------------------------------------------------------*/
                    /* This region is two or more points. Draw a line.        */
                    /*--------------------------------------------------------*/
                    {
                        #pragma message("[WORKAROUND]: Fix line drawing graphical glitch")
                        int tempCtr = 0;
                        for (tempCtr = regionY0[maskCtr]; tempCtr <= regionYf[maskCtr]; tempCtr++)
                        {
                            NEUIK_Canvas_MoveTo(dwg, 
                                pltOffsetX + ctr, pltOffsetY + tempCtr);
                            NEUIK_Canvas_DrawPoint(dwg);
                        }
                    }
                    // NEUIK_Canvas_DrawLine(dwg, 
                    //  tic_xmin + ctr, tic_ymax + regionYf[maskCtr]);
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* This region is but a single point.                     */
                    /*--------------------------------------------------------*/
                    NEUIK_Canvas_DrawPoint(dwg);
                }
            }
        }
    }


    /*------------------------------------------------------------------------*/
    /* Finally, have the entire visual redraw itself. It will only redraw the */
    /* drawing portion and with the correct sizing.                           */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_NeedsRedraw(plot->visual))
    {
        if (neuik_Element_Render(plot->visual, &rs, rlMod, rend, mock))
        {
            eNum = 5;
            goto out;
        }
    }
out:
    if (eBase != NULL)
    {
        if (!mock) eBase->eSt.doRedraw = 0;
    }
    if (maskMap != NULL) neuik_Object_Free(maskMap);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_UpdateAxesRanges
 *
 *  Description:   Update the stored X/Y-Axis ranges.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Plot2D_UpdateAxesRanges(
    NEUIK_Plot2D * plot2d)
{
    int              ctr        = 0;
    int              eNum       = 0; /* which error to report (if any) */
    int              boundsSet  = FALSE;
    unsigned int     uCtr       = 0;
    double           xMin       = 0.0;
    double           xMax       = 0.0;
    double           yMin       = 0.0;
    double           yMax       = 0.0;
    double           xMin64     = 0.0;
    double           xMax64     = 0.0;
    double           yMin64     = 0.0;
    double           yMax64     = 0.0;
    double           xRangeMin  = 0.0;
    double           xRangeMax  = 0.0;
    double           yRangeMin  = 0.0;
    double           yRangeMax  = 0.0;
    double           xAxisRange = 0.0;
    double           yAxisRange = 0.0;
    double           ticSize    = 0.0;
    double           ticVal     = 0.0;
    NEUIK_Label    * newTicLbl  = NULL;
    NEUIK_Fill     * newFill    = NULL;
    NEUIK_Plot     * plot       = NULL;
    NEUIK_PlotData * data       = NULL;
    char             ticMarkLbl[100]; 
    static char      funcName[] = "neuik_Plot2D_UpdateAxesRanges";
    static char    * errMsgs[]  = {"", // [0] no error
        "Argument `plot2d` is not of Plot2D class.",                       // [1]
        "Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Unsupported `precision` used within included PlotData.",          // [3]
        "Failure in function `NEUIK_Container_DeleteElements()`.",         // [4]
        "Failure in function `NEUIK_MakeLabel()`.",                        // [5]
        "Failure in function `NEUIK_Container_AddElement()`.",             // [6]
        "Failure in function `NEUIK_NewVFill()`.",                         // [7]
        "Failure in function `NEUIK_Element_Configure()`.",                // [8]
        "Failure in function `NEUIK_NewHFill()`.",                         // [9]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
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

    /*------------------------------------------------------------------------*/
    /* Determine the maximum X-Y range of values from all data sets.          */
    /*------------------------------------------------------------------------*/
    for (uCtr = 0; uCtr < plot->n_used; uCtr++)
    {
        data = (NEUIK_PlotData*)(plot->data_sets[uCtr]);
        if (!data->boundsSet) continue;

        /*--------------------------------------------------------------------*/
        /* Extract the bounds for a particular PlotData set as doubles.       */
        /*--------------------------------------------------------------------*/
        switch (data->precision)
        {
            case 32:
                xMin64 = (double)(data->bounds_32.x_min);
                xMax64 = (double)(data->bounds_32.x_max);
                yMin64 = (double)(data->bounds_32.y_min);
                yMax64 = (double)(data->bounds_32.y_max);
                break;
            case 64:
                xMin64 = data->bounds_64.x_min;
                xMax64 = data->bounds_64.x_max;
                yMin64 = data->bounds_64.y_min;
                yMax64 = data->bounds_64.y_max;
                break;
            default:
                /*------------------------------------------------------------*/
                /* Unsupported floating point precision.                      */
                /*------------------------------------------------------------*/
                eNum = 3;
                goto out;
                break;
        }

        /*--------------------------------------------------------------------*/
        /* Update the overall X-Y ranges of values from all data sets.        */
        /*--------------------------------------------------------------------*/
        if (!boundsSet)
        {
            xMin = xMin64;
            xMax = xMax64;
            yMin = yMin64;
            yMax = yMax64;
        }
        else
        {
            if (xMin64 < xMin)
            {
                xMin = xMin64;
            }
            if (xMax64 > xMax)
            {
                xMax = xMax64;
            }
            if (yMin64 < yMin)
            {
                yMin = yMin64;
            }
            if (yMax64 > yMax)
            {
                yMax = yMax64;
            }
        }

        boundsSet = TRUE;
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the X bounds to use for the overall plot.                    */
    /*------------------------------------------------------------------------*/
    xAxisRange = xMax - xMin;

    if (xAxisRange == 0.0)
    {
        xRangeMin = xMin - 1;
        xRangeMax = xMin + 1;
    }
    else if (xAxisRange <= 0.5)
    {
        xRangeMin = floor(xMin);
        xRangeMax = xRangeMin + 1;
    }
    else if (xAxisRange <= 1.0)
    {
        xRangeMin = floor(xMin);
        xRangeMax = xRangeMin + 2;
    }
    else if (xAxisRange <= 10.0)
    {
        xRangeMin = xMin;
        // xRangeMin = floor(xMin - fmod(xMin, 10));
        xRangeMax = xRangeMin + 12;
        if (xMax <= xRangeMin + 10)
        {
            xRangeMax = xRangeMin + 10;
        }
    }
    else if (xAxisRange <= 50.0)
    {
        xRangeMin = xMin;
        // xRangeMin = floor(xMin - fmod(xMin, 10));
        xRangeMax = xRangeMin + 60;
        if (xMax <= xRangeMin + 50)
        {
            xRangeMax = xRangeMin + 50;
        }
    }
    else if (xAxisRange <= 100.0)
    {
        xRangeMin = floor(xMin - fmod(xMin, 100));
        xRangeMax = xRangeMin + 120;
        if (xMax <= xRangeMin + 100)
        {
            xRangeMax = xRangeMin + 100;
        }
    }
    else
    {
        printf("neuik_Plot2D_UpdateAxesRanges(): xAxisRange unhandled!!!\n");
        #pragma message ("[TODO] Improve calculation of xAxisRange")
    }
    if (plot->x_range_cfg == NEUIK_PLOTRANGECONFIG_AUTO)
    {
        plot->x_range_min = xRangeMin;
        plot->x_range_max = xRangeMax;
    }
    else if (plot->x_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED)
    {
        xRangeMin = plot->x_range_min;
        xRangeMax = plot->x_range_max;
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the Y bounds to use for the overall plot.                    */
    /*------------------------------------------------------------------------*/
    yAxisRange = yMax - yMin;

    if (yAxisRange == 0.0)
    {
        yRangeMin = yMin - 1;
        yRangeMax = yMin + 1;
    }
    else if (yAxisRange <= 0.5)
    {
        yRangeMin = floor(yMin);
        yRangeMax = yRangeMin + 1;
    }
    else if (yAxisRange < 1.0)
    {
        yRangeMin = floor(yMin);
        yRangeMax = yRangeMin + 2;
    }
    else if (yAxisRange <= 10.0)
    {
        yRangeMin = yMin;
        // yRangeMin = floor(yMin - fmod(yMin, 10));
        yRangeMax = yRangeMin + 12;
        if (yMax <= yRangeMin + 10)
        {
            yRangeMax = yRangeMin + 10;
        }
    }
    else if (yAxisRange <= 100.0)
    {
        yRangeMin = floor(yMin - fmod(yMin, 100));
        yRangeMax = yRangeMin + 120;
        if (yMax <= yRangeMin + 100)
        {
            yRangeMax = yRangeMin + 100;
        }
    }
    else
    {
        printf("neuik_Plot2D_UpdateAxesRanges(): yAxisRange unhandled!!!\n");
        #pragma message ("[TODO] Improve calculation of yAxisRange")
    }
    if (plot->y_range_cfg == NEUIK_PLOTRANGECONFIG_AUTO)
    {
        plot->y_range_min = yRangeMin;
        plot->y_range_max = yRangeMax;
    }
    else if (plot->y_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED)
    {
        yRangeMin = plot->y_range_min;
        yRangeMax = plot->y_range_max;
    }

    /*------------------------------------------------------------------------*/
    /* Remove all existing X/Y Axis ticmark labels before adding new ones.    */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Container_DeleteElements(plot2d->drawing_y_axis_ticmarks))
    {
        eNum = 4;
        goto out;
    }
    if (NEUIK_Container_DeleteElements(plot2d->drawing_x_axis_ticmarks))
    {
        eNum = 4;
        goto out;
    }

    /*========================================================================*/
    /* Generate the Y-Axis Ticmark Labels.                                    */
    /*========================================================================*/
    /* Create and add the Y Axis maximum value ticmark label.                 */
    /*------------------------------------------------------------------------*/
    if (plot2d->yAxisCfg.showTicLabels)
    {
        sprintf(ticMarkLbl, "%g", plot->y_range_max);
        if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
        {
            eNum = 5;
            goto out;
        }
        if (NEUIK_Element_Configure(newTicLbl, "HFill", "HJustify=right", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_y_axis_ticmarks, newTicLbl))
        {
            eNum = 6;
            goto out;
        }
    }
    else
    {
        if (NEUIK_NewHFill(&newFill))
        {
            eNum = 9;
            goto out;
        }
        if (NEUIK_Element_Configure(newFill, "PadAll=2", "PadRight=0", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_y_axis_ticmarks, newFill))
        {
            eNum = 6;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Create and add a Y Axis ticmark label spacer.                          */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewVFill(&newFill))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Container_AddElement(plot2d->drawing_y_axis_ticmarks, newFill))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create and add the internal Y Axis ticmark labels and spacers.         */
    /*------------------------------------------------------------------------*/
    if (plot2d->yAxisCfg.showTicLabels)
    {
        if (plot2d->yAxisCfg.nTicmarks > 2)
        {
            ticVal = plot->y_range_max;
            ticSize = (yRangeMax - yRangeMin) / 
                ((double)(plot2d->yAxisCfg.nTicmarks - 1));
            for (ctr = 1; ctr < plot2d->yAxisCfg.nTicmarks - 1; ctr++)
            {
                ticVal -= ticSize;
                sprintf(ticMarkLbl, "%g", ticVal);
                if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
                {
                    eNum = 5;
                    goto out;
                }
                if (NEUIK_Element_Configure(
                    newTicLbl, "HFill", "HJustify=right", NULL))
                {
                    eNum = 8;
                    goto out;
                }
                if (NEUIK_Container_AddElement(
                    plot2d->drawing_y_axis_ticmarks, newTicLbl))
                {
                    eNum = 6;
                    goto out;
                }

                /*------------------------------------------------------------*/
                /* Create and add a Y Axis ticmark label spacer.              */
                /*------------------------------------------------------------*/
                if (NEUIK_NewVFill(&newFill))
                {
                    eNum = 7;
                    goto out;
                }
                if (NEUIK_Container_AddElement(
                    plot2d->drawing_y_axis_ticmarks, newFill))
                {
                    eNum = 6;
                    goto out;
                }
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Create and add the Y Axis minimum value ticmark label.                 */
    /*------------------------------------------------------------------------*/
    if (plot2d->yAxisCfg.showTicLabels)
    {
        sprintf(ticMarkLbl, "%g", plot->y_range_min);
        if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
        {
            eNum = 5;
            goto out;
        }
        if (NEUIK_Element_Configure(newTicLbl, "HFill", "HJustify=right", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_y_axis_ticmarks, newTicLbl))
        {
            eNum = 6;
            goto out;
        }
    }
    else
    {
        if (NEUIK_NewHFill(&newFill))
        {
            eNum = 9;
            goto out;
        }
        if (NEUIK_Element_Configure(newFill, "PadAll=2", "PadRight=0", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_y_axis_ticmarks, newFill))
        {
            eNum = 6;
            goto out;
        }
    }

    /*========================================================================*/
    /* Generate the X-Axis Ticmark Labels.                                    */
    /*========================================================================*/
    /* Create and add the X-Axis minimum value ticmark label.                 */
    /*------------------------------------------------------------------------*/
    if (plot2d->xAxisCfg.showTicLabels)
    {
        sprintf(ticMarkLbl, "%g", plot->x_range_min);
        if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
        {
            eNum = 5;
            goto out;
        }
        if (NEUIK_Element_Configure(newTicLbl, "VFill", "VJustify=top", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_x_axis_ticmarks, newTicLbl))
        {
            eNum = 6;
            goto out;
        }
    }
    else
    {
        if (NEUIK_NewVFill(&newFill))
        {
            eNum = 7;
            goto out;
        }
        if (NEUIK_Element_Configure(newFill, "PadAll=2", "PadTop=0", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_x_axis_ticmarks, newFill))
        {
            eNum = 6;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Create and add a X-Axis ticmark label spacer.                          */
    /*------------------------------------------------------------------------*/
    if (NEUIK_NewHFill(&newFill))
    {
        eNum = 9;
        goto out;
    }
    if (NEUIK_Container_AddElement(plot2d->drawing_x_axis_ticmarks, newFill))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create and add the internal X Axis ticmark labels and spacers.         */
    /*------------------------------------------------------------------------*/
    if (plot2d->xAxisCfg.showTicLabels)
    {
        if (plot2d->xAxisCfg.nTicmarks > 2)
        {
            ticVal = plot->x_range_min;
            ticSize = (xRangeMax - xRangeMin) /
                ((double)(plot2d->xAxisCfg.nTicmarks - 1));
            for (ctr = 1; ctr < plot2d->xAxisCfg.nTicmarks - 1; ctr++)
            {
                ticVal += ticSize;
                sprintf(ticMarkLbl, "%g", ticVal);
                if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
                {
                    eNum = 5;
                    goto out;
                }
                if (NEUIK_Element_Configure(
                    newTicLbl, "VFill", "VJustify=top", NULL))
                {
                    eNum = 8;
                    goto out;
                }
                if (NEUIK_Container_AddElement(
                    plot2d->drawing_x_axis_ticmarks, newTicLbl))
                {
                    eNum = 6;
                    goto out;
                }

                /*------------------------------------------------------------*/
                /* Create and add a Y Axis ticmark label spacer.              */
                /*------------------------------------------------------------*/
                if (NEUIK_NewHFill(&newFill))
                {
                    eNum = 9;
                    goto out;
                }
                if (NEUIK_Container_AddElement(
                    plot2d->drawing_x_axis_ticmarks, newFill))
                {
                    eNum = 6;
                    goto out;
                }
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Create and add the X-Axis maximum value ticmark label.                 */
    /*------------------------------------------------------------------------*/
    if (plot2d->xAxisCfg.showTicLabels)
    {
        sprintf(ticMarkLbl, "%g", plot->x_range_max);
        if (NEUIK_MakeLabel(&newTicLbl, ticMarkLbl))
        {
            eNum = 5;
            goto out;
        }
        if (NEUIK_Element_Configure(newTicLbl, "VFill", "VJustify=top", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_x_axis_ticmarks, newTicLbl))
        {
            eNum = 6;
            goto out;
        }
    }
    else
    {
        if (NEUIK_NewVFill(&newFill))
        {
            eNum = 7;
            goto out;
        }
        if (NEUIK_Element_Configure(newFill, "PadAll=2", "PadTop=0", NULL))
        {
            eNum = 8;
            goto out;
        }
        if (NEUIK_Container_AddElement(
            plot2d->drawing_x_axis_ticmarks, newFill))
        {
            eNum = 6;
            goto out;
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot2D_AddPlotData
 *
 *  Description:   Add the specified PlotData to this plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot2D_AddPlotData(
    NEUIK_Plot2D   * plot2d,
    NEUIK_PlotData * data,
    const char     * label)
{
    int                   eNum = 0; /* which error to report (if any) */
    int                   sLen = 0;
    unsigned int          uCtr = 0;
    neuik_PlotDataConfig * dataCfg    = NULL;
    NEUIK_Plot           * plot       = NULL;
    RenderSize             rSize;
    RenderLoc              rLoc;
    static NEUIK_Color     color0 = {0, 0, 0, 0};
    static char            funcName[] = "NEUIK_Plot2D_AddPlotData";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `plot2d` is not of Plot2D class.",                       // [1]
        "Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Argument `data` is not of PlotData class.",                       // [3]
        "Failure to reallocate memory.",                                   // [4]
        "PlotData `uniqueName` already in use within this Plot.",          // [5]
        "Failure to allocate memory.",                                     // [6]
        "Failure in `neuik_Plot2D_UpdateAxesRanges()`.",                   // [7]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                // [8]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
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

    if (!neuik_Object_IsClass(data, neuik__Class_PlotData))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check to see if the DataSet slots need to be reallocated.              */
    /*------------------------------------------------------------------------*/
    if (plot->n_used >= plot->n_allocated)
    {
        /*--------------------------------------------------------------------*/
        /* More space will be needed for tracking DataSets; reallocate.       */
        /*--------------------------------------------------------------------*/
        plot->data_sets = realloc(plot->data_sets, 
            (plot->n_allocated+5)*sizeof(NEUIK_Object));
        if (plot->data_sets == NULL)
        {
            eNum = 4;
            goto out;
        }
        plot->data_configs = realloc(plot->data_configs,
            (plot->n_allocated+5)*sizeof(neuik_PlotDataConfig));
        if (plot->data_configs == NULL)
        {
            eNum = 4;
            goto out;
        }
        plot->n_allocated += 5;

        for (uCtr = plot->n_used; uCtr < plot->n_allocated; uCtr++)
        {
            dataCfg = &(plot->data_configs[uCtr]);
            dataCfg->uniqueName = NULL;
            dataCfg->label      = NULL;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Make sure the uniqueName for this PlotData isn't already in use within */
    /* this Plot.                                                             */
    /*------------------------------------------------------------------------*/
    for (uCtr = 0; uCtr < plot->n_used; uCtr++)
    {
        if (!strcmp(plot->data_configs[uCtr].uniqueName, data->uniqueName))
        {
            eNum = 5;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Add the PlotData to the first available slot.                          */
    /*------------------------------------------------------------------------*/
    plot->data_sets[plot->n_used] = data;

    dataCfg = &(plot->data_configs[plot->n_used]);
    if (dataCfg->uniqueName != NULL)
    {
        free(dataCfg->uniqueName);
        dataCfg->uniqueName = NULL;
    }
    if (dataCfg->label != NULL)
    {
        free(dataCfg->label);
        dataCfg->label = NULL;
    }

    sLen = strlen(data->uniqueName);
    dataCfg->uniqueName = malloc((1+sLen)*sizeof(char));
    if (dataCfg->uniqueName == NULL)
    {
        eNum = 6;
        goto out;
    }
    strcpy(dataCfg->uniqueName, data->uniqueName);

    sLen = strlen(label);
    dataCfg->label = malloc((1+sLen)*sizeof(char));
    if (dataCfg->label == NULL)
    {
        eNum = 6;
        goto out;
    }
    strcpy(dataCfg->label, label);
    plot->n_used++;

    /*------------------------------------------------------------------------*/
    /* Set standard default values for the PlotDataConfig.                    */
    /*------------------------------------------------------------------------*/
    dataCfg->lineThickness      = 1.0;
    dataCfg->lineColorSpecified = FALSE;
    dataCfg->lineColor          = color0;

    if (neuik_Plot2D_UpdateAxesRanges(plot2d))
    {
        eNum = 7;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Request a redraw of the old size at old location. This will make sure  */
    /* the content is erased (in case the new content is smaller).            */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(plot2d, &rSize, &rLoc))
    {
        eNum = 8;
        goto out;
    }
    neuik_Element_RequestRedraw(plot2d, rLoc, rSize);

    /*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP*/
    #pragma message("There should be double-linkage between Plot2D and PlotData.")
    /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
    /* This is so changes in plot data can trigger redraws of the Plot2D and  */
    /* also, so removal of the PlotData from curve, can remove the linkage    */
    /* from the PlotData side.                                                */
    /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot2D_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_Plot2D_Configure(
    NEUIK_Plot2D * plot2d,
    const char   * set0,
    ...)
{
    int                  ns; /* number of items from sscanf */
    int                  ctr;
    int                  nCtr;
    int                  eNum      = 0; /* which error to report (if any) */
    int                  doRedraw  = FALSE;
    int                  isBool    = FALSE;
    int                  boolVal   = FALSE;
    int                  typeMixup;
    int                  valInt    = 0;
    double               floatMin  = 0.0;
    double               floatMax  = 0.0;
    char                 buf[4096];
    RenderSize           rSize;
    RenderLoc            rLoc;
    va_list              args;
    char               * strPtr    = NULL;
    char               * name      = NULL;
    char               * value     = NULL;
    const char         * set       = NULL;
    NEUIK_Plot         * plot      = NULL;
    NEUIK_Color          clr;
    /*------------------------------------------------------------------------*/
    /* If a `name=value` string with an unsupported name is found, check to   */
    /* see if a boolName was mistakenly used instead.                         */
    /*------------------------------------------------------------------------*/
    static char * boolNames[] = {
        "xAxisGridlines",
        "yAxisGridlines",
        "xAxisTicLabels",
        "yAxisTicLabels",
        NULL,
    };
    /*------------------------------------------------------------------------*/
    /* If a boolName string with an unsupported name is found, check to see   */
    /* if a supported nameValue type was mistakenly used instead.             */
    /*------------------------------------------------------------------------*/
    static char * valueNames[] = {
        "GridlineColor",
        "xAxisGridlineColor",
        "yAxisGridlineColor",
        "xAxisRange",
        "yAxisRange",
        "xAxisNumTics",
        "yAxisNumTics",
        NULL,
    };
    static char   funcName[] = "NEUIK_Plot2D_Configure";
    static char * errMsgs[]  = {"", // [ 0] no error
        "Argument `plot2d` does not implement Label class.",                 // [ 1]
        "`name=value` string is too long.",                                  // [ 2]
        "Invalid `name=value` string.",                                      // [ 3]
        "ValueType name used as BoolType, skipping.",                        // [ 4]
        "BoolType name unknown, skipping.",                                  // [ 5]
        "NamedSet.name is NULL, skipping..",                                 // [ 6]
        "NamedSet.name is blank, skipping..",                                // [ 7]
        "GridlineColor value invalid; should be comma separated RGBA.",      // [ 8]
        "GridlineColor value invalid; RGBA value range is 0-255.",           // [ 9]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                  // [10]
        "BoolType name used as ValueType, skipping.",                        // [11]
        "NamedSet.name type unknown, skipping.",                             // [12]
        "xAxisNumTics value invalid; must be an integer value.",             // [13]
        "xAxisNumTics value invalid; Valid integer values are -1 or >=0.",   // [14]
        "yAxisNumTics value invalid; must be an integer value.",             // [15]
        "yAxisNumTics value invalid; Valid integer values are -1 or >=0.",   // [16]
        "xAxisGridlineColor value invalid; should be comma separated RGBA.", // [17]
        "xAxisGridlineColor value invalid; RGBA value range is 0-255.",      // [18]
        "yAxisGridlineColor value invalid; should be comma separated RGBA.", // [19]
        "yAxisGridlineColor value invalid; RGBA value range is 0-255.",      // [20]
        "xAxisRange value invalid; must be comma separated float values.",   // [21]
        "xAxisRange value invalid; float values cannot be identical.",       // [22]
        "xAxisRange value invalid; `xMin` must be less than `xMax`.",        // [23]
        "Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.",   // [24]
        "yAxisRange value invalid; must be comma separated float values.",   // [25]
        "yAxisRange value invalid; float values cannot be identical.",       // [26]
        "yAxisRange value invalid; `yMin` must be less than `yMax`.",        // [27]
    };

    if (!neuik_Object_IsClass(plot2d, neuik__Class_Plot2D))
    {
        eNum = 1;
        goto out;
    }

    if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
    {
        eNum = 24;
        goto out;
    }

    set = set0;

    va_start(args, set0);

    for (ctr = 0;; ctr++)
    {
        if (ctr > 0)
        {
            /* before starting */
            set = va_arg(args, const char *);
        }

        isBool = FALSE;
        name   = NULL;
        value  = NULL;

        if (set == NULL) break;

        if (strlen(set) > 4095)
        {
            NEUIK_RaiseError(funcName, errMsgs[2]);
            set = va_arg(args, const char *);
            continue;
        }
        else
        {
            strcpy(buf, set);
            /* Find the equals and set it to '\0' */
            strPtr = strchr(buf, '=');
            if (strPtr == NULL)
            {
                /*------------------------------------------------------------*/
                /* Bool type configuration (or a mistake)                     */
                /*------------------------------------------------------------*/
                if (buf[0] == 0)
                {
                    NEUIK_RaiseError(funcName, errMsgs[3]);
                }

                isBool  = TRUE;
                boolVal = TRUE;
                name    = buf;
                if (buf[0] == '!')
                {
                    boolVal = FALSE;
                    name    = buf + 1;
                }
                if (boolVal)
                {
                    /*--------------------------------------------------------*/
                    /* Do nothing; this is to resolve an unused var warning.  */
                    /*--------------------------------------------------------*/
                }
            }
            else
            {
                *strPtr = 0;
                strPtr++;
                if (*strPtr == 0)
                {
                    /* `name=value` string is missing a value */
                    NEUIK_RaiseError(funcName, errMsgs[3]);
                    set = va_arg(args, const char *);
                    continue;
                }
                name  = buf;
                value = strPtr;
            }
        }

        if (isBool)
        {
            /*----------------------------------------------------------------*/
            /* Check for boolean parameter setting.                           */
            /*----------------------------------------------------------------*/
            if (!strcmp("xAxisGridlines", name))
            {
                if (plot2d->xAxisCfg.showGridlines == boolVal) continue;

                /* else: The previous setting was changed */
                plot2d->xAxisCfg.showGridlines = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("yAxisGridlines", name))
            {
                if (plot2d->yAxisCfg.showGridlines == boolVal) continue;

                /* else: The previous setting was changed */
                plot2d->yAxisCfg.showGridlines = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("xAxisTicLabels", name))
            {
                if (plot2d->xAxisCfg.showTicLabels == boolVal) continue;

                /* else: The previous setting was changed */
                plot2d->xAxisCfg.showTicLabels = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("yAxisTicLabels", name))
            {
                if (plot2d->yAxisCfg.showTicLabels == boolVal) continue;

                /* else: The previous setting was changed */
                plot2d->yAxisCfg.showTicLabels = boolVal;
                doRedraw = 1;
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Bool parameter not found; may be mixup or mistake.         */
                /*------------------------------------------------------------*/
                typeMixup = FALSE;
                for (nCtr = 0;; nCtr++)
                {
                    if (valueNames[nCtr] == NULL) break;

                    if (!strcmp(valueNames[nCtr], name))
                    {
                        typeMixup = TRUE;
                        break;
                    }
                }

                if (typeMixup)
                {
                    /* A value type was mistakenly used as a bool type */
                    NEUIK_RaiseError(funcName, errMsgs[4]);
                }
                else
                {
                    /* An unsupported name was used as a bool type */
                    NEUIK_RaiseError(funcName, errMsgs[5]);
                }
            }
        }
        else
        {
            if (name == NULL)
            {
                NEUIK_RaiseError(funcName, errMsgs[6]);
            }
            else if (name[0] == 0)
            {
                NEUIK_RaiseError(funcName, errMsgs[7]);
            }
            else if (!strcmp("GridlineColor", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                    continue;
                }

                ns = sscanf(value, "%d,%d,%d,%d", &clr.r, &clr.g, &clr.b, &clr.a);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 4)
                #else
                    if (ns < 4)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                    continue;
                }

                if (clr.r < 0 || clr.r > 255 ||
                    clr.g < 0 || clr.g > 255 ||
                    clr.b < 0 || clr.b > 255 ||
                    clr.a < 0 || clr.a > 255)
                {
                    NEUIK_RaiseError(funcName, errMsgs[9]);
                    continue;
                }
                if (plot2d->colorGridline.r == clr.r &&
                    plot2d->colorGridline.g == clr.g &&
                    plot2d->colorGridline.b == clr.b &&
                    plot2d->colorGridline.a == clr.a) continue;

                /* else: The previous setting was changed */
                plot2d->colorGridline = clr;
                doRedraw = TRUE;
            }
            else if (!strcmp("xAxisGridlineColor", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[17]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[17]);
                    continue;
                }

                ns = sscanf(value, "%d,%d,%d,%d", &clr.r, &clr.g, &clr.b, &clr.a);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 4)
                #else
                    if (ns < 4)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[17]);
                    continue;
                }

                if (clr.r < 0 || clr.r > 255 ||
                    clr.g < 0 || clr.g > 255 ||
                    clr.b < 0 || clr.b > 255 ||
                    clr.a < 0 || clr.a > 255)
                {
                    NEUIK_RaiseError(funcName, errMsgs[18]);
                    continue;
                }
                if (plot2d->xAxisCfg.colorGridline.r == clr.r &&
                    plot2d->xAxisCfg.colorGridline.g == clr.g &&
                    plot2d->xAxisCfg.colorGridline.b == clr.b &&
                    plot2d->xAxisCfg.colorGridline.a == clr.a) continue;

                /* else: The previous setting was changed */
                plot2d->xAxisCfg.colorGridline = clr;
                doRedraw = TRUE;
            }
            else if (!strcmp("yAxisGridlineColor", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[19]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[19]);
                    continue;
                }

                ns = sscanf(value, "%d,%d,%d,%d", &clr.r, &clr.g, &clr.b, &clr.a);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 4)
                #else
                    if (ns < 4)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[19]);
                    continue;
                }

                if (clr.r < 0 || clr.r > 255 ||
                    clr.g < 0 || clr.g > 255 ||
                    clr.b < 0 || clr.b > 255 ||
                    clr.a < 0 || clr.a > 255)
                {
                    NEUIK_RaiseError(funcName, errMsgs[20]);
                    continue;
                }
                if (plot2d->yAxisCfg.colorGridline.r == clr.r &&
                    plot2d->yAxisCfg.colorGridline.g == clr.g &&
                    plot2d->yAxisCfg.colorGridline.b == clr.b &&
                    plot2d->yAxisCfg.colorGridline.a == clr.a) continue;

                /* else: The previous setting was changed */
                plot2d->yAxisCfg.colorGridline = clr;
                doRedraw = TRUE;
            }
            else if (!strcmp("xAxisRange", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[21]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[21]);
                    continue;
                }

                ns = sscanf(value, "%lf,%lf", &floatMin, &floatMax);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 2)
                #else
                    if (ns < 2)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[21]);
                    continue;
                }

                if (floatMin == floatMax)
                {
                    NEUIK_RaiseError(funcName, errMsgs[22]);
                    continue;
                }
                if (floatMin > floatMax)
                {
                    NEUIK_RaiseError(funcName, errMsgs[23]);
                    continue;
                }

                if (plot->x_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED &&
                    plot->x_range_min == floatMin &&
                    plot->x_range_max == floatMax) continue;

                /* else: The previous setting was changed */
                plot->x_range_cfg = NEUIK_PLOTRANGECONFIG_SPECIFIED;
                plot->x_range_min = floatMin;
                plot->x_range_max = floatMax;

                doRedraw = TRUE;
            }
            else if (!strcmp("yAxisRange", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[25]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[25]);
                    continue;
                }

                ns = sscanf(value, "%lf,%lf", &floatMin, &floatMax);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 2)
                #else
                    if (ns < 2)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[25]);
                    continue;
                }

                if (floatMin == floatMax)
                {
                    NEUIK_RaiseError(funcName, errMsgs[26]);
                    continue;
                }
                if (floatMin > floatMax)
                {
                    NEUIK_RaiseError(funcName, errMsgs[27]);
                    continue;
                }

                if (plot->y_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED &&
                    plot->y_range_min == floatMin &&
                    plot->y_range_max == floatMax) continue;

                /* else: The previous setting was changed */
                plot->y_range_cfg = NEUIK_PLOTRANGECONFIG_SPECIFIED;
                plot->y_range_min = floatMin;
                plot->y_range_max = floatMax;

                doRedraw = TRUE;
            }
            else if (!strcmp("xAxisNumTics", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[13]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[13]);
                    continue;
                }

                ns = sscanf(value, "%d", &valInt);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 1)
                #else
                    if (ns < 1)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[13]);
                    continue;
                }

                if (valInt < -1)
                {
                    NEUIK_RaiseError(funcName, errMsgs[14]);
                    continue;
                }
                if (valInt != -1)
                {
                    valInt += 2;
                }

                if (plot2d->xAxisCfg.nTicmarks == valInt) continue;

                /* else: The previous setting was changed */
                plot2d->xAxisCfg.nTicmarks = valInt;
                doRedraw = TRUE;
            }
            else if (!strcmp("yAxisNumTics", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[15]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[15]);
                    continue;
                }

                ns = sscanf(value, "%d", &valInt);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
                #ifndef WIN32
                    if (ns == EOF || ns < 1)
                #else
                    if (ns < 1)
                #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[15]);
                    continue;
                }

                if (valInt < -1)
                {
                    NEUIK_RaiseError(funcName, errMsgs[16]);
                    continue;
                }
                if (valInt != -1)
                {
                    valInt += 2;
                }

                if (plot2d->yAxisCfg.nTicmarks == valInt) continue;

                /* else: The previous setting was changed */
                plot2d->yAxisCfg.nTicmarks = valInt;
                doRedraw = TRUE;
            }
            else
            {
                typeMixup = FALSE;
                for (nCtr = 0;; nCtr++)
                {
                    if (boolNames[nCtr] == NULL) break;

                    if (!strcmp(boolNames[nCtr], name))
                    {
                        typeMixup = TRUE;
                        break;
                    }
                }

                if (typeMixup)
                {
                    /* A bool type was mistakenly used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[11]);
                }
                else
                {
                    /* An unsupported name was used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[12]);
                }
            }
        }
    }
    va_end(args);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }
    if (doRedraw)
    {
        if (neuik_Element_GetSizeAndLocation(plot2d, &rSize, &rLoc))
        {
            eNum = 10;
            NEUIK_RaiseError(funcName, errMsgs[eNum]);
            eNum = 1;
        }
        else
        {
            neuik_Element_RequestRedraw(plot2d, rLoc, rSize);
        }
    }

    return eNum;
}

