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
#include "NEUIK_Fill.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Fill(void **);
int neuik_Object_Free__Fill(void *);

int neuik_Element_GetMinSize__Fill(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Fill(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Fill_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__Fill,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__Fill,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Fill_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__Fill,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__Fill,

    /* CaptureEvent(): Determine if this element caputures a given event */
    NULL,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Fill
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Fill()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_Fill";
    static char  * errMsgs[]  = {"",                // [0] no error
        "NEUIK library must be initialized first.", // [1]
        "Failed to register `Fill` object class .", // [2]
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
        "NEUIK_Fill",                                         // className
        "An element which fills vertically or horizontally.", // classDescription
        neuik__Set_NEUIK,                                     // classSet
        neuik__Class_Element,                                 // superClass
        &neuik_Fill_BaseFuncs,                                // baseFuncs
        NULL,                                                 // classFuncs
        &neuik__Class_Fill))                                  // newClass
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
 *  Name:          neuik_Object_New__Fill
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Fill(
        void ** fillPtr)
{
    int             eNum       = 0;
    NEUIK_Fill    * fill       = NULL;
    NEUIK_Element * sClassPtr  = NULL;
    static char     funcName[] = "neuik_Object_New__Fill";
    static char   * errMsgs[]  = {"",                                // [0] no error
        "Output Argument `fillPtr` is NULL.",                        // [1]
        "Failure to allocate memory.",                               // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.",                  // [3]
        "Failure in function `neuik_Object_New`.",                   // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",         // [5]
        "Failure in `NEUIK_Element_SetBackgroundColorTransparent`.", // [6]
    };

    if (fillPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*fillPtr) = (NEUIK_Fill*) malloc(sizeof(NEUIK_Fill));
    fill = *fillPtr;
    if (fill == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_Fill, 
            NULL,
            &(fill->objBase)))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(fill->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(fill, &neuik_Fill_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorTransparent(fill, "normal"))
    {
        eNum = 6;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorTransparent(fill, "selected"))
    {
        eNum = 6;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorTransparent(fill, "hovered"))
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
 *  Name:          neuik_Object_Free__Fill
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Fill(
    void  * fillPtr)
{
    int           eNum       = 0;    /* which error to report (if any) */
    NEUIK_Fill  * fill       = NULL;
    static char   funcName[] = "neuik_Object_Free__Fill";
    static char * errMsgs[]  = {"",                       // [0] no error
        "Argument `fillPtr` is NULL.",                    // [1]
        "Argument `fillPtr` is not of NEUIK_Fill class.", // [2]
        "Failure in function `neuik_Object_Free`.",       // [3]
    };

    if (fillPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(fillPtr, neuik__Class_Fill))
    {
        eNum = 2;
        goto out;
    }
    fill = (NEUIK_Fill*)fillPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(fill->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    free(fill);
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
 *  Name:          NEUIK_NewHFill
 *
 *  Description:   Create a new horizontal NEUIK_Fill.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewHFill(
    NEUIK_Fill ** fillPtr)
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_Fill        * fill       = NULL;
    NEUIK_ElementBase * eBase      = NULL; 
    static char         funcName[] = "NEUIK_NewHFill";
    static char       * errMsgs[]  = {"",                                   // [0] no error
        "Failure in function `neuik_Object_New__Fill`.",                    // [1]
        "Argument `fillPtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
    };

    if (neuik_Object_New__Fill((void**)fillPtr))
    {
        eNum = 1;
        goto out;
    }
    fill = *fillPtr;

    /*------------------------------------------------------------------------*/
    /* Configure the fill to be horizontal                                    */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(fill, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    eBase->eCfg.HFill = 1;

    fill->orientation = 0;
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
 *  Name:          NEUIK_NewVFill
 *
 *  Description:   Create a new vertical NEUIK_Fill.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewVFill(
    NEUIK_Fill ** fillPtr)
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_Fill        * fill       = NULL;
    NEUIK_ElementBase * eBase      = NULL; 
    static char         funcName[] = "NEUIK_NewVFill";
    static char       * errMsgs[]  = {"",                                   // [0] no error
        "Failure in function `neuik_Object_New__Fill`.",                    // [1]
        "Argument `fillPtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
    };

    if (neuik_Object_New__Fill((void**)fillPtr))
    {
        eNum = 1;
        goto out;
    }
    fill = *fillPtr;

    /*------------------------------------------------------------------------*/
    /* Configure the fill to be horizontal                                    */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(fill, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    eBase->eCfg.VFill = 1;

    fill->orientation = 1;
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
 *  Name:          neuik_Element_GetMinSize__Fill
 *
 *  Description:   Returns the rendered size of a given fill.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Fill(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int           eNum       = 0;    /* which error to report (if any) */
    NEUIK_Fill  * fill       = NULL;
    static char   funcName[] = "neuik_Element_GetMinSize__Fill";
    static char * errMsgs[]  = {"",                    // [0] no error
        "Argument `elem` is not of NEUIK_Fill class.", // [1]
        "Invalid fill orientation.",                   // [2]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(elem, neuik__Class_Fill))
    {
        eNum = 1;
        goto out;
    }
    fill = (NEUIK_Fill*)elem;
    
    rSize->w = 1;
    rSize->h = 1;
    if (fill->orientation > 1)
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
 *  Name:          neuik_Element_Render__Fill
 *
 *  Description:   Renders the Fill element as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetMinSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Fill(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_Fill        * fill       = NULL;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_Render__Fill";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of NEUIK_Fill class.",                   // [1]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "", // [3]
        "Invalid specified `rSize` (negative values).",                  // [4]
        "Failure in `neuik_Element_RedrawBackground`.",                  // [6]
    };

    if (!neuik_Object_IsClass(elem, neuik__Class_Fill))
    {
        eNum = 1;
        goto out;
    }
    fill = (NEUIK_Fill*)elem;

    if (neuik_Object_GetClassObject(fill, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 4;
        goto out;
    }
    if (mock)
    {
        /*--------------------------------------------------------------------*/
        /* This is a mock render operation; don't draw anything...            */
        /*--------------------------------------------------------------------*/
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_RedrawBackground(elem, rlMod, NULL))
    {
        eNum = 5;
        goto out;
    }

out:
    if (eBase != NULL)
    {
        if (!mock) eBase->eSt.doRedraw = 0;
    }

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

