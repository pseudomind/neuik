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
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_ToggleButton.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ToggleButton(void ** btnPtr);
int neuik_Object_Free__ToggleButton(void * btnPtr);
int neuik_Element_GetMinSize__ToggleButton(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ToggleButton(
    NEUIK_Element, SDL_Event*);
int neuik_Element_Render__ToggleButton(
    NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ToggleButton_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ToggleButton,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ToggleButton,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ToggleButton_FuncTable = {
    /* GetMinSize(): Get the minimum required size for the element  */
    neuik_Element_GetMinSize__ToggleButton,

    /* Render(): Redraw the element  element  */
    neuik_Element_Render__ToggleButton,

    /* CaptureEvent(): Determine if this element caputures a given event */
    neuik_Element_CaptureEvent__ToggleButton,

    /* Defocus(): This function will be called when an element looses focus */
    NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ToggleButton
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ToggleButton()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ToggleButton";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",         // [1]
        "Failed to register `ToggleButton` object class .", // [2]
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
        "ToggleButton",                                // className
        "A GUI toggle button which may contain text.", // classDescription
        neuik__Set_NEUIK,                              // classSet
        neuik__Class_Element,                          // superClass
        &neuik_ToggleButton_BaseFuncs,                 // baseFuncs
        NULL,                                          // classFuncs
        &neuik__Class_ToggleButton))                   // newClass
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
 *  Name:          neuik_Object_New__ToggleButton
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ToggleButton(
    void ** btnPtr)
{
    int                   eNum       = 0; /* which error to report (if any) */
    NEUIK_ToggleButton  * btn        = NULL;
    NEUIK_Element       * sClassPtr  = NULL;
    static char           funcName[] = "neuik_Object_New__ToggleButton";
    static char         * errMsgs[]  = {"", // [0] no error
        "Failure to allocate memory.",                            // [1]
        "Failure in NEUIK_NewToggleButtonConfig.",                // [2]
        "Output Argument `btnPtr` is NULL.",                      // [3]
        "Failure in function `neuik_Object_New`.",                // [4]
        "Failure in function `neuik_Element_SetFuncTable`.",      // [5]
        "Failure in `neuik_GetObjectBaseOfClass`.",               // [6]
        "Failure in `NEUIK_Element_SetBackgroundColorGradient`.", // [7]
    };

    if (btnPtr == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*btnPtr) = (NEUIK_ToggleButton*) malloc(sizeof(NEUIK_ToggleButton));
    btn = *btnPtr;
    if (btn == NULL)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ToggleButton, 
            NULL,
            &(btn->objBase)))
    {
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(btn->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
        goto out;
    }
    if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_ToggleButton_FuncTable))
    {
        eNum = 5;
        goto out;
    }

    /* Allocation successful */
    btn->cfg          = NULL;
    btn->cfgPtr       = NULL;
    btn->selected     = 0;
    btn->wasSelected  = 0;
    btn->activated    = 0;
    btn->isActive     = 0;
    btn->clickOrigin  = 0;
    btn->needsRedraw  = 1;
    btn->cfgPtr       = NULL;

    if (NEUIK_NewToggleButtonConfig(&btn->cfg))
    {
        eNum = 2;
        goto out;
    }
    
    /*------------------------------------------------------------------------*/
    /* Set the default element background redraw styles.                      */
    /*------------------------------------------------------------------------*/
    if (NEUIK_Element_SetBackgroundColorGradient(btn, "normal", 'v',
        "220,220,220,255,0.0",
        "200,200,200,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(btn, "selected", 'v',
        "120,120,120,255,0.0",
        "165,165,165,255,1.0",
        NULL))
    {
        eNum = 7;
        goto out;
    }
    if (NEUIK_Element_SetBackgroundColorGradient(btn, "hovered", 'v',
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
 *  Name:          neuik_Object_Free__ToggleButton
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ToggleButton(
    void * btnPtr)  /* [out] the button to free */
{
    int                  eNum       = 0; /* which error to report (if any) */
    NEUIK_ToggleButton * btn        = NULL;
    static char          funcName[] = "neuik_Object_Free__ToggleButton";
    static char        * errMsgs[]  = {"", // [0] no error
        "Argument `btnPtr` is not of Button class.", // [1]
        "Failure in function `neuik_Object_Free`.",  // [2]
        "Argument `btnPtr` is NULL.",                // [3]
    };

    if (btnPtr == NULL)
    {
        eNum = 3;
        goto out;
    }
    btn = (NEUIK_ToggleButton*)btnPtr;

    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(btn->objBase.superClassObj))
    {
        eNum = 2;
        goto out;
    }
    if(btn->text != NULL) free(btn->text);
    if(neuik_Object_Free(btn->cfg))
    {
        eNum = 2;
        goto out;
    }

    free(btn);
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
 *  Name:          NEUIK_NewToggleButton
 *
 *  Description:   Create a new NEUIK_ToggleButton without contained text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewToggleButton(
    NEUIK_ToggleButton ** btnPtr) /* [out] The newly created NEUIK_ToggleButton. */
{
    return neuik_Object_New__ToggleButton((void **)btnPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeToggleButton
 *
 *  Description:   Create a new NEUIK_ToggleButton with specified text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeToggleButton(
    NEUIK_ToggleButton ** btnPtr,  /* [out] The newly created object  */
    const char          * text)    /* [in]  Initial button text. */
{
    size_t               sLen       = 1;
    int                  eNum       = 0; /* which error to report (if any) */
    NEUIK_ToggleButton * btn        = NULL;
    static char          funcName[] = "NEUIK_MakeToggleButton";
    static char        * errMsgs[]  = {"", // [0] no error
        "Failure in function `neuik_Object_New__ToggleButton`.", // [1]
        "Failure to allocate memory.",                           // [2]
    };

    if (neuik_Object_New__ToggleButton((void**)btnPtr))
    {
        eNum = 1;
        goto out;
    }
    btn = *btnPtr;

    /*------------------------------------------------------------------------*/
    /* Set the new Button text contents                                       */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* button will contain no text */
        btn->text = NULL;
    }
    else if (text[0] == '\0')
    {
        /* button will contain no text */
        btn->text = NULL;
    }
    else
    {
        sLen += strlen(text);
        btn->text = (char*)malloc(sLen*sizeof(char));
        if (btn->text == NULL) {
            eNum = 2;
            goto out;
        }
        /* Allocation successful */
        strcpy(btn->text, text);
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
 *  Name:          neuik_Element_GetMinSize__ToggleButton
 *
 *  Description:   Returns the rendered size of a given object.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ToggleButton(
    NEUIK_Element    elem,
    RenderSize     * rSize)
{
    int                        tW;
    int                        tH;
    int                        eNum       = 0;    /* which error to report (if any) */
    TTF_Font                 * font       = NULL;
    NEUIK_ToggleButton       * btn        = NULL;
    NEUIK_ToggleButtonConfig * aCfg       = NULL; /* the active button config */
    static char                funcName[] = 
        "neuik_Element_GetMinSize__ToggleButton";
    static char              * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of Button class.", // [1]
        "ToggleButtonConfig* is NULL.",            // [2]
        "ToggleButtonConfig->FontSet is NULL.",    // [3]
        "FontSet_GetFont returned NULL.",          // [4]
    };

    /*------------------------------------------------------------------------*/
    /* Calculate the required size of the resultant texture                   */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(elem, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }
    btn = (NEUIK_ToggleButton*)elem;
    
    /* select the correct button config to use (pointer or internal) */
    if (btn->cfgPtr != NULL)
    {
        aCfg = btn->cfgPtr;
    }
    else 
    {
        aCfg = btn->cfg;
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

    if (btn->text != NULL)
    {
        /* this button contains text */
        TTF_SizeText(font, btn->text, &tW, &tH);
    }
    else
    {
        /* this button does not contain text */
        TTF_SizeText(font, " ", &tW, &tH);
    }

    rSize->w = tW + aCfg->fontEmWidth;
    rSize->h = (int)(1.5 * (float)TTF_FontHeight(font));
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
 *  Name:          NEUIK_ToggleButton_SetText
 *
 *  Description:   Update the text in a NEUIK_ToggleButton.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ToggleButton_SetText(
    NEUIK_ToggleButton * btn,
    const char         * text)
{
    size_t         sLen = 1;
    int            eNum = 0; /* which error to report (if any) */
    RenderSize     rSize;
    RenderLoc      rLoc;
    static char    funcName[] = "NEUIK_ToggleButton_SetText";
    static char  * errMsgs[] = {"", // [0] no error
        "Argument `btn` is not of Button class.",           // [1]
        "Failure to allocate memory.",                      // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
    };

    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check first if the button already contained the desired text.          */
    /*------------------------------------------------------------------------*/
    if (btn->text != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Button currently contains text; check if new text is the same.     */
        /*--------------------------------------------------------------------*/
        if (text != NULL)
        {
            if (!strcmp(btn->text, text))
            {
                /* no change in button text; return */
                goto out;
            }
        }
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* Button currently contains no text; check if new text is also empty */
        /*--------------------------------------------------------------------*/
        if (text == NULL)
        {
            /* no change in button text; return */
            goto out;
        }
        else if (text[0] == '\0')
        {
            /* no change in button text; return */
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Conditionally free button text before setting the new contents         */
    /*------------------------------------------------------------------------*/
    if (btn->text != NULL) {
        free(btn->text);
    }

    /*------------------------------------------------------------------------*/
    /* Set the new Button text contents                                       */
    /*------------------------------------------------------------------------*/
    if (text == NULL){
        /* button will contain no text */
        btn->text = NULL;
    }
    else if (text[0] == '\0')
    {
        /* button will contain no text */
        btn->text = NULL;
    }
    else
    {
        sLen += strlen(text);
        btn->text = (char*)malloc(sLen*sizeof(char));
        if (btn->text == NULL) {
            eNum = 2;
            goto out;
        }
        /* Allocation successful */
        strcpy(btn->text, text);
    }

    if (neuik_Element_GetSizeAndLocation(btn, &rSize, &rLoc))
    {
        eNum = 3;
        goto out;
    }
    neuik_Element_RequestRedraw(btn, rLoc, rSize);
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
 *  Name:          NEUIK_ToggleButton_GetText
 *
 *  Description:   Get a pointer to the text in a NEUIK_ToggleButton.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
const char * NEUIK_ToggleButton_GetText(
    NEUIK_ToggleButton * btn)
{
    int           eNum       = 0; /* which error to report (if any) */
    const char  * rvPtr      = NULL;
    static char   emptyStr[] = "";
    static char   funcName[] = "NEUIK_ToggleButton_GetText";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `btn` is not of ToggleButton class.", // [1]
    };

    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the new Button text contents                                       */
    /*------------------------------------------------------------------------*/
    if (btn->text == NULL){
        /* button will contain no text */
        rvPtr = emptyStr;
    }
    else
    {
        rvPtr = btn->text;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        rvPtr = NULL;
    }

    return rvPtr;
}

void neuik_ToggleButton_Configure_capture_segv(
    int sig_num)
{
    static char funcName[] = "NEUIK_ToggleButton_Configure";
    static char errMsg[] = 
        "SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

    NEUIK_RaiseError(funcName, errMsg);
    NEUIK_BacktraceErrors();
    exit(1);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ToggleButton_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_ToggleButton_Configure(
    NEUIK_ToggleButton * tbtn,
    const char         * set0,
    ...)
{
    int                   ns; /* number of items from sscanf */
    int                   ctr;
    int                   nCtr;
    int                   eNum      = 0; /* which error to report (if any) */
    int                   isBool;
    int                   doRedraw  = 0;
    int                   boolVal   = 0;
    int                   typeMixup;
    int                   fontSize;
    char                  buf[4096];
    RenderSize            rSize;
    RenderLoc             rLoc;
    va_list               args;
    char                * strPtr    = NULL;
    char                * name      = NULL;
    char                * value     = NULL;
    const char          * set       = NULL;
    NEUIK_Color           clr;
    NEUIK_ToggleButtonConfig  * cfg       = NULL;
    /*------------------------------------------------------------------------*/
    /* If a `name=value` string with an unsupported name is found, check to   */
    /* see if a boolName was mistakenly used instead.                         */
    /*------------------------------------------------------------------------*/
    static char          * boolNames[] = {
        "FontBold",
        "FontItalic",
        NULL,
    };
    /*------------------------------------------------------------------------*/
    /* If a boolName string with an unsupported name is found, check to see   */
    /* if a supported nameValue type was mistakenly used instead.             */
    /*------------------------------------------------------------------------*/
    static char          * valueNames[] = {
        "FontSize"
        "FontColor",
        "FontColorPressed",
        "BGColorGrad",
        "BGColorGradPressed",
        NULL,
    };
    static char           funcName[] = "NEUIK_ToggleButton_Configure";
    static char         * errMsgs[] = {"", // [ 0] no error
        "Argument `tbtn` does not implement ToggleButton class.",          // [ 1]
        "`name=value` string is too long.",                                // [ 2]
        "Invalid `name=value` string.",                                    // [ 3]
        "ValueType name used as BoolType, skipping.",                      // [ 4]
        "BoolType name unknown, skipping.",                                // [ 5]
        "NamedSet.name is NULL, skipping..",                               // [ 6]
        "NamedSet.name is blank, skipping..",                              // [ 7]
        "FontColor value invalid; should be comma separated RGBA.",        // [ 8]
        "FontColor value invalid; RGBA value range is 0-255.",             // [ 9]
        "FontColorPressed value invalid; should be comma separated RGBA.", // [10]
        "FontColorPressed value invalid; RGBA value range is 0-255.",      // [11]
        "FontSize value is invalid; must be int.",                         // [12]
        "BoolType name used as ValueType, skipping.",                      // [13]
        "NamedSet.name type unknown, skipping.",                           // [14]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",                // [15]
    };

    if (!neuik_Object_IsClass(tbtn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }
    set = set0;

    /*------------------------------------------------------------------------*/
    /* select the correct button config to use (pointer or internal)          */
    /*------------------------------------------------------------------------*/
    cfg = tbtn->cfg;
    if (tbtn->cfgPtr != NULL)
    {
        cfg = tbtn->cfgPtr;
    }

    va_start(args, set0);

    for (ctr = 0;; ctr++)
    {
        isBool = 0;
        name   = NULL;
        value  = NULL;

        if (set == NULL) break;

        #ifndef NO_NEUIK_SIGNAL_TRAPPING
            signal(SIGSEGV, neuik_ToggleButton_Configure_capture_segv);
        #endif

        if (strlen(set) > 4095)
        {
            #ifndef NO_NEUIK_SIGNAL_TRAPPING
                signal(SIGSEGV, NULL);
            #endif
            NEUIK_RaiseError(funcName, errMsgs[2]);
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
                    NEUIK_RaiseError(funcName, errMsgs[3]);
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
            if (!strcmp("FontBold", name))
            {
                if (cfg->fontBold == boolVal) continue;

                /* else: The previous setting was changed */
                cfg->fontBold = boolVal;
                doRedraw = 1;
            }
            else if (!strcmp("FontItalic", name))
            {
                if (cfg->fontItalic == boolVal) continue;

                /* else: The previous setting was changed */
                cfg->fontItalic = boolVal;
                doRedraw = 1;
            }
            else 
            {
                /*------------------------------------------------------------*/
                /* Bool parameter not found; may be mixup or mistake .        */
                /*------------------------------------------------------------*/
                typeMixup = 0;
                for (nCtr = 0;; nCtr++)
                {
                    if (valueNames[nCtr] == NULL) break;

                    if (!strcmp(valueNames[nCtr], name))
                    {
                        typeMixup = 1;
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
            else if (!strcmp("FontColor", name))
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

                ns = sscanf(value, "%d,%d,%d,%d", 
                    &clr.r, &clr.g, &clr.b, &clr.a);
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

                cfg->fgColor = clr;
                doRedraw = 1;
            }
            else if (!strcmp("FontColorPressed", name))
            {
                /*------------------------------------------------------------*/
                /* Check for empty value errors.                              */
                /*------------------------------------------------------------*/
                if (value == NULL)
                {
                    NEUIK_RaiseError(funcName, errMsgs[10]);
                    continue;
                }
                if (value[0] == '\0')
                {
                    NEUIK_RaiseError(funcName, errMsgs[10]);
                    continue;
                }

                ns = sscanf(value, "%d,%d,%d,%d", 
                    &clr.r, &clr.g, &clr.b, &clr.a);
                /*------------------------------------------------------------*/
                /* Check for EOF, incorrect # of values, & out of range vals. */
                /*------------------------------------------------------------*/
            #ifndef WIN32
                if (ns == EOF || ns < 4)
            #else
                if (ns < 4)
            #endif /* WIN32 */
                {
                    NEUIK_RaiseError(funcName, errMsgs[10]);
                    continue;
                }

                if (clr.r < 0 || clr.r > 255 ||
                    clr.g < 0 || clr.g > 255 ||
                    clr.b < 0 || clr.b > 255 ||
                    clr.a < 0 || clr.a > 255)
                {
                    NEUIK_RaiseError(funcName, errMsgs[11]);
                    continue;
                }

                cfg->fgColorPressed = clr;
                doRedraw = 1;
            }
            else if (!strcmp("FontSize", name))
            {
                /* Set autoResize parameters for both width and height */

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
                    NEUIK_RaiseError(funcName, errMsgs[12]);
                    continue;
                }
                cfg->fontSize = fontSize;
                doRedraw = 1;
            }
            else if (!strcmp("BGColorGrad", name))
            {
                #pragma message("TODO: NEUIK_ToggleButton_Configure(\"BGColorGrad=...\")")
            }
            else if (!strcmp("BGColorGradPressed", name))
            {
                #pragma message("TODO: NEUIK_ ToggleButton_Configure(\"BGColorGradPressed=...\")")
            }
            else
            {
                typeMixup = 0;
                for (nCtr = 0;; nCtr++)
                {
                    if (boolNames[nCtr] == NULL) break;

                    if (!strcmp(boolNames[nCtr], name))
                    {
                        typeMixup = 1;
                        break;
                    }
                }

                if (typeMixup)
                {
                    /* A bool type was mistakenly used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[13]);
                }
                else
                {
                    /* An unsupported name was used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[14]);
                }
            }
        }

        /* before starting */
        set = va_arg(args, const char *);
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
        if (neuik_Element_GetSizeAndLocation(tbtn, &rSize, &rLoc))
        {
            eNum = 15;
            goto out;
        }
        neuik_Element_RequestRedraw(tbtn, rLoc, rSize);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__ToggleButton
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__ToggleButton(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                        eNum       = 0;    /* which error to report (if any) */
    int                        textW      = 0;
    int                        textH      = 0;
    SDL_Rect                   rect;
    SDL_Renderer             * rend       = NULL;
    SDL_Texture              * tTex       = NULL; /* text texture */
    TTF_Font                 * font       = NULL;
    const NEUIK_Color        * fgClr      = NULL;
    const NEUIK_Color        * bClr       = NULL; /* border color */
    NEUIK_ToggleButtonConfig * aCfg       = NULL; /* the active button config */
    NEUIK_ToggleButton       * btn        = NULL;
    NEUIK_ElementBase        * eBase      = NULL;
    neuik_MaskMap            * maskMap    = NULL;
    RenderLoc                  rl;
    static char                funcName[] = "neuik_Element_Render__ToggleButton";
    static char              * errMsgs[]  = {"", // [0] no error
        "Argument `elem` is not of ToggleButton class.",                 // [1]
        "Failure in `neuik_MakeMaskMap()`",                              // [2]
        "FontSet_GetFont returned NULL.",                                // [3]
        "", // [4]
        "RenderText returned NULL.",                                     // [5]
        "Invalid specified `rSize` (negative values).",                  // [6]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [7]
        "Failure in neuik_Element_RedrawBackground().",                  // [8]
    };

    if (!neuik_Object_IsClass(elem, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }
    btn = (NEUIK_ToggleButton*)elem;

    if (neuik_Object_GetClassObject(btn, neuik__Class_Element, (void**)&eBase))
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
    /* select the correct button config to use (pointer or internal)          */
    /*------------------------------------------------------------------------*/
    aCfg = btn->cfg;
    if (btn->cfgPtr != NULL)
    {
        aCfg = btn->cfgPtr;
    }

    /*------------------------------------------------------------------------*/
    /* Fill the background with it's color                                    */
    /*------------------------------------------------------------------------*/
    if ((btn->selected  && !btn->activated) || 
        (!btn->selected && btn->activated))
    {
        eBase->eSt.focusstate = NEUIK_FOCUSSTATE_SELECTED;
        fgClr = &(aCfg->fgColorPressed);
    }
    else
    {
        /* use the unselected colors */
        eBase->eSt.focusstate = NEUIK_FOCUSSTATE_NORMAL;
        fgClr = &(aCfg->fgColor);
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
    /* Draw the border around the button.                                     */
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
    /* Render the button text                                                 */
    /*------------------------------------------------------------------------*/
    if (btn->text != NULL)
    {
        font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
            aCfg->fontBold, aCfg->fontItalic);
        if (font == NULL) 
        {
            eNum = 3;
            goto out;

        }

        tTex = NEUIK_RenderText(btn->text, font, *fgClr, rend, &textW, &textH);
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
 *  Name:          neuik_Element_CaptureEvent__ToggleButton
 *
 *  Description:   Check to see if this event is captured by an object.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ToggleButton(
    NEUIK_Element   elem,
    SDL_Event     * ev)
{
    neuik_EventState       evCaptured  = NEUIK_EVENTSTATE_NOT_CAPTURED;
    RenderSize             rSize;
    RenderLoc              rLoc;
    SDL_Event            * e;
    SDL_MouseMotionEvent * mouseMotEv;
    SDL_MouseButtonEvent * mouseButEv;
    NEUIK_ElementBase    * eBase       = NULL;
    NEUIK_ToggleButton   * btn;

    if (!neuik_Object_IsClass(elem, neuik__Class_ToggleButton))
    {
        goto out;
    }
    btn = (NEUIK_ToggleButton*)elem;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        /* not the right type of object */
        goto out;
    }

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
                /* This mouse click originated within this button */
                btn->clickOrigin = 1;
                btn->selected    = 1;
                btn->wasSelected = 1;
                evCaptured = NEUIK_EVENTSTATE_CAPTURED;
                neuik_Window_TakeFocus(eBase->eSt.window, btn);

                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(btn, rLoc, rSize);
                neuik_Element_TriggerCallback(btn, NEUIK_CALLBACK_ON_CLICK);
                if (!neuik_Object_IsNEUIKObject_NoError(btn))
                {
                    /* The object was freed/corrupted by the callback */
                    evCaptured = NEUIK_EVENTSTATE_OBJECT_FREED;
                    goto out;
                }
                goto out;
            }
        }
        break;
    case SDL_MOUSEBUTTONUP:
        mouseButEv = (SDL_MouseButtonEvent*)(e);
        if (btn->clickOrigin)
        {
            if (mouseButEv->y >= eBase->eSt.rLoc.y && 
                mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseButEv->x >= eBase->eSt.rLoc.x && 
                    mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    /* cursor is still within the button, activate cbFunc */
                    neuik_Element_TriggerCallback(btn, 
                        NEUIK_CALLBACK_ON_CLICKED);
                    if (!neuik_Object_IsNEUIKObject_NoError(btn))
                    {
                        /* The object was freed/corrupted by the callback */
                        evCaptured = NEUIK_EVENTSTATE_OBJECT_FREED;
                        goto out;
                    }
                    if (!btn->activated)
                    {
                        btn->activated = 1;
                        neuik_Element_TriggerCallback(btn, 
                            NEUIK_CALLBACK_ON_ACTIVATED);
                        if (!neuik_Object_IsNEUIKObject_NoError(btn))
                        {
                            /* The object was freed/corrupted by the callback */
                            evCaptured = NEUIK_EVENTSTATE_OBJECT_FREED;
                            goto out;
                        }
                    }
                    else
                    {
                        btn->activated = 0;
                        neuik_Element_TriggerCallback(btn, 
                            NEUIK_CALLBACK_ON_DEACTIVATED);
                        if (!neuik_Object_IsNEUIKObject_NoError(btn))
                        {
                            /* The object was freed/corrupted by the callback */
                            evCaptured = NEUIK_EVENTSTATE_OBJECT_FREED;
                            goto out;
                        }
                    }
                }
            }
            btn->selected    = 0;
            btn->wasSelected = 0;
            btn->clickOrigin = 0;
            evCaptured = NEUIK_EVENTSTATE_CAPTURED;
            goto out;
        }
        break;

    case SDL_MOUSEMOTION:
        mouseMotEv = (SDL_MouseMotionEvent*)(e);

        if (btn->clickOrigin)
        {
            /*----------------------------------------------------------------*/
            /* The mouse was initially clicked within the button. If the user */
            /* moves the cursor out of the button area, deselect it.          */
            /*----------------------------------------------------------------*/
            btn->selected = 0;
            if (mouseMotEv->y >= eBase->eSt.rLoc.y && 
                mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
            {
                if (mouseMotEv->x >= eBase->eSt.rLoc.x && 
                    mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
                {
                    btn->selected = 1;
                }
            }

            if (btn->wasSelected != btn->selected)
            {
                rSize = eBase->eSt.rSize;
                rLoc  = eBase->eSt.rLoc;
                neuik_Element_RequestRedraw(btn, rLoc, rSize);
            }
            btn->wasSelected = btn->selected;
            evCaptured = NEUIK_EVENTSTATE_CAPTURED;
            goto out;
        }

        break;
    }
out:
    return evCaptured;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ToggleButton_Activate
 *
 *  Description:   Activate the NEUIK_ToggleButton if not already active.
 *
 *  Returns:       Non-zero if there was an error; 0 otherwise
 *
 ******************************************************************************/
int NEUIK_ToggleButton_Activate(
    NEUIK_ToggleButton * btn)
{
    RenderSize  rSize;
    RenderLoc   rLoc;
    int         eNum       = 0; /* which error to report (if any) */
    static char funcName[] = "NEUIK_ToggleButton_Activate";
    static char *errMsgs[] = {"", // [0] no error
        "Argument `btn` is not of ToggleButton class.",     // [1]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [2]
        "Failure in `neuik_Element_RequestRedraw()`.",      // [3]
    };


    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }

    /* Only activate this ToggleButton if it isn't already activated */
    if (!btn->activated)
    {
        btn->activated = 1;
        neuik_Element_TriggerCallback(btn, NEUIK_CALLBACK_ON_ACTIVATED);

        if (neuik_Element_GetSizeAndLocation(btn, &rSize, &rLoc))
        {
            eNum = 2;
            goto out;
        }
        if (neuik_Element_RequestRedraw(btn, rLoc, rSize))
        {
            eNum = 3;
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
 *  Name:          NEUIK_ToggleButton_Deactivate
 *
 *  Description:   Deactivate the NEUIK_ToggleButton if not already deactivated.
 *
 *  Returns:       Non-zero if there was an error; 0 otherwise
 *
 ******************************************************************************/
int NEUIK_ToggleButton_Deactivate(
    NEUIK_ToggleButton * btn)
{
    RenderSize  rSize;
    RenderLoc   rLoc;
    int         eNum       = 0; /* which error to report (if any) */
    static char funcName[] = "NEUIK_ToggleButton_Deactivate";
    static char *errMsgs[] = {"", // [0] no error
        "Argument `btn` is not of ToggleButton class.",     // [1]
        "Failure in `neuik_Element_GetSizeAndLocation()`.", // [2]
        "Failure in `neuik_Element_RequestRedraw()`.",      // [3]
    };

    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        eNum = 1;
        goto out;
    }

    /* Only deactivate this ToggleButton if it isn't already deactivated */
    if (btn->activated)
    {
        btn->activated = 0;
        neuik_Element_TriggerCallback(btn, NEUIK_CALLBACK_ON_DEACTIVATED);

        if (neuik_Element_GetSizeAndLocation(btn, &rSize, &rLoc))
        {
            eNum = 2;
            goto out;
        }
        if (neuik_Element_RequestRedraw(btn, rLoc, rSize))
        {
            eNum = 3;
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
 *  Name:          NEUIK_ToggleButton_Deactivate
 *
 *  Description:   Deactivate the NEUIK_ToggleButton if not already deactivated.
 *
 *  Returns:       Negative if there was an error; Otherwise 0 or 1.
 *
 ******************************************************************************/
int NEUIK_ToggleButton_IsActivated(
    NEUIK_ToggleButton * btn)
{
    int rv = -1;
    static char funcName[] = "NEUIK_ToggleButton_IsActivated";
    static char errMsg[]   = "Argument `btn` is not of ToggleButton class.";

    if (!neuik_Object_IsClass(btn, neuik__Class_ToggleButton))
    {
        /* This ToggleButton has failed the NEUIK_CheckType */
        NEUIK_RaiseError(funcName, errMsg);
    }
    else
    {
        /* This ToggleButton has passed the NEUIK_CheckType */
        rv = btn->activated;
    }

    return rv;
}

