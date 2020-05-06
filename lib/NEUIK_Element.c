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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "NEUIK_colors.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Event.h"
#include "NEUIK_Window.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;
extern int neuik__Report_Debug;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Element(void ** elemPtr);
int neuik_Object_Free__Element(void * elemPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
int neuik_Element_Free(NEUIK_Element ** elemPtr);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Element_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__Element,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__Element,
};

NEUIK_ElementBackground neuik_default_ElementBackground = {
    NEUIK_BGSTYLE_SOLID, /* style to use when element is unselected */
    NEUIK_BGSTYLE_SOLID, /* style to use when element is selected */
    NEUIK_BGSTYLE_SOLID, /* style to use when element is hovered */
    COLOR_LLGRAY,        /* solid color to use under normal condtions */
    COLOR_LLGRAY,        /* solid color to use when selected */
    COLOR_LLGRAY,        /* solid color to use being hovered over */
    'v',                 /* direction to use for the gradient (`v` or `h`) */
    NULL,                /* color gradient to use under normal condtions */
    NULL,                /* color gradient to use when selected */
    NULL,                /* color gradient to use being hovered over */
};

NEUIK_ElementConfig neuik_default_ElementConfig = {
     1.0,                   /* Scale Factor : 0 = Doesn't stretch; other value does */
     1.0,                   /* Scale Factor : 0 = Doesn't stretch; other value does */
     0,                     /* Element fills Vertically   : 1 = true; 0 = false */
     0,                     /* Element fills Horizontally : 1 = true; 0 = false */
    NEUIK_VJUSTIFY_DEFAULT, /* Vertical   justification */
    NEUIK_HJUSTIFY_DEFAULT, /* Horizontal justification */
     0,                     /* Pad the top of the element with transparent space */
     0,                     /* Pad the bottom of the element with transparent space */
     0,                     /* Pad the left of the element with transparent space */
     0,                     /* Pad the right of the element with transparent space */
    -1,                     /* Minimum Width */
    -1,                     /* Maximum Width */
    -1,                     /* Minimum Height */
    -1,                     /* Maximum Height */
     1,                     /* Element is being shown */
};

NEUIK_ElementState neuik_default_ElementState = {
    1,                                        /* Element needs to be redrawn */
    0,                                        /* Element does not have Window focus */
    0,                                        /* Element does not require an alpha blending */
    0,                                        /* Element is not active by default */
    NEUIK_FOCUSSTATE_NORMAL,                  /* Element is unselected */
    NULL,                                     /* (NEUIK_Window *) containing window */
    NULL,                                     /* (NEUIK_Element)  parent element */
    NULL,                                     /* (NEUIK_Element)  popup element */
    NULL,                                     /* (SDL_Texture *)  rendered texture */
    NULL,                                     /* (SDL_Surface *)  surface for this element */
    NULL,                                     /* (SDL_Renderer *) renderer for this surface */
    NULL,                                     /* (SDL_Renderer *) last used extRenderer */
    {0, 0},                                   /* render size */
    {NEUIK_INVALID_SIZE, NEUIK_INVALID_SIZE}, /* old render size */
    {0, 0},                                   /* render loc  */
    {0, 0},                                   /* render loc, relative to parent  */
    {NEUIK_INVALID_SIZE, NEUIK_INVALID_SIZE}, /* Minimum size of the element */
    {NEUIK_INVALID_SIZE, NEUIK_INVALID_SIZE}, /* Minimum size of the element (previous frame) */
    NEUIK_MINSIZE_NOCHANGE,                   /* How min elem width changed */
    NEUIK_MINSIZE_NOCHANGE,                   /* How min elem height changed */
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Element
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_Element()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_Element";
    static char  * errMsgs[]  = {"",                   // [0] no error
        "NEUIK library must be initialized first.",    // [1]
        "Failed to register `Element` object class .", // [2]
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
        "NEUIK_Element",                 // className
        "The basic NEUIK_Object Class.", // classDescription
        neuik__Set_NEUIK,                // classSet
        NULL,                            // superClass
        &neuik_Element_BaseFuncs,        // baseFuncs
        NULL,                            // classFuncs XXXXX
        &neuik__Class_Element))          // newClass
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
 *  Name:          neuik_NewElement
 *
 *  Description:   Allocate memory and set default values for Element.
 *
 *                 An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__Element(
    void ** elemPtr)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * elem       = NULL;
    static char         funcName[] = "neuik_NewElement";
    static char       * errMsgs[]  = {"",           // [0] no error
        "Output Argument `elemPtr` is NULL.",       // [1]
        "Failure to allocate memory.",              // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.", // [3]
    };

    if (elemPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*elemPtr) = (NEUIK_Element*) malloc(sizeof(NEUIK_ElementBase));
    elem = (NEUIK_ElementBase*)(*elemPtr);
    if (elem == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocation successful; set default values.                             */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK,
            neuik__Class_Element,
            NULL,
            &(elem->objBase)))
    {
        eNum = 3;
        goto out;
    }

    elem->eFT  = NULL;
    elem->eCfg = neuik_default_ElementConfig;
    elem->eSt  = neuik_default_ElementState;
    elem->eBg  = neuik_default_ElementBackground;
    elem->eCT  = NEUIK_NewCallbackTable();
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


int neuik_Element_SetFuncTable(
    NEUIK_Element             elem,
    NEUIK_Element_FuncTable * eFT)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_SetFuncTable";
    static char       * errMsgs[]  = {"",                                // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Argument `eFT` is NULL.",                                       // [2]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (eFT == NULL)
    {
        eNum = 2;
        goto out;
    }

    eBase->eFT = eFT;
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
 *  Name:          neuik_Element_Free
 *
 *  Description:   Free memory allocated for this object and NULL out pointer.
 *
 *                 An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__Element(
    void * elemPtr)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_Free";
    static char       * errMsgs[]  = {"",                                   // [0] no error
        "Argument `elemPtr` is NULL.",                                      // [1]
        "Argument `elemPtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
    };

    if (elemPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (neuik_Object_GetClassObject(elemPtr, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }

    free(eBase);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


NEUIK_ElementConfig * neuik_Element_GetConfig(NEUIK_Element elem)
{
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return NULL;
    }

    return &(eBase->eCfg);
}

NEUIK_ElementConfig neuik_GetDefaultElementConfig()
{
    return neuik_default_ElementConfig;
}

void neuik_Element_Configure_capture_segv(
    int sig_num)
{
    static char funcName[] = "NEUIK_Element_Configure";
    static char errMsg[] =
        "SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

    NEUIK_RaiseError(funcName, errMsg);
    NEUIK_BacktraceErrors();
    exit(1);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_Configure
 *
 *  Description:   Configure one or more settings for an element.
 *
 *                 NOTE: This list of settings must be terminated by a NULL
 *                 pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_Configure(
    NEUIK_Element   elem,
    const char    * set0,
    ...)
{
    int                   ctr;
    int                   nCtr;
    int                   isBool;
    int                   boolVal    = 0;
    int                   doRedraw   = 0;
    int                   fullRedraw = 0;
    int                   intVal     = 0;
    int                   typeMixup;
    RenderSize            rSize;
    RenderLoc             rLoc;
    va_list               args;
    char                * strPtr     = NULL;
    char                * name       = NULL;
    char                * value      = NULL;
    const char          * set        = NULL;
    char                  buf[4096];
    NEUIK_ElementBase   * eBase      = NULL;
    NEUIK_ElementConfig * eCfg       = NULL;
    /*------------------------------------------------------------------------*/
    /* If a `name=value` string with an unsupported name is found, check to   */
    /* see if a boolName was mistakenly used instead.                         */
    /*------------------------------------------------------------------------*/
    static char         * boolNames[] = {
        "FillAll",
        "HFill",
        "VFill",
        "Show",
        NULL,
    };
    /*------------------------------------------------------------------------*/
    /* If a boolName string with an unsupported name is found, check to see   */
    /* if a supported nameValue type was mistakenly used instead.             */
    /*------------------------------------------------------------------------*/
    static char         * valueNames[] = {
        "HScale",
        "VScale",
        "HJustify",
        "VJustify",
        "PadLeft",
        "PadRight",
        "PadTop",
        "PadBottom",
        "PadAll",
        NULL,
    };
    static char           funcName[] = "NEUIK_Element_Configure";
    static char         * errMsgs[] = {"", // [ 0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.",   // [ 1]
        "NamedSet.name is NULL, skipping.",                                // [ 2]
        "NamedSet.name is blank, skipping.",                               // [ 3]
        "NamedSet.name type unknown, skipping.",                           // [ 4]
        "`name=value` string is too long.",                                // [ 5]
        "Set string is empty.",                                            // [ 6]
        "HJustify value is invalid.",                                      // [ 7]
        "VJustify value is invalid.",                                      // [ 8]
        "BoolType name unknown, skipping.",                                // [ 9]
        "Invalid `name=value` string.",                                    // [10]
        "ValueType name used as BoolType, skipping.",                      // [11]
        "BoolType name used as ValueType, skipping.",                      // [12]
        "Failure in `neuik_Window_RequestFullRedraw()`.",                  // [13]
        "Failure in `neuik_Element_PropagateIndeterminateMinSizeDelta()`", // [14]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        NEUIK_RaiseError(funcName, errMsgs[1]);
        return 1;
    }

    set = set0;

    eCfg = neuik_Element_GetConfig(eBase);

    va_start(args, set0);

    for (ctr = 0;; ctr++)
    {
        isBool = 0;
        name   = NULL;
        value  = NULL;

        if (set == NULL) break;

        #ifndef NO_NEUIK_SIGNAL_TRAPPING
            signal(SIGSEGV, neuik_Element_Configure_capture_segv);
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
                    NEUIK_RaiseError(funcName, errMsgs[6]);
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
                    NEUIK_RaiseError(funcName, errMsgs[10]);
                    set = va_arg(args, const char *);
                    continue;
                }
                name  = buf;
                value = strPtr;
            }
        }

        if (isBool)
        {
            if (!strcmp("VFill", name))
            {
                if (eCfg->VFill != boolVal)
                {
                    eCfg->VFill = boolVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("HFill", name))
            {
                if (eCfg->HFill != boolVal)
                {
                    eCfg->HFill = boolVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("FillAll", name))
            {
                if (eCfg->HFill != boolVal || eCfg->VFill != boolVal)
                {
                    eCfg->HFill = boolVal;
                    eCfg->VFill = boolVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("Show", name))
            {
                if (eCfg->Show != boolVal)
                {
                    eCfg->Show = boolVal;
                    doRedraw   = 1;
                    /*--------------------------------------------------------*/
                    /* Showing/hiding elements can result in drastic changes  */
                    /* to locations and exactly how things must be redrawn.   */
                    /* For now the safest thing to do is a complete redraw.   */
                    /*--------------------------------------------------------*/
                    fullRedraw = 1;
                }
            }
            else
            {
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
                    NEUIK_RaiseError(funcName, errMsgs[11]);
                }
                else
                {
                    /* An unsupported name was used as a bool type */
                    NEUIK_RaiseError(funcName, errMsgs[9]);
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
            else if (!strcmp("VScale", name))
            {
                eCfg->VScale = (float)(atof(value));
                doRedraw = 1;
            }
            else if (!strcmp("HScale", name))
            {
                eCfg->HScale = (float)(atof(value));
                doRedraw = 1;
            }
            else if (!strcmp("HJustify", name))
            {
                if (!strcmp("left", value))
                {
                    if (eCfg->HJustify != NEUIK_HJUSTIFY_LEFT)
                    {
                        eCfg->HJustify = NEUIK_HJUSTIFY_LEFT;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("center", value))
                {
                    if (eCfg->HJustify != NEUIK_HJUSTIFY_CENTER)
                    {
                        eCfg->HJustify = NEUIK_HJUSTIFY_CENTER;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("right", value))
                {
                    if (eCfg->HJustify != NEUIK_HJUSTIFY_RIGHT)
                    {
                        eCfg->HJustify = NEUIK_HJUSTIFY_RIGHT;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("default", value))
                {
                    if (eCfg->HJustify != NEUIK_HJUSTIFY_DEFAULT)
                    {
                        eCfg->HJustify = NEUIK_HJUSTIFY_DEFAULT;
                        doRedraw = 1;
                    }
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
                    if (eCfg->VJustify != NEUIK_VJUSTIFY_TOP)
                    {
                        eCfg->VJustify = NEUIK_VJUSTIFY_TOP;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("center", value))
                {
                    if (eCfg->VJustify != NEUIK_VJUSTIFY_CENTER)
                    {
                        eCfg->VJustify = NEUIK_VJUSTIFY_CENTER;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("bottom", value))
                {
                    if (eCfg->VJustify != NEUIK_VJUSTIFY_BOTTOM)
                    {
                        eCfg->VJustify = NEUIK_VJUSTIFY_BOTTOM;
                        doRedraw = 1;
                    }
                }
                else if (!strcmp("default", value))
                {
                    if (eCfg->VJustify != NEUIK_VJUSTIFY_DEFAULT)
                    {
                        eCfg->VJustify = NEUIK_VJUSTIFY_DEFAULT;
                        doRedraw = 1;
                    }
                }
                else
                {
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                }
            }
            else if (!strcmp("PadLeft", name))
            {
                intVal = atoi(value);
                if (eCfg->PadLeft != intVal)
                {
                    eCfg->PadLeft = intVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("PadRight", name))
            {
                intVal = atoi(value);
                if (eCfg->PadRight != intVal)
                {
                    eCfg->PadRight = intVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("PadTop", name))
            {
                intVal = atoi(value);
                if (eCfg->PadTop != intVal)
                {
                    eCfg->PadTop = intVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("PadBottom", name))
            {
                intVal = atoi(value);
                if (eCfg->PadBottom != intVal)
                {
                    eCfg->PadBottom = intVal;
                    doRedraw = 1;
                }
            }
            else if (!strcmp("PadAll", name))
            {
                intVal = atoi(value);
                if (eCfg->PadLeft   != intVal ||
                    eCfg->PadRight  != intVal ||
                    eCfg->PadTop    != intVal ||
                    eCfg->PadBottom != intVal)
                {
                    eCfg->PadLeft   = intVal;
                    eCfg->PadRight  = intVal;
                    eCfg->PadTop    = intVal;
                    eCfg->PadBottom = intVal;
                    doRedraw = 1;
                }

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
                    NEUIK_RaiseError(funcName, errMsgs[12]);
                }
                else
                {
                    /* An unsupported name was used as a value type */
                    NEUIK_RaiseError(funcName, errMsgs[4]);
                }
            }
        }

        /* before starting */
        set = va_arg(args, const char *);
    }
    va_end(args);

    if (doRedraw)
    {
        if (fullRedraw)
        {
            if (eBase->eSt.parent != NULL)
            {
                if (neuik_Element_PropagateIndeterminateMinSizeDelta(
                    eBase->eSt.parent))
                {
                    NEUIK_RaiseError(funcName, errMsgs[14]);
                }
            }

            if (neuik_Window_RequestFullRedraw((NEUIK_Window*)eBase->eSt.window))
            {
                NEUIK_RaiseError(funcName, errMsgs[13]);
            }
        }
        rSize = eBase->eSt.rSize;
        rLoc  = eBase->eSt.rLoc;
        neuik_Element_RequestRedraw(elem, rLoc, rSize);
    }

    return 0;
}


NEUIK_ElementState neuik_GetDefaultElementState()
{
    return neuik_default_ElementState;
}


void neuik_SetDefaultElementConfig(NEUIK_ElementConfig eCfg)
{
    neuik_default_ElementConfig = eCfg;
}


int neuik_Element_GetMinSize(
    NEUIK_Element   elem,
    RenderSize    * rSize)
{
    int                  eNum       = 0;
    NEUIK_ElementBase  * eBase      = NULL;
    static int           nRecurse   = 0; /* number of times recursively called */
    static char          funcName[] = "neuik_Element_GetMinSize";
    static char        * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Element Function Table is NULL (missing or not set).",          // [2]
        "Failure in implementation of function `GetMinSize`.",           // [3]
        "Failure in `neuik_Element_StoreFrameMinSize()`",                // [4]
    };

    nRecurse++;
    if (nRecurse > NEUIK_MAX_RECURSION)
    {
        /*--------------------------------------------------------------------*/
        /* This is likely a case of appears to be runaway recursion; report   */
        /* an error to the user.                                              */
        /*--------------------------------------------------------------------*/
        neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
        goto out;
    }

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (eBase->eFT == NULL)
    {
        eNum = 2;
        goto out;
    }

    if ((eBase->eSt.minSize.w == NEUIK_INVALID_SIZE) && 
        (eBase->eSt.minSize.h == NEUIK_INVALID_SIZE))
    {
        /*--------------------------------------------------------------------*/
        /* These values should only be invalid if this is the first frame to  */
        /* be drawn. The minSize will need to be calculated.                  */
        /*--------------------------------------------------------------------*/
        if ((eBase->eFT->GetMinSize)(elem, rSize))
        {
            if (neuik_HasFatalError())
            {
                eNum = 1;
                goto out2;
            }
            eNum = 3;
            goto out;
        }

        if (neuik_Element_StoreFrameMinSize(elem, rSize))
        {
            eNum = 4;
            goto out;
        }
    }
    else if ((eBase->eSt.wDelta == NEUIK_MINSIZE_NOCHANGE) &&
             (eBase->eSt.hDelta == NEUIK_MINSIZE_NOCHANGE))
    {
        /*--------------------------------------------------------------------*/
        /* No change to the minimum size of this element. Use old value.      */
        /*--------------------------------------------------------------------*/
        *rSize = eBase->eSt.minSize;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* There is a change to the minimum size of this element. The minSize */
        /* will need to be recalculated.                                      */
        /*--------------------------------------------------------------------*/
        if ((eBase->eFT->GetMinSize)(elem, rSize))
        {
            if (neuik_HasFatalError())
            {
                eNum = 1;
                goto out2;
            }
            eNum = 3;
            goto out;
        }

        if (neuik_Element_StoreFrameMinSize(elem, rSize))
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
out2:
    nRecurse--;

    return eNum;
}


int neuik_Element_GetLocation(
    NEUIK_Element   elem,
    RenderLoc     * rLoc)
{
    int                  eNum      = 0;
    NEUIK_ElementBase  * eBase     = NULL;
    static char          funcName[] = "neuik_Element_GetLocation";
    static char        * errMsgs[] = {"",                                // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `rLoc` is NULL.",                               // [2]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (rLoc == NULL)
    {
        eNum = 2;
        goto out;
    }

    (*rLoc) = eBase->eSt.rLoc;
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
 *  Name:          neuik_Element_UpdateMinSizeDeltas
 *
 *  Description:   Compare the current frame minimum element size to the minimum
 *                 element size from the previous frame and set the [w/h]Delta
 *                 indicators.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_UpdateMinSizeDeltas(
    NEUIK_Element   elem)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_UpdateMinSizeDeltas";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    /*------------------------------------------------------------------------*/
    /* Update the Delta Indicator for the element Width.                      */
    /*------------------------------------------------------------------------*/
    eBase->eSt.wDelta = NEUIK_MINSIZE_NOCHANGE;
    if (eBase->eSt.minSize.w > eBase->eSt.minSizeOld.w)
    {
        eBase->eSt.wDelta = NEUIK_MINSIZE_INCREASE;
    }
    else if (eBase->eSt.minSize.w < eBase->eSt.minSizeOld.w)
    {
        eBase->eSt.wDelta = NEUIK_MINSIZE_DECREASE;
    }

    /*------------------------------------------------------------------------*/
    /* Update the Delta Indicator for the element Height.                     */
    /*------------------------------------------------------------------------*/
    eBase->eSt.hDelta = NEUIK_MINSIZE_NOCHANGE;
    if (eBase->eSt.minSize.h > eBase->eSt.minSizeOld.h)
    {
        eBase->eSt.hDelta = NEUIK_MINSIZE_INCREASE;
    }
    else if (eBase->eSt.minSize.h < eBase->eSt.minSizeOld.h)
    {
        eBase->eSt.hDelta = NEUIK_MINSIZE_DECREASE;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


int neuik_Element_GetSize(
    NEUIK_Element   elem,
    RenderSize    * rSize)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_GetSize";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `rSize` is NULL.",                              // [2]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (rSize == NULL)
    {
        eNum = 2;
        goto out;
    }

    (*rSize) = eBase->eSt.rSize;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


int neuik_Element_GetSizeAndLocation(
    NEUIK_Element   elem,
    RenderSize    * rSize,
    RenderLoc     * rLoc)
{
    int                  eNum      = 0;
    NEUIK_ElementBase  * eBase     = NULL;
    static char          funcName[] = "neuik_Element_GetSizeAndLocation";
    static char        * errMsgs[] = {"",                                // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `rSize` is NULL.",                              // [2]
        "Output argument `rLoc` is NULL.",                               // [3]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (rSize == NULL)
    {
        eNum = 2;
        goto out;
    }
    if (rLoc == NULL)
    {
        eNum = 3;
        goto out;
    }

    (*rSize) = eBase->eSt.rSize;
    (*rLoc)  = eBase->eSt.rLoc;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


int
    NEUIK_Element_SetSize(NEUIK_Element elem, RenderSize * rSize);


int neuik_Element_Render(
    NEUIK_Element   elem,
    RenderSize    * rSize, /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend, /* The external renderer to prepare the texture for */
    int             mock)  /* If true; calculate sizes/locations but don't draw */
{
    int                 result;
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return 1;
    }
    if (eBase->eFT == NULL) return 1;
    if (eBase->eFT->Render == NULL) return 1;

    result = (eBase->eFT->Render)(elem, rSize, rlMod, xRend, mock);
    if (result)
    {
        return result;
    }
    if (!mock)
    {
        eBase->eSt.hDelta = NEUIK_MINSIZE_NOCHANGE;
        eBase->eSt.wDelta = NEUIK_MINSIZE_NOCHANGE;
    }
    return result;
}


int neuik_Element_RenderRotate(
    NEUIK_Element   elem,
    RenderSize    * rSize,    /* in/out the size the tex occupies when complete */
    RenderLoc     * rlMod,    /* A relative location modifier (for rendering) */
    SDL_Renderer  * xRend,    /* The external renderer to prepare the texture for */
    int             mock,     /* If true; calculate sizes/locations but don't draw */
    double          rotation)
{
    NEUIK_ElementBase * eBase;
    SDL_Surface       * cpSurf = NULL;
    SDL_Renderer      * cpRend = NULL;
    SDL_Texture       * cpTex  = NULL;
    SDL_Surface       * imSurf = NULL;
    SDL_Texture       * imTex  = NULL;
    Uint32            * destPixels;
    Uint32            * srcPixels;
    Uint32              rmask;
    Uint32              gmask;
    Uint32              bmask;
    Uint32              amask;
    int                 destX      = 0;
    int                 destY      = 0;
    int                 destOffset = 0;
    int                 srcX       = 0;
    int                 srcY       = 0;
    int                 srcOffset  = 0;
    SDL_Rect            destRect   = {0, 0, 0, 0};  /* destination rectangle */
    RenderLoc           rl;
    RenderLoc           rlAdj;             /* loc. including adjustments */
    int                 eNum       = 0;
    static char         funcName[] = "neuik_Element_RenderRotate";
    static char       * errMsgs[]  = {"",                                // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Failed to create RGB surface.",                                 // [2]
        "Failed to create software renderer.",                           // [3]
        "Failure in `neuik_Element_Render()`",                           // [4]
        "SDL_CreateTextureFromSurface returned NULL.",                   // [5]
        "NEUIK_Element Function Table is NULL.",                         // [6]
        "`Render` unimplemented in NEUIK_Element Function Table.",       // [7]
    };

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xFF000000;
        gmask = 0x00FF0000;
        bmask = 0x0000FF00;
        amask = 0x000000FF;
    #else
        rmask = 0x000000FF;
        gmask = 0x0000FF00;
        bmask = 0x00FF0000;
        amask = 0xFF000000;
    #endif

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (eBase->eFT == NULL)
    {
        eNum = 6;
        goto out;
    }
    if (eBase->eFT->Render == NULL)
    {
        eNum = 7;
        goto out;
    }

    if (rotation == 0.0)
    {
        return neuik_Element_Render(elem, rSize, rlMod, xRend, mock);
    }

    rl = eBase->eSt.rLoc;
    rlAdj = rl;
    if (rlMod != NULL)
    {
        rlAdj.x += rlMod->x;
        rlAdj.y += rlMod->y;
    }
    rl = rlAdj;

    /*------------------------------------------------------------------------*/
    /* Create a new surface which is the size of the source texture.          */
    /*------------------------------------------------------------------------*/
    cpSurf = SDL_CreateRGBSurface(0,
        rSize->w, rSize->h, 32, rmask, gmask, bmask, amask);
    if (cpSurf == NULL)
    {
        eNum = 2;
        goto out;
    }

    cpRend = SDL_CreateSoftwareRenderer(cpSurf);
    if (cpRend == NULL)
    {
        eNum = 3;
        goto out;
    }
    srcPixels = cpSurf->pixels;

    /*------------------------------------------------------------------------*/
    /* Fill the entire surface background with a transparent color.           */
    /*------------------------------------------------------------------------*/
    SDL_SetRenderDrawColor(cpRend, 255, 255, 255, 0);
    SDL_RenderClear(cpRend);

    if ((eBase->eFT->Render)(elem, rSize, rlMod, cpRend, mock))
    {
        eNum = 4;
        goto out;
    }
    SDL_RenderPresent(cpRend);
    if (mock)
    {
        /*--------------------------------------------------------------------*/
        /* This is a mock render operation; don't draw anything...            */
        /*--------------------------------------------------------------------*/
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Attempt to directly copy the pixels from the source to destination     */
    /*------------------------------------------------------------------------*/
    if (rotation == 90.0)
    {
        /*--------------------------------------------------------------------*/
        /* Create a new surface which is the size of the rotated texture.     */
        /*--------------------------------------------------------------------*/
        imSurf = SDL_CreateRGBSurface(0,
            rSize->h, rSize->w, 32, rmask, gmask, bmask, amask);
        if (imSurf == NULL)
        {
            eNum = 2;
            goto out;
        }
        destPixels = imSurf->pixels;

        /*--------------------------------------------------------------------*/
        /* Copy pixel data...                                                 */
        /*                                                                    */
        /* - from : "top-to-bottom, left-to-right"                            */
        /* -   to : "right-to-left, top-to-bottom"                            */
        /*--------------------------------------------------------------------*/
        destX = rSize->h - 1;
        srcOffset  = 0;
        for (srcY = 0; srcY < rSize->h; srcY++)
        {
            destOffset = destX;
            for (srcX = 0; srcX < rSize->w; srcX++)
            {
                destPixels[destOffset] = srcPixels[srcOffset];

                /*------------------------------------------------------------*/
                /* increment/decrement the pixel offsets                      */
                /*------------------------------------------------------------*/
                srcOffset++;
                destOffset += rSize->h;
            }
            destX--;
        }
    }
    else if (rotation == 180.0)
    {
        /*--------------------------------------------------------------------*/
        /* Create a new surface which is the size of the rotated texture.     */
        /*--------------------------------------------------------------------*/
        imSurf = SDL_CreateRGBSurface(0,
            rSize->w, rSize->h, 32, rmask, gmask, bmask, amask);
        if (imSurf == NULL)
        {
            eNum = 2;
            goto out;
        }
        destPixels = imSurf->pixels;

        destY = rSize->h - 1;
        srcOffset  = 0;
        for (srcY = 0; srcY < rSize->h; srcY++)
        {
            destX = rSize->w - 1;
            destOffset = (1 + destY)*(rSize->w) - 1;
            for (srcX = 0; srcX < rSize->w; srcX++)
            {
                destPixels[destOffset] = srcPixels[srcOffset];

                /*------------------------------------------------------------*/
                /* increment/decrement the pixel offsets                      */
                /*------------------------------------------------------------*/
                srcOffset++;
                destOffset--;
                destX--;
            }
            destY--;
        }
    }
    else if (rotation == 270.0)
    {
        /*--------------------------------------------------------------------*/
        /* Create a new surface which is the size of the rotated texture.     */
        /*--------------------------------------------------------------------*/
        imSurf = SDL_CreateRGBSurface(0,
            rSize->h, rSize->w, 32, rmask, gmask, bmask, amask);
        if (imSurf == NULL)
        {
            eNum = 2;
            goto out;
        }
        destPixels = imSurf->pixels;

        /*--------------------------------------------------------------------*/
        /* Copy pixel data...                                                 */
        /*                                                                    */
        /* - from : "top-to-bottom, left-to-right"                            */
        /* -   to : "left-to-right, bottom-to-top"                            */
        /*--------------------------------------------------------------------*/
        destX = 0;
        destY = rSize->w - 1;
        srcOffset  = 0;
        for (srcY = 0; srcY < rSize->h; srcY++)
        {
            destOffset = (destY)*(rSize->h) + destX;
            for (srcX = 0; srcX < rSize->w; srcX++)
            {
                destPixels[destOffset] = srcPixels[srcOffset];

                /*------------------------------------------------------------*/
                /* increment/decrement the pixel offsets                      */
                /*------------------------------------------------------------*/
                srcOffset++;
                destOffset -= rSize->h;
            }
            destX++;
        }
    }

    imTex = SDL_CreateTextureFromSurface(xRend, imSurf);
    if (imTex == NULL)
    {
        eNum = 5;
        goto out;
    }

    destRect.x = rl.x;
    destRect.y = rl.y;
    destRect.w = imSurf->w;
    destRect.h = imSurf->h;
    SDL_RenderCopy(xRend, imTex, NULL, &destRect);

    eBase->eSt.hDelta = NEUIK_MINSIZE_NOCHANGE;
    eBase->eSt.wDelta = NEUIK_MINSIZE_NOCHANGE;
out:
    if (imTex  != NULL) SDL_DestroyTexture(imTex);
    if (cpTex  != NULL) SDL_DestroyTexture(cpTex);
    if (cpRend != NULL) SDL_DestroyRenderer(cpRend);
    if (imSurf != NULL) SDL_FreeSurface(imSurf);
    if (cpSurf != NULL) SDL_FreeSurface(cpSurf);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent    (virtual-function)
 *
 *  Description:   Pass an SDL_Event to an object and see if it was captured.
 *
 *                 This operation of this function may be redefined by a
 *                 Element subclass.
 *
 *  Returns:       1 if the event was captured, 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent(
    NEUIK_Element   elem,
    SDL_Event     * ev)
{
    neuik_EventState     captured = 0;
    neuik_EventState  (* funcImp) (NEUIK_Element, SDL_Event*);
    NEUIK_ElementBase  * eBase;

    /*------------------------------------------------------------------------*/
    /* Try the standard element implementation                                */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return 0;
    }

    if (eBase->eFT != NULL)
    {
        if (eBase->eFT->CaptureEvent != NULL)
        {
            captured = (eBase->eFT->CaptureEvent)(elem, ev);
            goto out;
        }
    }

    // if (eBase->eFT == NULL) return 0;
    // if (eBase->eFT->CaptureEvent == NULL) return 0;

    /*------------------------------------------------------------------------*/
    /* ELSE: Try and use an implemented virtual function (if implemented)     */
    /*------------------------------------------------------------------------*/
    funcImp = neuik_VirtualFunc_GetImplementation(
        neuik_Element_vfunc_CaptureEvent, elem);
    if (funcImp != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* A virtual reimplementation is availible for this function          */
        /*--------------------------------------------------------------------*/
        captured = (*funcImp)(elem, ev);
    }


    /*----------------------------------------------------------------*/
    /* If an element does not have a capture event function, it can   */
    /* not capture events.                                            */
    /*----------------------------------------------------------------*/

    // return (eBase->eFT->CaptureEvent)(elem, ev);
out:
    return captured;
}


void neuik_Element_StoreSizeAndLocation(
    NEUIK_Element elem,
    RenderSize    rSize,
    RenderLoc     rLoc,
    RenderLoc     rRelLoc)
{
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return;
    }

    eBase->eSt.rSize   = rSize;
    eBase->eSt.rLoc    = rLoc;
    eBase->eSt.rRelLoc = rRelLoc;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorGradient
 *
 *  Description:   Set the specified background style to a color gradient.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorGradient(
    NEUIK_Element   elem,
    const char    * styleName,
    char            direction,
    const char    * colorStop0,
    ...)
{
    int                   ns; /* number of items from sscanf */
    int                   ctr;
    int                   eNum       = 0;
    int                   nStops     = 1;
    NEUIK_Color           clr;
    float                 frac;
    char                  buf[4096];
    RenderSize            rSize;
    RenderLoc             rLoc;
    va_list               args;
    NEUIK_ElementBase   * eBase      = NULL;
    NEUIK_ColorStop   *** cstops     = NULL; /* pointer to the active colorstops */
    NEUIK_ColorStop     * cs;
    const char          * cs_str     = NULL;
    static char           funcName[] = "NEUIK_Element_SetBackgroundColorGradient";
    static char         * errMsgs[]  = {"",                               // [ 0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.",  // [ 1]
        "Argument `styleName` is NULL.",                                  // [ 2]
        "Argument `styleName` is blank.",                                 // [ 3]
        "Argument `styleName` has unexpected value.",                     // [ 4]
        "Failure in function `neuik_Element_RequestRedraw`.",             // [ 5]
        "Argument `direction` has unexpected value.",                     // [ 6]
        "`colorStop` string is too long.",                                // [ 7]
        "`colorStop` string invalid; should be comma separated RGBAF.",   // [ 8]
        "`colorStop` string invalid; RGBA value range is 0-255."          // [ 9]
        "`colorStop` string invalid; fraction value range is 0.0-1.0."    // [10]
        "Failed to allocate memory.",                                     // [11]
        "Failed to reallocate memory.",                                   // [12]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Read in the styleName and set the pointer to the appropriate style     */
    /*------------------------------------------------------------------------*/
    if (styleName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (styleName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("normal", styleName))
    {
        eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_GRADIENT;
        cstops = &(eBase->eBg.gradient_normal);
    }
    else if (!strcmp("selected", styleName))
    {
        eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_GRADIENT;
        cstops = &(eBase->eBg.gradient_selected);
    }
    else if (!strcmp("hovered", styleName))
    {
        eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_GRADIENT;
        cstops = &(eBase->eBg.gradient_hover);
    }
    else
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Free and NULL out the currently allocated ColorStops (if allocated).   */
    /*------------------------------------------------------------------------*/
    if (*cstops != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            if ((*cstops)[ctr] == NULL)
            {
                break;
            }
            free((*cstops)[ctr]);
        }
        free(*cstops);
        (*cstops) = NULL;
    }

    /*------------------------------------------------------------------------*/
    /* Set the gradient direction.                                            */
    /*------------------------------------------------------------------------*/
    switch (direction)
    {
        case 'h':
        case 'v':
            eBase->eBg.gradient_dirn = direction;
            break;
        default:
            /* Unsupported gradient direction */
            eNum = 6;
            goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Iterate over the list of colorstops.                                   */
    /*------------------------------------------------------------------------*/
    cs_str = colorStop0;

    va_start(args, colorStop0);

    for (ctr = 0;; ctr++)
    {
        if (cs_str == NULL)
        {
            if (*cstops != NULL)
            {
                (*cstops)[ctr] = NULL;
            }
            break;
        }

        // #ifndef NO_NEUIK_SIGNAL_TRAPPING
        //  signal(SIGSEGV, neuik_Element_Configure_capture_segv);
        // #endif

        if (strlen(cs_str) > 4095)
        {
            // #ifndef NO_NEUIK_SIGNAL_TRAPPING
            //  signal(SIGSEGV, NULL);
            // #endif
            eNum = 7;
            goto out;
        }
        // #ifndef NO_NEUIK_SIGNAL_TRAPPING
        //  signal(SIGSEGV, NULL);
        // #endif
        strcpy(buf, cs_str);
        /*--------------------------------------------------------------------*/
        /* Check for empty value errors.                                      */
        /*--------------------------------------------------------------------*/
        if (buf[0] == '\0')
        {
            NEUIK_RaiseError(funcName, errMsgs[8]);
            continue;
        }

        ns = sscanf(buf, "%d,%d,%d,%d,%f", &clr.r, &clr.g, &clr.b, &clr.a, &frac);
        /*--------------------------------------------------------------------*/
        /* Check for EOF, incorrect # of values, & out of range vals.         */
        /*--------------------------------------------------------------------*/
        if (ns == EOF || ns < 5)
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
        if (frac < 0.0 || frac > 1.0)
        {
            NEUIK_RaiseError(funcName, errMsgs[10]);
            continue;
        }

        /*--------------------------------------------------------------------*/
        /* Allocate/Reallocate memory for the colorStop array                 */
        /*--------------------------------------------------------------------*/
        if (*cstops != NULL)
        {
            /*----------------------------------------------------------------*/
            /* Reallocate memory for the colorStop array                      */
            /*----------------------------------------------------------------*/
            (*cstops) = (NEUIK_ColorStop **)realloc((*cstops),
                (nStops + 1)*sizeof(NEUIK_ColorStop*));
            if ((*cstops) == NULL)
            {
                eNum = 12;
                goto out;
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* Allocate memory for the colorStop array                        */
            /*----------------------------------------------------------------*/
            (*cstops) = (NEUIK_ColorStop **)malloc(2*sizeof(NEUIK_ColorStop*));
            if ((*cstops) == NULL)
            {
                eNum = 11;
                goto out;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Allocate memory for this new colorStop                             */
        /*--------------------------------------------------------------------*/
        (*cstops)[nStops - 1] = (NEUIK_ColorStop *)malloc(sizeof(NEUIK_ColorStop));
        cs = (*cstops)[nStops - 1];
        if (cs == NULL)
        {
            eNum = 11;
            goto out;
        }

        /*--------------------------------------------------------------------*/
        /* Set the value of the new ColorStop                                 */
        /*--------------------------------------------------------------------*/
        cs->color.r = clr.r;
        cs->color.g = clr.g;
        cs->color.b = clr.b;
        cs->color.a = clr.a;
        cs->frac    = frac;

        /* before starting */
        cs_str = va_arg(args, const char *);
        nStops++;
    }
    va_end(args);

    rSize = eBase->eSt.rSize;
    rLoc  = eBase->eSt.rLoc;
    if (neuik_Element_RequestRedraw(elem, rLoc, rSize))
    {
        eNum = 5;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_StoreFrameMinSize
 *
 *  Description:   Store a new frame minimum element size and preserve the 
 *                 previous minimum element size.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_StoreFrameMinSize(
    NEUIK_Element   elem,
    RenderSize    * size)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_StoreFrameMinSize";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Argument `size` is NULL.",                                      // [2]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (size == NULL)
    {
        eNum = 2;
        goto out;
    }

    eBase->eSt.minSizeOld = eBase->eSt.minSize;
    eBase->eSt.minSize    = *size;
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
 *  Name:          NEUIK_Element_SetBackgroundColorSolid
 *
 *  Description:   Set the specified background style to a solid color.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorSolid(
    NEUIK_Element   elem,
    const char    * styleName,
    unsigned char   r,
    unsigned char   g,
    unsigned char   b,
    unsigned char   a)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    NEUIK_Color       * aClr       = NULL;
    RenderSize          rSize;
    RenderLoc           rLoc;
    static char         funcName[] = "NEUIK_Element_SetBackgroundColorSolid";
    static char       * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Argument `styleName` is NULL.",                                 // [2]
        "Argument `styleName` is blank.",                                // [3]
        "Argument `styleName` has unexpected value.",                    // [4]
        "Failure in function `neuik_Element_RequestRedraw`.",            // [5]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (styleName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (styleName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("normal", styleName))
    {
        eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_normal);
    }
    else if (!strcmp("selected", styleName))
    {
        eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_selected);
    }
    else if (!strcmp("hovered", styleName))
    {
        eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_hover);
    }
    else
    {
        eNum = 4;
        goto out;
    }

    aClr->r = r;
    aClr->g = g;
    aClr->b = b;
    aClr->a = a;

    rSize = eBase->eSt.rSize;
    rLoc  = eBase->eSt.rLoc;
    if (neuik_Element_RequestRedraw(elem, rLoc, rSize))
    {
        eNum = 5;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorSolid_noRedraw
 *
 *  Description:   Set the specified background style to a solid color.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorSolid_noRedraw(
    NEUIK_Element   elem,
    const char    * styleName,
    unsigned char   r,
    unsigned char   g,
    unsigned char   b,
    unsigned char   a)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    NEUIK_Color       * aClr       = NULL;
    static char         funcName[] = "NEUIK_Element_SetBackgroundColorSolid_noRedraw";
    static char       * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Argument `styleName` is NULL.",                                 // [2]
        "Argument `styleName` is blank.",                                // [3]
        "Argument `styleName` has unexpected value.",                    // [4]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (styleName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (styleName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("normal", styleName))
    {
        eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_normal);
    }
    else if (!strcmp("selected", styleName))
    {
        eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_selected);
    }
    else if (!strcmp("hovered", styleName))
    {
        eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_SOLID;
        aClr = &(eBase->eBg.solid_hover);
    }
    else
    {
        eNum = 4;
        goto out;
    }

    aClr->r = r;
    aClr->g = g;
    aClr->b = b;
    aClr->a = a;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorTransparent
 *
 *  Description:   Set the specified background style to transparent.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorTransparent(
    NEUIK_Element   elem,
    const char    * styleName)
{
    int                 eNum       = 0;
    NEUIK_ElementBase * eBase      = NULL;
    RenderSize          rSize;
    RenderLoc           rLoc;
    static char         funcName[] = "NEUIK_Element_SetBackgroundColorTransparent";
    static char       * errMsgs[]  = {"",                                // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Argument `styleName` is NULL.",                                 // [2]
        "Argument `styleName` is blank.",                                // [3]
        "Argument `styleName` has unexpected value.",                    // [4]
        "Failure in function `neuik_Element_RequestRedraw`.",            // [5]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (styleName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (styleName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("normal", styleName))
    {
        if (eBase->eBg.bgstyle_normal != NEUIK_BGSTYLE_TRANSPARENT)
        {
            eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_TRANSPARENT;
            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            if (neuik_Element_RequestRedraw(elem, rLoc, rSize))
            {
                eNum = 5;
                goto out;
            }
        }
    }
    else if (!strcmp("selected", styleName))
    {
        if (eBase->eBg.bgstyle_selected != NEUIK_BGSTYLE_TRANSPARENT)
        {
            eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_TRANSPARENT;
            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            if (neuik_Element_RequestRedraw(elem, rLoc, rSize))
            {
                eNum = 5;
                goto out;
            }
        }
    }
    else if (!strcmp("hovered", styleName))
    {
        if (eBase->eBg.bgstyle_hover != NEUIK_BGSTYLE_TRANSPARENT)
        {
            eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_TRANSPARENT;
            rSize = eBase->eSt.rSize;
            rLoc  = eBase->eSt.rLoc;
            if (neuik_Element_RequestRedraw(elem, rLoc, rSize))
            {
                eNum = 5;
                goto out;
            }
        }
    }
    else
    {
        eNum = 4;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetCallback
 *
 *  Description:   Set the function and arguments for the named callback event.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetCallback(
    NEUIK_Element    elem,    /* the element to se the callback for */
    const char     * cbName,  /* the name of the callback to set */
    void           * cbFunc,  /* the function to use for the callback */
    void           * cbArg1,  /* the first argument to pass to the cbFunc */
    void           * cbArg2)  /* the second argument to pass to the cbFunc */
{
    int                  eNum       = 0;
    NEUIK_ElementBase  * eBase      = NULL;
    static char          funcName[] = "NEUIK_Element_SetCallback";
    static char        * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Callback Name `cbName` is NULL.",                               // [2]
        "Callback Name `cbName` is blank.",                              // [3]
        "Callback Name `cbName` unknown.",                               // [4]
    };


    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (cbName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (cbName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("OnClick", cbName))
    {
        if (eBase->eCT.OnClick) free(eBase->eCT.OnClick);
        eBase->eCT.OnClick = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnClicked", cbName))
    {
        if (eBase->eCT.OnClicked) free(eBase->eCT.OnClicked);
        eBase->eCT.OnClicked = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnHover", cbName))
    {
        if (eBase->eCT.OnHover) free(eBase->eCT.OnHover);
        eBase->eCT.OnHover = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnMouseEnter", cbName))
    {
        if (eBase->eCT.OnMouseEnter) free(eBase->eCT.OnMouseEnter);
        eBase->eCT.OnMouseEnter = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnMouseLeave", cbName))
    {
        if (eBase->eCT.OnMouseLeave) free(eBase->eCT.OnMouseLeave);
        eBase->eCT.OnMouseLeave = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnSelected", cbName))
    {
        if (eBase->eCT.OnSelected) free(eBase->eCT.OnSelected);
        eBase->eCT.OnSelected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnDeselected", cbName))
    {
        if (eBase->eCT.OnDeselected) free(eBase->eCT.OnDeselected);
        eBase->eCT.OnDeselected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnActivated", cbName))
    {
        if (eBase->eCT.OnActivated) free(eBase->eCT.OnActivated);
        eBase->eCT.OnActivated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnDeactivated", cbName))
    {
        if (eBase->eCT.OnDeactivated) free(eBase->eCT.OnDeactivated);
        eBase->eCT.OnDeactivated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnTextChanged", cbName))
    {
        if (eBase->eCT.OnTextChanged) free(eBase->eCT.OnTextChanged);
        eBase->eCT.OnTextChanged = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else if (!strcmp("OnCursorMoved", cbName))
    {
        if (eBase->eCT.OnCursorMoved) free(eBase->eCT.OnCursorMoved);
        eBase->eCT.OnCursorMoved = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
    }
    else
    {
        eNum = 4;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBindingCallback
 *
 *  Description:   Set the bindID to be sent when the specified callback is
 *                 triggered.
 *
 *                 This alternative callback procedure should only be used if
 *                 the standard `NEUIK_Element_SetCallback` function can not be
 *                 used, like for instance in a binding with another language.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBindingCallback(
    NEUIK_Element   elem,   /* the element to se the callback for */
    const char    * cbName, /* the name of the callback to set */
    unsigned int    bindID) /* A unique number to identify this callback instance */
{
    int                  eNum       = 0;
    NEUIK_ElementBase  * eBase      = NULL;
    static char          funcName[] = "NEUIK_Element_SetBindingCallback";
    static char        * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Callback Name `cbName` is NULL.",                               // [2]
        "Callback Name `cbName` is blank.",                              // [3]
        "Callback Name `cbName` unknown.",                               // [4]
    };


    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (cbName == NULL)
    {
        eNum = 2;
        goto out;
    }
    else if (cbName[0] == 0)
    {
        eNum = 3;
        goto out;
    }
    else if (!strcmp("OnClick", cbName))
    {
        if (eBase->eCT.OnClick) free(eBase->eCT.OnClick);
        eBase->eCT.OnClick = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnClicked", cbName))
    {
        if (eBase->eCT.OnClicked) free(eBase->eCT.OnClicked);
        eBase->eCT.OnClicked = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnHover", cbName))
    {
        if (eBase->eCT.OnHover) free(eBase->eCT.OnHover);
        eBase->eCT.OnHover = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnMouseEnter", cbName))
    {
        if (eBase->eCT.OnMouseEnter) free(eBase->eCT.OnMouseEnter);
        eBase->eCT.OnMouseEnter = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnMouseLeave", cbName))
    {
        if (eBase->eCT.OnMouseLeave) free(eBase->eCT.OnMouseLeave);
        eBase->eCT.OnMouseLeave = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnSelected", cbName))
    {
        if (eBase->eCT.OnSelected) free(eBase->eCT.OnSelected);
        eBase->eCT.OnSelected = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnDeselected", cbName))
    {
        if (eBase->eCT.OnDeselected) free(eBase->eCT.OnDeselected);
        eBase->eCT.OnDeselected = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnActivated", cbName))
    {
        if (eBase->eCT.OnActivated) free(eBase->eCT.OnActivated);
        eBase->eCT.OnActivated = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnDeactivated", cbName))
    {
        if (eBase->eCT.OnDeactivated) free(eBase->eCT.OnDeactivated);
        eBase->eCT.OnDeactivated = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnTextChanged", cbName))
    {
        if (eBase->eCT.OnTextChanged) free(eBase->eCT.OnTextChanged);
        eBase->eCT.OnTextChanged = NEUIK_NewBindingCallback(bindID);
    }
    else if (!strcmp("OnCursorMoved", cbName))
    {
        if (eBase->eCT.OnCursorMoved) free(eBase->eCT.OnCursorMoved);
        eBase->eCT.OnCursorMoved = NEUIK_NewBindingCallback(bindID);
    }
    else
    {
        eNum = 4;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_TriggerCallback
 *
 *  Description:   Trigger a callback of the specified type (if not NULL).
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int  neuik_Element_TriggerCallback(
    NEUIK_Element      elem,    /* The element whose callback should be triggered */
    neuik_CallbackEnum cbType)  /* Which callback to trigger */
{
    int                  eNum       = 0;
    NEUIK_ElementBase  * eBase      = NULL;
    static char          funcName[] = "neuik_Element_TriggerCallback";
    static char        * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Unknown Callback Type `cbType`.",                               // [2]
    };


    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    switch (cbType)
    {
        case NEUIK_CALLBACK_ON_CLICK:
            if (eBase->eCT.OnClick)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnClick, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_CLICKED:
            if (eBase->eCT.OnClicked)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnClicked, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_CREATED:
            if (eBase->eCT.OnCreated)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnCreated, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_HOVER:
            if (eBase->eCT.OnHover)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnHover, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_MOUSE_ENTER:
            if (eBase->eCT.OnMouseEnter)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnMouseEnter, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_MOUSE_LEAVE:
            if (eBase->eCT.OnMouseLeave)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnMouseLeave, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_MOUSE_OVER:
            if (eBase->eCT.OnMouseOver)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnMouseOver, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_SELECTED:
            if (eBase->eCT.OnSelected)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnSelected, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_DESELECTED:
            if (eBase->eCT.OnDeselected)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnDeselected, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_ACTIVATED:
            if (eBase->eCT.OnActivated)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnActivated, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_DEACTIVATED:
            if (eBase->eCT.OnDeactivated)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnDeactivated, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_TEXT_CHANGED:
            if (eBase->eCT.OnTextChanged)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnTextChanged, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_EXPANDED:
            if (eBase->eCT.OnExpanded)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnExpanded, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_COLLAPSED:
            if (eBase->eCT.OnCollapsed)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnCollapsed, eBase->eSt.window);
            }
            break;

        case NEUIK_CALLBACK_ON_CURSOR_MOVED:
            if (eBase->eCT.OnCursorMoved)
            {
                NEUIK_Callback_Trigger(eBase->eCT.OnCursorMoved, eBase->eSt.window);
            }
            break;

        default:
            eNum = 2;
            goto out;
            break;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer    (virtual-function)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation of this function may be redefined by a
 *                 Element subclass.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_SetWindowPointer(
    NEUIK_Element   elem,
    void          * win)
{
    int                  eNum       = 0;
    NEUIK_ElementBase  * eBase      = NULL;
    int               (* funcImp) (NEUIK_Element, void*);
    static char          funcName[] = "neuik_Element_SetWindowPointer";
    static char        * errMsgs[]  = {"",                               // [0] no error
        "Argument `elem` does not implement Element class.",             // [1]
        "Argument `win` does not implement Window class.",               // [2]
        "Failure in virtual-function implementation.",                   // [3]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [4]
    };

    funcImp = neuik_VirtualFunc_GetImplementation(
        neuik_Element_vfunc_SetWindowPointer, elem);
    if (funcImp != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* A virtual reimplementation is availible for this function          */
        /*--------------------------------------------------------------------*/
        // printf("vfunc is valid...\n");
        if ((*funcImp)(elem, win))
        {
            eNum = 3;
        }
        goto out;
    }
    /*------------------------------------------------------------------------*/
    /* ELSE: Fall back to standard element_SetWindowPointer operation         */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
    {
        eNum = 1;
        printf("elem failed check\n");
        goto out;
    }

    if (!neuik_Object_ImplementsClass(win, neuik__Class_Window))
    {
        eNum = 2;
        printf("win failed check\n");
        goto out;
    }

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 4;
        goto out;
    }

    eBase->eSt.window = win;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


void neuik_Element_SetParentPointer(
    NEUIK_Element    elem,
    void           * parent)
{
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return;
    }

    eBase->eSt.parent = parent;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_ForceRedraw
 *
 *  Description:   This function marks the element as one which needs to be
 *                 redrawn.  This message will propagate upwards through its
 *                 parent elements until it has reached the top. Finally, the
 *                 parent window is also marked as needing a redraw.
 *
 *                 This function unsets the internal old element size, causing
 *                 the element to resize and redraw itself. Only use this
 *                 function if neuik_Element_RequestRedraw is failing to cause
 *                 a redraw of the element.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_ForceRedraw(
    NEUIK_Element elem)
{
    NEUIK_ElementBase  * eBase;
    static RenderSize    redrawSz = {-1, -1};
    static char          funcName[] = "neuik_Element_ForceRedraw";
    static char          errMsg[] =
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.";

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        NEUIK_RaiseError(funcName, errMsg);
        return 1;
    }

    /*------------------------------------------------------------------------*/
    /* Setting the old size to (-1, -1), will always cause resize->redraw     */
    /* since the element will think it has changed size.                      */
    /*------------------------------------------------------------------------*/
    eBase->eSt.rSizeOld = redrawSz;

    eBase->eSt.doRedraw = 1;
    if (eBase->eSt.parent != NULL)
    {
        neuik_Element_ForceRedraw(eBase->eSt.parent);
    }
    else
    {
        /* notify the parent window that it will probably need to be redrawn */
        ((NEUIK_Window *)(eBase->eSt.window))->doRedraw = 1;
    }

    /* No errors*/
    return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_RequestRedraw
 *
 *  Description:   This function marks the element as one which needs to be
 *                 redrawn.  This message will propagate upwards through its
 *                 parent elements until it has reached the top. Finally, the
 *                 parent window is also marked as needing a redraw.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_RequestRedraw(
    NEUIK_Element elem,
    RenderLoc     rLoc,
    RenderSize    rSize)
{
    NEUIK_ElementBase  * eBase;
    NEUIK_Window       * win;
    int               (*funcImp) (NEUIK_Element, RenderLoc, RenderSize);
    int                  eNum       = 0; /* which error to report (if any) */
    static char          funcName[] = "neuik_Element_RequestRedraw";
    static char        * errMsgs[] = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Failure in `neuik_MaskMap_UnmaskUnboundedRect()`",              // [2]
    };


    funcImp = neuik_VirtualFunc_GetImplementation(
        neuik_Element_vfunc_RequestRedraw, elem);
    if (funcImp != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* A virtual reimplementation is availible for this function          */
        /*--------------------------------------------------------------------*/
        return (*funcImp)(elem, rLoc, rSize);
    }
    /*------------------------------------------------------------------------*/
    /* ELSE: Fall back to standard Element_IsShown operation                  */
    /*------------------------------------------------------------------------*/

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    eBase->eSt.doRedraw = 1;
    if (eBase->eSt.parent != NULL)
    {
        neuik_Element_RequestRedraw(eBase->eSt.parent, rLoc, rSize);
    }
    else
    {
        /* notify the parent window that it will probably need to be redrawn */
        win = (NEUIK_Window*)(eBase->eSt.window);
        if (win != NULL)
        {
            if (win->redrawMask != NULL)
            {
                if (neuik__Report_Debug)
                {
                    printf("RequestRedraw: umasking[x,y,w,h]: %d, %d, %d, %d\n",
                        rLoc.x, rLoc.y, rSize.w, rSize.h);
                }
                if (neuik_MaskMap_UnmaskUnboundedRect(win->redrawMask,
                    rLoc.x, rLoc.y, rSize.w, rSize.h))
                {
                    eNum = 2;
                    goto out;
                }
            }
            win->doRedraw = 1;
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
 *  Name:          neuik_Element_PropagateIndeterminateMinSizeDelta
 *
 *  Description:   This function marks the element as one which has an 
 *                 indeterminate minimum size delta.  This message will 
 *                 propagate upwards through its parent elements until it has 
 *                 reached the top.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_PropagateIndeterminateMinSizeDelta(
    NEUIK_Element elem)
{
    NEUIK_ElementBase  * eBase;
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = 
        "neuik_Element_PropagateIndeterminateMinSizeDelta";
    static char  * errMsgs[]  = {"" // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    eBase->eSt.hDelta = NEUIK_MINSIZE_INDETERMINATE;
    eBase->eSt.wDelta = NEUIK_MINSIZE_INDETERMINATE;
    if (eBase->eSt.parent != NULL)
    {
        neuik_Element_PropagateIndeterminateMinSizeDelta(eBase->eSt.parent);
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
 *  Name:          neuik_Element_ShouldRedrawAll
 *
 *  Description:   This function is used to indicate (to child elements) that
 *                 a parent element requires a full redraw.
 *
 *  Returns:       TRUE (1) if a redraw is needed, FALSE (0) otherwise.
 *
 ******************************************************************************/
int neuik_Element_ShouldRedrawAll(
    NEUIK_Element elem)
{
    int (*funcImp) (NEUIK_Element);
    NEUIK_Element     * parent = NULL;
    NEUIK_ElementBase * eBase  = NULL;

    funcImp = neuik_VirtualFunc_GetImplementation(
        neuik_Element_vfunc_ShouldRedrawAll, elem);
    if (funcImp != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* A virtual reimplementation is availible for this function          */
        /*--------------------------------------------------------------------*/
        if ((*funcImp)(elem))
        {
            return TRUE;
        }
    }

    if (neuik_Object_GetClassObject_NoError(elem,
            neuik__Class_Element, (void**)&eBase))
    {
        return FALSE;
    }
    parent = eBase->eSt.parent;
    if (parent == NULL)
    {
        return FALSE;
    }
    return neuik_Element_ShouldRedrawAll(parent);
}


int neuik_Element_NeedsRedraw(
    NEUIK_Element elem)
{
    NEUIK_Window      * win;
    NEUIK_ElementBase * nextEBase;
    NEUIK_ElementBase * eBase;
    NEUIK_Element     * parent;

    if (neuik_Object_GetClassObject_NoError(elem,
            neuik__Class_Element, (void**)&eBase))
    {
        return FALSE;
    }

    if (eBase->eSt.window != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Check of a full-window redraw was requested...                     */
        /*--------------------------------------------------------------------*/
        win = (NEUIK_Window*)(eBase->eSt.window);
        if (win->redrawAll)
        {
            return TRUE;
        }

        /*--------------------------------------------------------------------*/
        /* Check if a parent container has requested a full redraw.           */
        /*--------------------------------------------------------------------*/
        nextEBase = eBase;
        parent = nextEBase->eSt.parent;
        /*----------------------------------------------------------------*/
        /* The toplevel element within a window has no parent elem; break */
        /*----------------------------------------------------------------*/
        if (parent == NULL)
        {
            return FALSE;
        }

        if (neuik_Element_ShouldRedrawAll(parent))
        {
            return TRUE;
        }
    }

    return  eBase->eSt.doRedraw;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_RedrawBackgroundGradient
 *
 *  Description:   Renders a color gradient using the specified ColorStops.
 *
 *                 Vertical gradients start at the top and go down from there.
 *                 Horizontal gradients start at the left and go right from there.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_RedrawBackgroundGradient(
    NEUIK_Element      elem,
    NEUIK_ColorStop ** cs,
    RenderLoc        * rlMod,   /* A relative location modifier (for rendering) */
    neuik_MaskMap    * maskMap) /* Identifies regions of background to not draw */
{
    int                 ctr;
    int                 gCtr;             /* gradient counter */
    int                 nClrs;
    int                 eNum      = 0;    /* which error to report (if any) */
    int                 clrR;
    int                 clrG;
    int                 clrB;
    int                 clrA;
    int                 clrFound;
    int                 maskCtr;          /* maskMap counter */
    int                 maskRegions;      /* number of regions in maskMap */
    const int         * regionX0;         /* Array of region X0 values */
    const int         * regionXf;         /* Array of region Xf values */
    char                dirn;             /* Direction of the gradient 'v' or 'h' */
    float               lastFrac  = -1.0;
    float               frac;
    float               fracDelta;        /* fraction between ColorStop 1 & 2 */
    float               fracStart = 0.0;  /* fraction at ColorStop 1 */
    float               fracEnd   = 1.0;  /* fraction at ColorStop 2 */
    RenderSize          rSize;            /* Size of the element background to fill */
    RenderLoc           rl;               /* Location of element background */
    SDL_Rect            srcRect;
    NEUIK_ElementBase * eBase     = NULL;
    SDL_Renderer      * rend      = NULL;
    colorDeltas       * deltaPP   = NULL;
    colorDeltas       * clrDelta;
    NEUIK_Color       * clr;
    static char         funcName[] = "neuik_Element_RedawBackgroundGradient";
    static char       * errMsgs[] = {"", // [0] no error
        "Pointer to ColorStops is NULL.",                                // [1]
        "Unsupported gradient direction.",                               // [2]
        "Invalid RenderSize supplied.",                                  // [3]
        "Unable to create RGB surface.",                                 // [4]
        "SDL_CreateTextureFromSurface failed.",                          // [5]
        "ColorStops array is empty.",                                    // [6]
        "Invalid ColorStop fraction (<0 or >1).",                        // [7]
        "ColorStops array fractions not in ascending order.",            // [8]
        "Failure to allocate memory.",                                   // [9]
        "Failed to create software renderer.",                           // [10]
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [11]
        "Failure in `neuik_MaskMap_GetUnmaskedRegionsOnHLine`.",         // [12]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 11;
        goto out;
    }

    rl    = eBase->eSt.rLoc;
    rSize = eBase->eSt.rSize;
    rend  = eBase->eSt.rend;
    dirn  = eBase->eBg.gradient_dirn;

    /*------------------------------------------------------------------------*/
    /* Check for easily issues before attempting to render the gradient       */
    /*------------------------------------------------------------------------*/
    if (cs == NULL)
    {
        eNum = 1;
        goto out;
    }
    else if (*cs == NULL)
    {
        eNum = 6;
        goto out;
    }
    if (dirn != 'v' && dirn != 'h')
    {
        eNum = 2;
        goto out;
    }
    if (rSize.w <= 0 || rSize.h <= 0)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Count the number of color stops and check that the color stop          */
    /* fractions are in increasing order                                      */
    /*------------------------------------------------------------------------*/
    for (nClrs = 0;; nClrs++)
    {
        if (cs[nClrs] == NULL) break; /* this is the number of ColorStops */
        if (cs[nClrs]->frac < 0.0 || cs[nClrs]->frac > 1.0)
        {
            eNum = 7;
            goto out;
        }
        else if (cs[nClrs]->frac < lastFrac)
        {
            eNum = 8;
            goto out;
        }
        else
        {
            lastFrac = cs[nClrs]->frac;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Allocate memory for delta-per-px array and calculate the ColorStop     */
    /* delta-per-px values.                                                   */
    /*------------------------------------------------------------------------*/
    if (nClrs > 1)
    {
        deltaPP = (colorDeltas *)malloc((nClrs - 1)*sizeof(colorDeltas));
        if (deltaPP == NULL)
        {
            eNum = 9;
            goto out;
        }
        for (ctr = 0; ctr < nClrs-1; ctr++)
        {
            deltaPP[ctr].r = (float)((cs[ctr+1]->color).r - (cs[ctr]->color).r);
            deltaPP[ctr].g = (float)((cs[ctr+1]->color).g - (cs[ctr]->color).g);
            deltaPP[ctr].b = (float)((cs[ctr+1]->color).b - (cs[ctr]->color).b);
            deltaPP[ctr].a = (float)((cs[ctr+1]->color).a - (cs[ctr]->color).a);
        }
    }

    /*------------------------------------------------------------------------*/
    /* Fill in the colors of the gradient                                     */
    /*------------------------------------------------------------------------*/
    if (nClrs == 1)
    {
        /*--------------------------------------------------------------------*/
        /* A single color; this will just be a filled rectangle               */
        /*--------------------------------------------------------------------*/
        srcRect.x = rl.x;
        srcRect.y = rl.y;
        srcRect.w = rSize.w;
        srcRect.h = rSize.h;

        clr = &(cs[0]->color);
        SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
        SDL_RenderFillRect(rend, &srcRect);
    }
    else if (dirn == 'v')
    {
        /*--------------------------------------------------------------------*/
        /* Draw a vertical gradient                                           */
        /*--------------------------------------------------------------------*/
        for (gCtr = 0; gCtr < rSize.h; gCtr++)
        {
            /* calculate the fractional position within the gradient */
            frac = (float)(gCtr+1)/(float)(rSize.h);


            /* determine which ColorStops/colorDeltas should be used */
            fracStart = cs[0]->frac;
            clr       = &(cs[0]->color);
            clrDelta  = NULL;
            clrFound  = 0;
            for (ctr = 0;;ctr++)
            {
                if (cs[ctr] == NULL) break;

                if (frac < cs[ctr]->frac)
                {
                    /* apply delta from this clr */
                    fracEnd  = cs[ctr]->frac;
                    clrFound = 1;
                    break;
                }

                clr      = &(cs[ctr]->color);
                clrDelta = &(deltaPP[ctr]);
            }

            if (!clrFound)
            {
                /* line is beyond the final ColorStop; use that color */
                clrDelta = NULL;
            }

            /* calculate and set the color for this gradient line */
            if (clrDelta != NULL)
            {
                /* between two ColorStops, blend the color */
                fracDelta = (frac - fracStart)/(fracEnd - fracStart);
                clrR = clr->r + (int)((clrDelta->r)*fracDelta);
                clrG = clr->g + (int)((clrDelta->g)*fracDelta);
                clrB = clr->b + (int)((clrDelta->b)*fracDelta);
                clrA = clr->a + (int)((clrDelta->a)*fracDelta);
                SDL_SetRenderDrawColor(rend, clrR, clrG, clrB, clrA);
            }
            else
            {
                /* not between two ColorStops, use a single color */
                SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
            }

            if (maskMap != NULL)
            {
                /*------------------------------------------------------------*/
                /* A transparency mask is included, draw unmasked regions.    */
                /*------------------------------------------------------------*/
                if (neuik_MaskMap_GetUnmaskedRegionsOnHLine(
                        maskMap, gCtr, &maskRegions, &regionX0, &regionXf))
                {
                    eNum = 12;
                    goto out;
                }

                for (maskCtr = 0; maskCtr < maskRegions; maskCtr++)
                {
                    SDL_RenderDrawLine(rend,
                        rl.x + regionX0[maskCtr], rl.y + gCtr,
                        rl.x + regionXf[maskCtr], rl.y + gCtr);
                }
            }
            else
            {
                /*------------------------------------------------------------*/
                /* There are no masked off (transparent areas) draw full line */
                /*------------------------------------------------------------*/
                SDL_RenderDrawLine(rend,
                    rl.x,                 rl.y + gCtr,
                    rl.x + (rSize.w - 1), rl.y + gCtr);
            }
        }
    }
    else if (dirn == 'h')
    {
        /*--------------------------------------------------------------------*/
        /* Draw a horizontal gradient                                         */
        /*--------------------------------------------------------------------*/
        #pragma message("TODO/FIXME: Implement horizontal gradient")
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    if (deltaPP != NULL) free(deltaPP);

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_GetCurrentBGStyle
 *
 *  Description:   Returns the current active background style for the element.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetCurrentBGStyle(
    NEUIK_Element        elem,
    enum neuik_bgstyle * bgStyle) /* The current BGStyle is stored here */
{
    int                 eNum       = 0;    /* which error to report (if any) */
    NEUIK_ElementBase * eBase      = NULL;
    static char         funcName[] = "neuik_Element_GetCurrentBGStyle";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Ouput Argument `bgStyle` is NULL.",                             // [2]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }
    if (bgStyle == NULL)
    {
        eNum = 2;
        goto out;
    }

    switch (eBase->eSt.focusstate)
    {
        case NEUIK_FOCUSSTATE_NORMAL:
            *bgStyle = eBase->eBg.bgstyle_normal;
            break;
        case NEUIK_FOCUSSTATE_SELECTED:
            *bgStyle = eBase->eBg.bgstyle_selected;
            break;
        case NEUIK_FOCUSSTATE_HOVERED:
            *bgStyle = eBase->eBg.bgstyle_hover;
            break;
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
 *  Name:          neuik_Element_RedrawBackground
 *
 *  Description:   Redraw the element background to the stored renderer.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_RedrawBackground(
    NEUIK_Element   elem,
    RenderLoc     * rlMod,   /* A relative location modifier (for rendering) */
    neuik_MaskMap * maskMap) /* Identifies regions of background to not draw */
{
    int                    y              = 0;    /* current y-position */
    int                    y0             = 0;    /* first y-position to draw */
    int                    yf             = 0;    /* final y-position to draw */
    int                    eNum           = 0;    /* which error to report (if any) */
    int                    maskCtr;               /* maskMap counter */
    int                    maskRegions;           /* number of regions in maskMap */
    const int            * regionX0;              /* Array of region X0 values */
    const int            * regionXf;              /* Array of region Xf values */
    NEUIK_ElementBase    * eBase          = NULL;
    SDL_Renderer         * rend           = NULL;
    SDL_Texture          * tex            = NULL;
    SDL_Rect               srcRect;
    enum neuik_bgstyle     bgstyle;               /* active background style */
    NEUIK_Color          * color_solid    = NULL; /* pointer to active solid color */
    NEUIK_ColorStop    *** color_gradient = NULL; /* color gradient to use under normal condtions */
    RenderLoc              rl;                    /* Location of element background */
    RenderSize             rSize;                 /* Size of the element background to fill */
    static char            funcName[] = "neuik_Element_RedrawBackground";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Failure in `neuik_MaskMap_GetUnmaskedRegionsOnHLine`.",         // [2]
        "Unhandled Element FOCUSSTATE.",                                 // [3]
    };

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    rl    = eBase->eSt.rLoc;
    rSize = eBase->eSt.rSize;
    rend = eBase->eSt.rend;

    /*------------------------------------------------------------------------*/
    /* Identify both the background style to use as well as the color(s) to   */
    /* use to render the background.                                          */
    /*------------------------------------------------------------------------*/
    switch (eBase->eSt.focusstate)
    {
        case NEUIK_FOCUSSTATE_NORMAL:
            bgstyle = eBase->eBg.bgstyle_normal;
            switch (bgstyle)
            {
                case NEUIK_BGSTYLE_SOLID:
                    color_solid = &(eBase->eBg.solid_normal);
                    break;
                case NEUIK_BGSTYLE_GRADIENT:
                    color_gradient = &(eBase->eBg.gradient_normal);
                    break;
                case NEUIK_BGSTYLE_TRANSPARENT:
                    break;
            }
            break;
        case NEUIK_FOCUSSTATE_SELECTED:
            bgstyle = eBase->eBg.bgstyle_selected;
            switch (bgstyle)
            {
                case NEUIK_BGSTYLE_SOLID:
                    color_solid = &(eBase->eBg.solid_selected);
                    break;
                case NEUIK_BGSTYLE_GRADIENT:
                    color_gradient = &(eBase->eBg.gradient_selected);
                    break;
                case NEUIK_BGSTYLE_TRANSPARENT:
                    break;
            }
            break;
        case NEUIK_FOCUSSTATE_HOVERED:
            bgstyle = eBase->eBg.bgstyle_hover;
            switch (bgstyle)
            {
                case NEUIK_BGSTYLE_SOLID:
                    color_solid = &(eBase->eBg.solid_hover);
                    break;
                case NEUIK_BGSTYLE_GRADIENT:
                    color_gradient = &(eBase->eBg.gradient_hover);
                    break;
                case NEUIK_BGSTYLE_TRANSPARENT:
                    break;
            }
            break;
        default:
            /*----------------------------------------------------------------*/
            /* Unhandled Element FOCUSSTATE.                                  */
            /*----------------------------------------------------------------*/
            eNum = 3;
            goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Render the background.                                                 */
    /*------------------------------------------------------------------------*/
    switch (bgstyle)
    {
        case NEUIK_BGSTYLE_SOLID:
            /*----------------------------------------------------------------*/
            /* Fill the entire surface background with a solid color.         */
            /*----------------------------------------------------------------*/
            SDL_SetRenderDrawColor(rend,
                color_solid->r, color_solid->g, color_solid->b, color_solid->a);

            if (maskMap != NULL)
            {
                /*------------------------------------------------------------*/
                /* A transparency mask is included, draw unmasked regions.    */
                /*------------------------------------------------------------*/
                y0 = 0;
                yf = rSize.h;
                for (y = y0; y < yf; y++)
                {
                    if (neuik_MaskMap_GetUnmaskedRegionsOnHLine(
                            maskMap, y, &maskRegions, &regionX0, &regionXf))
                    {
                        eNum = 2;
                        goto out;
                    }

                    for (maskCtr = 0; maskCtr < maskRegions; maskCtr++)
                    {
                        SDL_RenderDrawLine(rend,
                            rl.x + regionX0[maskCtr], rl.y + y,
                            rl.x + regionXf[maskCtr], rl.y + y);
                    }
                }
            }
            else
            {
                /*------------------------------------------------------------*/
                /* There are no masked off (transparent areas) fill in rect.  */
                /*------------------------------------------------------------*/
                srcRect.x = rl.x;
                srcRect.y = rl.y;
                srcRect.w = rSize.w;
                srcRect.h = rSize.h;

                SDL_RenderFillRect(rend, &srcRect);
            }

            break;
        case NEUIK_BGSTYLE_GRADIENT:
            neuik_Element_RedrawBackgroundGradient(
                elem, *color_gradient, rlMod, maskMap);
            break;
        case NEUIK_BGSTYLE_TRANSPARENT:
            /*----------------------------------------------------------------*/
            /* The entire surface background is transparent; do nothing.      */
            /*----------------------------------------------------------------*/
            break;
    }

    /* No errors*/
    ConditionallyDestroyTexture((SDL_Texture **)&(tex));
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


int neuik_Element_Resize(
    NEUIK_Element elem,
    RenderSize    rSize)
{
    int                 eNum = 0; /* which error to report (if any) */
    NEUIK_ElementBase * eBase;
    Uint32              rmask;
    Uint32              gmask;
    Uint32              bmask;
    Uint32              amask;
    static char         funcName[] = "neuik_Element_Resize";
    static char       * errMsgs[] = {"",                                 // [0] no error
        "Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Failed to create RGB surface.",                                 // [2]
        "Failed to create software renderer.",                           // [3]
    };

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xFF000000;
        gmask = 0x00FF0000;
        bmask = 0x0000FF00;
        amask = 0x000000FF;
    #else
        rmask = 0x000000FF;
        gmask = 0x0000FF00;
        bmask = 0x00FF0000;
        amask = 0xFF000000;
    #endif

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 1;
        goto out;
    }

    if (eBase->eSt.surf != NULL) SDL_FreeSurface(eBase->eSt.surf);
    if (eBase->eSt.rend != NULL) SDL_DestroyRenderer(eBase->eSt.rend);

    eBase->eSt.surf = SDL_CreateRGBSurface(0,
        rSize.w, rSize.h, 32, rmask, gmask, bmask, amask);
    if (eBase->eSt.surf == NULL)
    {
        eNum = 2;
        goto out;
    }

    eBase->eSt.rend = SDL_CreateSoftwareRenderer(eBase->eSt.surf);
    if (eBase->eSt.rend == NULL)
    {
        eNum = 3;
        goto out;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


int neuik_Element_SetChildPopup(
    NEUIK_Element  parent,
    NEUIK_Element  pu)
{
    int                  eNum       = 0; /* which error to report (if any) */
    NEUIK_ElementBase  * eBase      = NULL;
    static char          funcName[] = "neuik_Element_SetChildPopup";
    static char        * errMsgs[]  = {"",                                 // [0] no error
        "Argument `pu` does not implement Element class.",                 // [1]
        "Argument `parent` caused `neuik_Object_GetClassObject` to fail.", // [2]
    };

    if (!neuik_Object_ImplementsClass(pu, neuik__Class_Element))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(parent, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }

    eBase->eSt.popup = pu;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Defocus__Container (virtual-function)
 *
 *  Description:   Call Element defocus function.
 *
 ******************************************************************************/
void neuik_Element_Defocus(
    NEUIK_Element  elem)
{
    NEUIK_ElementBase * eBase;

    if (!neuik_Object_IsNEUIKObject_NoError(elem))
    {
        /* The object may have been freed/corrupted; ignore defocus call */
        return;
    }

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return;
    }

    eBase->eSt.hasFocus = 0;

    /*------------------------------------------------------------------------*/
    /* Check to see if this element may contain other elements. If so,        */
    /* recursively defocus these items.                                       */
    /*------------------------------------------------------------------------*/
    if (eBase->eFT->Defocus != NULL)
    {
        (eBase->eFT->Defocus)(elem);
    }
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_IsShown    (virtual-function)
 *
 *  Description:   This function reports whether or not an element is currently
 *                 being shown.
 *
 *                 This operation of this function may be redefined by a
 *                 Element subclass.
 *
 *  Returns:       1 if element is shown, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_IsShown(
    NEUIK_Element  elem)
{
    NEUIK_ElementBase  * eBase;
    int               (*funcImp) (NEUIK_Element);

    funcImp = neuik_VirtualFunc_GetImplementation(
        neuik_Element_vfunc_IsShown, elem);
    if (funcImp != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* A virtual reimplementation is available for this function          */
        /*--------------------------------------------------------------------*/
        return (*funcImp)(elem);
    }
    /*------------------------------------------------------------------------*/
    /* ELSE: Fall back to standard Element_IsShown operation                  */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
    {
        return 0;
    }

    return eBase->eCfg.Show;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetActive
 *
 *  Description:   Set the `isActive` parameter of an element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_SetActive(
    NEUIK_Element elem,
    int           isActive)
{
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase)) return;

    eBase->eSt.isActive = isActive;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_IsActive
 *
 *  Description:   Return the `isActive` parameter of an element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
int neuik_Element_IsActive(
    NEUIK_Element elem)
{
    NEUIK_ElementBase * eBase;

    if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase)) return 0;

    return eBase->eSt.isActive;
}

