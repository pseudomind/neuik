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
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_ProgressBar.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int   neuik__isInitialized;
extern float neuik__HighDPI_Scaling;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ProgressBar(void ** wPtr);
int neuik_Object_Free__ProgressBar(void * wPtr);
int neuik_Element_GetMinSize__ProgressBar(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ProgressBar(NEUIK_Element, SDL_Event*);
int neuik_Element_Render__ProgressBar(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ProgressBar_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ProgressBar,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ProgressBar,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ProgressBar_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__ProgressBar,

    /* Render(): Redraw the element */
    neuik_Element_Render__ProgressBar,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__ProgressBar,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ProgressBar
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ProgressBar()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ProgressBar";
    static char  * errMsgs[]  = {"",                       // [0] no error
        "NEUIK library must be initialized first.",        // [1]
        "Failed to register `ProgressBar` object class .", // [2]
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
        "ProgressBar",                             // className
        "A GUI which displays activity progress.", // classDescription
        neuik__Set_NEUIK,                          // classSet
        neuik__Class_Element,                      // superClass
        &neuik_ProgressBar_BaseFuncs,              // baseFuncs
        NULL,                                      // classFuncs
        &neuik__Class_ProgressBar))                // newClass
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
 *  Name:          neuik_Object_New__ProgressBar
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ProgressBar(
    void ** pbPtr)
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_ProgressBar * pb         = NULL;
    NEUIK_Element     * sClassPtr  = NULL;

    static char         funcName[] = "neuik_Object_New__ProgressBar";
    static char       * errMsgs[]  = {"",                         // [0] no error
        "Failure to allocate memory.",                            // [1]
        "Failure in NEUIK_NewProgressBarConfig.",                 // [2]
        "Output Argument `pbPtr` is NULL.",                       // [3]
        "Failure in function `neuik_Object_New`.",                // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",      // [5]
        "Failure in `neuik_GetObjectBaseOfClass`.",               // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorGradient`.", // [7]
    };

    if (pbPtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*pbPtr) = (NEUIK_ProgressBar*) malloc(sizeof(NEUIK_ProgressBar));
    pb = *pbPtr;
    if (pb == NULL)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ProgressBar, 
            NULL,
            &(pb->objBase)))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(pb->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_ProgressBar_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    /* Allocation successful */
    pb->selected     = 0;
    pb->wasSelected  = 0;
    pb->isActive     = 0;
    pb->clickOrigin  = 0;
    pb->needsRedraw  = 1;
    pb->cfg          = NULL;
    pb->cfgPtr       = NULL;


    if (NEUIK_NewProgressBarConfig(&pb->cfg))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the new ProgressBar text contents                                  */
    /*------------------------------------------------------------------------*/
    pb->frac = 0.0;
    if (pb->cfg->decimalPlaces == 0)
    {
        strcpy(pb->fracText, "0%");
    }
    else
    {
        strcpy(pb->fracText, "0.0%");
    }

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorGradient(pb, "normal", 'v',
        "103,150,166,255,0.0",
        "70,120,166,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(pb, "selected", 'v',
        "103,150,166,255,0.0",
        "70,120,166,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(pb, "hovered", 'v',
        "103,150,166,255,0.0",
        "70,120,166,255,1.0",
        NULL))
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
 *  Name:          neuik_Object_Free__ProgressBar
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ProgressBar(
    void * objPtr)  /* [out] the object to free */
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_ProgressBar * pb         = NULL;
    static char         funcName[] = "neuik_Object_Free__ProgressBar";
    static char       * errMsgs[]  = {"",                 // [0] no error
        "Argument `objPtr` is not of ProgressBar class.", // [1]
        "Failure in function `neuik_Object_Free`.",       // [2]
        "Argument `objPtr` is NULL.",                     // [3]
    };

    if (objPtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    if (!neuik_Object_IsClass(objPtr, neuik__Class_ProgressBar))
    {
        eNum = 1;
        goto out;
    }
    pb = (NEUIK_ProgressBar*)objPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(pb->objBase.superClassObj))
    {
        eNum = 2;
        goto out;
    }
    if(neuik_Object_Free((void**)pb->cfg))
    {
        eNum = 2;
        goto out;
    }

    free(pb);
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
 *  Name:          neuik_Element_GetMinSize__ProgressBar
 *
 *  Description:   Returns the rendered size of a given ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ProgressBar(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int                       tW;
    int                       tH;
    int                       eNum = 0;    /* which error to report (if any) */
    TTF_Font                * font = NULL;
    NEUIK_ProgressBar       * pb   = NULL;
    NEUIK_ProgressBarConfig * aCfg = NULL; /* the active ProgressBar config */
    static char               funcName[] = "neuik_Element_GetMinSize__ProgressBar";
    static char * errMsgs[] = {"",                      // [0] no error
        "Argument `elem` is not of ProgressBar class.", // [1]
        "ProgressBarConfig* is NULL.",                  // [2]
        "ProgressBarConfig->FontSet is NULL.",          // [3]
        "FontSet_GetFont returned NULL.",               // [4]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(elem, neuik__Class_ProgressBar))
    {
        eNum = 1;
        goto out;
    }
    pb = (NEUIK_ProgressBar*)elem;
    
    /* select the correct ProgressBar config to use (pointer or internal) */
    if (pb->cfgPtr != NULL)
    {
        aCfg = pb->cfgPtr;
    }
    else 
    {
        aCfg = pb->cfg;
    }

    if (aCfg == NULL)
    {
        eNum = 2;
        goto out;
    } 

    if (aCfg->fontSet == NULL)
    {
        eNum = 3;
        goto out;
    }

    font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize, 
        aCfg->fontBold, aCfg->fontItalic);
    if (font == NULL) 
    {
        eNum = 4;
        goto out;
    }

    if (strlen(pb->fracText) > 0)
    {
        /* this ProgressBar contains text */
        TTF_SizeText(font, pb->fracText, &tW, &tH);

    }
    else
    {
        /* this ProgressBar does not contain text */
        TTF_SizeText(font, " ", &tW, &tH);
    }

    rSize->w = tW + aCfg->fontEmWidth;
    rSize->h = (int)(1.5 * (float)TTF_FontHeight(font));

    if (neuik__HighDPI_Scaling >= 2.0)
    {
        /*--------------------------------------------------------------------*/
        /* Add in additional pixels of width/height to accomodate for thicker */
        /* border lines.                                                      */
        /*--------------------------------------------------------------------*/
        rSize->w += 2*(int)(neuik__HighDPI_Scaling/2.0);
        rSize->h += 2*(int)(neuik__HighDPI_Scaling/2.0);
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
 *  Name:          NEUIK_NewProgressBar
 *
 *  Description:   Create a new NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewProgressBar(
    NEUIK_ProgressBar ** pbPtr)  /* [out] The newly created object. */
{
    return neuik_Object_New__ProgressBar((void **)pbPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ProgressBar_GetFraction
 *
 *  Description:   Return the current fraction of an NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ProgressBar_GetFraction(
    NEUIK_ProgressBar  * pb,
    double             * frac)
{
    int           eNum = 0;    /* which error to report (if any) */
    static char   funcName[] = "NEUIK_ProgressBar_GetFraction";
    static char * errMsgs[] = {"",                    // [0] no error
        "Argument `pb` is not of ProgressBar class.", // [1]
    };

    if (!neuik_Object_IsClass(pb, neuik__Class_ProgressBar))
    {
        eNum = 1;
        goto out;
    }

    (*frac) = pb->frac;
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
 *  Name:          NEUIK_ProgressBar_SetFraction
 *
 *  Description:   Update the fraction of a NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ProgressBar_SetFraction(
    NEUIK_ProgressBar * pb,
    double              frac)
{
    int                       eNum = 0;    /* which error to report (if any) */
    RenderSize                rSize;
    RenderLoc                 rLoc;
    NEUIK_ProgressBarConfig * aCfg = NULL; /* the active ProgressBar config */
    static char               funcName[] = "NEUIK_ProgressBar_SetFraction";
    static char             * errMsgs[] = {"",              // [0] no error
        "Argument `pb` is not of ProgressBar class.",       // [1]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [2]
    };

    if (!neuik_Object_IsClass(pb, neuik__Class_ProgressBar))
    {
        eNum = 1;
        goto out;
    }

    if (frac != pb->frac)
    {
        /*--------------------------------------------------------------------*/
        /* ProgressBar fraction value has changed, updated text and request a */
        /* redraw.                                                            */
        /*--------------------------------------------------------------------*/
        pb->frac = frac;

        /*--------------------------------------------------------------------*/
        /* select the correct ProgressBar config to use (pointer or internal) */
        /*--------------------------------------------------------------------*/
        if (pb->cfgPtr != NULL)
        {
            aCfg = pb->cfgPtr;
        }
        else 
        {
            aCfg = pb->cfg;
        }

        if (aCfg->decimalPlaces == 0)
        {
            sprintf(pb->fracText, "%d%%", (unsigned int)(100.0*pb->frac));
        }
        else
        {
            sprintf(pb->fracText, "%.2f%%", 100.0*pb->frac);
        }

        if (neuik_Element_GetSizeAndLocation(pb, &rSize, &rLoc))
        {
            eNum = 2;
            goto out;
        }
        neuik_Element_RequestRedraw(pb, rLoc, rSize);
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
 *  Name:          neuik_Element_Render__ProgressBar
 *
 *  Description:   Renders a single ProgressBar as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__ProgressBar(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                       ctr     = 0;
    int                       borderW = 1; /* width of button border line */
    int                       progW   = 0; /* pixel width of entire shadable region */
    int                       shadeW  = 0; /* width of shaded progress bar region */
    int                       textW   = 0;
    int                       textH   = 0;
    int                       eNum    = 0; /* which error to report (if any) */
    SDL_Renderer            * rend    = NULL;
    SDL_Texture             * gTex    = NULL; /* gradient progress texture */
    SDL_Texture             * tTex    = NULL; /* text texture */
    TTF_Font                * font    = NULL;
    SDL_Rect                  rect;
    NEUIK_ProgressBar       * pb      = NULL;
    NEUIK_ElementBase       * eBase   = NULL;
    colorDeltas             * deltaPP = NULL;
    RenderLoc                 rl;
    neuik_MaskMap           * maskMap = NULL;
    const NEUIK_Color       * fgClr   = NULL;
    const NEUIK_Color       * bgClr   = NULL;
    const NEUIK_Color       * bClr    = NULL; /* border color */
    NEUIK_ProgressBarConfig * aCfg    = NULL; /* the active ProgressBar config */
    static char               funcName[] = "neuik_Element_Render__ProgressBar";
    static char             * errMsgs[] = {"", // [0] no error
        "Argument `elem` is not of ProgressBar class.",                  // [1]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Invalid specified `rSize` (negative values).",                  // [3]
        "Failure in `neuik_Element_RedrawBackground()`.",                // [4]
        "FontSet_GetFont returned NULL.",                                // [5]
        "RenderText returned NULL.",                                     // [6]
        "Failure in `neuik_MakeMaskMap()`",                              // [7]
    };

    if (!neuik_Object_IsClass(elem, neuik__Class_ProgressBar))
    {
        eNum = 1;
        goto out;
    }
    pb = (NEUIK_ProgressBar*)elem;

    if (neuik_Object_GetClassObject(pb, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 3;
        goto out;
    }
    if (mock)
    {
        /*--------------------------------------------------------------------*/
        /* This is a mock render operation; don't draw anything...            */
        /*--------------------------------------------------------------------*/
        goto out;
    }

    eBase->eSt.rend = xRend;
    rend = eBase->eSt.rend;

    if (neuik__HighDPI_Scaling >= 2.0)
    {
        borderW = 2*(int)(neuik__HighDPI_Scaling/2.0);
    }

    /*------------------------------------------------------------------------*/
    /* select the correct ProgressBar config to use (pointer or internal)     */
    /*------------------------------------------------------------------------*/
    aCfg = pb->cfg;
    if (pb->cfgPtr != NULL)
    {
        aCfg = pb->cfgPtr;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    rl = eBase->eSt.rLoc;
    bgClr = &(aCfg->bgColor);
    fgClr = &(aCfg->fgColor);

    if (pb->frac == 0.0)
    {
        /*--------------------------------------------------------------------*/
        /* Currently the progress bare is completely "unfinished".            */
        /*--------------------------------------------------------------------*/
        rect.x = rl.x + 1;
        rect.y = rl.y + 1;
        rect.w = rSize->w - 2;
        rect.h = rSize->h - 2;

        SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
        SDL_RenderFillRect(rend, &rect);
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* Create a MaskMap an mark off the trasnparent pixels.               */
        /*--------------------------------------------------------------------*/
        if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
        {
            eNum = 7;
            goto out;
        }

        /*--------------------------------------------------------------------*/
        /* Mark off the rounded sections of the ProgressBar within the        */
        /* MaskMap.                                                           */
        /*--------------------------------------------------------------------*/
        /* Apply transparent pixels to (round off) the upper-left corner */
        neuik_MaskMap_MaskPoint(maskMap, 0, 0);
        neuik_MaskMap_MaskPoint(maskMap, 0, 1);
        neuik_MaskMap_MaskPoint(maskMap, 1, 0);

        /* Apply transparent pixels to (round off) the lower-left corner */
        neuik_MaskMap_MaskPoint(maskMap, 0, rSize->h - 1);
        neuik_MaskMap_MaskPoint(maskMap, 0, rSize->h - 2);
        neuik_MaskMap_MaskPoint(maskMap, 1, rSize->h - 1);

        /* Apply transparent pixels to (round off) the upper-right corner */
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 1, 0);
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 1, 1);
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 2, 0);

        /* Apply transparent pixels to (round off) the lower-right corner */
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 1, rSize->h - 1);
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 1, rSize->h - 2);
        neuik_MaskMap_MaskPoint(maskMap, rSize->w - 2, rSize->h - 1);

        /*--------------------------------------------------------------------*/
        /* The progress bar is "in-progress"; draw in the background gradient */
        /*--------------------------------------------------------------------*/
        if (neuik_Element_RedrawBackground(elem, rlMod, maskMap))
        {
            eNum = 4;
            goto out;
        }

        /*--------------------------------------------------------------------*/
        /* Cover up the "unfinished" progress section of the progress bar.    */
        /*--------------------------------------------------------------------*/
        progW  = (rSize->w - 2);
        shadeW = (int)((1.0 - pb->frac) * (double)(progW));

        rect.x = (rl.x + 1 + progW) - shadeW;
        rect.y = rl.y + 1;
        rect.w = shadeW;
        rect.h = (rSize->h - 2);

        SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
        SDL_RenderFillRect(rend, &rect);
    }

    /*------------------------------------------------------------------------*/
    /* Draw the border around the ProgressBar.                                */
    /*------------------------------------------------------------------------*/
    bClr = &(aCfg->borderColor);
    SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

    /* Draw upper-left corner border pixels */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawPoint(rend, rl.x + 1 + ctr, rl.y + 1 + ctr);
        SDL_RenderDrawPoint(rend, rl.x + 1 + ctr, rl.y + 2 + ctr);
        SDL_RenderDrawPoint(rend, rl.x + 2 + ctr, rl.y + 1 + ctr);
    }

    /* Draw lower-left corner border pixels */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawPoint(rend, rl.x + 1 + ctr, rl.y + (rSize->h - 2) - ctr);
        SDL_RenderDrawPoint(rend, rl.x + 1 + ctr, rl.y + (rSize->h - 3) - ctr);
        SDL_RenderDrawPoint(rend, rl.x + 2 + ctr, rl.y + (rSize->h - 2) - ctr);
    }

    /* Draw upper-right corner border pixels */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2) - ctr, rl.y + 1 + ctr);
        SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2) - ctr, rl.y + 2 + ctr);
        SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 3) - ctr, rl.y + 1 + ctr);
    }

    /* Draw lower-right corner border pixels */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawPoint(rend, 
            rl.x + (rSize->w - 2) - ctr, 
            rl.y + (rSize->h - 2) - ctr);
        SDL_RenderDrawPoint(rend, 
            rl.x + (rSize->w - 2) - ctr, 
            rl.y + (rSize->h - 3) - ctr);
        SDL_RenderDrawPoint(rend, 
            rl.x + (rSize->w - 3) - ctr, 
            rl.y + (rSize->h - 2) - ctr);
    }

    /* upper border line */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            rl.x + 2,              rl.y + ctr, 
            rl.x + (rSize->w - 3), rl.y + ctr); 
    }

    /* left border line */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            rl.x + ctr, rl.y + 2, 
            rl.x + ctr, rl.y + (rSize->h - 3));
    }
    /* right border line */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            rl.x + (rSize->w - 1) - ctr, rl.y + 2, 
            rl.x + (rSize->w - 1) - ctr, rl.y + (rSize->h - 3));
    }

    /* lower border line */
    bClr = &(aCfg->borderColorDark);
    SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            rl.x + 2 + ctr,              rl.y + (rSize->h - 1) - ctr, 
            rl.x + (rSize->w - 3) - ctr, rl.y + (rSize->h - 1) - ctr);
    }

    /*------------------------------------------------------------------------*/
    /* Render the ProgressBar text                                            */
    /*------------------------------------------------------------------------*/
    if (strlen(pb->fracText) > 0)
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
        if (font == NULL) 
        {
            eNum = 5;
            goto out;
        }

        tTex = NEUIK_RenderText(pb->fracText, font, *fgClr, rend, &textW, &textH);
        if (tTex == NULL)
        {
            eNum = 6;
            goto out;
        }

        rect.x = rl.x;
        rect.y = rl.y;
        rect.w = textW;
        rect.h = textH;

        switch (eBase->eCfg.HJustify)
        {
            case NEUIK_HJUSTIFY_LEFT:
                rect.x += 6;
                rect.y += (int) ((float)(rSize->h - textH)/2.0);
                break;

            case NEUIK_HJUSTIFY_CENTER:
            case NEUIK_HJUSTIFY_DEFAULT:
                rect.x += (int) ((float)(rSize->w - textW)/2.0);
                rect.y += (int) ((float)(rSize->h - textH)/2.0);
                break;

            case NEUIK_HJUSTIFY_RIGHT:
                rect.x += (int) (rSize->w - textW - 6);
                rect.y += (int) ((float)(rSize->h - textH)/2.0);
                break;
        }

        SDL_RenderCopy(rend, tTex, NULL, &rect);
    }
out:
    if (eBase != NULL)
    {
        if (!mock) eBase->eSt.doRedraw = 0;
    }

    ConditionallyDestroyTexture(&tTex);
    ConditionallyDestroyTexture(&gTex);
    if (maskMap != NULL) neuik_Object_Free(maskMap);
    if (deltaPP != NULL) free(deltaPP);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__ProgressBar
 *
 *  Description:   Check to see if this event is captured by the element.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ProgressBar(
    NEUIK_Element   elem,
    SDL_Event     * ev)
{
    neuik_EventState       evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
    RenderSize             rSize;
    RenderLoc              rLoc;
    NEUIK_ProgressBar    * pb         = NULL;
    NEUIK_ElementBase    * eBase      = NULL;
    SDL_Event            * e;
    SDL_MouseMotionEvent * mouseMotEv;
    SDL_MouseButtonEvent * mouseButEv;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        /* not the right type of object */
        goto out;
    }
    pb = (NEUIK_ProgressBar *)elem;

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the menu (mouseclick/mousemotion).   */
    /*------------------------------------------------------------------------*/
    e = (SDL_Event*)ev;
    switch (e->type)
    {
    case SDL_MOUSEBUTTONDOWN:
        mouseButEv = (SDL_MouseButtonEvent*)(e);
        if (mouseButEv->y >= eBase->eSt.rLoc.y &&
            mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
        {
            if (mouseButEv->x >= eBase->eSt.rLoc.x &&
                mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
            {
                /* This mouse click originated within this ProgressBar */
                pb->clickOrigin = 1;
                pb->selected    = 1;
                pb->wasSelected = 1;
                neuik_Window_TakeFocus(eBase->eSt.window, pb);

                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(pb, rLoc, rSize);
                neuik_Element_TriggerCallback(pb, NEUIK_CALLBACK_ON_CLICK);
                evCaputred = NEUIK_EVENTSTATE_CAPTURED;
                if (!neuik_Object_IsNEUIKObject_NoError(pb))
                {
                    /* The object was freed/corrupted by the callback */
                    evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                    goto out;
                }
                goto out;
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        mouseButEv = (SDL_MouseButtonEvent*)(e);
        if (pb->clickOrigin)
        {
            if (mouseButEv->y >= eBase->eSt.rLoc.y && 
                mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseButEv->x >= eBase->eSt.rLoc.x && 
                    mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    /* cursor is still within the ProgressBar, activate cbFunc */
                    neuik_Element_TriggerCallback(pb, NEUIK_CALLBACK_ON_CLICKED);
                    if (!neuik_Object_IsNEUIKObject_NoError(pb))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                        goto out;
                    }
                }
            }
            pb->selected    = 0;
            pb->wasSelected = 0;
            pb->clickOrigin = 0;

            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            neuik_Element_RequestRedraw(pb, rLoc, rSize);
            evCaputred = NEUIK_EVENTSTATE_CAPTURED;
            goto out;
        }
        break;

    case SDL_MOUSEMOTION:
        mouseMotEv = (SDL_MouseMotionEvent*)(e);

        if (pb->clickOrigin)
        {
            /*----------------------------------------------------------------*/
            /* The mouse was initially clicked within the ProgressBar. If the */
            /* user moves the cursor out of the ProgressBar area, deselect    */
            /* it.                                                            */
            /*----------------------------------------------------------------*/
            pb->selected = 0;
            if (mouseMotEv->y >= eBase->eSt.rLoc.y && 
                mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseMotEv->x >= eBase->eSt.rLoc.x && 
                    mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    pb->selected = 1;
                }
            }

            if (pb->wasSelected != pb->selected)
            {
                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(pb, rLoc, rSize);
            }
            pb->wasSelected = pb->selected;
            evCaputred = NEUIK_EVENTSTATE_CAPTURED;
            goto out;
        }

        break;
    }

out:
    return evCaputred;
}

