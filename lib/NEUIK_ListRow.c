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
#include "NEUIK_ListRow.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Container.h"
#include "NEUIK_Container_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ListRow(void ** rowPtr);
int neuik_Object_Free__ListRow(void * rowPtr);

int neuik_Element_GetMinSize__ListRow(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ListRow(NEUIK_Element rowElem, SDL_Event * ev);
int neuik_Element_Render__ListRow(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);
void neuik_Element_Defocus__ListRow(NEUIK_Element rowElem);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ListRow_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ListRow,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ListRow,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ListRow_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__ListRow,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__ListRow,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__ListRow,

    /* Defocus(): This function will be called when an element looses focus */
    neuik_Element_Defocus__ListRow,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ListRow
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ListRow()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ListRow";
    static char  * errMsgs[]  = {"",                   // [0] no error
        "NEUIK library must be initialized first.",    // [1]
        "Failed to register `ListRow` object class .", // [2]
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
        "NEUIK_ListRow",                                         // className
        "An element container which horizontally groups items.", // classDescription
        neuik__Set_NEUIK,                                        // classSet
        neuik__Class_Container,                                  // superClass
        &neuik_ListRow_BaseFuncs,                                // baseFuncs
        NULL,                                                    // classFuncs
        &neuik__Class_ListRow))                                  // newClass
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
 *  Name:          neuik_Object_New__ListRow
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ListRow(
    void ** rowPtr)
{
    int                   eNum        = 0;
    NEUIK_Container     * cont        = NULL;
    NEUIK_ListRow       * row         = NULL;
    NEUIK_Element       * sClassPtr   = NULL;
    NEUIK_ElementConfig * eCfg        = NULL;
    NEUIK_Color           bgSelectClr = COLOR_MBLUE;
    NEUIK_Color           bgOddClr    = COLOR_WHITE;
    NEUIK_Color           bgEvenClr   = COLOR_MLLWHITE;
    static char           funcName[]  = "neuik_Object_New__ListRow";
    static char         * errMsgs[]   = {"",                               // [0] no error
        "Output Argument `rowPtr` is NULL.",                               // [1]
        "Failure to allocate memory.",                                     // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",                        // [3]
        "Failure in function `neuik.NewElement`.",                         // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",               // [5]
        "Argument `rowPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
        "Element_GetConfig returned NULL.",                                // [7]
        "Failure in `NEUIK_Element_SetBackgroundColorSolid`.",             // [8]
    };

    if (rowPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*rowPtr) = (NEUIK_ListRow*) malloc(sizeof(NEUIK_ListRow));
    row = *rowPtr;
    if (row == NULL)
    {
        eNum = 2;
        goto out;
    }
    row->HSpacing      = 1;
    row->isOddRow      = 1;
    row->selectable    = 1;
    row->selected      = 0;
    row->wasSelected   = 0;
    row->isActive      = 0;
    row->clickOrigin   = 0;
    row->timeLastClick = 0;
    row->colorBGSelect = bgSelectClr; /* color to use for the selected text */
    row->colorBGOdd    = bgOddClr;    /* background color to use for unselected odd rows */
    row->colorBGEven   = bgEvenClr;   /* background color to use for unselected even rows */

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ListRow, 
            NULL,
            &(row->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(row->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Container, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(row, &neuik_ListRow_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_Object_GetClassObject(row, neuik__Class_Container, (void**)&cont))
    {
        eNum = 6;
        goto out;
    }
    cont->cType        = NEUIK_CONTAINER_MULTI;
    cont->shownIfEmpty = 0;

    eCfg = neuik_Element_GetConfig(row);
    if (eCfg == NULL)
    {
        eNum = 7;
        goto out;
    }
    eCfg->HFill = 1;

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorSolid(row, "normal",
        bgOddClr.r, bgOddClr.g, bgOddClr.b, bgOddClr.a))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(row, "selected",
        bgSelectClr.r, bgSelectClr.g, bgSelectClr.b, bgSelectClr.a))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(row, "hovered",
        bgOddClr.r, bgOddClr.g, bgOddClr.b, bgOddClr.a))
    {
        eNum = 8;
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
 *  Name:          NEUIK_NewListRow
 *
 *  Description:   Create and return a pointer to a new NEUIK_ListRow.
 *
 *                 Wrapper function to neuik_Object_New__ListRow.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewListRow(
    NEUIK_ListRow ** rowPtr)
{
    return neuik_Object_New__ListRow((void**)rowPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ListRow_SetHSpacing
 *
 *  Description:   Set the horizontal spacing parameter of a horizontal group.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListRow_SetHSpacing(
    NEUIK_ListRow  * row,
    int              spacing)
{
    int            eNum       = 0;    /* which error to report (if any) */
    static char    funcName[] = "NEUIK_ListRow_SetHSpacing";
    static char  * errMsgs[]  = {"",               // [0] no error
        "Argument `row` is not of ListRow class.", // [1]
        "Argument `spacing` can not be negative.", // [2]
    };

    if (!neuik_Object_IsClass(row, neuik__Class_ListRow))
    {
        eNum = 1;
        goto out;
    }
    if (spacing < 0)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* if there is no effective change in spacing; don't do anything          */
    /*------------------------------------------------------------------------*/
    if (spacing == row->HSpacing) goto out;

    row->HSpacing = spacing;
    // neuik_Element_RequestRedraw(vg);
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
 *  Name:          NEUIK_ListRow_SetSelected
 *
 *  Description:   Set this particular row as selected or deselect it.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListRow_SetSelected(
    NEUIK_ListRow  * row,
    int              isSelected)
{
    RenderSize     rSize;
    RenderLoc      rLoc;
    int            eNum       = 0;    /* which error to report (if any) */
    static char    funcName[] = "NEUIK_ListRow_SetSelected";
    static char  * errMsgs[]  = {"",                            // [0] no error
        "Argument `row` is not of ListRow class.",              // [1]
        "Argument `isSelected` is invalid may be zero or one.", // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",     // [3]
    };

    if (!neuik_Object_IsClass(row, neuik__Class_ListRow))
    {
        eNum = 1;
        goto out;
    }
    if (isSelected != 0 && isSelected != 1)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* if there is no effective change in spacing; don't do anything          */
    /*------------------------------------------------------------------------*/
    if (isSelected == row->selected) goto out;

    row->selected = isSelected;
    if (isSelected)
    {
        neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_SELECTED);
    }
    else
    {
        row->clickOrigin = 0;
        row->selected    = 0;
        row->wasSelected = 0;
        neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_DESELECTED);
    }

    if (neuik_Element_GetSizeAndLocation(row, &rSize, &rLoc))
    {
        eNum = 3;
        goto out;
    }
    neuik_Element_RequestRedraw(row, rLoc, rSize);
    neuik_Container_RequestFullRedraw(row);
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
 *  Name:          NEUIK_ListRow_IsSelected
 *
 *  Description:   Reports whether or not the ListRow is selected
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListRow_IsSelected(
    NEUIK_ListRow * row)
{
    if (!neuik_Object_IsClass(row, neuik__Class_ListRow)) return 0;

    return row->selected;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Free__ListRow
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ListRow(
    void * rowPtr)
{
    int             eNum       = 0;    /* which error to report (if any) */
    NEUIK_ListRow * row        = NULL;
    static char     funcName[] = "neuik_Object_Free__ListRow";
    static char   * errMsgs[]  = {"",                 // [0] no error
        "Argument `rowPtr` is NULL.",                 // [1]
        "Argument `rowPtr` is not of ListRow class.", // [2]
        "Failure in function `neuik_Object_Free`.",   // [3]
    };

    if (rowPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    row = (NEUIK_ListRow*)rowPtr;

    if (!neuik_Object_IsClass(row, neuik__Class_ListRow))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(row->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    free(row);
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
 *  Name:          neuik_Element_GetMinSize__ListRow
 *
 *  Description:   Returns the rendered size of a given ListRow.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ListRow(
    NEUIK_Element    rowElem,
    RenderSize     * rSize)
{
    int                   tempH;
    int                   ctr        = 0;
    int                   vctr       = 0;   /* valid counter; for elements shown */
    int                   maxMinW    = 0;   /* largest min-width of all HFill elements */
    int                   thisMinW   = 0;   /* this HFill element's min-width */
    int                   eNum       = 0;   /* which error to report (if any) */
    float                 thisW      = 0.0;
    RenderSize            rs;
    NEUIK_Element         elem       = NULL;
    NEUIK_ElementBase   * eBase      = NULL;
    NEUIK_ElementConfig * eCfg       = NULL;
    NEUIK_Container     * cont       = NULL;
    NEUIK_ListRow       * row        = NULL;
    static char           funcName[] = "neuik_Element_GetMinSize__ListRow";
    static char         * errMsgs[]  = {"",                                 // [0] no error
        "Argument `rowElem` is not of ListRow class.",                      // [1]
        "Element_GetMinSize Failed.",                                       // [2]
        "Element_GetConfig returned NULL.",                                 // [3]
        "Argument `rowElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
    };

    rSize->w = 0;
    rSize->h = 0;

    /*------------------------------------------------------------------------*/
    /* Check for problems before proceding                                    */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(rowElem, neuik__Class_ListRow))
    {
        eNum = 1;
        goto out;
    }
    row = (NEUIK_ListRow*)rowElem;

    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Container, (void**)&cont))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 4;
        goto out;
    }

    if (cont->elems == NULL) {
        /* there are no UI elements contained by this ListRow */
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Determine the (maximum) height required by any one of the elements.    */
    /*                                                                        */
    /*    and                                                                 */
    /*                                                                        */
    /* Find the largest minimum width of all the horizontally filling items.  */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        eCfg = neuik_Element_GetConfig(elem);
        if (eCfg == NULL)
        {
            eNum = 3;
            goto out;
        }

        if (!NEUIK_Element_IsShown(elem)) continue;

        if (neuik_Element_GetMinSize(elem, &rs) != 0)
        {
            eNum = 2;
            goto out;
        }

        /*--------------------------------------------------------------------*/
        /* Get the (maximum) height required by any one of the elements       */
        /*--------------------------------------------------------------------*/
        tempH = rs.h + (eCfg->PadTop + eCfg->PadBottom);
        if (tempH > rSize->h)
        {
            rSize->h = tempH;
        }

        /*--------------------------------------------------------------------*/
        /* Get the largest min-width of all the horizontally filling items    */
        /*--------------------------------------------------------------------*/
        if (eCfg->HFill)
        {
            /* This element is fills space horizontally */
            thisMinW = rs.w;

            if (thisMinW > maxMinW)
            {
                maxMinW = thisMinW;
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Determine the required horizontal width                                */
    /*------------------------------------------------------------------------*/
    vctr = 0;
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        eCfg = neuik_Element_GetConfig(elem);
        if (eCfg == NULL)
        {
            eNum = 3;
            goto out;
        }

        if (!NEUIK_Element_IsShown(elem)) continue;
        vctr++;

        if (vctr > 0)
        {
            /* subsequent UI element is valid, add Horizontal Spacing */
            thisW += (float)(row->HSpacing);
        }

        if (neuik_Element_GetMinSize(elem, &rs))
        {
            eNum = 2;
            goto out;
        }

        if (eCfg->HFill)
        {
            /* This element is fills space horizontally */
            thisW += (eCfg->HScale)*(float)(maxMinW);
        }
        else
        {
            thisW += (float)(rs.w);
        }
        thisW += (float)(eCfg->PadLeft + eCfg->PadRight);
    }

    rSize->w = (int)(thisW);
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
 *  Name:          neuik_Element_Render__ListRow
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__ListRow(
    NEUIK_Element   rowElem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    RenderSize            rs;
    RenderLoc             rl         = {0, 0};
    RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
    SDL_Rect              rect;
    int                   tempH;
    int                   ctr        = 0;
    int                   vctr       = 0;    /* valid counter; for elements shown */
    int                   yPos       = 0;
    int                   elHeight   = 0;
    int                   eNum       = 0;    /* which error to report (if any) */
    float                 hFillPx    = 0.0;
    float                 xPos       = 0.0;
    float                 xSize      = 0.0;
    float                 xFree      = 0.0;  /* px of space for hFill elems */
    float                 tScale     = 0.0;  /* total vFill scaling factors */
    NEUIK_ElementConfig * eCfg       = NULL;
    NEUIK_Element         elem       = NULL;
    NEUIK_Container     * cont       = NULL;
    NEUIK_ElementBase   * eBase      = NULL;
    NEUIK_ListRow       * row        = NULL;
    const NEUIK_Color   * bgClr      = NULL; /* background color */
    SDL_Renderer        * rend       = NULL;
    static RenderSize     rsZero     = {0, 0};
    neuik_MaskMap       * maskMap    = NULL; /* FREE upon return */
    enum neuik_bgstyle    bgStyle;
    static char           funcName[] = "neuik_Element_Render__ListRow";
    static char         * errMsgs[]  = {"",                           // [0] no error
        "Argument `rowElem` is not of ListRow class.",                      // [1]
        "Failure in `neuik_Element_GetCurrentBGStyle()`.",                  // [2]
        "Element_GetConfig returned NULL.",                                 // [3]
        "Element_GetMinSize Failed.",                                       // [4]
        "Element_Render returned NULL.",                                    // [5]
        "Invalid specified `rSize` (negative values).",                     // [6]
        "Failure in `neuik_MakeMaskMap()`",                                 // [7]
        "Argument `rowElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
        "Failure in `NEUIK_Element_SetBackgroundColorSolid`.",              // [9]
        "Failure in `neuik_Element_RedrawBackground()`.",                   // [10]
        "Failure in `neuik_Window_FillTranspMaskFromLoc()`",                // [11]
    };

    if (!neuik_Object_IsClass(rowElem, neuik__Class_ListRow))
    {
        eNum = 1;
        goto out;
    }
    row = (NEUIK_ListRow*)rowElem;

    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 8;
        goto out;
    }
    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Container, (void**)&cont))
    {
        eNum = 8;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 6;
        goto out;
    }
    xFree = (float)(rSize->w); /* free X-px: start with the full width and deduct as used */

    eBase->eSt.rend = xRend;
    rend = eBase->eSt.rend;

    /*------------------------------------------------------------------------*/
    /* Fill the row with the appropriate background color.                    */
    /*------------------------------------------------------------------------*/
    if (row->selected)
    {
        eBase->eSt.focusstate = NEUIK_FOCUSSTATE_SELECTED;
    }
    else if (row->isOddRow)
    {
        bgClr = &(row->colorBGOdd);
        eBase->eSt.focusstate = NEUIK_FOCUSSTATE_NORMAL;
        if (NEUIK_Element_SetBackgroundColorSolid_noRedraw(row, "normal",
            bgClr->r, bgClr->g, bgClr->b, bgClr->a))
        {
            eNum = 9;
            goto out;
        }
    }
    else
    {
        bgClr = &(row->colorBGEven);
        eBase->eSt.focusstate = NEUIK_FOCUSSTATE_NORMAL;
        if (NEUIK_Element_SetBackgroundColorSolid_noRedraw(row, "normal",
            bgClr->r, bgClr->g, bgClr->b, bgClr->a))
        {
            eNum = 9;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik_Element_GetCurrentBGStyle(row, &bgStyle))
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
                eNum = 11;
                goto out;
            }

            if (neuik_Element_RedrawBackground(row, rlMod, maskMap))
            {
                eNum = 10;
                goto out;
            }
        }
    }
    
    /*------------------------------------------------------------------------*/
    /* Draw the UI elements into the ListRow                                  */
    /*------------------------------------------------------------------------*/
    if (cont->elems != NULL)
    {
        /*-------------------------------------------------------*/
        /* Determine the (maximum) height of any of the elements */
        /*-------------------------------------------------------*/
        elHeight = rSize->h;
        vctr = 0;
        for (ctr = 0;; ctr++)
        {
            elem = (NEUIK_Element)cont->elems[ctr];
            if (elem == NULL) break;

            eCfg = neuik_Element_GetConfig(elem);
            if (eCfg == NULL)
            {
                eNum = 3;
                goto out;
            }

            if (!NEUIK_Element_IsShown(elem)) continue;
            vctr++;

            if (vctr > 0)
            {
                /* subsequent UI element is valid, deduct Vertical Spacing */
                xFree -= row->HSpacing;
            }

            if (neuik_Element_GetMinSize(elem, &rs))
            {
                eNum = 4;
                goto out;
            }

            tempH = rs.h + eCfg->PadTop + eCfg->PadBottom;
            if (tempH > elHeight)
            {
                elHeight = tempH;
            }
            xFree -= rs.w;
        }

        if (vctr == 0)
        {
            goto out;
        }

        /*-------------------------------------------------------------*/
        /* Check if there are any elements which can fill horizontally */
        /*-------------------------------------------------------------*/
        for (ctr = 0;; ctr++)
        {
            elem = (NEUIK_Element)cont->elems[ctr];
            if (elem == NULL) break;

            eCfg = neuik_Element_GetConfig(elem);
            if (eCfg == NULL)
            {
                eNum = 3;
                goto out;
            }

            if (!NEUIK_Element_IsShown(elem)) continue;

            if (eCfg->HFill)
            {
                /* This element is fills space horizontally */
                tScale += eCfg->HScale;
                rs = rsZero; /* (0,0); use default calculated size */
                if (neuik_Element_GetMinSize(elem, &rs))
                {
                    eNum = 4;
                    goto out;
                }
                xFree += rs.w;
            }
        }

        /* calculate the number of horizontal px per 1.0 of HScaling */
        hFillPx = (float)((int)((float)(xFree) / tScale));

        /*--------------------------------------------------------------------*/
        /* Render and place the child elements                                */
        /*--------------------------------------------------------------------*/
        vctr = 0;
        for (ctr = 0;; ctr++)
        {
            elem = (NEUIK_Element)cont->elems[ctr];
            if (elem == NULL) break;

            eCfg = neuik_Element_GetConfig(elem);
            if (eCfg == NULL)
            {
                eNum = 3;
                goto out;
            }

            if (!NEUIK_Element_IsShown(elem)) continue;
            vctr++;

            if (vctr > 0)
            {
                /* add vertical spacing between subsequent elements */
                xPos += row->HSpacing;
            }

            /*----------------------------------------------------------------*/
            /* Start with the default calculated element size                 */
            /*----------------------------------------------------------------*/
            if (neuik_Element_GetMinSize(elem, &rs))
            {
                eNum = 4;
                goto out;
            }
            xSize = (float)(rs.w);

            /*----------------------------------------------------------------*/
            /* Check for and apply if necessary Horizontal and Veritcal fill  */
            /*----------------------------------------------------------------*/
            if (eCfg->VFill)
            {
                /* This element is configured to fill space vertically */
                rs.h = elHeight - (eCfg->PadTop + eCfg->PadBottom);
            }
            if (eCfg->HFill)
            {
                /* This element is configured to fill space horizontally */
                xSize = hFillPx * (eCfg->HScale) - (eCfg->PadLeft + eCfg->PadRight);
                rs.w  = (int)(xSize);
            }

            /*----------------------------------------------------------------*/
            /* Update the stored location before rendering the element. This  */
            /* is necessary as the location of this object will propagate to  */
            /* its child objects.                                             */
            /*----------------------------------------------------------------*/
            rect.x = (int)(xPos + eCfg->PadLeft);
            rect.y = yPos + eCfg->PadTop;
            rect.w = rs.w;
            rect.h = rs.h;
            rl.x = (eBase->eSt.rLoc).x + rect.x;
            rl.y = (eBase->eSt.rLoc).y + rect.y;
            rlRel.x = rect.x;
            rlRel.y = rect.y;
            neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

            if (neuik_Element_NeedsRedraw(elem))
            {
                if (neuik_Element_Render(elem, &rs, rlMod, rend, mock))
                {
                    eNum = 5;
                    goto out;
                }
            }

            xPos += xSize + (eCfg->PadLeft + eCfg->PadRight) ;
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
 *  Name:          neuik_Element_CaptureEvent__ListRow
 *
 *  Description:   Check to see if this event is captured by a NEUIK_ListRow.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ListRow(
    NEUIK_Element   rowElem,
    SDL_Event     * ev)
{
    int                    wasSelected = 0;
    RenderSize             rSize;
    RenderLoc              rLoc;
    neuik_EventState       evCaputred  = NEUIK_EVENTSTATE_NOT_CAPTURED;
    RenderLoc              eLoc;
    RenderSize             eSz;
    NEUIK_ListRow        * row        = NULL;
    NEUIK_ElementBase    * eBase      = NULL;
    SDL_MouseButtonEvent * mouseButEv;
    SDL_KeyboardEvent    * keyEv;

    /*------------------------------------------------------------------------*/
    /* Check for problems before proceding                                    */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(rowElem, neuik__Class_ListRow))
    {
        goto out;
    }
    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Element, (void**)&eBase))
    {
        /* not the right type of object */
        goto out;
    }
    row = (NEUIK_ListRow*)rowElem;
    
    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the row (mouseclick/mousemotion).    */
    /*------------------------------------------------------------------------*/
    switch (ev->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        mouseButEv = (SDL_MouseButtonEvent*)(ev);
        eLoc = eBase->eSt.rLoc;
        eSz  = eBase->eSt.rSize;

        if (mouseButEv->y >= eLoc.y && mouseButEv->y <= eLoc.y + eSz.h)
        {
            if (mouseButEv->x >= eLoc.x && mouseButEv->x <= eLoc.x + eSz.w)
            {
                /* This mouse click originated within this row */
                if (!row->selected)
                {
                    wasSelected = 1;
                }
                else if (SDL_GetTicks() - row->timeLastClick < NEUIK_DOUBLE_CLICK_TIMEOUT)
                {
                    /*--------------------------------------------------------*/
                    /* This would be a double click activation event.         */
                    /*--------------------------------------------------------*/
                    neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_ACTIVATED);
                    evCaputred = NEUIK_EVENTSTATE_CAPTURED;
                    if (!neuik_Object_IsNEUIKObject_NoError(row))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                    }
                    goto out;
                }
                row->clickOrigin   = 1;
                row->selected      = 1;
                row->wasSelected   = 0;
                row->timeLastClick = SDL_GetTicks();
                neuik_Window_TakeFocus(eBase->eSt.window, row);

                neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_CLICK);
                evCaputred = NEUIK_EVENTSTATE_CAPTURED;
                if (!neuik_Object_IsNEUIKObject_NoError(row))
                {
                    /* The object was freed/corrupted by the callback */
                    evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                    goto out;
                }

                if (wasSelected)
                {
                    neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_SELECTED);
                    if (!neuik_Object_IsNEUIKObject_NoError(row))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                        goto out;
                    }
                } 

                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(row, rLoc, rSize);
                neuik_Container_RequestFullRedraw(row);
                goto out;
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        mouseButEv = (SDL_MouseButtonEvent*)(ev);
        eLoc = eBase->eSt.rLoc;
        eSz  = eBase->eSt.rSize;

        if (row->clickOrigin)
        {
            if (mouseButEv->y >= eLoc.y && mouseButEv->y <= eLoc.y + eSz.h)
            {
                if (mouseButEv->x >= eLoc.x && mouseButEv->x <= eLoc.x + eSz.w)
                {
                    /* cursor is still within the row, activate cbFunc */
                    neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_CLICKED);
                    if (!neuik_Object_IsNEUIKObject_NoError(row))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                        goto out;
                    }
                }
            }
            row->clickOrigin = 0;
            evCaputred = NEUIK_EVENTSTATE_CAPTURED;

            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            neuik_Element_RequestRedraw(row, rLoc, rSize);
            neuik_Container_RequestFullRedraw(row);
            goto out;
        }
        break;
    }

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the menu (enter/space).             */
    /*------------------------------------------------------------------------*/
    if (row->selected)
    {
        switch (ev->type)
        {
        case SDL_KEYDOWN:
            keyEv = (SDL_KeyboardEvent*)(ev);
            switch (keyEv->keysym.sym)
            {
            case SDLK_SPACE:
            case SDLK_RETURN:
                /* row was selected, activate the row */
                neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_ACTIVATED);
                evCaputred = NEUIK_EVENTSTATE_CAPTURED;
                if (!neuik_Object_IsNEUIKObject_NoError(row))
                {
                    /* The object was freed/corrupted by the callback */
                    evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                    goto out;
                }
                goto out;
                break;
            }
        }       
    }
out:
    return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Defocus__ListRow
 *
 *  Description:   Deselect this list row.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_Defocus__ListRow(
    NEUIK_Element   rowElem)
{
    NEUIK_ListRow * row = NULL;
    RenderSize   rSize  = {0, 0};
    RenderLoc    rLoc   = {0, 0};

    /*------------------------------------------------------------------------*/
    /* Check for problems before proceding                                    */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(rowElem, neuik__Class_ListRow)) return;
    row = (NEUIK_ListRow*)rowElem;

    if (row->selected)
    {
        row->clickOrigin = 0;
        row->selected    = 0;
        row->wasSelected = 0;
        neuik_Element_TriggerCallback(row, NEUIK_CALLBACK_ON_DESELECTED);

        if (neuik_Element_GetSizeAndLocation(row, &rSize, &rLoc))
        {
            return;
        }

        neuik_Element_RequestRedraw(row, rLoc, rSize);
        neuik_Container_RequestFullRedraw(row);
    }
    else
    {
        row->clickOrigin = 0;
        row->selected    = 0;
        row->wasSelected = 0;
    }
}

