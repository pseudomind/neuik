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
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "NEUIK_defs.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_platform.h"
#include "NEUIK_TextEntry.h"
#include "NEUIK_TextEntry_internal.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int   neuik__Report_Debug;
extern float neuik__HighDPI_Scaling;

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_TextEntry_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__TextEntry,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__TextEntry,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__TextEntry,

    /* Defocus(): This function will be called when an element looses focus */
    neuik_Element_Defocus__TextEntry,

    /* RequestRedraw(): This function will be called when redraw is requested */
    NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_TextEntry_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__TextEntry,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__TextEntry,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_TextEntry
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_TextEntry()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_TextEntry";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",      // [1]
        "Failed to register `TextEntry` object class .", // [2]
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
        "NEUIK_TextEntry",             // className
        "An editible GUI text field.", // classDescription
        neuik__Set_NEUIK,              // classSet
        neuik__Class_Element,          // superClass
        &neuik_TextEntry_BaseFuncs,    // baseFuncs
        NULL,                          // classFuncs
        &neuik__Class_TextEntry))      // newClass
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
 *  Name:          neuik_Object_New__TextEntry
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__TextEntry(
    void ** tePtr)
{
    int                sLen       = 1;
    int                eNum       = 0; /* which error to report (if any) */
    NEUIK_Element    * sClassPtr  = NULL;
    NEUIK_TextEntry  * te         = NULL;
    NEUIK_Color        bgClr      = COLOR_WHITE;
    static char        funcName[] = "neuik_Object_New__TextEntry";
    static char      * errMsgs[]  = {"", // [0] no error
        "Failure to allocate memory.",                         // [1]
        "Failure in NEUIK_NewTextEntryConfig.",                // [2]
        "Output Argument `tePtr` is NULL.",                    // [3]
        "Failure in function `neuik_Object_New`.",             // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",   // [5]
        "Failure in `neuik_GetObjectBaseOfClass`.",            // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorSolid`.", // [7]
    };

    if (tePtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*tePtr) = (NEUIK_TextEntry*) malloc(sizeof(NEUIK_TextEntry));
    te = *tePtr;
    if (te == NULL)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_TextEntry, 
            NULL,
            &(te->objBase)))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(te->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_TextEntry_FuncTable))
    {
        eNum = 5;
        goto out;
    }


    /* allocate for a minimum of 50 char (larger if it starts with more) */
    sLen = 50;
    te->text = (char*)malloc(sLen*sizeof(char));
    if (te->text == NULL) {
        eNum = 1;
        goto out;
    }
    /* Allocation successful */
    te->text[0] = '\0';
    te->textLen = 0;
    te->textAllocSize = sLen;

    /*------------------------------------------------------------------------*/
    /* All allocations successful                                             */
    /*------------------------------------------------------------------------*/
    te->cursorPos      = 0;
    te->cursorX        = 0;
    te->selected       = 0;
    te->wasSelected    = 0;
    te->highlightBegin = -1;
    te->highlightStart = -1;
    te->highlightEnd   = -1;
    te->panX           = 0;
    te->panCursor      = 0;
    te->isActive       = 0;
    te->clickOrigin    = -1;
    te->clickHeld      = 0;
    te->needsRedraw    = 1;
    te->timeLastClick  = 0;
    te->cfg            = NULL; 
    te->cfgPtr         = NULL; 
    te->textSurf       = NULL;
    te->textTex        = NULL;
    te->textRend       = NULL;

    // if (NEUIK_TextEntry_CopyConfig(te, NEUIK_GetDefaultTextEntryConfig(&err)))
    if (NEUIK_NewTextEntryConfig(&te->cfg))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorSolid(te, "normal",
        bgClr.r, bgClr.g, bgClr.b, bgClr.a))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(te, "selected",
        bgClr.r, bgClr.g, bgClr.b, bgClr.a))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(te, "hovered",
        bgClr.r, bgClr.g, bgClr.b, bgClr.a))
    {
        eNum = 7;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        /* free any dynamically allocated memory */
        if (te != NULL)
        {
            if (te->text != NULL) free(te->text);
            free(te);
        }
        te = NULL;
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewTextEntry
 *
 *  Description:   Wrapper function for the Object_New function.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewTextEntry(
    NEUIK_TextEntry ** tePtr)
{
    return neuik_Object_New__TextEntry((void**)tePtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeTextEntry
 *
 *  Description:   Create a new NEUIK_TextEntry and assign text to it.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeTextEntry(
    NEUIK_TextEntry ** tePtr, /* [out] The newly created NEUIK_TextEntry. */
    const char       * text)  /* [in]  Initial TextEntry text. */
{
    size_t            sLen       = 1;
    int               eNum       = 0; /* which error to report (if any) */
    NEUIK_TextEntry * te         = NULL;
    static char       funcName[] = "NEUIK_MakeTextEntry";
    static char     * errMsgs[]  = {"", // [0] no error
        "Failure in function `neuik_Object_New__TextEntry`.", // [1]
        "Failure to allocate memory.",                        // [2]
        "Failure to reallocate memory.",                      // [3]
    };

    if (neuik_Object_New__TextEntry((void**)tePtr))
    {
        eNum = 1;
        goto out;
    }
    te = *tePtr;

    if (text == NULL)
    {
        /* textEntry will contain no text */
        if (te->text != NULL) te->text[0] = '\0';
        goto out;
    }
    else if (text[0] == '\0')
    {
        /* textEntry will contain no text */
        if (te->text != NULL) te->text[0] = '\0';
        goto out;
    }
    else
    {
        te->textLen  = strlen(text);    
        sLen        += te->textLen; 
    }

    /*------------------------------------------------------------------------*/
    /* Make sure the allocated text buffer is large enough to store the text  */
    /*------------------------------------------------------------------------*/
    if (te->text == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Memory not currently allocated; allocate now.                      */
        /*--------------------------------------------------------------------*/
        te->text = (char*)malloc((sLen+10)*sizeof(char));
        if (te->text == NULL)
        {
            eNum = 2;
            goto out;
        }
        te->textAllocSize = sLen+10;
    }
    else if (sLen > te->textAllocSize)
    {
        /*--------------------------------------------------------------------*/
        /* Reallocate memory so that the string can be fit.                   */
        /*--------------------------------------------------------------------*/
        te->text = (char*)realloc(te->text, (sLen+10)*sizeof(char));
        if (te->text == NULL)
        {
            eNum = 3;
            goto out;
        }
        te->textAllocSize = sLen+10;
    }

    if (text != NULL)
    {
        strcpy(te->text, text);
    }
    else
    {
        te->text[0] = '\0';
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
 *  Name:          neuik_Object_Free__TextEntry
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__TextEntry(
    void * tePtr)  /* [out] the button to free */
{
    int               eNum       = 0; /* which error to report (if any) */
    NEUIK_TextEntry * te         = NULL;
    static char       funcName[] = "neuik_Object_Free__TextEntry";
    static char     * errMsgs[]  = {"", // [0] no error
        "Argument `btnPtr` is not of Button class.", // [1]
        "Failure in function `neuik_Object_Free`.",  // [2]
        "Argument `btnPtr` is NULL.",                // [3]
    };

    if (tePtr == NULL)
    {
        eNum = 3;
        goto out;
    }
    te = (NEUIK_TextEntry*)tePtr;

    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(te->objBase.superClassObj))
    {
        eNum = 2;
        goto out;
    }
    if (te->text     != NULL) free(te->text);
    if (te->textSurf != NULL) SDL_FreeSurface(te->textSurf);
    if (te->textTex  != NULL) SDL_DestroyTexture(te->textTex);
    if (te->textRend != NULL) SDL_DestroyRenderer(te->textRend);

    if(neuik_Object_Free(te->cfg))
    {
        eNum = 2;
        goto out;
    }

    free(te);
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
 *  Name:          neuik_Element_GetMinSize__TextEntry
 *
 *  Description:   Returns the rendered size of a given button.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__TextEntry(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int                     tW;
    int                     tH;
    int                     eNum = 0;    /* which error to report (if any) */
    TTF_Font              * font = NULL;
    NEUIK_TextEntry       * te   = NULL;
    NEUIK_TextEntryConfig * aCfg = NULL; /* the active button config */
    static char             funcName[] = "neuik_Element_GetMinSize__TextEntry";
    static char           * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of TextEntry class.", // [1]
        "TextEntryConfig* is NULL.",                  // [2]
        "TextEntryConfig->FontSet is NULL.",          // [3]
        "FontSet_GetFont returned NULL.",             // [4]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    te = (NEUIK_TextEntry *)elem;
    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }
    
    /* select the correct button config to use (pointer or internal) */
    aCfg = te->cfgPtr;
    if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

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

    /* this textEntry does not contain text */
    TTF_SizeText(font, " ", &tW, &tH);

    rSize->w = tW + aCfg->fontEmWidth;
    rSize->h = 2 + (int)(1.5 * (float)TTF_FontHeight(font));

    if (neuik__HighDPI_Scaling >= 2.0)
    {
        /*--------------------------------------------------------------------*/
        /* Add in additional pixels of width/height to accomodate for thicker */
        /* button borders.                                                    */
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
 *  Name:          NEUIK_TextEntry_SetText
 *
 *  Description:   Update the text in a NEUIK_TextEntry.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEntry_SetText(
    NEUIK_TextEntry * te,
    const char      * text)
{
    size_t          sLen    = 1;
    size_t          textLen = 0;
    RenderSize      rSize;
    RenderLoc       rLoc;
    int             eNum    = 0; /* which error to report (if any) */
    static char     funcName[] = "NEUIK_TextEntry_SetText";
    static char   * errMsgs[] = {"", // [0] no error
        "Argument `te` is not of TextEntry class.",         // [1]
        "Failure to allocate memory.",                      // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the new TextEntry text contents                                    */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* textentry will contain no text */
        if (te->textAllocSize > 0)
        {
            te->text[0] = '\0';
        }
    }
    else if (text[0] == '\0')
    {
        /* textentry will contain no text */
        if (te->textAllocSize > 0)
        {
            te->text[0] = '\0';
        }
    }
    else
    {
        textLen = strlen(text);
        if (textLen > te->textAllocSize)
        {
            sLen += textLen;
            if (te->text != NULL) {
                te->text = (char*)realloc(te->text, sLen*sizeof(char));
                if (te->text == NULL) {
                    eNum = 2;
                    goto out;
                }
            }
            else
            {
                te->text = (char*)malloc(sLen*sizeof(char));
                if (te->text == NULL) {
                    eNum = 2;
                    goto out;
                }
            }
            te->textAllocSize = sLen;
        }
        /* Allocation successful */
        strcpy(te->text, text);
    }

    te->textLen        = textLen;
    te->highlightBegin = -1;
    te->highlightStart = -1;
    te->highlightEnd   = -1;
    te->cursorPos      =  0;
    te->cursorX        =  0;
    te->clickOrigin    =  0;
    te->clickHeld      =  0;
    if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
    {
        eNum = 3;
        goto out;
    }
    neuik_Element_RequestRedraw(te, rLoc, rSize);
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
 *  Name:          NEUIK_TextEntry_GetText
 *
 *  Description:   Get a pointer to the text in a NEUIK_TextEntry.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
const char * NEUIK_TextEntry_GetText(
    NEUIK_TextEntry * te)
{
    int            eNum = 0; /* which error to report (if any) */
    const char   * rvPtr      = NULL;
    static char    emptyStr[] = "";
    static char    funcName[] = "NEUIK_TextEntry_GetText";
    static char  * errMsgs[]  = {"", // [0] no error
        "Argument `te` is not of TextEntry class.", // [1]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the new TextEntry text contents                                    */
    /*------------------------------------------------------------------------*/
    if (te->text == NULL){
        /* button will contain no text */
        rvPtr = emptyStr;
    }
    else
    {
        rvPtr = te->text;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        rvPtr = NULL;
    }

    return rvPtr;
}

void neuik_TextEntry_Configure_capture_segv(
    int sig_num)
{
    int unused = 0;
    static char funcName[] = "NEUIK_TextEntry_Configure";
    static char errMsg[] = 
        "SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

    if (sig_num) { unused++; }
    if (unused) { unused++; }

    NEUIK_RaiseError(funcName, errMsg);
    NEUIK_BacktraceErrors();
    exit(1);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_TextEntry_Configure
 *
 *  Description:   Configure a number of properties specific to NEUIK_TextEntry.
 *
 *                 This list of named sets must be terminated by a NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEntry_Configure(
    NEUIK_TextEntry  * te,
    const char       * set0,
    ...)
{
    int                     ctr;
    int                     eNum       = 0; /* which error to report (if any) */
    int                     vaStarted  = 0;
    va_list                 args;
    char                    buf[4096];
    char                  * strPtr     = NULL;
    char                  * name       = NULL;
    char                  * value      = NULL;
    const char            * set        = NULL;
    NEUIK_TextEntryConfig * aCfg       = NULL; /* the active button config */
    static char             funcName[] = "NEUIK_TextEntry_Configure";
    static char           * errMsgs[]  = {"",       // [0] no error
        "Argument `te` is not of TextEntry class.", // [1]
        "NamedSet.name is NULL, skipping..",        // [2]
        "NamedSet.name is blank, skipping..",       // [3]
        "NamedSet.name type unknown, skipping.",    // [4]
        "`name=value` string is too long.",         // [5]
        "Invalid `name=value` string.",             // [6]
        "HJustify value is invalid.",               // [7]
        "VJustify value is invalid.",               // [7]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }
    set = set0;

    /*------------------------------------------------------------------------*/
    /* select the correct entry config to use (pointer or internal)           */
    /*------------------------------------------------------------------------*/
    aCfg = te->cfgPtr;
    if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

    va_start(args, set0);
    vaStarted = 1;

    for (ctr = 0;; ctr++)
    {
        name  = NULL;
        value = NULL;

        if (set == NULL) break;

        #ifndef NO_NEUIK_SIGNAL_TRAPPING
            signal(SIGSEGV, neuik_TextEntry_Configure_capture_segv);
        #endif

        if (strlen(set) > 4095)
        {
            #ifndef NO_NEUIK_SIGNAL_TRAPPING
                signal(SIGSEGV, NULL);
            #endif
            NEUIK_RaiseError(funcName, errMsgs[5]);
            set = va_arg(args, const char *);
            continue;
        }
        else
        {
            #ifndef NO_NEUIK_SIGNAL_TRAPPING
                signal(SIGSEGV, NULL);
            #endif
            strcpy(buf, set);
            /* Find the equals and set it to '\0' */
            strPtr = strchr(buf, '=');
            if (strPtr == NULL)
            {
                /* `name=value` string is missing the '=' char */
                NEUIK_RaiseError(funcName, errMsgs[6]);
                set = va_arg(args, const char *);
                continue;
            }
            *strPtr = 0;
            strPtr++;
            if (*strPtr == 0)
            {
                /* `name=value` string is missing a value */
                NEUIK_RaiseError(funcName, errMsgs[6]);
                set = va_arg(args, const char *);
                continue;
            }
            name  = buf;
            value = strPtr;
        }

        if (name == NULL)
        {
            NEUIK_RaiseError(funcName, errMsgs[2]);
        }
        else if (name[0] == 0)
        {
            NEUIK_RaiseError(funcName, errMsgs[3]);
        }
        else if (!strcmp("HJustify", name))
        {
            if (!strcmp("left", value))
            {
                aCfg->textHJustify = NEUIK_HJUSTIFY_LEFT;
            }
            else if (!strcmp("center", value))
            {
                aCfg->textHJustify = NEUIK_HJUSTIFY_CENTER;
            }
            else if (!strcmp("right", value))
            {
                aCfg->textHJustify = NEUIK_HJUSTIFY_RIGHT;
            }
            else 
            {
                NEUIK_RaiseError(funcName, errMsgs[7]);
            }
        }
        else if (!strcmp("VJustify", name))
        {
            if (!strcmp("top", value))
            {
                aCfg->textVJustify = NEUIK_VJUSTIFY_TOP;
            }
            else if (!strcmp("center", value))
            {
                aCfg->textVJustify = NEUIK_VJUSTIFY_CENTER;
            }
            else if (!strcmp("bottom", value))
            {
                aCfg->textVJustify = NEUIK_VJUSTIFY_BOTTOM;
            }
            else 
            {
                NEUIK_RaiseError(funcName, errMsgs[7]);
            }
        }

        else
        {
            NEUIK_RaiseError(funcName, errMsgs[4]);
        }

        /* before starting */
        set = va_arg(args, const char *);
    }
out:
    if (vaStarted) va_end(args);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__TextEntry
 *
 *  Description:   Renders a single TextEntry as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__TextEntry(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    char                    tempChar   = 0; /* a temporary character */
    int                     borderW    = 1; /* width of entry border line */
    int                     ctr        = 0;
    int                     eNum       = 0; /* which error to report (if any) */
    int                     textW      = 0;
    int                     textH      = 0;
    int                     textWFull  = 0;
    int                     textHFull  = 0;
    int                     normWidth  = 0;
    int                     hlWidth    = 0; /* highlight bg Width */
    SDL_Rect                rect;
    SDL_Rect                srcRect;
    const NEUIK_Color     * fgClr      = NULL;
    const NEUIK_Color     * bgClr      = NULL;
    const NEUIK_Color     * bClr       = NULL; /* border color */
    SDL_Renderer          * rend       = NULL;
    SDL_Texture           * tTex       = NULL; /* text texture */
    TTF_Font              * font       = NULL;
    NEUIK_ElementBase     * eBase      = NULL;
    RenderLoc               rl;
    neuik_MaskMap         * maskMap    = NULL;
    NEUIK_TextEntry       * te         = NULL;
    NEUIK_TextEntryConfig * aCfg       = NULL; /* the active textEntry config */
    static char             funcName[] = "neuik_Element_Render__TextEntry";
    static char           * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of TextEntry class.",                    // [1]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "", // [3]
        "Invalid specified `rSize` (negative values).",                  // [4]
        "Failure in `neuik_MakeMaskMap()`",                              // [5]
        "FontSet_GetFont returned NULL.",                                // [6]
        "", // [7]
        "Failure in neuik_Element_RedrawBackground().",                  // [8]
    };

    te = (NEUIK_TextEntry *)elem;
    if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
    {
        eNum = 1;
        goto out;
    }

    if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
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

    eBase->eSt.rend = xRend;
    rend = eBase->eSt.rend;

    if (neuik__HighDPI_Scaling >= 2.0)
    {
        borderW = 2*(int)(neuik__HighDPI_Scaling/2.0);
    }

    /*------------------------------------------------------------------------*/
    /* Select the correct entry config to use (pointer or internal)           */
    /*------------------------------------------------------------------------*/
    aCfg = te->cfg;  /* Fallback to internal config */
    if (te->cfgPtr != NULL)
    {
        /* Switch to pointer config */
        aCfg = te->cfgPtr;
    }

    /* extract the current fg/bg colors */
    bgClr = &(aCfg->bgColor);
    fgClr = &(aCfg->fgColor);

    /*------------------------------------------------------------------------*/
    /* Get the pointer to the currently active font (if text is present)      */
    /*------------------------------------------------------------------------*/
    if (te->text != NULL)
    {
        if (te->text[0] != '\0')
        {
            /* Determine the full size of the rendered text content */
            font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
                aCfg->fontBold, aCfg->fontItalic);
            if (font == NULL) 
            {
                eNum = 6;
                goto out;
            }
        }
    }

    if (te->textSurf != NULL)
    {
        SDL_FreeSurface(te->textSurf);
        te->textSurf = NULL;
    }
    if (te->textRend != NULL)
    {
        SDL_DestroyRenderer(te->textRend);
        te->textRend = NULL;
    }
    if (te->textTex != NULL)
    {
        SDL_DestroyTexture(te->textTex);
        te->textTex = NULL;
    }

    /*------------------------------------------------------------------------*/
    /* Redraw the contained text and highlighting (if present)                */
    /*------------------------------------------------------------------------*/
    if (te->text != NULL)
    {
        if (te->text[0] != '\0')
        {
            /* Determine the full size of the rendered text content */
            TTF_SizeText(font, te->text, &textW, &textH);
            textWFull = textW;

            /*----------------------------------------------------------------*/
            /* Create an SDL_Surface for the text within the element          */
            /*----------------------------------------------------------------*/
            textHFull = rSize->h - 2;
            te->textSurf = SDL_CreateRGBSurface(0, textW+1, textHFull, 32, 0, 0, 0, 0);
            if (te->textSurf == NULL)
            {
                eNum = 8;
                goto out;
            }


            te->textRend = SDL_CreateSoftwareRenderer(te->textSurf);
            if (te->textRend == NULL)
            {
                eNum = 9;
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* Fill the background with it's color                            */
            /*----------------------------------------------------------------*/
            SDL_SetRenderDrawColor(te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
            SDL_RenderClear(te->textRend);

            /*----------------------------------------------------------------*/
            /* Render the Text now, it will be copied on after highlighting   */
            /*----------------------------------------------------------------*/
            tTex = NEUIK_RenderText(te->text, font, *fgClr, te->textRend, &textW, &textH);
            if (tTex == NULL)
            {
                eNum = 6;
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* Check for and fill in highlight text selection background      */
            /*----------------------------------------------------------------*/
            if (eBase->eSt.hasFocus && te->highlightBegin != -1)
            {
                rect.x = 0;
                rect.y = (int) ((float)(rSize->h - textH)/2.0);
                rect.w = textW;
                rect.h = (int)(1.1*textH);

                textW = 0;
                textH = 0;
                /* determine the point of the start of the bgkd highlight */
                if (te->highlightStart != 0)
                {
                    tempChar = te->text[te->highlightStart];
                    te->text[te->highlightStart] = '\0';
                    TTF_SizeText(font, te->text, &textW, &textH);
                    te->text[te->highlightStart] = tempChar;
                }
                rect.x += textW;

                /* determine the point of the start of the bgkd highlight */
                if (te->highlightEnd < te->textLen)
                {
                    tempChar = te->text[1 + te->highlightEnd];
                    te->text[1 + te->highlightEnd] = '\0';
                    TTF_SizeText(font, te->text + te->highlightStart, &textW, &textH);
                    te->text[1 + te->highlightEnd] = tempChar;
                }
                else
                {
                    TTF_SizeText(font, te->text + te->highlightStart, &textW, &textH);
                }
                hlWidth = textW;
                rect.w = hlWidth;

                bgClr = &(aCfg->bgColorHl);
                SDL_SetRenderDrawColor(te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
                SDL_RenderFillRect(te->textRend, &rect);
            }

            /*----------------------------------------------------------------*/
            /* Copy over the previously rendered text.                        */
            /*----------------------------------------------------------------*/
            rect.x = 0;
            rect.y = (int) ((float)(rSize->h - textH)/2.0);
            rect.w = textWFull;
            rect.h = (int)(1.1*textH);

            SDL_RenderCopy(te->textRend, tTex, NULL, &rect);

            /*----------------------------------------------------------------*/
            /* Draw the cursor (if textedit is focused)                       */
            /*----------------------------------------------------------------*/
            if (eBase->eSt.hasFocus)
            {
                /*------------------------------------------------------------*/
                /* Draw the cursor line into the textedit element             */
                /*------------------------------------------------------------*/
                SDL_SetRenderDrawColor(te->textRend, fgClr->r, fgClr->g, fgClr->b, 255);

                tempChar = te->text[te->cursorPos];
                if (tempChar == '\0')
                {
                    rect.x = textWFull - 1;
                }
                else
                {
                    te->text[te->cursorPos] = '\0';
                    TTF_SizeText(font, te->text, &textW, &textH);
                    te->text[te->cursorPos] = tempChar;

                    /* this will be the positin of the cursor */
                    rect.x = textW;
                }
                te->cursorX = rect.x;
                SDL_RenderDrawLine(te->textRend, 
                    rect.x, rect.y, 
                    rect.x, rect.y + rect.h); 
            }

            SDL_RenderPresent(te->textRend);
            te->textTex = SDL_CreateTextureFromSurface(rend, te->textSurf);
            if (te->textTex == NULL)
            {
                eNum = 7;
                goto out;
            }
        }
    }

    /*------------------------------------------------------------------------*/
    /* Create a MaskMap an mark off the trasnparent pixels.                   */
    /*------------------------------------------------------------------------*/
    if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
    {
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Mark off the rounded sections of the button within the MaskMap.        */
    /*------------------------------------------------------------------------*/
    /* upper border line */
    neuik_MaskMap_MaskLine(maskMap, 
        0,            0, 
        rSize->w - 1, 0); 
    /* left border line */
    neuik_MaskMap_MaskLine(maskMap, 
        0, 0, 
        0, rSize->h - 1); 
    /* right border line */
    neuik_MaskMap_MaskLine(maskMap, 
        rSize->w - 1, 0, 
        rSize->w - 1, rSize->h - 1); 
    /* lower border line */
    neuik_MaskMap_MaskLine(maskMap, 
        0,            rSize->h - 1, 
        rSize->w - 1, rSize->h - 1);

    /*------------------------------------------------------------------------*/
    /* Redraw the background surface before continuing.                       */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_RedrawBackground(te, rlMod, maskMap))
    {
        eNum = 8;
        goto out;
    }
    rl = eBase->eSt.rLoc;

    if (te->textTex != NULL)
    {
        normWidth = rSize->w - 12; 

        if (textWFull < normWidth) 
        {
            TTF_SizeText(font, te->text, &textW, &textH);

            rect.x = rl.x;
            rect.y = rl.y + 1;
            rect.w = textW + 1;
            rect.h = rSize->h - 2;

            switch (aCfg->textHJustify)
            {
                case NEUIK_HJUSTIFY_LEFT:
                    rect.x += 6;
                    break;

                case NEUIK_HJUSTIFY_CENTER:
                    rect.x += (int) ((float)(rSize->w - textW)/2.0);
                    break;

                case NEUIK_HJUSTIFY_RIGHT:
                    rect.x += (int) (rSize->w - textW - 6);
                    break;
            }

            SDL_RenderCopy(rend, te->textTex, NULL, &rect);
        }
        else
        {
            TTF_SizeText(font, te->text, &textW, &textH);

            rect.x = rl.x + 6;
            rect.y = rl.y + 1;
            rect.w = normWidth;
            rect.h = rSize->h - 2;

            srcRect.x = te->panCursor;
            srcRect.y = 0;
            srcRect.w = normWidth;
            srcRect.h = textHFull;
            if (neuik__Report_Debug)
            {
                printf("Redraw: panCursor = %d, cursorX = %d, normW = %d\n", 
                    te->panCursor, te->cursorX, normWidth);
            }

            SDL_RenderCopy(rend, te->textTex, &srcRect, &rect);
        }
    }

    if (eBase->eSt.hasFocus)
    {
        /*--------------------------------------------------------------------*/
        /* Draw the border in its selected way.                               */
        /*--------------------------------------------------------------------*/
        bgClr = &(aCfg->bgColorSelect);

        SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);

        /*--------------------------------------------------------------------*/
        /* Draw the Upper Border Line.                                        */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < borderW + 2; ctr++)
        {
            if (ctr != 0)
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + 1,              rl.y + ctr, 
                    rl.x + (rSize->w - 2), rl.y + ctr); 
            }
            else
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + 2,              rl.y, 
                    rl.x + (rSize->w - 3), rl.y); 
            }
        }

        /*--------------------------------------------------------------------*/
        /* Draw the Left Border Line.                                         */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < borderW + 2; ctr++)
        {
            if (ctr != 0)
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + ctr, rl.y + 1,  
                    rl.x + ctr, rl.y + (rSize->h - 2)); 
            }
            else
            {
                SDL_RenderDrawLine(rend, 
                    rl.x , rl.y + 1,  
                    rl.x , rl.y + (rSize->h - 3));
            }
        }

        /*--------------------------------------------------------------------*/
        /* Draw the Right Border Line.                                        */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < borderW + 2; ctr++)
        {
            if (ctr != 0)
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + (rSize->w - 1) - ctr, rl.y + 1,
                    rl.x + (rSize->w - 1) - ctr, rl.y + (rSize->h - 2)); 
            }
            else
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + (rSize->w - 1), rl.y + 2,
                    rl.x + (rSize->w - 1), rl.y + (rSize->h - 3));
            }
        }

        /*--------------------------------------------------------------------*/
        /* Draw the Lower Border Line.                                        */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < borderW + 2; ctr++)
        {
            if (ctr != 0)
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + 1,              rl.y + (rSize->h - 1) - ctr, 
                    rl.x + (rSize->w - 2), rl.y + (rSize->h - 1) - ctr); 
            }
            else
            {
                SDL_RenderDrawLine(rend, 
                    rl.x + 2,              rl.y + (rSize->h - 1), 
                    rl.x + (rSize->w - 3), rl.y + (rSize->h - 1)); 
            }
        }

        /*--------------------------------------------------------------------*/
        /* Draw the inner rounding selected pixels.                           */
        /*--------------------------------------------------------------------*/
        /* upper left */
        SDL_RenderDrawPoint(rend, 
            rl.x + (borderW + 2), 
            rl.y + (borderW + 2));
        /* lower left */
        SDL_RenderDrawPoint(rend, 
            rl.x + (borderW + 2), 
            rl.y + rSize->h - (1 + (borderW + 2)));
        /* upper right */
        SDL_RenderDrawPoint(rend, 
            rl.x + rSize->w - (1 + (borderW + 2)),
            rl.y + (borderW + 2));
        /* lower right */
        SDL_RenderDrawPoint(rend, 
            rl.x + rSize->w - (1 + (borderW + 2)),
            rl.y + rSize->h - (1 + (borderW + 2)));
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* Draw the border around the TextEntry.                              */
        /*--------------------------------------------------------------------*/
        bClr = &(aCfg->borderColor);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

        /* upper border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                rl.x + 1,              (rl.y + 1) + ctr, 
                rl.x + (rSize->w - 2), (rl.y + 1) + ctr); 
        }
        /* left border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                (rl.x + 1) + ctr, rl.y + 1, 
                (rl.x + 1) + ctr, rl.y + (rSize->h - 2));
        }

        /* right border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                rl.x + (rSize->w - 2) - ctr, rl.y + 1, 
                rl.x + (rSize->w - 2) - ctr, rl.y + (rSize->h - 2));
        }

        /* lower border line */
        bClr = &(aCfg->borderColorDark);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                rl.x + 2 + ctr,              rl.y + (rSize->h - 2) - ctr, 
                rl.x + (rSize->w - 3) - ctr, rl.y + (rSize->h - 2) - ctr);
        }
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
 *  Name:          neuik_Element_Defocus__TextEntry
 *
 *  Description:   Defocus the TextEntry element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_Defocus__TextEntry(
    NEUIK_Element el)
{
    NEUIK_TextEntry * te;
    RenderSize        rSize = {0, 0};
    RenderLoc         rLoc  = {0, 0};

    SDL_StopTextInput();
    te = (NEUIK_TextEntry*) el;

    if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
    {
        return;
    }
    neuik_Element_RequestRedraw(te, rLoc, rSize);
    te->highlightBegin = -1;
    te->highlightStart = -1;
    te->highlightEnd   = -1;
    te->clickOrigin    =  0;
    te->clickHeld      =  0;
}

