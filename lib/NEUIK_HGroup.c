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
#include "NEUIK_HGroup.h"
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
int neuik_Object_New__HGroup(void ** hgPtr);
int neuik_Object_Free__HGroup(void * hgPtr);

int neuik_Element_GetMinSize__HGroup(NEUIK_Element, RenderSize*);
int neuik_Element_Render__HGroup(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_HGroup_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__HGroup,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__HGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_HGroup_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__HGroup,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__HGroup,

    /* CaptureEvent(): Determine if this element caputures a given event */
    NULL,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_HGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_HGroup()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_HGroup";
    static char  * errMsgs[]  = {"",                  // [0] no error
        "NEUIK library must be initialized first.",   // [1]
        "Failed to register `HGroup` object class .", // [2]
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
        "NEUIK_HGroup",                                          // className
        "An element container which horizontally groups items.", // classDescription
        neuik__Set_NEUIK,                                        // classSet
        neuik__Class_Container,                                  // superClass
        &neuik_HGroup_BaseFuncs,                                 // baseFuncs
        NULL,                                                    // classFuncs
        &neuik__Class_HGroup))                                   // newClass
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
 *  Name:          neuik_Object_New__HGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__HGroup(
        void ** hgPtr)
{
    int               eNum       = 0;
    NEUIK_Container * cont       = NULL;
    NEUIK_HGroup    * hg         = NULL;
    NEUIK_Element   * sClassPtr  = NULL;
    static char       funcName[] = "neuik_Object_New__HGroup";
    static char     * errMsgs[]  = {"",                                   // [0] no error
        "Output Argument `hgPtr` is NULL.",                               // [1]
        "Failure to allocate memory.",                                    // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
        "Failure in function `neuik.NewElement`.",                        // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",              // [5]
        "Argument `hgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",      // [7]
    };

    if (hgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*hgPtr) = (NEUIK_HGroup*) malloc(sizeof(NEUIK_HGroup));
    hg = *hgPtr;
    if (hg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /* Allocation successful */
    hg->HSpacing = 1;
    hg->isActive = 0;

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_HGroup, 
            NULL,
            &(hg->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(hg->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Container, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(hg, &neuik_HGroup_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_Object_GetClassObject(hg, neuik__Class_Container, (void**)&cont))
    {
        eNum = 6;
        goto out;
    }
    cont->cType        = NEUIK_CONTAINER_MULTI;
    cont->shownIfEmpty = 0;

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorTransparent(cont, "normal"))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorTransparent(cont, "selected"))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorTransparent(cont, "hovered"))
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
 *  Name:          NEUIK_NewHGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_HGroup.
 *
 *                 Wrapper function to neuik_Object_New__HGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewHGroup(
    NEUIK_HGroup ** hgPtr)
{
    return neuik_Object_New__HGroup((void**)hgPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_HGroup_SetHSpacing
 *
 *  Description:   Set the horizontal spacing parameter of a horizontal group.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_HGroup_SetHSpacing(
    NEUIK_HGroup  * hg,
    int             spacing)
{
    int            eNum       = 0;    /* which error to report (if any) */
    static char    funcName[] = "NEUIK_HGroup_SetHSpacing";
    static char  * errMsgs[]  = {"",               // [0] no error
        "Argument `hg` is not of VGroup class.",   // [1]
        "Argument `spacing` can not be negative.", // [2]
    };

    if (!neuik_Object_IsClass(hg, neuik__Class_HGroup))
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
    if (spacing == hg->HSpacing) goto out;

    hg->HSpacing = spacing;
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
 *  Name:          neuik_Object_Free__HGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__HGroup(
    void * hgPtr)
{
    int            eNum       = 0;    /* which error to report (if any) */
    NEUIK_HGroup * hg         = NULL;
    static char    funcName[] = "neuik_Object_Free__HGroup";
    static char  * errMsgs[]  = {"",                // [0] no error
        "Argument `hgPtr` is NULL.",                // [1]
        "Argument `hgPtr` is not of HGroup class.", // [2]
        "Failure in function `neuik_Object_Free`.", // [3]
    };

    if (hgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(hgPtr, neuik__Class_HGroup))
    {
        eNum = 2;
        goto out;
    }
    hg = (NEUIK_HGroup*)hgPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(hg->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    free(hg);
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
 *  Name:          neuik_Element_GetMinSize__HGroup
 *
 *  Description:   Returns the rendered size of a given HGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__HGroup(
    NEUIK_Element    hgElem,
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
    NEUIK_HGroup        * hg         = NULL;
    static char           funcName[] = "neuik_Element_GetMinSize__HGroup";
    static char         * errMsgs[]  = {"",                                // [0] no error
        "Argument `hgElem` is not of HGroup class.",                       // [1]
        "Element_GetMinSize Failed.",                                      // [2]
        "Element_GetConfig returned NULL.",                                // [3]
        "Argument `hgElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
        "Failure to allocate memory.",                                     // [5]
        "Unexpected NULL... Investigate.",                                 // [6]
    };

    rSize->w = 0;
    rSize->h = 0;

    /*------------------------------------------------------------------------*/
    /* Check for problems before proceding                                    */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(hgElem, neuik__Class_HGroup))
    {
        eNum = 1;
        goto out;
    }
    hg = (NEUIK_HGroup*)hgElem;

    if (neuik_Object_GetClassObject(hgElem, neuik__Class_Container, (void**)&cont))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Object_GetClassObject(hgElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 4;
        goto out;
    }

    if (cont->elems == NULL) {
        /* there are no UI elements contained by this HGroup */
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
                thisW += (float)(hg->HSpacing);
            }
            else
            {
                thisW += (float)(hg->HSpacing)*neuik__HighDPI_Scaling;
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
 *  Name:          neuik_Element_Render__HGroup
 *
 *  Description:   Renders a horizontal group of elements.
 *
 *  Returns:       0 if there were no issues; otherwise 1.
 *
 ******************************************************************************/
int neuik_Element_Render__HGroup(
    NEUIK_Element   hgElem,
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
    SDL_Renderer         * rend          = NULL;
    NEUIK_Container      * cont          = NULL;
    NEUIK_ElementBase    * eBase         = NULL;
    NEUIK_Element          elem          = NULL;
    NEUIK_ElementConfig  * eCfg          = NULL;
    NEUIK_HGroup         * hg            = NULL;
    neuik_MaskMap        * maskMap       = NULL; /* FREE upon return */
    enum neuik_bgstyle     bgStyle;
    static char            funcName[]    = "neuik_Element_Render__HGroup";
    static char          * errMsgs[]     = {"", // [0] no error
        "Argument `hgElem` is not of HGroup class.",                       // [1]
        "Argument `hgElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
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

    if (!neuik_Object_IsClass(hgElem, neuik__Class_HGroup))
    {
        eNum = 1;
        goto out;
    }
    hg = (NEUIK_HGroup*)hgElem;

    if (neuik_Object_GetClassObject(hgElem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (neuik_Object_GetClassObject(hgElem, neuik__Class_Container, (void**)&cont))
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
        fltHspacingSc = (float)(hg->HSpacing);
    }
    else
    {
        fltHspacingSc = (float)(hg->HSpacing)*neuik__HighDPI_Scaling;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (!mock)
    {
        if (neuik_Element_GetCurrentBGStyle(hgElem, &bgStyle))
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

            if (neuik_Element_RedrawBackground(hgElem, rlMod, maskMap))
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
