/*******************************************************************************
 * Copyright (c) 2014-2021, Michael Leimon <mike@leimon.net>
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
#include "NEUIK_ListGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "NEUIK_Window_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int   neuik__isInitialized;
extern float neuik__HighDPI_Scaling;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ListGroup(void ** lgPtr);
int neuik_Object_Free__ListGroup(void * lgPtr);

int neuik_Element_GetMinSize__ListGroup(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ListGroup(NEUIK_Element lgElem, SDL_Event * ev);
int neuik_Element_Render__ListGroup(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ListGroup_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ListGroup,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ListGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ListGroup_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__ListGroup,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__ListGroup,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__ListGroup,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,

    /* RequestRedraw(): This function will be called when redraw is requested */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ListGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ListGroup()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ListGroup";
    static char  * errMsgs[]  = {"",                     // [0] no error
        "NEUIK library must be initialized first.",      // [1]
        "Failed to register `ListGroup` object class .", // [2]
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
        "NEUIK_ListGroup",                                       // className
        "An element container which horizontally groups items.", // classDescription
        neuik__Set_NEUIK,                                        // classSet
        neuik__Class_Container,                                  // superClass
        &neuik_ListGroup_BaseFuncs,                              // baseFuncs
        NULL,                                                    // classFuncs
        &neuik__Class_ListGroup))                                // newClass
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
 *  Name:          neuik_Object_New__ListGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ListGroup(
    void ** lgPtr)
{
    int               eNum        = 0;
    NEUIK_Container * cont        = NULL;
    NEUIK_ListGroup * lg          = NULL;
    NEUIK_Element   * sClassPtr   = NULL;
    NEUIK_Color       bdrClr      = COLOR_GRAY;
    NEUIK_Color       bgSelectClr = COLOR_MBLUE;
    NEUIK_Color       bgOddClr    = COLOR_WHITE;
    NEUIK_Color       bgEvenClr   = COLOR_LWHITE;
    RenderSize        rSize;
    RenderLoc         rLoc;
    static char       funcName[]  = "neuik_Object_New__ListGroup";
    static char     * errMsgs[]   = {"",                                  // [0] no error
        "Output Argument `lgPtr` is NULL.",                               // [1]
        "Failure to allocate memory.",                                    // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
        "Failure in function `neuik.NewElement`.",                        // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",              // [5]
        "Argument `lgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorSolid()`.",          // [7]
        "Failure in `neuik_Element_RequestRedraw()`.",                    // [8]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",               // [9]
    };

    if (lgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*lgPtr) = (NEUIK_ListGroup*) malloc(sizeof(NEUIK_ListGroup));
    lg = *lgPtr;
    if (lg == NULL)
    {
        eNum = 2;
        goto out;
    }
    lg->VSpacing      = 0;
    lg->WidthBorder   = 1;           /* thickness of border (px) */
    lg->colorBorder   = bdrClr;      /* color to use for the border */
    lg->colorBGSelect = bgSelectClr; /* color to use for the selected text */
    lg->colorBGOdd    = bgOddClr;    /* background color to use for unselected odd rows */
    lg->colorBGEven   = bgEvenClr;   /* background color to use for unselected even rows */

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ListGroup, 
            NULL,
            &(lg->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(lg->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Container, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(lg, &neuik_ListGroup_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_Object_GetClassObject(lg, neuik__Class_Container, (void**)&cont))
    {
        eNum = 6;
        goto out;
    }
    cont->cType        = NEUIK_CONTAINER_NO_DEFAULT_ADD_SET;
    cont->shownIfEmpty = 1;

    if (neuik_Element_GetSizeAndLocation(lg, &rSize, &rLoc))
    {
        eNum = 9;
        goto out;
    }
    if (neuik_Element_RequestRedraw(lg, rLoc, rSize))
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorSolid(cont, "normal",
        bgOddClr.r, bgOddClr.g, bgOddClr.b, bgOddClr.a))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(cont, "selected",
        bgOddClr.r, bgOddClr.g, bgOddClr.b, bgOddClr.a))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(cont, "hovered",
        bgOddClr.r, bgOddClr.g, bgOddClr.b, bgOddClr.a))
    {
        eNum = 7;
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
 *  Name:          NEUIK_NewListGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_ListGroup.
 *
 *                 Wrapper function to neuik_Object_New__ListGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewListGroup(
    NEUIK_ListGroup ** lgPtr)
{
    return neuik_Object_New__ListGroup((void**)lgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Free__ListGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ListGroup(
    void * lgPtr)
{
    int               eNum       = 0;    /* which error to report (if any) */
    NEUIK_ListGroup * lg         = NULL;
    static char       funcName[] = "neuik_Object_Free__ListGroup";
    static char     * errMsgs[]  = {"",                // [0] no error
        "Argument `lgPtr` is NULL.",                   // [1]
        "Argument `lgPtr` is not of FlowGroup class.", // [2]
        "Failure in function `neuik_Object_Free`.",    // [3]
    };

    if (lgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    lg = (NEUIK_ListGroup*)lgPtr;

    if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(lg->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    free(lg);
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
 *  Name:          neuik_Element_GetMinSize__ListGroup
 *
 *  Description:   Returns the minimum rendered size of a given FlowGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ListGroup(
    NEUIK_Element    lgElem,
    RenderSize     * rSize)
{
    if (rSize != NULL)
    {
        rSize->w = 1;
        rSize->h = 1;
    }
    return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__ListGroup
 *
 *  Description:   Renders a vertical group of listRows.
 *
 *  Returns:       0 if there were no issues; otherwise 1.
 *
 ******************************************************************************/
int neuik_Element_Render__ListGroup(
    NEUIK_Element   lgElem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                    nAlloc        = 0;
    int                    borderW       = 1; /* width of border line */
    int                    xOffset       = 0; /* offset from left for rows */
    int                    tempH         = 0;
    int                    tempW         = 0;
    int                    offLeft       = 0;
    int                    offRight      = 0;
    int                    offTop        = 0;
    int                    offBottom     = 0;
    int                    ctr           = 0;
    int                    yFree         = 0; // px of space free for vFill elems
    int                    dH            = 0; // Change in height [px]
    int                    eNum          = 0; // which error to report (if any)
    int                    nVFill        = 0; // number of rows which can VFill
    int                    reqResizeH    = 0; // required resize height
    int                    vfillRowsMinH = 0; // min height for all vFill rows
    int                    vfillMaxMinH  = 0; // largest minimum row height 
                                              // among vertically filling rows.
    int                  * allHFill      = NULL; // Free upon returning; 
                                                 // Cols fills vertically? (per col)
    int                  * allVFill      = NULL; // Free upon returning; 
                                                 // Row fills vertically? (per row)
    int                  * allMaxMinH    = NULL; // Free upon returning; 
                                                 // The max min width (per row)
    int                    maxMinW       = 0;    // The max min width
    int                  * rendRowH      = NULL; // Free upon returning; 
                                                 // Rendered row height (per row)
    int                  * elemsShown    = NULL; // Free upon returning.
    float                  fltVspacingSc = 0.0;  // float VSpacing HighDPI scaled
    float                  yPos          = 0.0;
    RenderSize           * elemsMinSz    = NULL; // Free upon returning.
    NEUIK_ElementConfig ** elemsCfg      = NULL; // Free upon returning.
    RenderLoc              rl            = {0, 0};
    RenderLoc              rlRel         = {0, 0}; /* renderloc relative to parent */
    SDL_Rect               rect          = {0, 0, 0, 0};
    static RenderSize      rsZero        = {0, 0};
    RenderSize             rsMin         = {0, 0};
    RenderSize           * rs            = NULL;
    const NEUIK_Color    * bClr          = NULL; /* border color */
    SDL_Renderer         * rend          = NULL;
    NEUIK_Container      * cont          = NULL;
    NEUIK_ElementBase    * eBase         = NULL;
    NEUIK_Element          elem          = NULL;
    NEUIK_ElementConfig  * eCfg          = NULL;
    NEUIK_ListGroup      * lg            = NULL;
    neuik_MaskMap        * maskMap       = NULL; /* FREE upon return */
    enum neuik_bgstyle     bgStyle;
    static char           funcName[]     = "neuik_Element_Render__ListGroup";
    static char          * errMsgs[]     = {"", // [0] no error
        "Argument `lgElem` is not of ListGroup class.",                    // [1]
        "Argument `lgElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Invalid specified `rSize` (negative values).",                    // [3]
        "Failure in `neuik_Element_GetCurrentBGStyle()`.",                 // [4]
        "Failure in `neuik_MakeMaskMap()`",                                // [5]
        "Failure in `neuik_Window_FillTranspMaskFromLoc()`",               // [6]
        "Failure in neuik_Element_RedrawBackground().",                    // [7]
        "Failure to allocate memory.",                                     // [8]
        "Element_GetConfig returned NULL.",                                // [9]
        "Element_GetMinSize Failed.",                                      // [10]
        "Failure in `neuik_Element_Render()`",                             // [11]
    };

    if (!neuik_Object_IsClass(lgElem, neuik__Class_ListGroup))
    {
        eNum = 1;
        goto out;
    }
    lg = (NEUIK_ListGroup*)lgElem;

    if (neuik_Object_GetClassObject(lgElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (neuik_Object_GetClassObject(lgElem, neuik__Class_Container, (void**)&cont))
    {
        eNum = 2;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 3;
        goto out;
    }

    eBase->eSt.rend = xRend;
    rend = eBase->eSt.rend;

    if (neuik__HighDPI_Scaling <= 1.0)
    {
        fltVspacingSc = (float)(lg->VSpacing);
    }
    else
    {
        fltVspacingSc = (float)(lg->VSpacing)*neuik__HighDPI_Scaling;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik_Element_GetCurrentBGStyle(lgElem, &bgStyle))
        {
            eNum = 4;
            goto out;
        }
        if (bgStyle != NEUIK_BGSTYLE_TRANSPARENT)
        {
            /*----------------------------------------------------------------*/
            /* Create a MaskMap an mark off the trasnparent pixels.           */
            /*----------------------------------------------------------------*/
            if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
            {
                eNum = 5;
                goto out;
            }

            rl = eBase->eSt.rLoc;
            if (neuik_Window_FillTranspMaskFromLoc(
                    eBase->eSt.window, maskMap, rl.x, rl.y))
            {
                eNum = 6;
                goto out;
            }

            if (neuik_Element_RedrawBackground(lgElem, rlMod, maskMap))
            {
                eNum = 7;
                goto out;
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Draw the border of the ListGroup.                                      */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik__HighDPI_Scaling > 1.0)
        {
            borderW = (int)(neuik__HighDPI_Scaling);
        }

        bClr = &(lg->colorBorder);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

        offLeft   = rl.x;
        offRight  = rl.x + (rSize->w - 1);
        offTop    = rl.y;
        offBottom = rl.y + (rSize->h - 1);

        /* upper border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                offLeft,  offTop + ctr, 
                offRight, offTop + ctr); 
        }

        /* left border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                offLeft + ctr, offTop, 
                offLeft + ctr, offBottom); 
        }

        /* right border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                offRight - ctr, offTop, 
                offRight - ctr, offBottom); 
        }

        /* lower border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                offLeft,  offBottom - ctr, 
                offRight, offBottom - ctr); 
        }
    }

    if (cont->elems == NULL)
    {
        /* No elements are contained; don't do any more work here. */
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate memory for the calculated maximum minimum values, the         */
    /* VFill/HFill flags, and for the rendered row/column heights/widths.     */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;
    }
    nAlloc = ctr;

    allMaxMinH = malloc(nAlloc*sizeof(int));
    if (allMaxMinH == NULL)
    {
        eNum = 8;
        goto out;
    }
    allHFill = malloc(nAlloc*sizeof(int));
    if (allHFill == NULL)
    {
        eNum = 8;
        goto out;
    }
    allVFill = malloc(nAlloc*sizeof(int));
    if (allVFill == NULL)
    {
        eNum = 8;
        goto out;
    }
    rendRowH = malloc(nAlloc*sizeof(int));
    if (rendRowH == NULL)
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Zero out the initial maximum minimum values and HFill/VFill flags.     */
    /*------------------------------------------------------------------------*/
    maxMinW = 0;
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        allHFill[ctr]   = 0;
        allMaxMinH[ctr] = 0;
        allVFill[ctr]   = 0;
        rendRowH[ctr]   = 0;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate memory for lists of contained element properties.             */
    /*------------------------------------------------------------------------*/
    elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
    if (elemsCfg == NULL)
    {
        eNum = 8;
        goto out;
    }
    elemsShown = malloc(nAlloc*sizeof(int));
    if (elemsShown == NULL)
    {
        eNum = 8;
        goto out;
    }
    elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
    if (elemsMinSz == NULL)
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Zero out the values in the element minimum size array.                 */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        elemsMinSz[ctr] = rsZero;
    }

    /*------------------------------------------------------------------------*/
    /* Store the current properties for the contained elements.               */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        elemsShown[ctr] = NEUIK_Element_IsShown(elem);
        if (!elemsShown[ctr]) continue;

        elemsCfg[ctr] = neuik_Element_GetConfig(elem);
        if (elemsCfg[ctr] == NULL)
        {
            eNum = 9;
            goto out;
        }

        if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
        {
            eNum = 10;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the maximum minimum widths required for all of the columns.  */
    /* (i.e., for each column of elements, determine the maximum value of the */
    /* minimum widths required (among elements in the column)).               */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
        if (tempW > maxMinW)
        {
            maxMinW = tempW;
        }

        allMaxMinH[ctr] = rs->h + (eCfg->PadTop + eCfg->PadBottom);

        /*--------------------------------------------------------------------*/
        /* Check if the element can fill horizontally and if so, mark the     */
        /* whole listgroup as one that can fill horizontally.                 */
        /*--------------------------------------------------------------------*/
        if (eCfg->HFill)
        {
            allHFill[ctr] = 1;
        }
        /*--------------------------------------------------------------------*/
        /* Check if the element can fill vertically and if so, mark the whole */
        /* listGroup as one that can fill vertically.                         */
        /*--------------------------------------------------------------------*/
        if (eCfg->VFill)
        {
            allVFill[ctr] = 1;
        }
    }

    /*========================================================================*/
    /* Calculation of rendered row heights (accounts for VFill).              */
    /*========================================================================*/
    /* Determine the required minimum height and the total number of rows     */
    /* which can fill vertically.                                             */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        eCfg  = elemsCfg[ctr];
        rs    = &elemsMinSz[ctr];
        tempH = rs->h + (eCfg->PadTop + eCfg->PadBottom);
        allMaxMinH[ctr] = tempH;

        rsMin.h += allMaxMinH[ctr];
        nVFill += allVFill[ctr];
    }
    if (nAlloc > 1)
    {
        rsMin.h += (int)(fltVspacingSc*(float)(nAlloc - 1));
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the space occupied by all VFill rows and determine the value */
    /* of the largest minimum row height among vertically filling rows.       */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (allVFill[ctr])
        {
            vfillRowsMinH += allMaxMinH[ctr];
            if (vfillMaxMinH < allMaxMinH[ctr])
            {
                vfillMaxMinH = allMaxMinH[ctr];
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the amount of currently unused vertical space (beyond min).  */
    /*------------------------------------------------------------------------*/
    yFree = rSize->h - rsMin.h;

    /*------------------------------------------------------------------------*/
    /* Check if there is enough unused vertical space to bring all VFill rows */
    /* to the same height.                                                    */
    /*------------------------------------------------------------------------*/
    reqResizeH = nVFill*vfillMaxMinH - vfillRowsMinH; // required resize height
    if (yFree >= reqResizeH)
    {
        /*--------------------------------------------------------------------*/
        /* There is enough space; get all VFill rows to the same size first.  */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < nAlloc; ctr++)
        {
            rendRowH[ctr] = allMaxMinH[ctr];
            if (allVFill[ctr])
            {
                rendRowH[ctr] = vfillMaxMinH;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Evenly divide the remaining vSpace between the vFill rows.         */
        /*--------------------------------------------------------------------*/
        yFree -= reqResizeH;

        dH = (int)(floor( (float)(yFree)/(float)(nVFill) ));
        if (dH > 0)
        {
            /*----------------------------------------------------------------*/
            /* Increase the height of vFill rows all by the same quantity.    */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                if (allVFill[ctr])
                {
                    rendRowH[ctr] += dH;
                    yFree -= dH;
                }
            }
        }

        if (yFree > 0)
        {
            /*----------------------------------------------------------------*/
            /* Distribute the remaining vSpace to the vFill one pixel at a    */
            /* time (starting from the top row and moving down).              */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                if (allVFill[ctr])
                {
                    rendRowH[ctr] += 1;
                    yFree -= 1;
                    if (yFree == 0)
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* Evenly divide the remaining vSpace between the vFill rows.         */
        /*--------------------------------------------------------------------*/
        if (yFree == 0)
        {
            /*----------------------------------------------------------------*/
            /* There isn't enough space for any resizing.                     */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                rendRowH[ctr] = allMaxMinH[ctr];
            }

        }
        else
        {
            while (yFree > 0)
            {
                /*------------------------------------------------------------*/
                /* Distribute the remaining vSpace to the vFill one pixel at  */
                /* a time (starting from the top row and moving down).        */
                /*------------------------------------------------------------*/
                for (ctr = 0; ctr < nAlloc; ctr++)
                {
                    if (allVFill[ctr] && rendRowH[ctr] < vfillMaxMinH)
                    {
                        rendRowH[ctr] += 1;
                        yFree -= 1;
                        if (yFree == 0)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    /*========================================================================*/
    /* Render and place the child elements                                    */
    /*========================================================================*/
    xOffset = borderW;
    yPos = (float)(borderW);
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (ctr > 0)
        {
            yPos += (float)(rendRowH[ctr-1]) + fltVspacingSc;
        }
        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        elem = cont->elems[ctr];
        if (!neuik_Element_NeedsRedraw(elem)) continue;

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        tempH = rendRowH[ctr];

        /*--------------------------------------------------------------------*/
        /* Check for and apply if necessary Horizontal and Vertical fill.     */
        /*--------------------------------------------------------------------*/
        if (allHFill[ctr])
        {
            rs->w = rSize->w - (eCfg->PadLeft + eCfg->PadRight) - 2*xOffset + 1;
        }
        if (allVFill[ctr])
        {
            rs->h = tempH - (eCfg->PadTop + eCfg->PadBottom);
        }

        /*--------------------------------------------------------------------*/
        /* Update the stored location before rendering the element. This is   */
        /* necessary as the location of this object will propagate to its     */
        /* child objects.                                                     */
        /*--------------------------------------------------------------------*/
        switch (eCfg->HJustify)
        {
            case NEUIK_HJUSTIFY_DEFAULT:
                switch (cont->HJustify)
                {
                    case NEUIK_HJUSTIFY_LEFT:
                        rect.x = eCfg->PadLeft;
                        break;
                    case NEUIK_HJUSTIFY_CENTER:
                    case NEUIK_HJUSTIFY_DEFAULT:
                        rect.x = (rSize->w/2) - (rs->w/2);
                        break;
                    case NEUIK_HJUSTIFY_RIGHT:
                        rect.x = rSize->w - (rs->w + eCfg->PadRight);
                        break;
                }
                break;
            case NEUIK_HJUSTIFY_LEFT:
                rect.x = eCfg->PadLeft;
                break;
            case NEUIK_HJUSTIFY_CENTER:
                rect.x = (rSize->w/2) - (rs->w/2);
                break;
            case NEUIK_HJUSTIFY_RIGHT:
                rect.x = rSize->w - (rs->w + eCfg->PadRight);
                break;
        }
        switch (eCfg->VJustify)
        {
            case NEUIK_VJUSTIFY_DEFAULT:
                switch (cont->VJustify)
                {
                    case NEUIK_VJUSTIFY_TOP:
                        rect.y = (int)(yPos) + eCfg->PadTop;
                        break;
                    case NEUIK_VJUSTIFY_CENTER:
                    case NEUIK_VJUSTIFY_DEFAULT:
                        rect.y = ((int)(yPos) + rendRowH[ctr]/2) - (tempH/2);
                        break;
                    case NEUIK_VJUSTIFY_BOTTOM:
                        rect.y = ((int)(yPos) + rendRowH[ctr]) - 
                            (rs->h + eCfg->PadBottom);
                        break;
                }
                break;
            case NEUIK_VJUSTIFY_TOP:
                rect.y = (int)(yPos) + eCfg->PadTop;
                break;
            case NEUIK_VJUSTIFY_CENTER:
                rect.y = ((int)(yPos) + rendRowH[ctr]/2) - (tempH/2);
                break;
            case NEUIK_VJUSTIFY_BOTTOM:
                rect.y = ((int)(yPos) + rendRowH[ctr]) - 
                    (rs->h + eCfg->PadBottom);
                break;
        }

        rect.w = rs->w;
        rect.h = rendRowH[ctr];
        rl.x = (eBase->eSt.rLoc).x + rect.x;
        rl.y = (eBase->eSt.rLoc).y + rect.y;
        rlRel.x = rect.x;
        rlRel.y = rect.y;
        neuik_Element_StoreSizeAndLocation(elem, *rs, rl, rlRel);

        if (neuik_Element_Render(elem, rs, rlMod, rend, mock))
        {
            eNum = 11;
            goto out;
        }
    }
out:
    if (eBase != NULL)
    {
        if (!mock) eBase->eSt.doRedraw = 0;
    }
    if (maskMap != NULL) neuik_Object_Free(maskMap);

    if (elemsCfg   != NULL) free(elemsCfg);
    if (elemsShown != NULL) free(elemsShown);
    if (elemsMinSz != NULL) free(elemsMinSz);
    if (allMaxMinH != NULL) free(allMaxMinH);
    if (rendRowH   != NULL) free(rendRowH);
    if (allHFill   != NULL) free(allHFill);
    if (allVFill   != NULL) free(allVFill);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ListGroup_AddRow
 *
 *  Description:   Adds a row to a ListGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListGroup_AddRow(
    NEUIK_ListGroup * lg, 
    NEUIK_ListRow   * row)
{
    int                 len;
    int                 ctr;
    RenderSize          rSize;
    RenderLoc           rLoc;
    int                 newInd;            /* index for newly added item */
    int                 eNum       = 0;    /* which error to report (if any) */
    NEUIK_ElementBase * eBase      = NULL;
    NEUIK_Container   * cBase      = NULL;
    static char         funcName[] = "NEUIK_ListGroup_AddRow";
    static char       * errMsgs[]  = {"",                              // [0] no error
        "Argument `lg` is not of ListGroup class.",                    // [1]
        "Argument `lg` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Argument `row` is not of ListRow class.",                     // [3]
        "Failure to allocate memory.",                                 // [4]
        "Failure to reallocate memory.",                               // [5]
        "Failure in `neuik_Element_RequestRedraw()`.",                 // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",            // [7]
    };

    if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(lg, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 2;
        goto out;
    }
    if (!neuik_Object_IsClass(row, neuik__Class_ListRow))
    {
        eNum = 3;
        goto out;
    }

    if (cBase->elems == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* elems array currently unallocated; allocate now                    */
        /*--------------------------------------------------------------------*/
        cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
        if (cBase->elems == NULL)
        {
            eNum = 4;
            goto out;
        }
        newInd = 0;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* This is subsequent UI element, reallocate memory.                  */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length */
        for (ctr = 0;;ctr++)
        {
            if (cBase->elems[ctr] == NULL)
            {
                len = 2 + ctr;
                break;
            }
        }

        cBase->elems = (NEUIK_Element*)realloc(cBase->elems, len*sizeof(NEUIK_Element));
        if (cBase->elems == NULL)
        {
            eNum = 5;
            goto out;
        }
        newInd = ctr;
    }

    /*------------------------------------------------------------------------*/
    /* Set the Window and Parent Element pointers                             */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(lg, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (eBase->eSt.window != NULL)
    {
        neuik_Element_SetWindowPointer(row, eBase->eSt.window);
    }
    neuik_Element_SetParentPointer(row, lg);

    /*------------------------------------------------------------------------*/
    /* Set the odd/even flag of the row.                                      */
    /*------------------------------------------------------------------------*/
    row->isOddRow = 0;
    if ((newInd+1) % 2 == 1) row->isOddRow = 1;

    cBase->elems[newInd]   = row;
    cBase->elems[newInd+1] = NULL; /* NULLptr terminated array */

    /*------------------------------------------------------------------------*/
    /* When a new row is added, trigger a redraw                              */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(lg, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    if (neuik_Element_RequestRedraw(lg, rLoc, rSize))
    {
        eNum = 6;
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
 *  Name:          NEUIK_ListGroup_AddRows
 *
 *  Description:   Add multiple rows to a ListGroup.
 *
 *                 NOTE: the variable # of arguments must be terminated by a 
 *                 NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListGroup_AddRows(
    NEUIK_ListGroup * lg, 
    NEUIK_ListRow   * row0, 
    ...)
{
    int             ctr;
    int             vaOpen = 0;
    int             eNum   = 0; /* which error to report (if any) */
    va_list         args;
    NEUIK_ListRow * row    = NULL; 
    static char     funcName[] = "NEUIK_ListGroup_AddRows";
    static char   * errMsgs[]  = {"",               // [0] no error
        "Argument `lg` is not of ListGroup class.", // [1]
        "Failure in `ListGroup_AddRow()`.",         // [2]
    };

    if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
    {
        eNum = 1;
        goto out;
    }

    va_start(args, row0);
    vaOpen = 1;

    row = row0;
    for (ctr = 0;; ctr++)
    {
        if (row == NULL) break;

        if (NEUIK_ListGroup_AddRow(lg, row))
        {
            eNum = 2;
            goto out;
        }

        /* before starting */
        row = va_arg(args, NEUIK_Element);
    }
out:
    if (vaOpen) va_end(args);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__ListGroup
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ListGroup(
    NEUIK_Element   lgElem, 
    SDL_Event     * ev)
{
    int                 ctr         = 0;
    int                 indSelect   = 0;
    int                 wasSelected = 0;
    neuik_EventState    evCaputred  = NEUIK_EVENTSTATE_NOT_CAPTURED;
    NEUIK_Element       elem        = NULL;
    NEUIK_ElementBase * eBase       = NULL;
    NEUIK_Container   * cBase       = NULL;
    SDL_KeyboardEvent * keyEv       = NULL;

    if (neuik_Object_GetClassObject_NoError(
        lgElem, neuik__Class_Container, (void**)&cBase)) goto out;

    if (neuik_Object_GetClassObject_NoError(
        lgElem, neuik__Class_Element, (void**)&eBase)) goto out;

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by one of the contained rows.          */
    /*------------------------------------------------------------------------*/
    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            elem = cBase->elems[ctr];
            if (elem == NULL) break;

            if (!NEUIK_Element_IsShown(elem)) continue;

            wasSelected = NEUIK_ListRow_IsSelected(elem);
            evCaputred = neuik_Element_CaptureEvent(elem, ev);
            if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
            {
                goto out;
            }
            else if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
            {
                if (!wasSelected && NEUIK_ListRow_IsSelected(elem))
                {
                    indSelect = ctr;
                    /*--------------------------------------------------------*/
                    /* This event just caused this row to be selected.        */
                    /* Deselect the other rows.                               */
                    /*--------------------------------------------------------*/
                    for (ctr = 0;; ctr++)
                    {
                        elem = cBase->elems[ctr];
                        if (elem == NULL) break;
                        if (ctr == indSelect) continue;

                        NEUIK_ListRow_SetSelected(elem, 0);
                    }
                }

                neuik_Element_SetActive(lgElem, 1);
                goto out;
            }
        }
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* If there are no contained elements, there is probably no possible  */
        /* outcome to handling the event.                                     */
        /*--------------------------------------------------------------------*/
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the ListGroup itself.                */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_IsActive(lgElem))
    {
        switch (ev->type)
        {
        case SDL_KEYDOWN:
            keyEv = (SDL_KeyboardEvent*)(ev);
            switch (keyEv->keysym.sym)
            {
            case SDLK_UP:
                /*------------------------------------------------------------*/
                /* Determine the where the first selected item is             */
                /*------------------------------------------------------------*/
                for (ctr = 0;; ctr++)
                {
                    elem = cBase->elems[ctr];
                    if (elem == NULL) break;

                    if (!NEUIK_Element_IsShown(elem)) continue;
                    if (NEUIK_ListRow_IsSelected(elem))
                    {
                        indSelect = ctr;
                        break;
                    }
                }

                if (indSelect > 0)
                {
                    NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 0);
                    indSelect--;
                    NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 1);
                    neuik_Window_TakeFocus(eBase->eSt.window, cBase->elems[indSelect]);
                }
                break;
            case SDLK_DOWN:
                /*------------------------------------------------------------*/
                /* Determine the where the first selected item is             */
                /*------------------------------------------------------------*/
                for (ctr = 0;; ctr++)
                {
                    elem = cBase->elems[ctr];
                    if (elem == NULL) break;

                    if (!NEUIK_Element_IsShown(elem)) continue;
                    if (NEUIK_ListRow_IsSelected(elem))
                    {
                        indSelect = ctr;
                        break;
                    }
                }

                if (cBase->elems[indSelect+1] != NULL)
                {
                    NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 0);
                    indSelect++;
                    NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 1);
                    neuik_Window_TakeFocus(eBase->eSt.window, cBase->elems[indSelect]);
                }
                break;
            }
        }       
    }
out:
    return evCaputred;
}

