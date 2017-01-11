/*******************************************************************************
 * Copyright (c) 2014-2017, Michael Leimon <leimon@gmail.com>
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
int neuik_Object_Free__Container(void ** contPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
int neuik_Element_CaptureEvent__Container(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_IsShown__Container(NEUIK_Element);
int neuik_Element_SetWindowPointer__Container(NEUIK_Element, void*);


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
	cont->cType        = NEUIK_CONTAINER_UNSET;
	cont->shownIfEmpty = 0;

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
	void ** contPtr)
{
	int               ctr;
	int               eNum       = 0;    /* which error to report (if any) */
	NEUIK_Element     elem       = NULL;
	NEUIK_Container * cont       = NULL;
	static char       funcName[] = "neuik_Object_Free__Container";
	static char     * errMsgs[]  = {"",                  // [0] no error
		"Argument `contPtr` is NULL.",                   // [1]
		"Argument `contPtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",      // [3]
	};

	if (contPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(*contPtr, neuik__Class_Container))
	{
		eNum = 2;
		goto out;
	}
	cont = *contPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(&(cont->objBase.superClassObj)))
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

			if(neuik_Object_Free(&elem))
			{
				eNum = 3;
				goto out;
			}
		}
	}

	free(cont);
	(*contPtr) = NULL;
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
int neuik_Element_CaptureEvent__Container(
	NEUIK_Element   cont, 
	SDL_Event     * ev)
{
	int               ctr        = 0;
	int               evCaputred = 0;
	NEUIK_Element     elem;
	NEUIK_Container * cBase;

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
			if (evCaputred)
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

	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Container))
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
		/*----------------------------------------------------------------*/
		/* elems array currently unallocated; allocate now                */
		/*----------------------------------------------------------------*/
		cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
		if (cBase->elems == NULL)
		{
			eNum = 5;
			goto out;
		}
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
	static char         funcName[] = "NEUIK_Container_AddElement";
	static char       * errMsgs[]  = {"",                                 // [0] no error
		"Argument `cont` does not implement Container class.",            // [1]
		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
		"Argument `elem` does not implement Element class.",              // [3]
		"Argument `cont` is not a MultiElement Container.",               // [4]
		"Failure to allocate memory.",                                    // [5]
		"Failure to reallocate memory.",                                  // [6]
		"Argument `cont` does not allow the use of method AddElement().", // [7]
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
		newInd = 0;
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


