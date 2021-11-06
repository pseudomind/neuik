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
#include "NEUIK_ListRow.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Container.h"
#include "NEUIK_Container_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int   neuik__isInitialized;
extern float neuik__HighDPI_Scaling;

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

    /* RequestRedraw(): This function will be called when redraw is requested */
    NULL,
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

    int                    nAlloc     = 0;
    int                  * elemsShown = NULL; // Free upon returning.
    RenderSize           * elemsMinSz = NULL; // Free upon returning.
    NEUIK_ElementConfig ** elemsCfg   = NULL; // Free upon returning.

    RenderSize          * rs         = NULL;
    NEUIK_Element         elem       = NULL;
    NEUIK_ElementBase   * eBase      = NULL;
    NEUIK_ElementConfig * eCfg       = NULL;
    NEUIK_Container     * cont       = NULL;
    NEUIK_ListRow       * row        = NULL;
    static char           funcName[] = "neuik_Element_GetMinSize__ListRow";
    static char         * errMsgs[]  = {"", // [0] no error
        "Argument `rowElem` is not of ListRow class.",                      // [1]
        "Element_GetMinSize Failed.",                                       // [2]
        "Element_GetConfig returned NULL.",                                 // [3]
        "Argument `rowElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
        "Failure to allocate memory.",                                      // [5]
        "Unexpected NULL... Investigate.",                                  // [6]
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
    /* Determine the number of elements within the container.                 */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = cont->elems[ctr];
        if (elem == NULL) break;
    }
    nAlloc = ctr;

    /*------------------------------------------------------------------------*/
    /* Allocate memory for lists of contained element properties.             */
    /*------------------------------------------------------------------------*/
    elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
    if (elemsCfg == NULL)
    {
        eNum = 5;
        goto out;
    }
    elemsShown = malloc(nAlloc*sizeof(int));
    if (elemsShown == NULL)
    {
        eNum = 5;
        goto out;
    }
    elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
    if (elemsMinSz == NULL)
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Store the current properties for the contained elements.               */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        elem = cont->elems[ctr];
        if (elem == NULL)
        {
            eNum = 6;
            goto out;
        }

        elemsShown[ctr] = NEUIK_Element_IsShown(elem);
        if (!elemsShown[ctr]) continue;

        elemsCfg[ctr] = neuik_Element_GetConfig(elem);
        if (elemsCfg[ctr] == NULL)
        {
            eNum = 3;
            goto out;
        }

        if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
        {
            eNum = 2;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Determine the (maximum) height required by any one of the elements.    */
    /*                                                                        */
    /*    and                                                                 */
    /*                                                                        */
    /* Find the largest minimum width of all the horizontally filling items.  */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        /*--------------------------------------------------------------------*/
        /* Get the (maximum) height required by any one of the elements       */
        /*--------------------------------------------------------------------*/
        tempH = rs->h + (eCfg->PadTop + eCfg->PadBottom);
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
            thisMinW = rs->w;

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
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (!elemsShown[ctr]) continue; /* this elem isn't shown */
        vctr++;

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        if (vctr > 1)
        {
            /* subsequent UI element is valid, add Horizontal Spacing */
            if (neuik__HighDPI_Scaling <= 1.0)
            {
                thisW += (float)(row->HSpacing);
            }
            else
            {
                thisW += (float)(row->HSpacing)*neuik__HighDPI_Scaling;
            }
        }

        if (eCfg->HFill)
        {
            /* This element is fills space horizontally */
            thisW += (eCfg->HScale)*(float)(maxMinW);
        }
        else
        {
            thisW += (float)(rs->w);
        }
        thisW += (float)(eCfg->PadLeft + eCfg->PadRight);
    }

    rSize->w = (int)(thisW);
out:
    if (elemsCfg != NULL)   free(elemsCfg);
    if (elemsShown != NULL) free(elemsShown);
    if (elemsMinSz != NULL) free(elemsMinSz);

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
 *  Description:   Renders a horizontal row of list elements.
 *
 *  Returns:       0 if there were no issues; otherwise 1.
 *
 ******************************************************************************/
int neuik_Element_Render__ListRow(
    NEUIK_Element   rowElem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                    nAlloc        = 0;
    int                    tempH         = 0;
    int                    tempW         = 0;
    int                    ctr           = 0;
    int                    xFree         = 0; // px of space free for hFill elems
    int                    dW            = 0; // Change in width [px]
    int                    eNum          = 0; // which error to report (if any)
    int                    nHFill        = 0; // number of cols which can HFill
    int                    reqResizeW    = 0; // required resize width
    int                    hfillColsMinW = 0; // min width for all hFill cols
    int                    hfillMaxMinW  = 0; // largest minimum col width 
                                              // among vertically filling rows.
    int                  * allHFill      = NULL; // Free upon returning; 
                                                 // Cols fills vertically? (per col)
    int                  * allVFill      = NULL; // Free upon returning; 
                                                 // Row fills vertically? (per row)
    int                    maxMinH       = 0;    // The max min width (per row)
    int                  * allMaxMinW    = NULL; // Free upon returning; 
                                                 // The max min width (per column)
    int                  * rendColW      = NULL; // Free upon returning; 
                                                 // Rendered col width (per column)
    int                  * elemsShown    = NULL; // Free upon returning.
    float                  fltHspacingSc = 0.0;  // float HSpacing HighDPI scaled
    float                  xPos          = 0.0;
    RenderSize           * elemsMinSz    = NULL; // Free upon returning.
    NEUIK_ElementConfig ** elemsCfg      = NULL; // Free upon returning.
    RenderLoc              rl            = {0, 0};
    RenderLoc              rlRel         = {0, 0}; /* renderloc relative to parent */
    SDL_Rect               rect          = {0, 0, 0, 0};
    static RenderSize      rsZero        = {0, 0};
    RenderSize             rsMin         = {0, 0};
    RenderSize           * rs            = NULL;
    const NEUIK_Color    * bgClr         = NULL; /* background color */
    SDL_Renderer         * rend          = NULL;
    NEUIK_Container      * cont          = NULL;
    NEUIK_ElementBase    * eBase         = NULL;
    NEUIK_Element          elem          = NULL;
    NEUIK_ElementConfig  * eCfg          = NULL;
    NEUIK_ListRow        * row           = NULL;
    neuik_MaskMap        * maskMap       = NULL; /* FREE upon return */
    enum neuik_bgstyle     bgStyle;
    static char            funcName[]    = "neuik_Element_Render__ListRow";
    static char          * errMsgs[]     = {"", // [0] no error
        "Argument `rowElem` is not of HGroup class.",                       // [1]
        "Argument `rowElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Invalid specified `rSize` (negative values).",                    // [3]
        "Failure in `neuik_Element_GetCurrentBGStyle()`.",                 // [4]
        "Failure in `neuik_MakeMaskMap()`",                                // [5]
        "Failure in `neuik_Window_FillTranspMaskFromLoc()`",               // [6]
        "Failure in `neuik_Element_RedrawBackground()`.",                  // [7]
        "Failure to allocate memory.",                                     // [8]
        "Element_GetConfig returned NULL.",                                // [9]
        "Element_GetMinSize Failed.",                                      // [10]
        "Failure in `neuik_Element_Render()`",                             // [11]
    };

    if (!neuik_Object_IsClass(rowElem, neuik__Class_ListRow))
    {
        eNum = 1;
        goto out;
    }
    row = (NEUIK_ListRow*)rowElem;

    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (neuik_Object_GetClassObject(rowElem, neuik__Class_Container, (void**)&cont))
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
        fltHspacingSc = (float)(row->HSpacing);
    }
    else
    {
        fltHspacingSc = (float)(row->HSpacing)*neuik__HighDPI_Scaling;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik_Element_GetCurrentBGStyle(rowElem, &bgStyle))
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

            /*----------------------------------------------------------------*/
            /* Fill the row with the appropriate background color.            */
            /*----------------------------------------------------------------*/
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

            if (neuik_Element_RedrawBackground(rowElem, rlMod, maskMap))
            {
                eNum = 7;
                goto out;
            }
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

    allMaxMinW = malloc(nAlloc*sizeof(int));
    if (allMaxMinW == NULL)
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
    rendColW = malloc(nAlloc*sizeof(int));
    if (rendColW == NULL)
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Zero out the initial maximum minimum values and HFill/VFill flags.     */
    /*------------------------------------------------------------------------*/
    maxMinH = 0;
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        allHFill[ctr]   = 0;
        allMaxMinW[ctr] = 0;
        allVFill[ctr]   = 0;
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
    /* Calculate the maximum minimum heights required for all of the rows.    */
    /* (i.e., for each row of elements, determine the maximum value of the    */
    /* minimum heights required (among elements in the row)).                 */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        tempH = rs->h + (eCfg->PadTop + eCfg->PadBottom);
        if (tempH > maxMinH)
        {
            maxMinH = tempH;
        }

        /*--------------------------------------------------------------------*/
        /* Check if the element can fill horizontally and if so, mark the     */
        /* whole vgroup as one that can fill horizontally.                    */
        /*--------------------------------------------------------------------*/
        if (eCfg->HFill)
        {
            allHFill[ctr] = 1;
        }
        /*--------------------------------------------------------------------*/
        /* Check if the element can fill vertically and if so, mark the whole */
        /* vgroup as one that can fill vertically.                            */
        /*--------------------------------------------------------------------*/
        if (eCfg->VFill)
        {
            allVFill[ctr] = 1;
        }
    }

    /*========================================================================*/
    /* Calculation of rendered column widths (accounts for HFill).            */
    /*========================================================================*/
    /* Determine the required minimum width and the total number of columns   */
    /* which can fill horizontally.                                           */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        elem = (NEUIK_Element)cont->elems[ctr];
        if (elem == NULL) break;

        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        eCfg = elemsCfg[ctr];
        rs    = &elemsMinSz[ctr];
        tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
        allMaxMinW[ctr] = tempW;

        rsMin.w += tempW;
        nHFill += allHFill[ctr];
    }
    if (nAlloc > 1)
    {
        rsMin.w += (int)(fltHspacingSc*(float)(nAlloc - 1));
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the space occupied by all HFill columns and determine the    */
    /* value of the largest minimum column width among horizontally filling   */
    /* columns.                                                               */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (allHFill[ctr])
        {
            hfillColsMinW += allMaxMinW[ctr];
            if (hfillMaxMinW < allMaxMinW[ctr])
            {
                hfillMaxMinW = allMaxMinW[ctr];
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the amount of currently unused horizontal space (beyond min) */
    /*------------------------------------------------------------------------*/
    xFree = rSize->w - rsMin.w;

    /*------------------------------------------------------------------------*/
    /* Check if there is enough unused horizontal space to bring all HFill    */
    /* columns to the same width.                                             */
    /*------------------------------------------------------------------------*/
    reqResizeW = nHFill*hfillMaxMinW - hfillColsMinW; // required resize width
    if (xFree >= reqResizeW)
    {
        /*--------------------------------------------------------------------*/
        /* There is enough space; get all HFill cols to the same size first.  */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < nAlloc; ctr++)
        {
            rendColW[ctr] = allMaxMinW[ctr];
            if (allHFill[ctr])
            {
                rendColW[ctr] = hfillMaxMinW;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Evenly divide the remaining hSpace between the hFill columns.      */
        /*--------------------------------------------------------------------*/
        xFree -= reqResizeW;

        dW = (int)(floor( (float)(xFree)/(float)(nHFill) ));
        if (dW > 0)
        {
            /*----------------------------------------------------------------*/
            /* Increase the width of hFill columns all by the same quantity.  */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                if (allHFill[ctr])
                {
                    rendColW[ctr] += dW;
                    xFree -= dW;
                }
            }
        }

        if (xFree > 0)
        {
            /*----------------------------------------------------------------*/
            /* Distribute the remaining hSpace to the hFill one pixel at a    */
            /* time (starting from the left column and moving right).         */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                if (allHFill[ctr])
                {
                    rendColW[ctr] += 1;
                    xFree -= 1;
                    if (xFree == 0)
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
        /* Evenly divide the remaining hSpace between the hFill columns.      */
        /*--------------------------------------------------------------------*/
        while (xFree > 0)
        {
            /*----------------------------------------------------------------*/
            /* Distribute the remaining hSpace to the hFill one pixel at a    */
            /* time (starting from the left column and moving right).         */
            /*----------------------------------------------------------------*/
            for (ctr = 0; ctr < nAlloc; ctr++)
            {
                if (allHFill[ctr] && rendColW[ctr] < hfillMaxMinW)
                {
                    rendColW[ctr] += 1;
                    xFree -= 1;
                    if (xFree == 0)
                    {
                        break;
                    }
                }
            }
        }
    }

    /*========================================================================*/
    /* Render and place the child elements                                    */
    /*========================================================================*/
    xPos = 0.0;
    for (ctr = 0; ctr < nAlloc; ctr++)
    {
        if (ctr > 0)
        {
            xPos += (float)(rendColW[ctr-1]) + fltHspacingSc;
        }
        if (!elemsShown[ctr]) continue; /* this elem isn't shown */

        elem = cont->elems[ctr];
        if (!neuik_Element_NeedsRedraw(elem)) continue;

        eCfg = elemsCfg[ctr];
        rs   = &elemsMinSz[ctr];

        tempW = rendColW[ctr];

        /*----------------------------------------------------------------*/
        /* Check for and apply if necessary Horizontal and Vertical fill. */
        /*----------------------------------------------------------------*/
        if (allHFill[ctr])
        {
            rs->w = tempW - (eCfg->PadLeft + eCfg->PadRight);
        }
        if (allVFill[ctr])
        {
            rs->h = rSize->h - (eCfg->PadTop + eCfg->PadBottom);
        }

        /*----------------------------------------------------------------*/
        /* Update the stored location before rendering the element. This  */
        /* is necessary as the location of this object will propagate to  */
        /* its child objects.                                             */
        /*----------------------------------------------------------------*/
        switch (eCfg->HJustify)
        {
            case NEUIK_HJUSTIFY_DEFAULT:
                switch (cont->HJustify)
                {
                    case NEUIK_HJUSTIFY_LEFT:
                        rect.x = (int)(xPos) + eCfg->PadLeft;
                        break;
                    case NEUIK_HJUSTIFY_CENTER:
                    case NEUIK_HJUSTIFY_DEFAULT:
                        rect.x = ((int)(xPos) + rendColW[ctr]/2) - (tempW/2);
                        break;
                    case NEUIK_HJUSTIFY_RIGHT:
                        rect.x = ((int)(xPos) + rendColW[ctr]) - 
                            (rs->w + eCfg->PadRight);
                        break;
                }
                break;
            case NEUIK_HJUSTIFY_LEFT:
                rect.x = (int)(xPos) + eCfg->PadLeft;
                break;
            case NEUIK_HJUSTIFY_CENTER:
                rect.x = ((int)(xPos) + rendColW[ctr]/2) - (tempW/2);
                break;
            case NEUIK_HJUSTIFY_RIGHT:
                rect.x = ((int)(xPos) + rendColW[ctr]) - 
                    (rs->w + eCfg->PadRight);
                break;
        }
        switch (eCfg->VJustify)
        {
            case NEUIK_VJUSTIFY_DEFAULT:
                switch (cont->VJustify)
                {
                    case NEUIK_VJUSTIFY_TOP:
                        rect.y = eCfg->PadTop;
                        break;
                    case NEUIK_VJUSTIFY_CENTER:
                    case NEUIK_VJUSTIFY_DEFAULT:
                        rect.y = (rSize->h/2) - (rs->h/2);
                        break;
                    case NEUIK_VJUSTIFY_BOTTOM:
                        rect.y = rSize->h - (rs->h + eCfg->PadBottom);
                        break;
                }
                break;
            case NEUIK_VJUSTIFY_TOP:
                rect.y = eCfg->PadTop;
                break;
            case NEUIK_VJUSTIFY_CENTER:
                rect.y = (rSize->h/2) - (rs->h/2);
                break;
            case NEUIK_VJUSTIFY_BOTTOM:
                rect.y = rSize->h - (rs->h + eCfg->PadBottom);
                break;
        }

        rect.w = rendColW[ctr];
        rect.h = rs->h;
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
    if (allMaxMinW != NULL) free(allMaxMinW);
    if (rendColW   != NULL) free(rendColW);
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
            case SDLK_KP_ENTER:
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

