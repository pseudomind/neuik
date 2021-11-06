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
#include "NEUIK_TextEdit.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int   neuik__isInitialized;
extern float neuik__HighDPI_Scaling;

#define CURSORPAN_TEXT_INSERTED   0
#define CURSORPAN_TEXT_DELTETED   1
#define CURSORPAN_TEXT_ADD_REMOVE 2
#define CURSORPAN_MOVE_BACK       3
#define CURSORPAN_MOVE_FORWARD    4

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__TextEdit(void ** tePtr);
int neuik_Object_Free__TextEdit(void * tePtr);

int neuik_Element_GetMinSize__TextEdit(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__TextEdit(
    NEUIK_Element, SDL_Event*);
int neuik_Element_Render__TextEdit(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);
void neuik_Element_Defocus__TextEdit(NEUIK_Element);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_TextEdit_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__TextEdit,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__TextEdit,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__TextEdit,

    /* Defocus(): This function will be called when an element looses focus */
    neuik_Element_Defocus__TextEdit,

    /* RequestRedraw(): This function will be called when redraw is requested */
    NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_TextEdit_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__TextEdit,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__TextEdit,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_TextEdit
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_TextEdit()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_TextEdit";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",     // [1]
        "Failed to register `TextEdit` object class .", // [2]
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
        "NEUIK_TextEdit",             // className
        "An editible GUI text field.", // classDescription
        neuik__Set_NEUIK,              // classSet
        neuik__Class_Element,          // superClass
        &neuik_TextEdit_BaseFuncs,     // baseFuncs
        NULL,                          // classFuncs
        &neuik__Class_TextEdit))       // newClass
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
 *  Name:          neuik_Object_New__TextEdit
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__TextEdit(
    void ** tePtr)
{
    int                eNum       = 0; /* which error to report (if any) */
    NEUIK_Element    * sClassPtr  = NULL;
    NEUIK_TextEdit   * te         = NULL;
    NEUIK_Color        bgClr      = COLOR_WHITE;
    static char        funcName[] = "neuik_Object_New__TextEdit";
    static char      * errMsgs[]  = {"", // [0] no error
        "Failure to allocate memory.",                         // [1]
        "Failure in NEUIK_NewTextEditConfig.",                 // [2]
        "Output Argument `tePtr` is NULL.",                    // [3]
        "Failure in function `neuik_Object_New`.",             // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",   // [5]
        "Failure in `neuik_GetObjectBaseOfClass`.",            // [6]
        "Failure in function `neuik_NewTextBlock`.",           // [7]
        "Failure in `NEUIK_Element_SetBackgroundColorSolid`.", // [8]
    };

    if (tePtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*tePtr) = (NEUIK_TextEdit*) malloc(sizeof(NEUIK_TextEdit));
    te = *tePtr;
    if (te == NULL)
    {
        eNum = 1;
        goto out;
    }
    memset(te, 0, sizeof(NEUIK_TextEdit));

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_TextEdit, 
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
    if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_TextEdit_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_NewTextBlock(&te->textBlk, 0, 0))
    {
        eNum = 7;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* All allocations successful                                             */
    /*------------------------------------------------------------------------*/
    te->scrollPct          = 0.0;
    te->viewPct            = 0.0;
    te->cursorLine         = 0;
    te->cursorPos          = 0;
    te->vertMovePos        = UNDEFINED;
    te->vertPanLn          = 0;
    te->vertPanPx          = 0;
    te->cursorX            = 0;
    te->lastMouseX         = UNDEFINED;
    te->lastMouseY         = UNDEFINED;
    te->selected           = FALSE;
    te->wasSelected        = FALSE;
    te->highlightIsSet     = 0;
    te->highlightBeginPos  = 0;
    te->highlightBeginLine = 0;
    te->highlightStartPos  = 0;
    te->highlightStartLine = 0;
    te->highlightEndPos    = 0;
    te->highlightEndLine   = 0;
    te->panX               = 0;
    te->panCursor          = 0;
    te->isActive           = 0;
    te->clickOrigin        = UNDEFINED;
    te->clickHeld          = FALSE;
    te->needsRedraw        = TRUE;
    te->timeLastClick      = 0;
    te->timeClickMinus2    = 0;
    te->cfg                = NULL; 
    te->cfgPtr             = NULL; 
    te->textSurf           = NULL;
    te->textTex            = NULL;
    te->textRend           = NULL;

    if (NEUIK_NewTextEditConfig(&te->cfg))
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
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(te, "selected",
        bgClr.r, bgClr.g, bgClr.b, bgClr.a))
    {
        eNum = 8;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorSolid(te, "hovered",
        bgClr.r, bgClr.g, bgClr.b, bgClr.a))
    {
        eNum = 8;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        /* free any dynamically allocated memory */
        if (te != NULL)
        {
            neuik_Object_Free(te);
        }
        te = NULL;
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewTextEdit
 *
 *  Description:   Wrapper function for the Object_New function.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewTextEdit(
    NEUIK_TextEdit ** tePtr)
{
    return neuik_Object_New__TextEdit((void**)tePtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeTextEdit
 *
 *  Description:   Create a new NEUIK_TextEdit and assign text to it.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeTextEdit(
    NEUIK_TextEdit ** tePtr, /* [out] The newly created NEUIK_TextEdit. */
    const char      * text)  /* [in]  Initial TextEdit text. */
{
    int              eNum       = 0; /* which error to report (if any) */
    NEUIK_TextEdit * te         = NULL;
    static char      funcName[] = "NEUIK_MakeTextEdit";
    static char    * errMsgs[]  = {"", // [0] no error
        "Failure in function `neuik_Object_New__TextEdit`.", // [1]
        "Failure in function `neuik_TextBlock_SetText`.",    // [2]
    };

    if (neuik_Object_New__TextEdit((void**)tePtr))
    {
        eNum = 1;
        goto out;
    }
    te = *tePtr;

    if (text != NULL)
    {
        if (neuik_TextBlock_SetText(te->textBlk, text))
        {
            eNum = 2;
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
 *  Name:          neuik_Object_Free__TextEdit
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__TextEdit(
    void * tePtr)  /* [out] the button to free */
{
    int              eNum       = 0; /* which error to report (if any) */
    NEUIK_TextEdit * te         = NULL;
    static char      funcName[] = "neuik_Object_Free__TextEdit";
    static char    * errMsgs[]  = {"", // [0] no error
        "Argument `tePtr` is NULL.",                  // [1]
        "Argument `tePtr` is not of TextEdit class.", // [2]
        "Failure in function `neuik_Object_Free`.",   // [3]
    };

    if (tePtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    te = (NEUIK_TextEdit*)tePtr;

    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_Free(te->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    if (te->textSurf != NULL) SDL_FreeSurface(te->textSurf);
    if (te->textTex  != NULL) SDL_DestroyTexture(te->textTex);
    if (te->textRend != NULL) SDL_DestroyRenderer(te->textRend);

    if (neuik_Object_Free(te->cfg))
    {
        eNum = 3;
        goto out;
    }
    if (neuik_Object_Free(te->textBlk))
    {
        eNum = 3;
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
 *  Name:          neuik_Element_GetMinSize__TextEdit
 *
 *  Description:   Returns the rendered size of a given button.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__TextEdit(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int                    tW;
    int                    tH;
    int                    eNum = 0;    /* which error to report (if any) */
    TTF_Font             * font = NULL;
    NEUIK_TextEdit       * te   = NULL;
    NEUIK_TextEditConfig * aCfg = NULL; /* the active button config */
    static char            funcName[] = "neuik_Element_GetMinSize__TextEdit";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of TextEdit class.", // [1]
        "TextEditConfig* is NULL.",                  // [2]
        "TextEditConfig->FontSet is NULL.",          // [3]
        "FontSet_GetFont returned NULL.",            // [4]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    te = (NEUIK_TextEdit *)elem;
    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
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

    /* Determine the full size of the rendered text content */
    if (aCfg->fontMono)
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSetMS, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
    }
    else
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
    }
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
 *  Name:          NEUIK_TextEdit_SetText
 *
 *  Description:   Update the text in a NEUIK_TextEdit.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEdit_SetText(
    NEUIK_TextEdit * te,
    const char     * text)
{
    int           eNum    = 0; /* which error to report (if any) */
    RenderSize    rSize;
    RenderLoc     rLoc;
    static char   funcName[] = "NEUIK_TextEdit_SetText";
    static char * errMsgs[] = {"", // [0] no error
        "Argument `te` is not of TextEdit class.",          // [1]
        "Failure in function `neuik_TextBlock_SetText`.",   // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
    {
        eNum = 1;
        goto out;
    }


    /*------------------------------------------------------------------------*/
    /* Set the new TextEdit text contents                                    */
    /*------------------------------------------------------------------------*/
    if (text != NULL)
    {
        if (neuik_TextBlock_SetText(te->textBlk, text))
        {
            eNum = 2;
            goto out;
        }
    }

    te->highlightIsSet     = FALSE;
    te->highlightBeginPos  = 0;
    te->highlightBeginLine = 0;
    te->highlightStartPos  = 0;
    te->highlightStartLine = 0;
    te->highlightEndPos    = 0;
    te->highlightEndLine   = 0;
    te->clickOrigin        = 0;
    te->clickHeld          = FALSE;

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
 *  Name:          NEUIK_TextEdit_GetText
 *
 *  Description:   Get a copy of the text stored within a NEUIK_TextEdit.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEdit_GetText(
    NEUIK_TextEdit  * te,
    char           ** textPtr)
{
    int           eNum         = 0; /* which error to report (if any) */
    size_t        nLines       = 0;
    size_t        startLineNo  = 0;
    size_t        startLinePos = 0;
    size_t        endLineNo    = 0;
    size_t        endLinePos   = 0;
    static char   funcName[] = "NEUIK_TextEdit_GetText";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `te` is not of TextEdit class.",       // [1]
        "Output argument `textPtr` is NULL.",            // [2]
        "Failure in `neuik_TextBlock_GetLineCount()`.",  // [3]
        "Failure in `neuik_TextBlock_GetLineLength()`.", // [4]
        "Failure in `neuik_TextBlock_GetSection()`.",    // [5]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
    {
        eNum = 1;
        goto out;
    }
    if (textPtr == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Get text contents from TextBlock.                                      */
    /*------------------------------------------------------------------------*/
    if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
    {
        eNum = 3;
        goto out;
    }
    endLineNo = nLines - 1;

    if (neuik_TextBlock_GetLineLength(te->textBlk, endLineNo, &endLinePos))
    {
        eNum = 4;
        goto out;
    }

    if (neuik_TextBlock_GetSection(
        te->textBlk, startLineNo, startLinePos, endLineNo, endLinePos, textPtr))
    {
        eNum = 5;
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
 *  Name:          NEUIK_TextEdit_GetHighlightInfo
 *
 *  Description:   Get the number of lines and characters within the highlighted
 *                 selection.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
int NEUIK_TextEdit_GetHighlightInfo(
    NEUIK_TextEdit * te,
    size_t         * nLines,
    size_t         * nChars)
{
    int           eNum    = 0; /* which error to report (if any) */
    static char   funcName[] = "NEUIK_TextEdit_GetHighlightInfo";
    static char * errMsgs[] = {"", // [0] no error
        "Argument `te` is not of TextEdit class.",                   // [1]
        "Output Argument `nLines` is NULL.",                         // [2]
        "Output Argument `nChars` is NULL.",                         // [3]
        "Failure in function `neuik_TextBlock_GetSectionLength()`.", // [4]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
    {
        eNum = 1;
        goto out;
    }
    if (nLines == NULL)
    {
        eNum = 2;
        goto out;
    }
    if (nChars == NULL)
    {
        eNum = 3;
        goto out;
    }

    *nLines = 0;
    *nChars = 0;

    if (!te->highlightIsSet)
    {
        /*--------------------------------------------------------------------*/
        /* There is no highlight; return zeros.                               */
        /*--------------------------------------------------------------------*/
        goto out;
    }
    
    if (te->highlightStartLine == te->highlightEndLine)
    {
        /*--------------------------------------------------------------------*/
        /* All highlighted characters exist within the same line.             */
        /*--------------------------------------------------------------------*/
        *nChars = te->highlightEndPos - te->highlightStartPos;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* The highlighted characters span more than one line.                */
        /*--------------------------------------------------------------------*/
        *nLines = 1 + te->highlightEndLine - te->highlightStartLine;
        if (neuik_TextBlock_GetSectionLength(te->textBlk,
            te->highlightStartLine, te->highlightStartPos,
            te->highlightEndLine, te->highlightEndPos,
            nChars))
        {
            eNum = 4;
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


void neuik_TextEdit_Configure_capture_segv(
    int sig_num)
{
    static char funcName[] = "NEUIK_TextEdit_Configure";
    static char errMsg[] = 
        "SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

    NEUIK_RaiseError(funcName, errMsg);
    NEUIK_BacktraceErrors();
    exit(1);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_TextEdit_Configure
 *
 *  Description:   Configure a number of properties specific to NEUIK_TextEdit.
 *
 *                 This list of named sets must be terminated by a NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEdit_Configure(
    NEUIK_TextEdit * te,
    const char     * set0,
    ...)
{
    int                    ns; /* number of items from sscanf */
    int                    ctr;
    int                    nCtr;
    int                    eNum       = 0; /* which error to report (if any) */
    int                    doRedraw   = FALSE;
    int                    vaStarted  = 0;
    int                    isBool     = FALSE;
    int                    boolVal    = FALSE;
    int                    typeMixup  = FALSE;
    int                    fontSize;
    va_list                args;
    RenderSize             rSize;
    RenderLoc              rLoc;
    char                   buf[4096];
    char                 * strPtr     = NULL;
    char                 * name       = NULL;
    char                 * value      = NULL;
    const char           * set        = NULL;
    NEUIK_Color            clr;
    NEUIK_TextEditConfig * aCfg       = NULL; /* the active button config */
    /*------------------------------------------------------------------------*/
    /* If a `name=value` string with an unsupported name is found, check to   */
    /* see if a boolName was mistakenly used instead.                         */
    /*------------------------------------------------------------------------*/
    static char          * boolNames[] = {
        "FontBold",
        "FontItalic",
        "FontMono",
        NULL,
    };
    /*------------------------------------------------------------------------*/
    /* If a boolName string with an unsupported name is found, check to see   */
    /* if a supported nameValue type was mistakenly used instead.             */
    /*------------------------------------------------------------------------*/
    static char          * valueNames[] = {
        "HJustify",
        "VJustify",
        "FontColor",
        "FontSize",
        "FontColor",
        NULL,
    };
    static char            funcName[] = "NEUIK_TextEdit_Configure";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `te` is not of TextEdit class.",                  // [1]
        "NamedSet.name is NULL, skipping..",                        // [2]
        "NamedSet.name is blank, skipping..",                       // [3]
        "Invalid `bool` string.",                                   // [4]
        "`name=value` string is too long.",                         // [5]
        "Invalid `name=value` string.",                             // [6]
        "HJustify value is invalid.",                               // [7]
        "VJustify value is invalid.",                               // [8]
        "BoolType name used as ValueType, skipping.",               // [9]
        "NamedSet.name type unknown, skipping.",                    // [10]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",         // [11]
        "FontColor value invalid; should be comma separated RGBA.", // [12]
        "FontColor value invalid; RGBA value range is 0-255.",      // [13]
        "FontSize value is invalid; must be int.",                  // [14]
    };

    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
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
            signal(SIGSEGV, neuik_TextEdit_Configure_capture_segv);
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
                /*------------------------------------------------------------*/
                /* Bool type configuration (or a mistake)                     */
                /*------------------------------------------------------------*/
                if (buf[0] == 0)
                {
                    NEUIK_RaiseError(funcName, errMsgs[4]);
                }

                isBool  = 1;
                boolVal = 1;
                name    = buf;
                if (buf[0] == '!')
                {
                    boolVal = 0;
                    name    = buf + 1;
                }
            }
            else
            {
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
        }

        if (isBool)
        {
            /*----------------------------------------------------------------*/
            /* Check for boolean parameter setting.                           */
            /*----------------------------------------------------------------*/
            if (!strcmp("FontBold", name))
            {
                if (aCfg->fontBold == boolVal) continue;

                /* else: The previous setting was changed */
                aCfg->fontBold = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("FontItalic", name))
            {
                if (aCfg->fontItalic == boolVal) continue;

                /* else: The previous setting was changed */
                aCfg->fontItalic = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("FontMono", name))
            {
                if (aCfg->fontMono == boolVal) continue;

                /* else: The previous setting was changed */
                aCfg->fontMono = boolVal;
                doRedraw = 1;
            }
            else 
            {
                /*------------------------------------------------------------*/
                /* Bool parameter not found; may be mixup or mistake .        */
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
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                }
            }
            else if (!strcmp("FontColor", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[12]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[12]);
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
                    NEUIK_RaiseError(funcName, errMsgs[12]);
                    continue;
                }

                if (clr.r < 0 || clr.r > 255 ||
                    clr.g < 0 || clr.g > 255 ||
                    clr.b < 0 || clr.b > 255 ||
                    clr.a < 0 || clr.a > 255)
                {
                    NEUIK_RaiseError(funcName, errMsgs[13]);
                    continue;
                }
                if (aCfg->fgColor.r == clr.r &&
                    aCfg->fgColor.g == clr.g &&
                    aCfg->fgColor.b == clr.b &&
                    aCfg->fgColor.a == clr.a) continue;

                /* else: The previous setting was changed */
                aCfg->fgColor = clr;
                doRedraw = 1;
            }
            else if (!strcmp("FontSize", name))
            {
                ns = sscanf(value, "%d", &fontSize);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
            #ifndef WIN32
                if (ns == EOF || ns < 1)
            #else
                if (ns < 1)
            #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[14]);
                    continue;
                }
                if (aCfg->fontSize == fontSize) continue;

                /* else: The previous setting was changed */
                aCfg->fontSize = fontSize;
                doRedraw = 1;
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
                    NEUIK_RaiseError(funcName, errMsgs[9]);
                }
                else
                {
                    /* An unsupported name was used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[10]);
                }
            }
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
    if (doRedraw)
    {
        if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
        {
            eNum = 11;
            NEUIK_RaiseError(funcName, errMsgs[eNum]);
            eNum = 1;
        }
        else
        {
            neuik_Element_RequestRedraw(te, rLoc, rSize);
        }
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__TextEdit
 *
 *  Description:   Renders a single TextEdit as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__TextEdit(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    char                   tempChar;          /* a temporary character */
    float                  yPos        = 0;
    float                  blankH      = 0;
    int                    blankW      = 0;
    int                    borderW     = 1; /* width of button border line */
    int                    ctr         = 0;
    int                    textW       = 0;
    int                    textH       = 0;
    int                    textWFull   = 0;
    float                  textHFull   = 0;
    int                    hlWidth     = 0;    /* highlight bg Width */
    int                    eNum        = 0;    /* which error to report (if any) */
    int                    hasText     = 1;
    int                    scrollHt    = 0;
    int                    scrollX     = 0;
    int                    scrollY     = 0;
    int                    scrollDrawn = FALSE;
    int                    partialDraw = FALSE;
    int                    borderX     = 0;
    size_t                 lineLen;
    size_t                 lineCtr;
    size_t                 nLines;
    double                 scrollFrac  = 0.0;
    double                 scrollPct   = 0.0;
    double                 viewFrac    = 0.0;
    double                 viewPct     = 0.0;
    char                 * lineBytes   = NULL;
    RenderLoc              rl;
    SDL_Rect               srcRect;
    SDL_Rect               rect;
    SDL_Rect               scrollRect;
    const NEUIK_Color    * fgClr       = NULL;
    const NEUIK_Color    * bgClr       = NULL;
    const NEUIK_Color    * bClr        = NULL; /* border color */
    SDL_Renderer         * rend        = NULL;
    SDL_Texture          * tTex        = NULL; /* text texture */
    TTF_Font             * font        = NULL;
    NEUIK_ElementBase    * eBase       = NULL;
    neuik_MaskMap        * maskMap     = NULL;
    NEUIK_TextEdit       * te          = NULL;
    NEUIK_TextEditConfig * aCfg        = NULL; /* the active textEntry config */
    static char            funcName[]  = "neuik_Element_Render__TextEdit";
    static char          * errMsgs[]   = {"", // [0] no error
        "Argument `elem` is not of TextEdit class.",                     // [1]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "", // [3]
        "Invalid specified `rSize` (negative values).",                  // [4]
        "Failure in `neuik_MakeMaskMap()`",                              // [5]
        "FontSet_GetFont returned NULL.",                                // [6]
        "", // [7]
        "Failure in function `neuik_TextBlock_GetLineCount`.",           // [8]
        "Failure in function `neuik_TextBlock_GetLine`.",                // [9]
        "Failure in function `neuik_TextBlock_GetLineLength`.",          // [10]
        "Failure in neuik_Element_RedrawBackground().",                  // [11]
    };

    te = (NEUIK_TextEdit *)elem;
    if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
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
    aCfg = te->cfg;
    if (te->cfgPtr != NULL)
    {
        aCfg = te->cfgPtr;
    }

    /* extract the current fg/bg colors */
    fgClr = &(aCfg->fgColor);

    /*------------------------------------------------------------------------*/
    /* Get the pointer to the currently active font (if text is present)      */
    /*------------------------------------------------------------------------*/
    /* Determine the full size of the rendered text content */
    if (aCfg->fontMono)
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSetMS, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
    }
    else
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
    }
    if (font == NULL) 
    {
        eNum = 6;
        goto out;
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
    /* Create a MaskMap an mark off the transparent pixels.                   */
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
        eNum = 11;
        goto out;
    }
    bgClr = &(aCfg->bgColor);

    rl = eBase->eSt.rLoc;

    /*------------------------------------------------------------------------*/
    /* Redraw the contained text and highlighting (if present)                */
    /*------------------------------------------------------------------------*/
    if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* If there is only one line of text, check to see if there is any text   */
    /* data at all before going through the trouble of drawing text.          */
    /*------------------------------------------------------------------------*/
    if (nLines == 1)
    {
        if (neuik_TextBlock_GetLineLength(te->textBlk, 0, &lineLen))
        {
            eNum = 10;
            goto out;
        }
        if (lineLen == 0)
        {
            hasText = 0;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Only redraw the text section if there is valid text in this TextBlock. */
    /*------------------------------------------------------------------------*/
    if (!hasText) goto draw_border;

    TTF_SizeText(font, " ", &textW, &textH);
    blankW = (int)(0.65*(float)(textW));
    blankH = 1.1*(float)(TTF_FontHeight(font));

    /*------------------------------------------------------------------------*/
    /* There appears to be one or more lines of valid text in the Block.      */
    /* Place the lines one-at-a-time where they should go.                    */
    /*------------------------------------------------------------------------*/
    yPos = 2.0;
    for (lineCtr = te->vertPanLn; lineCtr < nLines; lineCtr++)
    {
        partialDraw = FALSE;
        if ((int)(yPos) > rSize->h)
        {
            /*----------------------------------------------------------------*/
            /* The next line of text exists at a position in excess of what   */
            /* should be drawn to the window.                                 */
            /*----------------------------------------------------------------*/
            break;
        }
        if (te->vertPanLn > 0 && lineCtr == te->vertPanLn)
        {
            /*----------------------------------------------------------------*/
            /* This line of text is the first line of text shown in a view    */
            /* that is partially scrolled. The top of the line will need to   */
            /* be cropped.                                                    */
            /*----------------------------------------------------------------*/
            partialDraw = TRUE;
        }

        if (neuik_TextBlock_GetLine(te->textBlk, lineCtr, &lineBytes))
        {
            eNum = 9;
            goto out;
        }

        if (lineBytes[0] != '\0')
        {
            /* Determine the full size of the rendered text content */
            TTF_SizeText(font, lineBytes, &textW, &textH);
            textWFull = textW;

            /*----------------------------------------------------------------*/
            /* Create an SDL_Surface for the text within the element          */
            /*----------------------------------------------------------------*/
            textHFull = 1.1*(float)(textH);
            te->textSurf = SDL_CreateRGBSurface(
                0, textW+blankW, (int)(textHFull), 32, 0, 0, 0, 0);
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
            SDL_SetRenderDrawColor(
                te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
            SDL_RenderClear(te->textRend);

            /*----------------------------------------------------------------*/
            /* Render the Text now, it will be copied on after highlighting.  */
            /*----------------------------------------------------------------*/
            tTex = NEUIK_RenderText(
                lineBytes, font, *fgClr, te->textRend, &textW, &textH);
            if (tTex == NULL)
            {
                eNum = 6;
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* Check for and fill in highlight text selection background.     */
            /*----------------------------------------------------------------*/
            if ( (eBase->eSt.hasFocus && te->highlightIsSet) &&
                    (lineCtr >= te->highlightStartLine && 
                     lineCtr <= te->highlightEndLine))
            {
                rect.x = 0;
                rect.y = 0;
                rect.w = textW + 1;
                rect.h = (int)(textHFull);

                textW = 0;
                textH = 0;
                /* determine the point of the start of the bgkd highlight */
                if (lineCtr > te->highlightStartLine)
                {
                    /*--------------------------------------------------------*/
                    /* The start of the line will be highlighted.             */
                    /*--------------------------------------------------------*/
                    if (lineCtr < te->highlightEndLine)
                    {
                        /*----------------------------------------------------*/
                        /* highlight the entire line.                         */
                        /*----------------------------------------------------*/
                        TTF_SizeText(font, lineBytes, &textW, &textH);
                        textW += blankW;
                    }
                    else if (te->highlightEndPos != 0)
                    {
                        /*----------------------------------------------------*/
                        /* The highlight ends within this line.               */
                        /*----------------------------------------------------*/
                        tempChar = lineBytes[te->highlightEndPos];
                        lineBytes[te->highlightEndPos] = '\0';
                        TTF_SizeText(font, lineBytes, &textW, &textH);
                        lineBytes[te->highlightEndPos] = tempChar;
                    }
                }
                else if (lineCtr == te->highlightStartLine)
                {
                    /*--------------------------------------------------------*/
                    /* The highlighted block starts on this line.             */
                    /*--------------------------------------------------------*/
                    if (te->highlightStartPos != 0)
                    {
                        tempChar = lineBytes[te->highlightStartPos];
                        lineBytes[te->highlightStartPos] = '\0';
                        TTF_SizeText(font, lineBytes, &textW, &textH);
                        lineBytes[te->highlightStartPos] = tempChar;
                    }
                    rect.x += textW;

                    /*--------------------------------------------------------*/
                    /* Determine the point of the start of the bgkd highlight */
                    /*--------------------------------------------------------*/
                    lineLen = strlen(lineBytes);

                    if (te->highlightEndLine > lineCtr)
                    {
                        /*----------------------------------------------------*/
                        /* highlight the rest of the line.                    */
                        /*----------------------------------------------------*/
                        TTF_SizeText(font, 
                            lineBytes + te->highlightStartPos, 
                            &textW, &textH);
                        textW += blankW;
                    }
                    else
                    {
                        tempChar = lineBytes[te->highlightEndPos];
                        lineBytes[te->highlightEndPos] = '\0';
                        TTF_SizeText(font, 
                            lineBytes + te->highlightStartPos, 
                            &textW, &textH);
                        lineBytes[te->highlightEndPos] = tempChar;
                    }
                }
                hlWidth = textW;
                rect.w = hlWidth;

                bgClr = &(aCfg->bgColorHl);
                SDL_SetRenderDrawColor(
                    te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
                SDL_RenderFillRect(te->textRend, &rect);
                bgClr = &(aCfg->bgColor);
            }

            /*----------------------------------------------------------------*/
            /* Copy over the previously rendered text.                        */
            /*----------------------------------------------------------------*/
            rect.x = 0;
            rect.y = 0;
            rect.w = textWFull + 1;
            rect.h = (int)(textHFull);

            SDL_RenderCopy(te->textRend, tTex, NULL, &rect);

            /*----------------------------------------------------------------*/
            /* Draw the cursor (if TextEdit is focused)                       */
            /*----------------------------------------------------------------*/
            if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
            {
                /*------------------------------------------------------------*/
                /* Draw the cursor line into the TextEdit element             */
                /*------------------------------------------------------------*/
                SDL_SetRenderDrawColor(
                    te->textRend, fgClr->r, fgClr->g, fgClr->b, 255);

                tempChar = lineBytes[te->cursorPos];
                if (tempChar == '\0')
                {
                    rect.x = textWFull - 2;
                }
                else
                {
                    lineBytes[te->cursorPos] = '\0';
                    TTF_SizeText(font, lineBytes, &textW, &textH);
                    lineBytes[te->cursorPos] = tempChar;

                    /* this will be the position of the cursor */
                    rect.x = textW;
                }
                te->cursorX = rect.x;
                SDL_RenderDrawLine(te->textRend, 
                    rect.x, rect.y, 
                    rect.x, rect.y + rect.h); 
                SDL_RenderDrawLine(te->textRend, 
                    rect.x+1, rect.y, 
                    rect.x+1, rect.y + rect.h); 
            }

            SDL_RenderPresent(te->textRend);
            te->textTex = SDL_CreateTextureFromSurface(rend, te->textSurf);
            if (te->textTex == NULL)
            {
                eNum = 7;
                goto out;
            }

            rect.x = rl.x + 6;
            rect.y = rl.y + (int)(yPos);
            rect.w = textWFull + blankW;
            rect.h = (int)(textHFull);

            if (partialDraw)
            {
                /*------------------------------------------------------------*/
                /* This line of text is the first line of text shown in a     */
                /* view that is partially scrolled. The top of the line will  */
                /* need to be cropped.                                        */
                /*------------------------------------------------------------*/
                srcRect.x = 0;
                srcRect.y = te->vertPanPx;
                srcRect.w = textWFull + blankW;
                srcRect.h = (int)(blankH) - te->vertPanPx;

                rect.h = srcRect.h;

                SDL_RenderCopy(rend, te->textTex, &srcRect, &rect);
                ConditionallyDestroyTexture(&tTex);
            }
            else if ((int)(yPos) + (int)(textHFull) <= rSize->h - 2)
            {
                /*------------------------------------------------------------*/
                /* This line of text has enough vertical space to be fully    */
                /* drawn.                                                     */
                /*------------------------------------------------------------*/
                SDL_RenderCopy(rend, te->textTex, NULL, &rect);
                ConditionallyDestroyTexture(&tTex);
            }
            else
            {
                /*------------------------------------------------------------*/
                /* The next line of text only has enough space to be drawn    */
                /* partially.                                                 */
                /*------------------------------------------------------------*/
                srcRect.x = 0;
                srcRect.y = 0;
                srcRect.w = textWFull + blankW;
                srcRect.h = rSize->h - (2 + (int)(yPos));

                rect.h = srcRect.h;

                SDL_RenderCopy(rend, te->textTex, &srcRect, &rect);
                ConditionallyDestroyTexture(&tTex);
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* This is an empty line, but if this empty line is included in a */
            /* highlighted section, a tiny section of the left side of the    */
            /* should be highlighted. Additionally, the cursor may be present */
            /* on this line.                                                  */
            /*----------------------------------------------------------------*/
            /* Check for and fill in highlight text selection background.     */
            /*----------------------------------------------------------------*/
            if ( (eBase->eSt.hasFocus && te->highlightIsSet) &&
                    (lineCtr >= te->highlightStartLine && 
                     lineCtr < te->highlightEndLine))
            {
                rect.x = rl.x + 6;
                rect.y = rl.y + (int)(yPos);
                rect.w = blankW + 1;
                rect.h = (int)(blankH);

                bgClr = &(aCfg->bgColorHl);
                SDL_SetRenderDrawColor(
                    rend, bgClr->r, bgClr->g, bgClr->b, 255);
                SDL_RenderFillRect(rend, &rect);
                bgClr = &(aCfg->bgColor);
            }

            /*----------------------------------------------------------------*/
            /* This is a blank line but the cursor may be present.            */
            /*----------------------------------------------------------------*/
            TTF_SizeText(font, " ", &textW, &textH);
            textHFull = 1.1*(float)(textH);

            /*----------------------------------------------------------------*/
            /* Conditionally draw the cursor line into the TextEdit.          */
            /*----------------------------------------------------------------*/
            if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
            {
                /*------------------------------------------------------------*/
                /* Position the cursor at the start of the line.              */
                /*------------------------------------------------------------*/
                rect.x = rl.x + 6;
                rect.y = rl.y + (int)(yPos);
                rect.h = (int)(textHFull);

                SDL_SetRenderDrawColor(rend, fgClr->r, fgClr->g, fgClr->b, 255);
                SDL_RenderDrawLine(rend, 
                    rect.x, rect.y, 
                    rect.x, rect.y + rect.h); 
                SDL_RenderDrawLine(rend, 
                    rect.x+1, rect.y, 
                    rect.x+1, rect.y + rect.h); 
            }
        }

        yPos += blankH;
        if (partialDraw)
        {
            yPos -= (float)(te->vertPanPx);
        }

        if (lineBytes != NULL)
        {
            free(lineBytes);
            lineBytes = NULL;
        }
        if (te->textTex != NULL)
        {
            SDL_DestroyTexture(te->textTex);
            te->textTex = NULL;
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
    }

    /*------------------------------------------------------------------------*/
    /* Update the scroll and view percentages; used for the scrollbar.        */
    /*------------------------------------------------------------------------*/
    scrollPct = 100.0*(
        ((double)(te->vertPanLn) + ((double)(te->vertPanPx) / (double)(blankH))) /
            (double)(nLines));
    if (nLines == te->vertPanLn + 1)
    {
        scrollPct = 100.0;      
    }
    viewPct   = 100.0*(((double)(rSize->h - 2)/(double)(blankH))/(double)(nLines));
    if (viewPct < 5.0)
    {
        /*--------------------------------------------------------------------*/
        /* If the view percent is less than five percent, make the scroll     */
        /* slider occupy 5%, otherwise it will be too small.                  */
        /*--------------------------------------------------------------------*/
        viewPct = 5.0;
    }

    if (viewPct < 100.0)
    {
        /*--------------------------------------------------------------------*/
        /* Draw the scrollbar on the right side of the TextEdit.              */
        /*--------------------------------------------------------------------*/
        bClr = &(aCfg->bgScrollColor);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

        /* Fill in the whole scrollbar area with its background color */
        if (neuik__HighDPI_Scaling <= 1.0)
        {
            scrollX = rl.x + (rSize->w - 12);
        }
        else
        {
            scrollX = rl.x + (rSize->w - 
                (2 + (int)(10.0*neuik__HighDPI_Scaling)));
        }

        scrollRect.x = scrollX;
        scrollRect.y = rl.y + (1);
        scrollRect.w = 10;
        if (neuik__HighDPI_Scaling > 1.0)
        {
            scrollRect.w = (int)(10.0*neuik__HighDPI_Scaling);
        }
        scrollRect.h = rSize->h - 2;

        SDL_RenderFillRect(rend, &scrollRect); 

        /* Draw the scrollbar slider */
        bClr = &(aCfg->scrollSliderColor);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

        scrollFrac = scrollPct/100.0;
        viewFrac   = viewPct/100.0;
        scrollHt   = (rSize->h - 2) - (int)(viewFrac*(double)(rSize->h - 2));

        /* Fill in the whole slider area */
        scrollY = rl.y + (1 + scrollHt*scrollFrac);

        scrollRect.x = scrollX;
        scrollRect.y = scrollY;
        scrollRect.w = 10;
        if (neuik__HighDPI_Scaling > 1.0)
        {
            scrollRect.w = (int)(10.0*neuik__HighDPI_Scaling);
        }
        scrollRect.h = (int)(viewFrac*(double)(rSize->h - 2));
        SDL_RenderFillRect(rend, &scrollRect); 

        bClr = &(aCfg->bgScrollColor);
        SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

        /* Round off the top of the slider */
        SDL_RenderDrawPoint(rend, scrollX, scrollY); 
        SDL_RenderDrawPoint(rend, scrollX + 9, scrollY); 

        /* Round off the bottom of the slider */
        scrollY = rl.y + (1 + scrollHt*scrollFrac) + 
            (int)(viewFrac*(double)(rSize->h - 2)) - 1;
        SDL_RenderDrawPoint(rend, scrollX, scrollY); 
        SDL_RenderDrawPoint(rend, scrollX + 9, scrollY);

        scrollDrawn = TRUE;
    }

draw_border:
    /*------------------------------------------------------------------------*/
    /* Draw the border around the TextEdit.                                   */
    /*------------------------------------------------------------------------*/
    bClr = &(aCfg->borderColor);
    SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

    borderX = rl.x + (rSize->w - 2);
    if (scrollDrawn)
    {
        borderX = scrollX;
    }

    /* upper border line */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            rl.x + 1, (rl.y + 1) + ctr, 
            borderX,  (rl.y + 1) + ctr); 
    }
    /* left border line */
    for (ctr = 0; ctr < borderW; ctr++)
    {
        SDL_RenderDrawLine(rend, 
            (rl.x + 1) + ctr, rl.y + 1, 
            (rl.x + 1) + ctr, rl.y + (rSize->h - 2));

    }

    if (!scrollDrawn)
    {
        /* right border line */
        for (ctr = 0; ctr < borderW; ctr++)
        {
            SDL_RenderDrawLine(rend, 
                rl.x + (rSize->w - 2) - ctr, rl.y + 1, 
                rl.x + (rSize->w - 2) - ctr, rl.y + (rSize->h - 2));
        }
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
 *  Name:          neuik_Element_Defocus__TextEdit
 *
 *  Description:   Defocus the TextEdit element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_Defocus__TextEdit(
    NEUIK_Element el)
{
    NEUIK_TextEdit * te;
    RenderSize       rSize = {0, 0};
    RenderLoc        rLoc  = {0, 0};

    SDL_StopTextInput();
    te = (NEUIK_TextEdit*) el;

    if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
    {
        return;
    }
    neuik_Element_RequestRedraw(te, rLoc, rSize);
    te->vertMovePos        = UNDEFINED;
    te->highlightIsSet     = 0;
    te->highlightBeginLine = 0;
    te->highlightBeginPos  = 0;
    te->highlightStartLine = 0;
    te->highlightStartPos  = 0;
    te->highlightEndLine   = 0;
    te->highlightEndPos    = 0;
    te->clickOrigin        = 0;
    te->clickHeld          = 0;
}

