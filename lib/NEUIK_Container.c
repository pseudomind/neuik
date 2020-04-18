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

#include "NEUIK_Container.h"
#include "NEUIK_error.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Event.h"
#include "NEUIK_Window.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Container(void ** contPtr);
int neuik_Object_Free__Container(void * contPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
neuik_EventState neuik_Element_CaptureEvent__Container(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_IsShown__Container(NEUIK_Element);
int neuik_Element_SetWindowPointer__Container(NEUIK_Element, void*);
int neuik_Element_ShouldRedrawAll__Container(NEUIK_Element);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Container_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__Container,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__Container,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Container
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Container()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_Container";
    static char * errMsgs[]  = {"",                                        // [0] no error
        "NEUIK library must be initialized first.",                        // [1]
        "Failed to register `Container` object class.",                    // [2]
        "Failed to register `Element_IsShown` virtual function.",          // [3]
        "Failed to register `Element_CaptureEvent` virtual function.",     // [4]
        "Failed to register `Element_SetWindowPointer` virtual function.", // [5]
        "Failed to register `Element_ShouldRedrawAll` virtual function.",  // [6]
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
        "NEUIK_Container",                                // className
        "This Element may contain one or more Elements.", // classDescription
        neuik__Set_NEUIK,                                 // classSet
        neuik__Class_Element,                             // superClass
        &neuik_Container_BaseFuncs,                       // baseFuncs
        NULL,                                             // classFuncs XXXXX
        &neuik__Class_Container))                         // newClass
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Register virtual function implementations                              */
    /*------------------------------------------------------------------------*/
    if (neuik_VirtualFunc_RegisterImplementation(
        &neuik_Element_vfunc_IsShown,
        neuik__Class_Container,
        neuik_Element_IsShown__Container))
    {
        eNum = 3;
        goto out;
    }

    if (neuik_VirtualFunc_RegisterImplementation(
        &neuik_Element_vfunc_CaptureEvent,
        neuik__Class_Container,
        neuik_Element_CaptureEvent__Container))
    {
        eNum = 4;
        goto out;
    }

    if (neuik_VirtualFunc_RegisterImplementation(
        &neuik_Element_vfunc_SetWindowPointer,
        neuik__Class_Container,
        neuik_Element_SetWindowPointer__Container))
    {
        eNum = 5;
        goto out;
    }

    if (neuik_VirtualFunc_RegisterImplementation(
        &neuik_Element_vfunc_ShouldRedrawAll,
        neuik__Class_Container,
        neuik_Element_ShouldRedrawAll__Container))
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
 *  Name:          neuik_Object_New__Container
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Container(
        void ** contPtr)
{
    int               eNum       = 0;
    NEUIK_Container * cont       = NULL;
    NEUIK_Element   * sClassPtr  = NULL;
    static char       funcName[] = "neuik_Object_New__Container";
    static char     * errMsgs[]  = {"",             // [0] no error
        "Output Argument `contPtr` is NULL.",       // [1]
        "Failure to allocate memory.",              // [2]
        "Failure in `neuik_GetObjectBaseOfClass`.", // [3]
        "Failure in function `neuik.NewElement`.",  // [4]
    };

    if (contPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    (*contPtr) = (NEUIK_Container*) malloc(sizeof(NEUIK_Container));
    cont = *contPtr;
    if (cont == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Successful allocation of Memory -- Create Base Class Object            */
    /*------------------------------------------------------------------------*/
    if (neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_Container, 
            NULL,
            &(cont->objBase)))
    {
        eNum = 3;
        goto out;
    }
    cont->elems        = NULL;
    cont->n_allocated  = 0;
    cont->n_used       = 0;
    cont->cType        = NEUIK_CONTAINER_UNSET;
    cont->shownIfEmpty = 0;
    cont->redrawAll    = 0;
    cont->VJustify     = NEUIK_VJUSTIFY_CENTER;
    cont->HJustify     = NEUIK_HJUSTIFY_CENTER;

    /*------------------------------------------------------------------------*/
    /* Create first level Base SuperClass Object                              */
    /*------------------------------------------------------------------------*/
    sClassPtr = (NEUIK_Element *) &(cont->objBase.superClassObj);
    if (neuik_Object_New(neuik__Class_Element, sClassPtr))
    {
        eNum = 4;
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
 *  Name:          neuik_Container_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Container(
    void * contPtr)
{
    int               ctr;
    int               eNum       = 0;    /* which error to report (if any) */
    NEUIK_Element     elem       = NULL;
    NEUIK_Container * cont       = NULL;
    static char       funcName[] = "neuik_Object_Free__Container";
    static char     * errMsgs[]  = {"",                          // [0] no error
        "Argument `contPtr` is NULL.",                           // [1]
        "Argument `contPtr` is not of Container class.",         // [2]
        "Failure in function `neuik_Object_Free` (superclass).", // [3]
        "Failure in function `neuik_Object_Free` (child).",      // [4]
    };

    if (contPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(contPtr, neuik__Class_Container))
    {
        eNum = 2;
        goto out;
    }
    cont = (NEUIK_Container*)contPtr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(cont->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    if (cont->elems != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Free all of the contained elements.                                */
        /*--------------------------------------------------------------------*/
        for (ctr = 0;; ctr++)
        {
            elem = cont->elems[ctr];
            if (elem == NULL) break; /* end of NULL-ptr terminated array */

            if(neuik_Object_Free(elem))
            {
                eNum = 4;
                goto out;
            }
        }
    }

    free(cont);
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
 *  Name:          neuik_Element_CaptureEvent__Container
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__Container(
    NEUIK_Element   cont, 
    SDL_Event     * ev)
{
    int                ctr        = 0;
    neuik_EventState   evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
    NEUIK_Element      elem;
    NEUIK_Container  * cBase;

    if (neuik_Object_GetClassObject_NoError(
        cont, neuik__Class_Container, (void**)&cBase)) goto out;

    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            elem = cBase->elems[ctr];
            if (elem == NULL) break;

            if (!NEUIK_Element_IsShown(elem)) continue;

            evCaputred = neuik_Element_CaptureEvent(elem, ev);
            if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
            {
                goto out;
            }
            if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
            {
                neuik_Element_SetActive(cont, 1);
                goto out;
            }
        }
    }
out:
    return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer__Container (redefined-vfunc)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
******************************************************************************/
int neuik_Element_SetWindowPointer__Container (
    NEUIK_Element   cont, 
    void          * win)
{
    int                  eNum     = 0;
    int                  ctr      = 0;
    NEUIK_Element        elem     = NULL; 
    NEUIK_ElementBase  * eBase    = NULL;
    NEUIK_Container    * cBase    = NULL;
    static int           nRecurse = 0; /* number of times recursively called */
    static char          funcName[] = "neuik_Element_SetWindowPointer__Container";
    static char        * errMsgs[]  = {"",                                    // [0] no error
        "Argument `elem` caused `GetClassObject` to fail. Not a Container?.", // [1]
        "Child Element caused `SetWindowPointer` to fail.",                   // [2]
        "Argument `elem` caused `GetClassObject` to fail. Not an Element?.",  // [3]
        "Argument `win` does not implement Window class.",                    // [4]
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

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 1;
        goto out;
    }

    if (cBase->elems != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Propagate this information to contained UI Elements                */
        /*--------------------------------------------------------------------*/
        for (ctr = 0;; ctr++)
        {
            elem = cBase->elems[ctr];
            if (elem == NULL) break;

            if (neuik_Element_SetWindowPointer(elem, win))
            {
                eNum = 2;
                goto out;
            }
        }
    }

    if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 3;
        goto out;
    }

    if (!neuik_Object_ImplementsClass(win, neuik__Class_Window))
    {
        eNum = 4;
        goto out;
    }

    eBase->eSt.window = win;
out:
    nRecurse--;
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_ShouldRedrawAll__Container (redefined-vfunc)
 *
 *  Description:   This function is used to indicate (to child elements) that
 *                 a parent element requires a full redraw.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       TRUE (1) if a redraw is needed, FALSE (0) otherwise.
 *
******************************************************************************/
int neuik_Element_ShouldRedrawAll__Container (
    NEUIK_Element   contPtr) /* [bool] output arg; if a redraw is needed*/
{
    NEUIK_Element     * parent = NULL;
    NEUIK_ElementBase * eBase  = NULL;
    NEUIK_Container   * cont   = NULL;

    if (neuik_Object_GetClassObject_NoError(
        contPtr, neuik__Class_Container, (void**)&cont))
    {
        return FALSE;
    }

    if (cont->redrawAll)
    {
        return TRUE;
    }

    if (neuik_Object_GetClassObject_NoError(contPtr, 
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


/*******************************************************************************
 *
 *  Name:          neuik_Container_RequestFullRedraw
 *
 *  Description:   Redraw the entire background for the container and force a
 *                 redraw of all contained elements.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int neuik_Container_RequestFullRedraw(
    NEUIK_Element cont)
{
    int               eNum       = 0; /* which error to report (if any) */
    NEUIK_Container * cBase      = NULL;
    RenderSize        rSize;
    RenderLoc         rLoc;
    static char       funcName[] = "neuik_Container_RequestFullRedraw";
    static char     * errMsgs[]  = {"", // [ 0] no error
        "Argument `cont` does not implement Container class.",           // [1]
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",              // [3]
        "Failure in `neuik_Element_RequestRedraw()`.",                   // [4]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 2;
        goto out;
    }

    cBase->redrawAll = 1;
    /*------------------------------------------------------------------------*/
    /* Make sure the window redraws the background for the entire size of the */
    /* current container.                                                     */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(cont, &rSize, &rLoc))
    {
        eNum = 3;
        goto out;
    }
    if (neuik_Element_RequestRedraw(cont, rLoc, rSize))
    {
        eNum = 4;
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
 *  Name:          NEUIK_Container_SetElement
 *
 *  Description:   Set the child element of a single-element container.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_SetElement(
    NEUIK_Element cont, 
    NEUIK_Element elem)
{
    int                 eNum       = 0; /* which error to report (if any) */
    NEUIK_ElementBase * eBase      = NULL;
    NEUIK_Container   * cBase      = NULL;
    static char         funcName[] = "NEUIK_Container_SetElement";
    static char       * errMsgs[]  = {"",                                 // [0] no error
        "Argument `cont` does not implement Container class.",            // [1]
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
        "Argument `elem` does not implement Element class.",              // [3]
        "Argument `cont` is not a SingleElement Container.",              // [4]
        "Failure to allocate memory.",                                    // [5]
        "Argument `cont` does not allow the use of method SetElement().", // [6]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 2;
        goto out;
    }
    if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* SetElement should only be used on single-element containers            */
    /*------------------------------------------------------------------------*/
    if (cBase->cType == NEUIK_CONTAINER_NO_DEFAULT_ADD_SET)
    {
        eNum = 6;
        goto out;
    }
    else if (cBase->cType != NEUIK_CONTAINER_SINGLE)
    {
        eNum = 4;
        goto out;
    }

    if (cBase->elems == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* elems array currently unallocated; allocate now                    */
        /*--------------------------------------------------------------------*/
        cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
        if (cBase->elems == NULL)
        {
            eNum = 5;
            goto out;
        }
        cBase->n_allocated = 1;
        cBase->n_used      = 1;
        cBase->elems[1] = NULL; /* NULLptr terminated array */
    }

    cBase->elems[0] = elem;

    /*------------------------------------------------------------------------*/
    /* Set the Window and Parent Element pointers                             */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (eBase->eSt.window != NULL)
    {
        neuik_Element_SetWindowPointer(elem, eBase->eSt.window);
    }
    neuik_Element_SetParentPointer(elem, cont);
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
 *  Name:          NEUIK_Container_AddElement
 *
 *  Description:   Add a child element to a multi-element container.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_AddElement(
    NEUIK_Element cont, 
    NEUIK_Element elem)
{
    int                 len;
    int                 ctr;
    int                 newInd;            /* index for newly added item */
    int                 eNum       = 0;    /* which error to report (if any) */
    NEUIK_ElementBase * eBase      = NULL;
    NEUIK_Container   * cBase      = NULL;
    RenderSize          rSize;
    RenderLoc           rLoc;
    static char         funcName[] = "NEUIK_Container_AddElement";
    static char       * errMsgs[]  = {"",                                 // [0] no error
        "Argument `cont` does not implement Container class.",            // [1]
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
        "Argument `elem` does not implement Element class.",              // [3]
        "Argument `cont` is not a MultiElement Container.",               // [4]
        "Failure to allocate memory.",                                    // [5]
        "Failure to reallocate memory.",                                  // [6]
        "Argument `cont` does not allow the use of method AddElement().", // [7]
        "Failure in `neuik_Element_RequestRedraw()`.",                    // [8]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",               // [9]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }
    if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* SetElement should only be used on single-element containers            */
    /*------------------------------------------------------------------------*/
    if (cBase->cType == NEUIK_CONTAINER_NO_DEFAULT_ADD_SET)
    {
        eNum = 7;
        goto out;
    }
    else if (cBase->cType != NEUIK_CONTAINER_MULTI)
    {
        eNum = 4;
        goto out;
    }

    if (cBase->elems == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* elems array currently unallocated; allocate now                    */
        /*--------------------------------------------------------------------*/
        cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
        if (cBase->elems == NULL)
        {
            eNum = 5;
            goto out;
        }
        cBase->n_allocated = 1;
        cBase->n_used      = 1;
        newInd             = 0;
    }
    else if (cBase->n_allocated < cBase->n_used)
    {
        /*--------------------------------------------------------------------*/
        /* This is subsequent UI element, but there is space available in the */
        /* container elements arrary.                                         */
        /*--------------------------------------------------------------------*/
        newInd = cBase->n_used;
        cBase->n_used++;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* This is subsequent UI element, reallocate memory.                  */
        /* This pointer array will be null terminated.                        */
        /*--------------------------------------------------------------------*/
        
        /* determine the current length */
        for (ctr = 0;;ctr++)
        {
            if (cBase->elems[ctr] == NULL)
            {
                len = 2 + ctr;
                break;
            }
        }

        cBase->elems = (NEUIK_Element*)realloc(cBase->elems, len*sizeof(NEUIK_Element));
        if (cBase->elems == NULL)
        {
            eNum = 6;
            goto out;
        }
        cBase->n_allocated++;
        cBase->n_used++;
        newInd = ctr;
    }

    /*------------------------------------------------------------------------*/
    /* Set the Window and Parent Element pointers                             */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
    {
        eNum = 2;
        goto out;
    }
    if (eBase->eSt.window != NULL)
    {
        neuik_Element_SetWindowPointer(elem, eBase->eSt.window);
    }
    neuik_Element_SetParentPointer(elem, cont);

    cBase->elems[newInd]   = elem;
    cBase->elems[newInd+1] = NULL; /* NULLptr terminated array */

    /*------------------------------------------------------------------------*/
    /* When a new element is added to a container trigger a redraw            */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(cont, &rSize, &rLoc))
    {
        eNum = 9;
        goto out;
    }
    if (neuik_Element_RequestRedraw(cont, rLoc, rSize))
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
out2:
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Container_AddElements
 *
 *  Description:   Add multiple child element to a multi-element container.
 *
 *                 NOTE: the variable # of arguments must be terminated by a 
 *                 NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_AddElements(
    NEUIK_Element cont, 
    NEUIK_Element elem0, 
    ...)
{
    int                  ctr;
    int                  vaOpen = 0;
    int                  eNum   = 0; /* which error to report (if any) */
    va_list              args;
    NEUIK_Element        elem; 
    static char          funcName[] = "NEUIK_Container_AddElements";
    static char        * errMsgs[] = {"",                                    // [0] no error
        "Argument `cont` does not implement Container class.",               // [1]
        "Failure in `Container_AddElement()`.",                              // [2]
        "SIGSEGV (segmentation fault) captured; is call `NULL` terminated?", // [3]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        if (neuik_HasFatalError())
        {
            eNum = 3;
            goto out;
        }
        eNum = 1;
        goto out;
    }

    va_start(args, elem0);
    vaOpen = 1;

    elem = elem0;
    for (ctr = 0;; ctr++)
    {
        if (elem == NULL) break;

        if (NEUIK_Container_AddElement(cont, elem))
        {
            if (neuik_HasFatalError())
            {
                eNum = 3;
                goto out;
            }
            eNum = 2;
            goto out;
        }

        /* before starting */
        elem = va_arg(args, NEUIK_Element);
    }
out:
    if (vaOpen) va_end(args);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Defocus__Container (redefined-vfunc)
 *
 *  Description:   Call Element defocus function.
 *
 *                 This operation is a virtual function redefinition.
 *
 ******************************************************************************/
void neuik_Element_Defocus__Container(
    NEUIK_Element cont)
{
    int               ctr;
    NEUIK_Element     elem;
    NEUIK_Container * cBase = NULL;

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        return;
    }

    /*------------------------------------------------------------------------*/
    /* Defocus all contained elements                                         */
    /*------------------------------------------------------------------------*/
    if (cBase->elems == NULL) return;
    elem = cBase->elems[0];

    for (ctr = 1;;ctr++)
    {
        if (elem == NULL) break;
        neuik_Element_Defocus(elem);
        elem = cBase->elems[ctr];
    }
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_IsShown__Container    (redefined-vfunc)
 *
 *  Description:   This function reports whether or not an element is currently
 *                 being shown.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if element is shown, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_IsShown__Container(
    NEUIK_Element  cont)
{
    int                 isShown  = 0;
    int                 anyShown = 0;
    int                 ctr;
    NEUIK_Element       elem;
    NEUIK_ElementBase * eBase;
    NEUIK_Container   * cBase = NULL;
    static int          nRecurse = 0; /* number of times recursively called */

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

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* First check if this element is being shown.                            */
    /*------------------------------------------------------------------------*/
    if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
    {
        goto out;
    }

    if (!eBase->eCfg.Show) goto out;

    /*------------------------------------------------------------------------*/
    /* Examine the contained elements to see if any of them are being shown.  */
    /*------------------------------------------------------------------------*/
    if (cBase->elems == NULL) goto out;
    elem = cBase->elems[0];

    for (ctr = 1;;ctr++)
    {
        if (elem == NULL) break;
        if (NEUIK_Element_IsShown(elem))
        {
            if (neuik_HasFatalError())
            {
                goto out;
            }
            anyShown = 1;
            break;
        }
        if (neuik_HasFatalError())
        {
            goto out;
        }
        elem = cBase->elems[ctr];
    }

    /*------------------------------------------------------------------------*/
    /* Even no child elements are shown; the container may still be shown.    */
    /*------------------------------------------------------------------------*/
    if (anyShown || cBase->shownIfEmpty)
    {
        isShown = 1;
    }
out:
    nRecurse--;
    return isShown;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Container_GetElementCount
 *
 *  Description:   Add multiple child element to a multi-element container.
 *
 *                 NOTE: the variable # of arguments must be terminated by a 
 *                 NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_GetElementCount(
    NEUIK_Element   cont,
    int           * elemCount) 
{
    int                  ctr;
    int                  count  = 0;
    int                  eNum   = 0; /* which error to report (if any) */
    NEUIK_Element        elem; 
    NEUIK_Container   * cBase = NULL;
    static char          funcName[] = "NEUIK_Container_GetElementCount";
    static char        * errMsgs[] = {"",                                // [0] no error
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [1]
    };

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 1;
        goto out;
    }

    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            elem = cBase->elems[ctr];
            if (elem == NULL) break;

            count += 1;
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    (*elemCount) = count;
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Container_GetFirstElement
 *
 *  Description:   Returns the pointer to the first stored element of a 
 *                 multi-element container, or NULL if the container doesn't 
 *                 currently contain any elements.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_GetFirstElement(
    NEUIK_Element   cont,
    NEUIK_Element * elem) 
{
    int               ctr;
    int               eNum   = 0; /* which error to report (if any) */
    NEUIK_Element     nextElem; 
    NEUIK_Container * cBase = NULL;
    static char       funcName[] = "NEUIK_Container_GetFirstElement";
    static char     * errMsgs[] = {"", // [0] no error
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `elem` is NULL.",                               // [2]
    };

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 1;
        goto out;
    }
    if (elem == NULL)
    {
        eNum = 2;
        goto out;
    }

    *elem = NULL;

    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            nextElem = cBase->elems[ctr];
            if (nextElem != NULL)
            {
                *elem = nextElem;
                break;
            }
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
 *  Name:          NEUIK_Container_GetLastElement
 *
 *  Description:   Returns the pointer to the last stored element of a 
 *                 multi-element container, or NULL if the container doesn't 
 *                 currently contain any elements.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_GetLastElement(
    NEUIK_Element   cont,
    NEUIK_Element * elem) 
{
    int               ctr;
    int               eNum   = 0; /* which error to report (if any) */
    NEUIK_Element     nextElem; 
    NEUIK_Container * cBase = NULL;
    static char       funcName[] = "NEUIK_Container_GetLastElement";
    static char     * errMsgs[] = {"", // [0] no error
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `elem` is NULL.",                               // [2]
    };

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 1;
        goto out;
    }
    if (elem == NULL)
    {
        eNum = 2;
        goto out;
    }

    *elem = NULL;

    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            nextElem = cBase->elems[ctr];
            if (nextElem == NULL) break;

            *elem = nextElem;
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
 *  Name:          NEUIK_Container_GetNthElement
 *
 *  Description:   Returns the pointer to the N'th stored element of a 
 *                 multi-element container, or NULL if the container doesn't 
 *                 currently contain the specfied N'th element.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_GetNthElement(
    NEUIK_Element   cont,
    int             n,
    NEUIK_Element * elem) 
{
    int               ctr;
    int               eNum   = 0; /* which error to report (if any) */
    NEUIK_Element     nextElem; 
    NEUIK_Container * cBase = NULL;
    static char       funcName[] = "NEUIK_Container_GetNthElement";
    static char     * errMsgs[] = {"", // [0] no error
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [1]
        "Output argument `elem` is NULL.",                               // [2]
    };

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        eNum = 1;
        goto out;
    }
    if (elem == NULL)
    {
        eNum = 2;
        goto out;
    }

    *elem = NULL;

    if (cBase->elems != NULL)
    {
        for (ctr = 0;; ctr++)
        {
            nextElem = cBase->elems[ctr];
            if (nextElem == NULL) break;
            if (ctr == n)
            {
                *elem = nextElem;
                break;
            }
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
 *  Name:          NEUIK_Container_RemoveElement
 *
 *  Description:   Remove an element from a container. NOTE; This does not free
 *                 memory associated with the element.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_RemoveElement(
    NEUIK_Element cont, 
    NEUIK_Element elem)
{
    int                 ctr;
    int                 wasLocated = 0;
    int                 eNum       = 0;    /* which error to report (if any) */
    NEUIK_Container   * cBase      = NULL;
    RenderSize          rSize;
    RenderLoc           rLoc;
    static char         funcName[] = "NEUIK_Container_RemoveElement";
    static char       * errMsgs[]  = {"", // [0] no error
        "Argument `cont` does not implement Container class.",            // [1]
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
        "Argument `elem` does not implement Element class.",              // [3]
        "Container does not contain any child elements.",                 // [4]
        "Unable to locate specified `elem` within Container.",            // [5]
        "Failure in `neuik_Element_RequestRedraw()`.",                    // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",               // [7]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }
    if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 3;
        goto out;
    }

    if (cBase->elems == NULL || cBase->n_used == 0)
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Search through the elements in the container and look for the element  */
    /* to be removed.                                                         */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;;ctr++)
    {
        if (cBase->elems[ctr] == NULL)
        {
            if (!wasLocated)
            {
                /*------------------------------------------------------------*/
                /* The container did not contain the desired element          */
                /*------------------------------------------------------------*/
                eNum = 5;
                goto out;
            }

            cBase->elems[ctr-1] = NULL;
            break;
        }

        if (wasLocated)
        {
            /*----------------------------------------------------------------*/
            /* Shuffle over the next value.                                   */
            /*----------------------------------------------------------------*/
            cBase->elems[ctr-1] = cBase->elems[ctr];
        }
        else if (cBase->elems[ctr] == elem)
        {
            /*----------------------------------------------------------------*/
            /* The element to be removed has been located.                    */
            /*----------------------------------------------------------------*/
            wasLocated = 1;
        }
    }

    cBase->n_used--;

    /*------------------------------------------------------------------------*/
    /* When an element is removed from a container; trigger a redraw          */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(cont, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    if (neuik_Element_RequestRedraw(cont, rLoc, rSize))
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
out2:
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Container_DeleteElements
 *
 *  Description:   Remove and Free all child elements from a container.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_DeleteElements(
    NEUIK_Element cont)
{
    int               ctr;
    int               eNum       = 0;    /* which error to report (if any) */
    NEUIK_Container * cBase      = NULL;
    RenderSize        rSize;
    RenderLoc         rLoc;
    static char       funcName[] = "NEUIK_Container_RemoveElements";
    static char     * errMsgs[]  = {"",                                  // [0] no error
        "Argument `cont` does not implement Container class.",           // [1]
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [2]
        "Argument `elem` does not implement Element class.",             // [3]
        "Container does not contain any child elements.",                // [4]
        "Unable to locate specified `elem` within Container.",           // [5]
        "Failure in `neuik_Element_RequestRedraw()`.",                   // [6]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",              // [7]
    };

    if (!neuik_Object_ImplementsClass(cont, neuik__Class_Container))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 1;
        goto out;
    }
    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        if (neuik_HasFatalError())
        {
            eNum = 1;
            goto out2;
        }
        eNum = 2;
        goto out;
    }

    if (cBase->elems == NULL || cBase->n_used == 0)
    {
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Search through the elements in the container and look for the element  */
    /* to be removed.                                                         */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;;ctr++)
    {
        if (cBase->elems[ctr] == NULL)
        {
            break;
        }
        else
        {
            cBase->n_used--;
            neuik_Object_Free(cBase->elems[ctr]);
            cBase->elems[ctr] = NULL;
        }
    }

    /*------------------------------------------------------------------------*/
    /* When an element is removed from a container; trigger a redraw          */
    /*------------------------------------------------------------------------*/
    if (neuik_Element_GetSizeAndLocation(cont, &rSize, &rLoc))
    {
        eNum = 7;
        goto out;
    }
    if (neuik_Element_RequestRedraw(cont, rLoc, rSize))
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
out2:
    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Container_Configure
 *
 *  Description:   Configure one or more settings for a container.
 *
 *                 NOTE: This list of settings must be terminated by a NULL
 *                 pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Container_Configure(
    NEUIK_Element   cont,
    const char    * set0,
    ...)
{
    int               ctr;
    // int               nCtr;
    int               isBool;
    int               doRedraw   = 0;
    int               typeMixup;
    va_list           args;
    char            * strPtr     = NULL;
    char            * name       = NULL;
    char            * value      = NULL;
    const char      * set        = NULL;
    char              buf[4096];
    RenderSize        rSize      = {0, 0};
    RenderLoc         rLoc       = {0, 0};;
    NEUIK_Container * cBase      = NULL;
    /*------------------------------------------------------------------------*/
    /* If a `name=value` string with an unsupported name is found, check to   */
    /* see if a boolName was mistakenly used instead.                         */
    /*------------------------------------------------------------------------*/
    // static char     * boolNames[] = {
    //  NULL,
    // };
    /*------------------------------------------------------------------------*/
    /* If a boolName string with an unsupported name is found, check to see   */
    /* if a supported nameValue type was mistakenly used instead.             */
    /*------------------------------------------------------------------------*/
    // static char     * valueNames[] = {
    //  "HJustify",
    //  "VJustify",
    //  NULL,
    // };
    static char       funcName[] = "NEUIK_Container_Configure";
    static char     * errMsgs[]  = {"",                                  // [ 0] no error
        "Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [ 1]
        "NamedSet.name is NULL, skipping.",                              // [ 2]
        "NamedSet.name is blank, skipping.",                             // [ 3]
        "NamedSet.name type unknown, skipping.",                         // [ 4]
        "`name=value` string is too long.",                              // [ 5]
        "Set string is empty.",                                          // [ 6]
        "HJustify value is invalid.",                                    // [ 7]
        "VJustify value is invalid.",                                    // [ 8]
        "BoolType name unknown, skipping.",                              // [ 9]
        "Invalid `name=value` string.",                                  // [10]
        "ValueType name used as BoolType, skipping.",                    // [11]
        "BoolType name used as ValueType, skipping.",                    // [12]
        "Failure in `neuik_Element_GetSizeAndLocation()`.",              // [13]
    };

    if (neuik_Object_GetClassObject(cont, neuik__Class_Container, (void**)&cBase))
    {
        NEUIK_RaiseError(funcName, errMsgs[1]);
        return 1;
    }

    set = set0;

    va_start(args, set0);

    for (ctr = 0;; ctr++)
    {
        isBool = 0;
        name   = NULL;
        value  = NULL;

        if (set == NULL) break;

        // #ifndef NO_NEUIK_SIGNAL_TRAPPING
        //  signal(SIGSEGV, neuik_Element_Configure_capture_segv);
        // #endif

        if (strlen(set) > 4095)
        {
            // #ifndef NO_NEUIK_SIGNAL_TRAPPING
            //  signal(SIGSEGV, NULL);
            // #endif
            NEUIK_RaiseError(funcName, errMsgs[5]);
            set = va_arg(args, const char *);
            continue;
        }
        else
        {
            // #ifndef NO_NEUIK_SIGNAL_TRAPPING
            //  signal(SIGSEGV, NULL);
            // #endif
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
                name    = buf;
                if (buf[0] == '!')
                {
                    name = buf + 1;
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

        if (!isBool)
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
                    cBase->HJustify = NEUIK_HJUSTIFY_LEFT;
                    doRedraw = 1;
                }
                else if (!strcmp("center", value))
                {
                    cBase->HJustify = NEUIK_HJUSTIFY_CENTER;
                    doRedraw = 1;
                }
                else if (!strcmp("right", value))
                {
                    cBase->HJustify = NEUIK_HJUSTIFY_RIGHT;
                    doRedraw = 1;
                }
                else if (!strcmp("default", value))
                {
                    cBase->HJustify = NEUIK_HJUSTIFY_DEFAULT;
                    doRedraw = 1;
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
                    cBase->VJustify = NEUIK_VJUSTIFY_TOP;
                    doRedraw = 1;
                }
                else if (!strcmp("center", value))
                {
                    cBase->VJustify = NEUIK_VJUSTIFY_CENTER;
                    doRedraw = 1;
                }
                else if (!strcmp("bottom", value))
                {
                    cBase->VJustify = NEUIK_VJUSTIFY_BOTTOM;
                    doRedraw = 1;
                }
                else if (!strcmp("default", value))
                {
                    cBase->VJustify = NEUIK_VJUSTIFY_DEFAULT;
                    doRedraw = 1;
                }
                else
                {
                    NEUIK_RaiseError(funcName, errMsgs[8]);
                }
            }
            else
            {
                typeMixup = 0;
                // for (nCtr = 0;; nCtr++)
                // {
                //  if (boolNames[nCtr] == NULL) break;

                //  if (!strcmp(boolNames[nCtr], name))
                //  {
                //      typeMixup = 1;
                //      break;
                //  }
                // }

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
        if (neuik_Element_GetSizeAndLocation(cont, &rSize, &rLoc))
        {
            NEUIK_RaiseError(funcName, errMsgs[13]);
            return 1;
        }
        neuik_Element_RequestRedraw(cont, rLoc, rSize);
    }

    return 0;
}

