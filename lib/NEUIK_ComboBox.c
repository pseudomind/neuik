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

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_ComboBox.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ComboBox(void ** cbPtr);
int neuik_Object_Free__ComboBox(void * cbPtr);
int neuik_Element_GetMinSize__ComboBox(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ComboBox(NEUIK_Element, SDL_Event*);
int neuik_Element_Render__ComboBox(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ComboBox_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ComboBox,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ComboBox,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ComboBox_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__ComboBox,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__ComboBox,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__ComboBox,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ComboBox
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ComboBox()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ComboBox";
    static char  * errMsgs[]  = {"",                    // [0] no error
        "NEUIK library must be initialized first.",     // [1]
        "Failed to register `ComboBox` object class .", // [2]
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
        "ComboBox",                                    // className
        "A GUI button which toggles a dropdown menu.", // classDescription
        neuik__Set_NEUIK,                              // classSet
        neuik__Class_Element,                          // superClass
        &neuik_ComboBox_BaseFuncs,                     // baseFuncs
        NULL,                                          // classFuncs
        &neuik__Class_ComboBox))                       // newClass
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
 *  Name:          neuik_Object_New__ComboBox
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ComboBox(
    void ** cbPtr)
{
    int              eNum       = 0; /* which error to report (if any) */
    NEUIK_ComboBox * cb        = NULL;
    NEUIK_Element  * sClassPtr  = NULL;
    static char     funcName[] = "neuik_Object_New__ComboBox";
    static char    * errMsgs[]  = {"",                            // [0] no error
        "Failure to allocate memory.",                            // [1]
        "Failure in NEUIK_NewComboBoxConfig.",                    // [2]
        "Output Argument `cbPtr` is NULL.",                       // [3]
        "Failure in function `neuik_Object_New`.",                // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",      // [5]
        "Failure in `neuik_GetObjectBaseOfClass`.",               // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorGradient`.", // [7]
    };

    if (cbPtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*cbPtr) = (NEUIK_ComboBox*) malloc(sizeof(NEUIK_ComboBox));
    cb = *cbPtr;
    if (cb == NULL)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ComboBox, 
            NULL,
            &(cb->objBase)))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(cb->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_ComboBox_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    /* Allocation successful */
    cb->cfg          = NULL;
    cb->cfgPtr       = NULL;
    cb->aEntry       = NULL;
    cb->selected     = 0;
    cb->wasSelected  = 0;
    cb->isActive     = 0;
    cb->expanded     = 0;
    cb->clickOrigin  = 0;
    cb->needsRedraw  = 1;

    if (NEUIK_NewComboBoxConfig(&cb->cfg))
    {
        eNum = 2;
        goto out;
    }
    
    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorGradient(cb, "normal", 'v',
        "220,220,220,255,0.0",
        "200,200,200,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(cb, "selected", 'v',
        "120,120,120,255,0.0",
        "165,165,165,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(cb, "hovered", 'v',
        "220,220,220,255,0.0",
        "200,200,200,255,1.0",
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
 *  Name:          neuik_Object_Free__ComboBox
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ComboBox(
    void * cbPtr)  /* [out] the comboBox to free */
{
    int            eNum       = 0; /* which error to report (if any) */
    NEUIK_ComboBox * cb        = NULL;
    static char    funcName[] = "neuik_Object_Free__ComboBox";
    static char  * errMsgs[]  = {"",                  // [0] no error
        "Argument `cbPtr` is not of ComboBox class.", // [1]
        "Failure in function `neuik_Object_Free`.",   // [2]
        "Argument `cbPtr` is NULL.",                  // [3]
    };

    if (cbPtr == NULL)
    {
        eNum = 3;
        goto out;
    }
    cb = (NEUIK_ComboBox*)cbPtr;

    if (!neuik_Object_IsClass(cb, neuik__Class_ComboBox))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(cb->objBase.superClassObj))
    {
        eNum = 2;
        goto out;
    }
    if(cb->aEntry != NULL) free(cb->aEntry);
    if(neuik_Object_Free(cb->cfg))
    {
        eNum = 2;
        goto out;
    }

    free(cb);
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
 *  Name:          neuik_Element_GetMinSize__ComboBox
 *
 *  Description:   Returns the rendered size of a given comboBox.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ComboBox(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int                  tW;
    int                  tH;
    int                  eNum       = 0;    /* which error to report (if any) */
    TTF_Font           * font       = NULL;
    NEUIK_ComboBox       * cb        = NULL;
    NEUIK_ComboBoxConfig * aCfg       = NULL; /* the active comboBox config */
    static char          funcName[] = "neuik_Element_GetMinSize__ComboBox";
    static char        * errMsgs[]  = {"",           // [0] no error
        "Argument `elem` is not of ComboBox class.", // [1]
        "ComboBoxConfig* is NULL.",                  // [2]
        "ComboBoxConfig->FontSet is NULL.",          // [3]
        "FontSet_GetFont returned NULL.",            // [4]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(elem, neuik__Class_ComboBox))
    {
        eNum = 1;
        goto out;
    }
    cb = (NEUIK_ComboBox*)elem;
    
    /* select the correct comboBox config to use (pointer or internal) */
    if (cb->cfgPtr != NULL)
    {
        aCfg = cb->cfgPtr;
    }
    else 
    {
        aCfg = cb->cfg;
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

    if (cb->aEntry != NULL)
    {
        /* this comboBox contains text */
        TTF_SizeText(font, cb->aEntry, &tW, &tH);
    }
    else
    {
        /* this comboBox does not contain text */
        TTF_SizeText(font, " ", &tW, &tH);
    }

    rSize->h = (int)(1.5 * (float)TTF_FontHeight(font));
    rSize->w = tW + aCfg->fontEmWidth + (1 + rSize->h);
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
 *  Name:          NEUIK_NewComboBox
 *
 *  Description:   Create a new NEUIK_ComboBox without contained text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewComboBox(
    NEUIK_ComboBox ** cbPtr)  /* [out] The newly created NEUIK_ComboBox.  */
{
    return neuik_Object_New__ComboBox((void **)cbPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeComboBox
 *
 *  Description:   Create a new NEUIK_ComboBox with specified text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeComboBox(
    NEUIK_ComboBox ** cbPtr,  /* [out] The newly created NEUIK_ComboBox.  */
    const char    * text)    /* [in]  Initial comboBox text. */
{
    size_t         sLen       = 1;
    int            eNum       = 0; /* which error to report (if any) */
    NEUIK_ComboBox * cb        = NULL;
    static char    funcName[] = "NEUIK_MakeComboBox";
    static char  * errMsgs[]  = {"",                       // [0] no error
        "Failure in function `neuik_Object_New__ComboBox`.", // [1]
        "Failure to allocate memory.",                     // [2]
    };

    if (neuik_Object_New__ComboBox((void**)cbPtr))
    {
        eNum = 1;
        goto out;
    }
    cb = *cbPtr;

    /*------------------------------------------------------------------------*/
    /* Set the new ComboBox text contents                                     */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* comboBox will contain no text */
        cb->aEntry = NULL;
    }
    else if (text[0] == '\0')
    {
        /* comboBox will contain no text */
        cb->aEntry = NULL;
    }
    else
    {
        sLen += strlen(text);
        cb->aEntry = (char*)malloc(sLen*sizeof(char));
        if (cb->aEntry == NULL) {
            eNum = 2;
            goto out;
        }
        /* Allocation successful */
        strcpy(cb->aEntry, text);
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
 *  Name:          NEUIK_ComboBox_SetText
 *
 *  Description:   Update the text in a NEUIK_ComboBox.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ComboBox_SetText(
    NEUIK_ComboBox * cb,
    const char   * text)
{
    RenderSize     rSize;
    RenderLoc      rLoc;
    size_t         sLen = 1;
    int            eNum = 0; /* which error to report (if any) */
    static char    funcName[] = "NEUIK_ComboBox_SetText";
    static char  * errMsgs[] = {"",                         // [0] no error
        "Argument `cb` is not of ComboBox class.",          // [1]
        "Failure to allocate memory.",                      // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
    };

    if (!neuik_Object_IsClass(cb, neuik__Class_ComboBox))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Conditionally free comboBox text before setting the new contents       */
    /*------------------------------------------------------------------------*/
    if (cb->aEntry != NULL) {
        free(cb->aEntry);
    }

    /*------------------------------------------------------------------------*/
    /* Set the new ComboBox text contents                                     */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* comboBox will contain no text */
        cb->aEntry = NULL;
    }
    else if (text[0] == '\0')
    {
        /* comboBox will contain no text */
        cb->aEntry = NULL;
    }
    else
    {
        sLen += strlen(text);
        cb->aEntry = (char*)malloc(sLen*sizeof(char));
        if (cb->aEntry == NULL) {
            eNum = 2;
            goto out;
        }
        /* Allocation successful */
        strcpy(cb->aEntry, text);
    }

    if (neuik_Element_GetSizeAndLocation(cb, &rSize, &rLoc))
    {
        eNum = 3;
        goto out;
    }
    neuik_Element_RequestRedraw(cb, rLoc, rSize);
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
 *  Name:          NEUIK_ComboBox_GetText
 *
 *  Description:   Get a pointer to the text in a NEUIK_ComboBox.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
const char * NEUIK_ComboBox_GetText(
    NEUIK_ComboBox * cb)
{
    int           eNum       = 0; /* which error to report (if any) */
    const char  * rvPtr      = NULL;
    static char   emptyStr[] = "";
    static char   funcName[] = "NEUIK_ComboBox_GetText";
    static char * errMsgs[]  = {"",                // [0] no error
        "Argument `cb` is not of ComboBox class.", // [1]
    };

    if (!neuik_Object_IsClass(cb, neuik__Class_ComboBox))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the new ComboBox text contents                                     */
    /*------------------------------------------------------------------------*/
    if (cb->aEntry == NULL){
        /* comboBox will contain no text */
        rvPtr = emptyStr;
    }
    else
    {
        rvPtr = cb->aEntry;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        rvPtr = NULL;
    }

    return rvPtr;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__ComboBox
 *
 *  Description:   Renders a single comboBox as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__ComboBox(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                    eNum       = 0;    /* which error to report (if any) */
    int                    textW      = 0;
    int                    textH      = 0;
    SDL_Rect               rect;
    SDL_Renderer         * rend       = NULL;
    SDL_Texture          * tTex       = NULL; /* text texture */
    SDL_Texture          * aTex       = NULL; /* arrow texture */
    TTF_Font             * font       = NULL;
    const NEUIK_Color    * fgClr      = NULL;
    const NEUIK_Color    * bClr       = NULL; /* border color */
    NEUIK_ComboBoxConfig * aCfg       = NULL; /* the active comboBox config */
    NEUIK_ComboBox       * cb         = NULL;
    NEUIK_ElementBase    * eBase      = NULL;
    neuik_MaskMap        * maskMap    = NULL;
    RenderLoc              rl;
    RenderSize             arrowSize;
    static char            funcName[] = "neuik_Element_Render__ComboBox";
    static char          * errMsgs[]  = {"",                             // [0] no error
        "Argument `elem` is not of ComboBox class.",                     // [1]
        "Failure in `neuik_MakeMaskMap()`",                              // [2]
        "FontSet_GetFont returned NULL.",                                // [3]
        "", // [4]
        "RenderText returned NULL.",                                     // [5]
        "Invalid specified `rSize` (negative values).",                  // [6]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [7]
        "Failure in `neuik_Element_RedrawBackground()`.",                // [8]
    };

    if (!neuik_Object_IsClass(elem, neuik__Class_ComboBox))
    {
        eNum = 1;
        goto out;
    }
    cb = (NEUIK_ComboBox*)elem;

    if (neuik_Object_GetClassObject(cb, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 7;
        goto out;
    }

    if (rSize->w < 0 || rSize->h < 0)
    {
        eNum = 6;
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

    /*------------------------------------------------------------------------*/
    /* select the correct comboBox config to use (pointer or internal)        */
    /*------------------------------------------------------------------------*/
    aCfg = cb->cfg;
    if (cb->cfgPtr != NULL)
    {
        aCfg = cb->cfgPtr;
    }

    /*------------------------------------------------------------------------*/
    /* Select the correct foreground color                                    */
    /*------------------------------------------------------------------------*/
    fgClr = &(aCfg->fgColor); /* use the unselected colors */
    if (cb->selected)
    {
        /* use the selected colors */
        fgClr = &(aCfg->fgColorSelect);
    }

    /*------------------------------------------------------------------------*/
    /* Create a MaskMap an mark off the trasnparent pixels.                   */
    /*------------------------------------------------------------------------*/
    if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Mark off the rounded sections of the button within the MaskMap.        */
    /*------------------------------------------------------------------------*/
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

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_RedrawBackground(elem, rlMod, maskMap))
    {
        eNum = 8;
        goto out;
    }
    rl = eBase->eSt.rLoc;

    /*------------------------------------------------------------------------*/
    /* Draw the border around the comboBox.                                   */
    /*------------------------------------------------------------------------*/
    bClr = &(aCfg->borderColor);
    SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

    /* Draw upper-left corner border pixels */
    SDL_RenderDrawPoint(rend, rl.x + 1, rl.y + 1);
    SDL_RenderDrawPoint(rend, rl.x + 1, rl.y + 2);
    SDL_RenderDrawPoint(rend, rl.x + 2, rl.y + 1);

    /* Draw lower-left corner border pixels */
    SDL_RenderDrawPoint(rend, rl.x + 1, rl.y + (rSize->h - 2));
    SDL_RenderDrawPoint(rend, rl.x + 1, rl.y + (rSize->h - 3));
    SDL_RenderDrawPoint(rend, rl.x + 2, rl.y + (rSize->h - 2));

    /* Draw upper-right corner border pixels */
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2), rl.y + 1);
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2), rl.y + 2);
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 3), rl.y + 1);

    /* Draw upper-right corner border pixels */
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2), rl.y + (rSize->h - 2));
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 2), rl.y + (rSize->h - 3));
    SDL_RenderDrawPoint(rend, rl.x + (rSize->w - 3), rl.y + (rSize->h - 2));


    /* upper border line */
    SDL_RenderDrawLine(rend, 
        rl.x + 2,              rl.y, 
        rl.x + (rSize->w - 3), rl.y); 
    /* left border line */
    SDL_RenderDrawLine(rend, 
        rl.x, rl.y + 2, 
        rl.x, rl.y + (rSize->h - 3));
    /* right border line */
    SDL_RenderDrawLine(rend, 
        rl.x + (rSize->w - 1), rl.y + 2, 
        rl.x + (rSize->w - 1), rl.y + (rSize->h - 3));

    /* lower border line */
    bClr = &(aCfg->borderColorDark);
    SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
    SDL_RenderDrawLine(rend, 
        rl.x + 2,              rl.y + (rSize->h - 1), 
        rl.x + (rSize->w - 3), rl.y + (rSize->h - 1));

    /*------------------------------------------------------------------------*/
    /* Render the comboBox down arrow                                         */
    /*------------------------------------------------------------------------*/
    arrowSize.w = (int)((0.5*(float)(rSize->h - 2)));
    if (arrowSize.w % 2 == 0) arrowSize.w--; /* make sure width is an odd number */
    arrowSize.h = (int)((0.3*(float)(rSize->h - 2)));
    if (arrowSize.h % 2 == 0) arrowSize.h--; /* make sure height is an odd number */

    aTex = NEUIK_RenderArrowDown(*fgClr, rend, arrowSize);

    /* Set the position for the down-arrow */
    rect.x = rl.x + (rSize->w - (rSize->h + 1)) + (rSize->h - arrowSize.w)/2;
    rect.y = rl.y + (rSize->h - arrowSize.h)/2;
    rect.w = arrowSize.w;
    rect.h = arrowSize.h;

    SDL_RenderCopy(rend, aTex, NULL, &rect);

    /*------------------------------------------------------------------------*/
    /* Render the comboBox active entry text                                  */
    /*------------------------------------------------------------------------*/
    if (cb->aEntry != NULL)
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize, 
            aCfg->fontBold, aCfg->fontItalic);
        if (font == NULL) 
        {
            eNum = 3;
            goto out;

        }

        tTex = NEUIK_RenderText(cb->aEntry, font, *fgClr, rend, &textW, &textH);
        if (tTex == NULL)
        {
            eNum = 5;
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
                rect.x += (int) ((float)(rSize->w - (1 + rSize->h) - textW)/2.0);
                rect.y += (int) ((float)(rSize->h - textH)/2.0);
                break;

            case NEUIK_HJUSTIFY_RIGHT:
                rect.x += (int) (rSize->w - textW - (7 + rSize->h));
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
 *  Name:          neuik_Element_CaptureEvent__ComboBox
 *
 *  Description:   Check to see if this event is captured by a NEUIK_ComboBox.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ComboBox(
    NEUIK_Element   elem,
    SDL_Event     * ev)
{
    neuik_EventState       evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
    RenderSize             rSize;
    RenderLoc              rLoc;
    SDL_Event            * e;
    NEUIK_ComboBox       * cb         = NULL;
    NEUIK_ElementBase    * eBase      = NULL;
    SDL_MouseMotionEvent * mouseMotEv;
    SDL_MouseButtonEvent * mouseButEv;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        /* not the right type of object */
        goto out;
    }
    cb = (NEUIK_ComboBox*)elem;

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
                /* This mouse click originated within this comboBox */
                cb->clickOrigin       = 1;
                eBase->eSt.focusstate = NEUIK_FOCUSSTATE_SELECTED;
                cb->selected          = 1;
                cb->wasSelected       = 1;
                neuik_Window_TakeFocus(eBase->eSt.window, cb);
                neuik_Element_TriggerCallback(cb, NEUIK_CALLBACK_ON_CLICK);
                evCaputred = NEUIK_EVENTSTATE_CAPTURED;
                if (!neuik_Object_IsNEUIKObject_NoError(cb))
                {
                    /* The object was freed/corrupted by the callback */
                    evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                    goto out;
                }

                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(cb, rLoc, rSize);
                goto out;
            }
        }
        break;
    
    case SDL_MOUSEBUTTONUP:
        mouseButEv = (SDL_MouseButtonEvent*)(e);
        if (cb->clickOrigin)
        {
            if (mouseButEv->y >= eBase->eSt.rLoc.y && 
                mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseButEv->x >= eBase->eSt.rLoc.x && 
                    mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    /* cursor is still within the comboBox, activate cbFunc */
                    neuik_Element_TriggerCallback(cb, NEUIK_CALLBACK_ON_CLICKED);
                    if (!neuik_Object_IsNEUIKObject_NoError(cb))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                        goto out;
                    }

                    neuik_Window_TakeFocus(eBase->eSt.window, cb);
                    if (!cb->expanded)
                    {
                        cb->expanded = 1;
                        neuik_Element_TriggerCallback(cb, NEUIK_CALLBACK_ON_EXPANDED);
                        if (!neuik_Object_IsNEUIKObject_NoError(cb))
                        {
                            /* The object was freed/corrupted by the callback */
                            evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                            goto out;
                        }
                    }
                    else
                    {
                        cb->expanded = 0;
                        neuik_Element_TriggerCallback(cb, NEUIK_CALLBACK_ON_COLLAPSED);
                        if (!neuik_Object_IsNEUIKObject_NoError(cb))
                        {
                            /* The object was freed/corrupted by the callback */
                            evCaputred = NEUIK_EVENTSTATE_OBJECT_FREED;
                            goto out;
                        }
                    }
                }
            }
            eBase->eSt.focusstate = NEUIK_FOCUSSTATE_NORMAL;
            cb->selected          = 0;
            cb->wasSelected       = 0;
            cb->clickOrigin       = 0;
            evCaputred            = NEUIK_EVENTSTATE_CAPTURED;

            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            neuik_Element_RequestRedraw(cb, rLoc, rSize);
            goto out;
        }
        break;

    case SDL_MOUSEMOTION:
        mouseMotEv = (SDL_MouseMotionEvent*)(e);

        if (cb->clickOrigin)
        {
            /*----------------------------------------------------------------*/
            /* The mouse was initially clicked within the comboBox. If the    */
            /* user moves the cursor out of the comboBox area, deselect it.   */
            /*----------------------------------------------------------------*/
            eBase->eSt.focusstate = NEUIK_FOCUSSTATE_NORMAL;
            cb->selected          = 0;
            if (mouseMotEv->y >= eBase->eSt.rLoc.y && 
                mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseMotEv->x >= eBase->eSt.rLoc.x && 
                    mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    eBase->eSt.focusstate = NEUIK_FOCUSSTATE_SELECTED;
                    cb->selected = 1;
                }
            }

            if (cb->wasSelected != cb->selected)
            {
                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(cb, rLoc, rSize);
            }
            cb->wasSelected = cb->selected;
            evCaputred      = NEUIK_EVENTSTATE_CAPTURED;
            goto out;
        }

        break;
    }
out:
    return evCaputred;
}
