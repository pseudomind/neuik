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

#include "NEUIK_Plot.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Container.h"
#include "NEUIK_Event.h"
#include "NEUIK_Window.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Label.h"
#include "NEUIK_Frame.h"
#include "NEUIK_Transformer.h"
#include "NEUIK_CelGroup.h"
#include "NEUIK_HGroup.h"
#include "NEUIK_VGroup.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Plot(void ** contPtr);
int neuik_Object_Free__Plot(void * contPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
neuik_EventState neuik_Element_CaptureEvent__Plot(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_IsShown__Plot(NEUIK_Element);
int neuik_Element_SetWindowPointer__Plot(NEUIK_Element, void*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Plot_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Plot,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Plot,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Plot
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Plot()
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_RegisterClass_Plot";
	static char * errMsgs[]  = {"",                                        // [0] no error
		"NEUIK library must be initialized first.",                        // [1]
		"Failed to register `Plot` object class.",                         // [2]
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
		"NEUIK_Plot",                              // className
		"This Element contains a plot of values.", // classDescription
		neuik__Set_NEUIK,                          // classSet
		neuik__Class_Element,                      // superClass
		&neuik_Plot_BaseFuncs,                     // baseFuncs
		NULL,                                      // classFuncs XXXXX
		&neuik__Class_Plot))                       // newClass
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Register virtual function implementations                              */
	/*------------------------------------------------------------------------*/
	// if (neuik_VirtualFunc_RegisterImplementation(
	// 	&neuik_Element_vfunc_IsShown,
	// 	neuik__Class_Plot,
	// 	neuik_Element_IsShown__Plot))
	// {
	// 	eNum = 3;
	// 	goto out;
	// }

	// if (neuik_VirtualFunc_RegisterImplementation(
	// 	&neuik_Element_vfunc_CaptureEvent,
	// 	neuik__Class_Plot,
	// 	neuik_Element_CaptureEvent__Plot))
	// {
	// 	eNum = 4;
	// 	goto out;
	// }

	if (neuik_VirtualFunc_RegisterImplementation(
		&neuik_Element_vfunc_SetWindowPointer,
		neuik__Class_Plot,
		neuik_Element_SetWindowPointer__Plot))
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
 *  Name:          neuik_Object_New__Plot
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Plot(
	void ** plotPtr)
{
	int             eNum       = 0;
	NEUIK_Label   * ylabel     = NULL;
	NEUIK_Plot    * plot       = NULL;
	NEUIK_Element * sClassPtr  = NULL;
	static char     funcName[] = "neuik_Object_New__Plot";
	static char   * errMsgs[]  = {"",                  // [0] no error
		"Output Argument `plotPtr` is NULL.",          // [1]
		"Failure to allocate memory.",                 // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",    // [3]
		"Failure in function `neuik.NewElement`.",     // [4]
		"Failure in `NEUIK_NewVGroup()`.",             // [5]
		"Failure in `NEUIK_MakeLabel()`.",             // [6]
		"Failure in `NEUIK_NewFrame()`.",              // [7]
		"Failure in `NEUIK_Container_AddElements()`.", // [8]
		"Failure in `NEUIK_NewHGroup()`.",             // [9]
		"Failure in `NEUIK_NewTransformer()`.",        // [10]
		"Failure in `NEUIK_Container_SetElement()`.",  // [11]
		"Failure in `NEUIK_Transformer_Configure()`.", // [12]
		"Failure in `NEUIK_NewCelGroup()`.",           // [13]
		"Failure in `NEUIK_Element_Configure()`.",     // [13]
	};

	if (plotPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*plotPtr) = (NEUIK_Plot*) malloc(sizeof(NEUIK_Plot));
	plot = *plotPtr;
	if (plot == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Plot, 
			NULL,
			&(plot->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate the child objects                                             */
	/*------------------------------------------------------------------------*/
	if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->visual))
	{
		eNum = 5;
		goto out;
	}
	if (NEUIK_Element_Configure(plot->visual, "FillAll", NULL))
	{
		eNum = 14;
		goto out;
	}

	if (NEUIK_NewVGroup((NEUIK_VGroup**)&plot->title))
	{
		eNum = 5;
		goto out;
	}
	if (NEUIK_NewHGroup((NEUIK_HGroup**)&plot->hg_data))
	{
		eNum = 9;
		goto out;
	}
	if (NEUIK_Element_Configure(plot->hg_data, "FillAll", NULL))
	{
		eNum = 14;
		goto out;
	}

	if (NEUIK_NewTransformer((NEUIK_Transformer**)&plot->y_label))
	{
		eNum = 10;
		goto out;
	}
	if (NEUIK_Transformer_Configure((NEUIK_Transformer*)plot->y_label, 
		"Rotation=270.0", NULL))
	{
		eNum = 12;
		goto out;
	}

	if (NEUIK_MakeLabel((NEUIK_Label**)&ylabel, "Plot y_label"))
	{
		eNum = 6;
		goto out;
	}
	if (NEUIK_Container_SetElement(plot->y_label, ylabel))
	{
		eNum = 11;
		goto out;
	}

	if (NEUIK_NewCelGroup((NEUIK_CelGroup**)&plot->drawing))
	{
		eNum = 13;
		goto out;
	}
	if (NEUIK_Element_Configure(plot->drawing, "FillAll", NULL))
	{
		eNum = 14;
		goto out;
	}

	if (NEUIK_MakeLabel((NEUIK_Label**)&plot->legend, "[Plot Legend]"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Container_AddElements(plot->hg_data, 
		plot->y_label,
		plot->drawing,
		plot->legend,
		NULL))
	{
		eNum = 9;
		goto out;
	}
	if (NEUIK_MakeLabel((NEUIK_Label**)&plot->x_label, "Plot x_label"))
	{
		eNum = 6;
		goto out;
	}

	if (NEUIK_Container_AddElements((NEUIK_VGroup*)plot->visual, 
			plot->title,
			plot->hg_data,
			plot->x_label,
			NULL))
	{
		eNum = 8;
		goto out;
	}

	plot->data_sets   = NULL;
	plot->n_allocated = 0;
	plot->n_used      = 0;
	plot->x_range_cfg = NEUIK_PLOTRANGECONFIG_AUTO;
	plot->x_range_min = 0;
	plot->x_range_max = 0;
	plot->y_range_cfg = NEUIK_PLOTRANGECONFIG_AUTO;
	plot->y_range_min = 0;
	plot->y_range_max = 0;

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(plot->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}

	if (NEUIK_Plot_SetTitle(plot, "Title of Plot"))
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
 *  Name:          neuik_Plot_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Plot(
	void * plotPtr)
{
	int             ctr;
	int             eNum       = 0;    /* which error to report (if any) */
	NEUIK_Element   elem       = NULL;
	NEUIK_Plot    * plot       = NULL;
	static char     funcName[] = "neuik_Object_Free__Plot";
	static char   * errMsgs[]  = {"",                            // [0] no error
		"Argument `plotPtr` is NULL.",                           // [1]
		"Argument `plotPtr` is not of Plot class.",              // [2]
		"Failure in function `neuik_Object_Free` (superclass).", // [3]
		"Failure in function `neuik_Object_Free` (title).",      // [4]
		"Failure in function `neuik_Object_Free` (x_label).",    // [5]
		"Failure in function `neuik_Object_Free` (y_label).",    // [6]
		"Failure in function `neuik_Object_Free` (legend).",     // [7]
		"Failure in function `neuik_Object_Free` (visual).",     // [8]
		"Failure in function `neuik_Object_Free` (data_set).",   // [9]
	};

	if (plotPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(plotPtr, neuik__Class_Plot))
	{
		eNum = 2;
		goto out;
	}
	plot = (NEUIK_Plot*)plotPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(plot->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	plot->data_sets   = NULL;

	/*------------------------------------------------------------------------*/
	/* Free the typical plot elements (if still allocated                     */
	/*------------------------------------------------------------------------*/
	if (plot->title != NULL)
	{
		if(neuik_Object_Free(plot->title))
		{
			eNum = 4;
			goto out;
		}
	}
	if (plot->x_label != NULL)
	{
		if(neuik_Object_Free(plot->x_label))
		{
			eNum = 5;
			goto out;
		}
	}
	if (plot->y_label != NULL)
	{
		if(neuik_Object_Free(plot->y_label))
		{
			eNum = 6;
			goto out;
		}
	}
	if (plot->drawing != NULL)
	{
		if(neuik_Object_Free(plot->drawing))
		{
			eNum = 6;
			goto out;
		}
	}
	if (plot->legend != NULL)
	{
		if(neuik_Object_Free(plot->legend))
		{
			eNum = 7;
			goto out;
		}
	}
	if (plot->visual != NULL)
	{
		if(neuik_Object_Free(plot->visual))
		{
			eNum = 8;
			goto out;
		}
	}

	if (plot->data_sets != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Free all of the contained data sets.                               */
		/*--------------------------------------------------------------------*/
		for (ctr = 0;; ctr++)
		{
			elem = plot->data_sets[ctr];
			if (elem == NULL) break; /* end of NULL-ptr terminated array */

			if(neuik_Object_Free(elem))
			{
				eNum = 9;
				goto out;
			}
		}
	}

	free(plot);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


// /*******************************************************************************
//  *
//  *  Name:          neuik_Element_CaptureEvent__Plot
//  *
//  *  Description:   A virtual function reimplementation of the function
//  *                 neuik_Element_CaptureEvent.
//  *
//  *  Returns:       1 if the event was captured; 0 otherwise.
//  *
//  ******************************************************************************/
// neuik_EventState neuik_Element_CaptureEvent__Plot(
// 	NEUIK_Element   cont, 
// 	SDL_Event     * ev)
// {
// 	int                ctr        = 0;
// 	neuik_EventState   evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
// 	NEUIK_Element      elem;
// 	NEUIK_Plot       * cBase;

// 	if (neuik_Object_GetClassObject_NoError(
// 		cont, neuik__Class_Plot, (void**)&cBase)) goto out;

// 	if (cBase->elems != NULL)
// 	{
// 		for (ctr = 0;; ctr++)
// 		{
// 			elem = cBase->elems[ctr];
// 			if (elem == NULL) break;

// 			if (!NEUIK_Element_IsShown(elem)) continue;

// 			evCaputred = neuik_Element_CaptureEvent(elem, ev);
// 			if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
// 			{
// 				goto out;
// 			}
// 			if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
// 			{
// 				neuik_Element_SetActive(cont, 1);
// 				goto out;
// 			}
// 		}
// 	}
// out:
// 	return evCaputred;
// }


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer__Plot (redefined-vfunc)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
******************************************************************************/
int neuik_Element_SetWindowPointer__Plot (
	NEUIK_Element   plotPtr, 
	void          * win)
{
	int                 eNum     = 0;
	int                 ctr      = 0;
	NEUIK_Element       elem     = NULL; 
	NEUIK_ElementBase * eBase    = NULL;
	NEUIK_Plot        * plot     = NULL;
	static int          nRecurse = 0; /* number of times recursively called */
	static char         funcName[] = "neuik_Element_SetWindowPointer__Plot";
	static char       * errMsgs[]  = {"",                                    // [0] no error
		"Argument `elem` caused `GetClassObject` to fail. Not a Plot?.",     // [1]
		"Child Element caused `SetWindowPointer` to fail.",                  // [2]
		"Argument `elem` caused `GetClassObject` to fail. Not an Element?.", // [3]
		"Argument `win` does not implement Window class.",                   // [4]
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

	if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the window pointers for typical plot elements (if present).        */
	/*------------------------------------------------------------------------*/
	if (plot->title != NULL)
	{
		if (neuik_Element_SetWindowPointer(plot->title, win))
		{
			eNum = 2;
			goto out;
		}
	}
	if (plot->x_label != NULL)
	{
		if (neuik_Element_SetWindowPointer(plot->x_label, win))
		{
			eNum = 2;
			goto out;
		}
	}
	if (plot->y_label != NULL)
	{
		if (neuik_Element_SetWindowPointer(plot->y_label, win))
		{
			eNum = 2;
			goto out;
		}
	}
	if (plot->legend != NULL)
	{
		if (neuik_Element_SetWindowPointer(plot->legend, win))
		{
			eNum = 2;
			goto out;
		}
	}
	if (plot->visual != NULL)
	{
		if (neuik_Element_SetWindowPointer(plot->visual, win))
		{
			eNum = 2;
			goto out;
		}
	}

	if (plot->data_sets != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Propagate this information to contained UI Elements                */
		/*--------------------------------------------------------------------*/
		for (ctr = 0;; ctr++)
		{
			elem = plot->data_sets[ctr];
			if (elem == NULL) break;

			if (neuik_Element_SetWindowPointer(elem, win))
			{
				eNum = 2;
				goto out;
			}
		}
	}

	if (neuik_Object_GetClassObject(plot, neuik__Class_Element, (void**)&eBase))
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


// /*******************************************************************************
//  *
//  *  Name:          NEUIK_Plot_SetElement
//  *
//  *  Description:   Set the child element of a single-element container.
//  *
//  *  Returns:       1 if there is an error; 0 otherwise.
//  *
//  ******************************************************************************/
// int NEUIK_Plot_SetElement(
// 	NEUIK_Element cont, 
// 	NEUIK_Element elem)
// {
// 	int                 eNum       = 0; /* which error to report (if any) */
// 	NEUIK_ElementBase * eBase      = NULL;
// 	NEUIK_Plot        * cBase      = NULL;
// 	static char         funcName[] = "NEUIK_Plot_SetElement";
// 	static char       * errMsgs[]  = {"",                                 // [0] no error
// 		"Argument `cont` does not implement Plot class.",                 // [1]
// 		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
// 		"Argument `elem` does not implement Element class.",              // [3]
// 		"Argument `cont` is not a SingleElement Plot.",                   // [4]
// 		"Failure to allocate memory.",                                    // [5]
// 		"Argument `cont` does not allow the use of method SetElement().", // [6]
// 	};

// 	if (!neuik_Object_ImplementsClass(cont, neuik__Class_Plot))
// 	{
// 		eNum = 1;
// 		goto out;
// 	}
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		eNum = 2;
// 		goto out;
// 	}
// 	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
// 	{
// 		eNum = 3;
// 		goto out;
// 	}

// 	------------------------------------------------------------------------
// 	/* SetElement should only be used on single-element containers            */
// 	/*------------------------------------------------------------------------*/
// 	if (cBase->cType == NEUIK_CONTAINER_NO_DEFAULT_ADD_SET)
// 	{
// 		eNum = 6;
// 		goto out;
// 	}
// 	else if (cBase->cType != NEUIK_CONTAINER_SINGLE)
// 	{
// 		eNum = 4;
// 		goto out;
// 	}

// 	if (cBase->elems == NULL)
// 	{
// 		/*----------------------------------------------------------------*/
// 		/* elems array currently unallocated; allocate now                */
// 		/*----------------------------------------------------------------*/
// 		cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
// 		if (cBase->elems == NULL)
// 		{
// 			eNum = 5;
// 			goto out;
// 		}
// 		cBase->n_allocated = 1;
// 		cBase->n_used      = 1;
// 		cBase->elems[1] = NULL; /* NULLptr terminated array */
// 	}

// 	cBase->elems[0] = elem;

// 	/*------------------------------------------------------------------------*/
// 	/* Set the Window and Parent Element pointers                             */
// 	/*------------------------------------------------------------------------*/
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
// 	{
// 		eNum = 2;
// 		goto out;
// 	}
// 	if (eBase->eSt.window != NULL)
// 	{
// 		neuik_Element_SetWindowPointer(elem, eBase->eSt.window);
// 	}
// 	neuik_Element_SetParentPointer(elem, cont);
// out:
// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 		eNum = 1;
// 	}

// 	return eNum;
// }


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_AddElement
 *
 *  Description:   Add a child element to a multi-element container.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
// int NEUIK_Plot_AddElement(
// 	NEUIK_Element cont, 
// 	NEUIK_Element elem)
// {
// 	int                 len;
// 	int                 ctr;
// 	int                 newInd;            /* index for newly added item */
// 	int                 eNum       = 0;    /* which error to report (if any) */
// 	NEUIK_ElementBase * eBase      = NULL;
// 	NEUIK_Plot        * cBase      = NULL;
// 	static char         funcName[] = "NEUIK_Plot_AddElement";
// 	static char       * errMsgs[]  = {"",                                 // [0] no error
// 		"Argument `cont` does not implement Plot class.",                 // [1]
// 		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.",  // [2]
// 		"Argument `elem` does not implement Element class.",              // [3]
// 		"Argument `cont` is not a MultiElement Plot.",                    // [4]
// 		"Failure to allocate memory.",                                    // [5]
// 		"Failure to reallocate memory.",                                  // [6]
// 		"Argument `cont` does not allow the use of method AddElement().", // [7]
// 		"Failure in `neuik_Element_RequestRedraw()`.",                    // [8]
// 	};

// 	if (!neuik_Object_ImplementsClass(cont, neuik__Class_Plot))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 1;
// 		goto out;
// 	}
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 2;
// 		goto out;
// 	}
// 	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 3;
// 		goto out;
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* SetElement should only be used on single-element containers            */
// 	/*------------------------------------------------------------------------*/
// 	if (cBase->cType == NEUIK_CONTAINER_NO_DEFAULT_ADD_SET)
// 	{
// 		eNum = 7;
// 		goto out;
// 	}
// 	else if (cBase->cType != NEUIK_CONTAINER_MULTI)
// 	{
// 		eNum = 4;
// 		goto out;
// 	}

// 	if (cBase->elems == NULL)
// 	{
// 		/*--------------------------------------------------------------------*/
// 		/* elems array currently unallocated; allocate now                    */
// 		/*--------------------------------------------------------------------*/
// 		cBase->elems = (NEUIK_Element*)malloc(2*sizeof(NEUIK_Element));
// 		if (cBase->elems == NULL)
// 		{
// 			eNum = 5;
// 			goto out;
// 		}
// 		cBase->n_allocated = 1;
// 		cBase->n_used      = 1;
// 		newInd             = 0;
// 	}
// 	else if (cBase->n_allocated < cBase->n_used)
// 	{
// 		/*--------------------------------------------------------------------*/
// 		/* This is subsequent UI element, but there is space available in the */
// 		/* container elements arrary.                                         */
// 		/*--------------------------------------------------------------------*/
// 		newInd = cBase->n_used;
// 		cBase->n_used++;
// 	}
// 	else
// 	{
// 		/*--------------------------------------------------------------------*/
// 		/* This is subsequent UI element, reallocate memory.                  */
// 		/* This pointer array will be null terminated.                        */
// 		/*--------------------------------------------------------------------*/
		
// 		/* determine the current length */
// 		for (ctr = 0;;ctr++)
// 		{
// 			if (cBase->elems[ctr] == NULL)
// 			{
// 				len = 2 + ctr;
// 				break;
// 			}
// 		}

// 		cBase->elems = (NEUIK_Element*)realloc(cBase->elems, len*sizeof(NEUIK_Element));
// 		if (cBase->elems == NULL)
// 		{
// 			eNum = 6;
// 			goto out;
// 		}
// 		cBase->n_allocated++;
// 		cBase->n_used++;
// 		newInd = ctr;
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* Set the Window and Parent Element pointers                             */
// 	/*------------------------------------------------------------------------*/
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
// 	{
// 		eNum = 2;
// 		goto out;
// 	}
// 	if (eBase->eSt.window != NULL)
// 	{
// 		neuik_Element_SetWindowPointer(elem, eBase->eSt.window);
// 	}
// 	neuik_Element_SetParentPointer(elem, cont);

// 	cBase->elems[newInd]   = elem;
// 	cBase->elems[newInd+1] = NULL; /* NULLptr terminated array */

// 	/*------------------------------------------------------------------------*/
// 	/* When a new element is added to a container trigger a redraw            */
// 	/*------------------------------------------------------------------------*/
// 	if (neuik_Element_RequestRedraw(cont))
// 	{
// 		eNum = 8;
// 		goto out;
// 	}
// out:
// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 		eNum = 1;
// 	}
// out2:
// 	return eNum;
// }


/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_AddElements
 *
 *  Description:   Add multiple child element to a multi-element container.
 *
 *                 NOTE: the variable # of arguments must be terminated by a 
 *                 NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
// int NEUIK_Plot_AddElements(
// 	NEUIK_Element cont, 
// 	NEUIK_Element elem0, 
// 	...)
// {
// 	int                  ctr;
// 	int                  vaOpen = 0;
// 	int                  eNum   = 0; /* which error to report (if any) */
// 	va_list              args;
// 	NEUIK_Element        elem; 
// 	static char          funcName[] = "NEUIK_Plot_AddElements";
// 	static char        * errMsgs[] = {"",                                    // [0] no error
// 		"Argument `cont` does not implement Plot class.",                    // [1]
// 		"Failure in `Plot_AddElement()`.",                                   // [2]
// 		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?", // [3]
// 	};

// 	if (!neuik_Object_ImplementsClass(cont, neuik__Class_Plot))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 3;
// 			goto out;
// 		}
// 		eNum = 1;
// 		goto out;
// 	}

// 	va_start(args, elem0);
// 	vaOpen = 1;

// 	elem = elem0;
// 	for (ctr = 0;; ctr++)
// 	{
// 		if (elem == NULL) break;

// 		if (NEUIK_Plot_AddElement(cont, elem))
// 		{
// 			if (neuik_HasFatalError())
// 			{
// 				eNum = 3;
// 				goto out;
// 			}
// 			eNum = 2;
// 			goto out;
// 		}

// 		/* before starting */
// 		elem = va_arg(args, NEUIK_Element);
// 	}
// out:
// 	if (vaOpen) va_end(args);

// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 		eNum = 1;
// 	}

// 	return eNum;
// }


// /*******************************************************************************
//  *
//  *  Name:          neuik_Element_Defocus__Plot (redefined-vfunc)
//  *
//  *  Description:   Call Element defocus function.
//  *
//  *                 This operation is a virtual function redefinition.
//  *
//  ******************************************************************************/
// void neuik_Element_Defocus__Plot(
// 	NEUIK_Element cont)
// {
// 	int               ctr;
// 	NEUIK_Element     elem;
// 	NEUIK_Plot * cBase = NULL;

// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		return;
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* Defocus all contained elements                                         */
// 	/*------------------------------------------------------------------------*/
// 	if (cBase->elems == NULL) return;
// 	elem = cBase->elems[0];

// 	for (ctr = 1;;ctr++)
// 	{
// 		if (elem == NULL) break;
// 		neuik_Element_Defocus(elem);
// 		elem = cBase->elems[ctr];
// 	}
// }


// /*******************************************************************************
//  *
//  *  Name:          neuik_Element_IsShown__Plot    (redefined-vfunc)
//  *
//  *  Description:   This function reports whether or not an element is currently
//  *                 being shown.
//  *
//  *                 This operation is a virtual function redefinition.
//  *
//  *  Returns:       1 if element is shown, 0 otherwise.
//  *
//  ******************************************************************************/
// int neuik_Element_IsShown__Plot(
// 	NEUIK_Element  cont)
// {
// 	int                 isShown  = 0;
// 	int                 anyShown = 0;
// 	int                 ctr;
// 	NEUIK_Element       elem;
// 	NEUIK_ElementBase * eBase;
// 	NEUIK_Plot        * cBase = NULL;
// 	static int          nRecurse = 0; /* number of times recursively called */

// 	nRecurse++;
// 	if (nRecurse > NEUIK_MAX_RECURSION)
// 	{
// 		/*--------------------------------------------------------------------*/
// 		/* This is likely a case of appears to be runaway recursion; report   */
// 		/* an error to the user.                                              */
// 		/*--------------------------------------------------------------------*/
// 		neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
// 		goto out;
// 	}

// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		goto out;
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* First check if this element is being shown.                            */
// 	/*------------------------------------------------------------------------*/
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Element, (void**)&eBase))
// 	{
// 		goto out;
// 	}

// 	if (!eBase->eCfg.Show) goto out;

// 	/*------------------------------------------------------------------------*/
// 	/* Examine the contained elements to see if any of them are being shown.  */
// 	/*------------------------------------------------------------------------*/
// 	if (cBase->elems == NULL) goto out;
// 	elem = cBase->elems[0];

// 	for (ctr = 1;;ctr++)
// 	{
// 		if (elem == NULL) break;
// 		if (NEUIK_Element_IsShown(elem))
// 		{
// 			if (neuik_HasFatalError())
// 			{
// 				goto out;
// 			}
// 			anyShown = 1;
// 			break;
// 		}
// 		if (neuik_HasFatalError())
// 		{
// 			goto out;
// 		}
// 		elem = cBase->elems[ctr];
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* Even no child elements are shown; the container may still be shown.    */
// 	/*------------------------------------------------------------------------*/
// 	if (anyShown || cBase->shownIfEmpty)
// 	{
// 		isShown = 1;
// 	}
// out:
// 	nRecurse--;
// 	return isShown;
// }


// /*******************************************************************************
//  *
//  *  Name:          NEUIK_Plot_GetElementCount
//  *
//  *  Description:   Add multiple child element to a multi-element container.
//  *
//  *                 NOTE: the variable # of arguments must be terminated by a 
//  *                 NULL pointer.
//  *
//  *  Returns:       1 if there is an error; 0 otherwise.
//  *
//  ******************************************************************************/
// int NEUIK_Plot_GetElementCount(
// 	NEUIK_Element   cont,
// 	int           * elemCount) 
// {
// 	int             ctr;
// 	int             count = 0;
// 	int             eNum  = 0;  which error to report (if any) 
// 	NEUIK_Element   elem; 
// 	NEUIK_Plot    * cBase = NULL;
// 	static char     funcName[] = "NEUIK_Plot_GetElementCount";
// 	static char   * errMsgs[] = {"",                                // [0] no error
// 		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [1]
// 	};

// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		eNum = 1;
// 		goto out;
// 	}

// 	if (cBase->elems != NULL)
// 	{
// 		for (ctr = 0;; ctr++)
// 		{
// 			elem = cBase->elems[ctr];
// 			if (elem == NULL) break;

// 			count += 1;
// 		}
// 	}
// out:
// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 		eNum = 1;
// 	}

// 	(*elemCount) = count;
// 	return eNum;
// }

// /*******************************************************************************
//  *
//  *  Name:          NEUIK_Plot_RemoveElement
//  *
//  *  Description:   Remove an element from a container.
//  *
//  *  Returns:       1 if there is an error; 0 otherwise.
//  *
//  ******************************************************************************/
// int NEUIK_Plot_RemoveElement(
// 	NEUIK_Element cont, 
// 	NEUIK_Element elem)
// {
// 	int           ctr;
// 	int           wasLocated = 0;
// 	int           eNum       = 0;    /* which error to report (if any) */
// 	NEUIK_Plot  * cBase      = NULL;
// 	static char   funcName[] = "NEUIK_Plot_RemoveElement";
// 	static char * errMsgs[]  = {"",                                      // [0] no error
// 		"Argument `cont` does not implement Plot class.",                // [1]
// 		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [2]
// 		"Argument `elem` does not implement Element class.",             // [3]
// 		"Plot does not contain any child elements.",                     // [4]
// 		"Unable to locate specified `elem` within Plot.",                // [5]
// 		"Failure in `neuik_Element_RequestRedraw()`.",                   // [6]
// 	};

// 	if (!neuik_Object_ImplementsClass(cont, neuik__Class_Plot))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 1;
// 		goto out;
// 	}
// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 2;
// 		goto out;
// 	}
// 	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
// 	{
// 		if (neuik_HasFatalError())
// 		{
// 			eNum = 1;
// 			goto out2;
// 		}
// 		eNum = 3;
// 		goto out;
// 	}

// 	if (cBase->elems == NULL || cBase->n_used == 0)
// 	{
// 		eNum = 4;
// 		goto out;
// 	}

// 	/*------------------------------------------------------------------------*/
// 	/* Search through the elements in the container and look for the element  */
// 	/* to be removed.                                                         */
// 	/*------------------------------------------------------------------------*/
// 	for (ctr = 0;;ctr++)
// 	{
// 		if (cBase->elems[ctr] == NULL)
// 		{
// 			if (!wasLocated)
// 			{
// 				/*------------------------------------------------------------*/
// 				/* The container did not contain the desired element          */
// 				/*------------------------------------------------------------*/
// 				eNum = 5;
// 				goto out;
// 			}

// 			cBase->elems[ctr-1] = NULL;
// 			break;
// 		}

// 		if (wasLocated)
// 		{
// 			/*----------------------------------------------------------------*/
// 			/* Shuffle over the next value.                                   */
// 			/*----------------------------------------------------------------*/
// 			cBase->elems[ctr-1] = cBase->elems[ctr];
// 		} else if (cBase->elems[ctr] == elem)
// 		{
// 			/*----------------------------------------------------------------*/
// 			/* The element to be removed has been located.                    */
// 			/*----------------------------------------------------------------*/
// 			wasLocated = 1;
// 		}
// 	}

// 	cBase->n_used--;

// 	/*------------------------------------------------------------------------*/
// 	/* When an element is removed from a container; trigger a redraw          */
// 	/*------------------------------------------------------------------------*/
// 	if (neuik_Element_RequestRedraw(cont))
// 	{
// 		eNum = 6;
// 		goto out;
// 	}
// out:
// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 		eNum = 1;
// 	}
// out2:
// 	return eNum;
// }

// /*******************************************************************************
//  *
//  *  Name:          NEUIK_Plot_Configure
//  *
//  *  Description:   Configure one or more settings for a container.
//  *
//  *                 NOTE: This list of settings must be terminated by a NULL
//  *                 pointer.
//  *
//  *  Returns:       1 if there is an error; 0 otherwise.
//  *
//  ******************************************************************************/
// int NEUIK_Plot_Configure(
// 	NEUIK_Element   cont,
// 	const char    * set0,
// 	...)
// {
// 	int          ctr;
// 	int          isBool;
// 	int          boolVal    = 0;
// 	int          doRedraw   = 0;
// 	int          typeMixup;
// 	va_list      args;
// 	char       * strPtr     = NULL;
// 	char       * name       = NULL;
// 	char       * value      = NULL;
// 	const char * set        = NULL;
// 	char         buf[4096];
// 	NEUIK_Plot * cBase      = NULL;
// 	/*------------------------------------------------------------------------*/
// 	/* If a `name=value` string with an unsupported name is found, check to   */
// 	/* see if a boolName was mistakenly used instead.                         */
// 	/*------------------------------------------------------------------------*/
// 	// static char     * boolNames[] = {
// 	// 	NULL,
// 	// };
// 	/*------------------------------------------------------------------------*/
// 	/* If a boolName string with an unsupported name is found, check to see   */
// 	/* if a supported nameValue type was mistakenly used instead.             */
// 	/*------------------------------------------------------------------------*/
// 	// static char     * valueNames[] = {
// 	// 	"HJustify",
// 	// 	"VJustify",
// 	// 	NULL,
// 	// };
// 	static char       funcName[] = "NEUIK_Plot_Configure";
// 	static char     * errMsgs[]  = {"",                                  // [ 0] no error
// 		"Argument `cont` caused `neuik_Object_GetClassObject` to fail.", // [ 1]
// 		"NamedSet.name is NULL, skipping.",                              // [ 2]
// 		"NamedSet.name is blank, skipping.",                             // [ 3]
// 		"NamedSet.name type unknown, skipping.",                         // [ 4]
// 		"`name=value` string is too long.",                              // [ 5]
// 		"Set string is empty.",                                          // [ 6]
// 		"HJustify value is invalid.",                                    // [ 7]
// 		"VJustify value is invalid.",                                    // [ 8]
// 		"BoolType name unknown, skipping.",                              // [ 9]
// 		"Invalid `name=value` string.",                                  // [10]
// 		"ValueType name used as BoolType, skipping.",                    // [11]
// 		"BoolType name used as ValueType, skipping.",                    // [12]
// 	};

// 	if (neuik_Object_GetClassObject(cont, neuik__Class_Plot, (void**)&cBase))
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[1]);
// 		return 1;
// 	}

// 	set = set0;

// 	va_start(args, set0);

// 	for (ctr = 0;; ctr++)
// 	{
// 		isBool = 0;
// 		name   = NULL;
// 		value  = NULL;

// 		if (set == NULL) break;

// 		// #ifndef NO_NEUIK_SIGNAL_TRAPPING
// 		// 	signal(SIGSEGV, neuik_Element_Configure_capture_segv);
// 		// #endif

// 		if (strlen(set) > 4095)
// 		{
// 			// #ifndef NO_NEUIK_SIGNAL_TRAPPING
// 			// 	signal(SIGSEGV, NULL);
// 			// #endif
// 			NEUIK_RaiseError(funcName, errMsgs[5]);
// 			set = va_arg(args, const char *);
// 			continue;
// 		}
// 		else
// 		{
// 			// #ifndef NO_NEUIK_SIGNAL_TRAPPING
// 			// 	signal(SIGSEGV, NULL);
// 			// #endif
// 			strcpy(buf, set);
// 			/* Find the equals and set it to '\0' */
// 			strPtr = strchr(buf, '=');
// 			if (strPtr == NULL)
// 			{
// 				/*------------------------------------------------------------*/
// 				/* Bool type configuration (or a mistake)                     */
// 				/*------------------------------------------------------------*/
// 				if (buf[0] == 0)
// 				{
// 					NEUIK_RaiseError(funcName, errMsgs[6]);
// 				}

// 				isBool  = 1;
// 				boolVal = 1;
// 				name    = buf;
// 				if (buf[0] == '!')
// 				{
// 					boolVal = 0;
// 					name    = buf + 1;
// 				}
// 			}
// 			else
// 			{
// 				*strPtr = 0;
// 				strPtr++;
// 				if (*strPtr == 0)
// 				{
// 					/* `name=value` string is missing a value */
// 					NEUIK_RaiseError(funcName, errMsgs[10]);
// 					set = va_arg(args, const char *);
// 					continue;
// 				}
// 				name  = buf;
// 				value = strPtr;
// 			}
// 		}

// 		if (!isBool)
// 		{
// 			if (name == NULL)
// 			{
// 				NEUIK_RaiseError(funcName, errMsgs[2]);
// 			}
// 			else if (name[0] == 0)
// 			{
// 				NEUIK_RaiseError(funcName, errMsgs[3]);
// 			}
// 			else if (!strcmp("HJustify", name))
// 			{
// 				if (!strcmp("left", value))
// 				{
// 					cBase->HJustify = NEUIK_HJUSTIFY_LEFT;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("center", value))
// 				{
// 					cBase->HJustify = NEUIK_HJUSTIFY_CENTER;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("right", value))
// 				{
// 					cBase->HJustify = NEUIK_HJUSTIFY_RIGHT;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("default", value))
// 				{
// 					cBase->HJustify = NEUIK_HJUSTIFY_DEFAULT;
// 					doRedraw = 1;
// 				}
// 				else
// 				{
// 					NEUIK_RaiseError(funcName, errMsgs[7]);
// 				}
// 			}
// 			else if (!strcmp("VJustify", name))
// 			{
// 				if (!strcmp("top", value))
// 				{
// 					cBase->VJustify = NEUIK_VJUSTIFY_TOP;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("center", value))
// 				{
// 					cBase->VJustify = NEUIK_VJUSTIFY_CENTER;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("bottom", value))
// 				{
// 					cBase->VJustify = NEUIK_VJUSTIFY_BOTTOM;
// 					doRedraw = 1;
// 				}
// 				else if (!strcmp("default", value))
// 				{
// 					cBase->VJustify = NEUIK_VJUSTIFY_DEFAULT;
// 					doRedraw = 1;
// 				}
// 				else
// 				{
// 					NEUIK_RaiseError(funcName, errMsgs[8]);
// 				}
// 			}
// 			else
// 			{
// 				typeMixup = 0;
// 				// for (nCtr = 0;; nCtr++)
// 				// {
// 				// 	if (boolNames[nCtr] == NULL) break;

// 				// 	if (!strcmp(boolNames[nCtr], name))
// 				// 	{
// 				// 		typeMixup = 1;
// 				// 		break;
// 				// 	}
// 				// }

// 				if (typeMixup)
// 				{
// 					/* A bool type was mistakenly used as a value type */
// 					NEUIK_RaiseError(funcName, errMsgs[12]);
// 				}
// 				else
// 				{
// 					/* An unsupported name was used as a value type */
// 					NEUIK_RaiseError(funcName, errMsgs[4]);
// 				}
// 			}
// 		}

// 		/* before starting */
// 		set = va_arg(args, const char *);
// 	}
// 	va_end(args);

// 	if (doRedraw) neuik_Element_RequestRedraw(cont);

// 	return 0;
// }

/*******************************************************************************
 *
 *  Name:          NEUIK_Plot_SetTitle
 *
 *  Description:   Update the title of a NEUIK_Plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot_SetTitle(
	NEUIK_Element   plotPtr,
	const char    * text)
{
	int           eNum = 0; /* which error to report (if any) */
	char *        textCopy = NULL; /* should be freed when done */
	char *        strPtr0  = NULL;
	char *        strPtr1  = NULL;
	NEUIK_Label * newLabel = NULL;
	NEUIK_Plot  * plot     = NULL;
	static char   funcName[] = "NEUIK_Plot_SetTitle";
	static char * errMsgs[] = {"",                                         // [0] no error
		"Argument `plot` does not implement Plot class.",                  // [1]
		"Argument `plot` caused `neuik_Object_GetClassObject()` to fail.", // [2]
		"Failure to allocate memory.",                                     // [3]
		"Failure in `NEUIK_MakeLabel()`.",                                 // [4]
		"Failure to `String_Duplicate()`.",                                // [5]
		"Failure to `NEUIK_Container_AddElement()`.",                      // [6]
	};

	if (!neuik_Object_ImplementsClass(plotPtr, neuik__Class_Plot))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(plotPtr, neuik__Class_Plot, (void**)&plot))
	{
		if (neuik_HasFatalError())
		{
			eNum = 1;
			goto out2;
		}
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Conditionally free Plot title before setting the new contents          */
	/*------------------------------------------------------------------------*/
	if (plot->title != NULL) {
		NEUIK_Container_RemoveElements(plot->title);
	}

	/*------------------------------------------------------------------------*/
	/* Set the new Label text contents                                        */
	/*------------------------------------------------------------------------*/
	if (text == NULL){
		/* Title will contain no text */
		goto out;
	}
	else if (text[0] == '\0')
	{
		/* Title will contain no text */
		goto out;
	}

	String_Duplicate(&textCopy, text);

	if (textCopy == NULL)
	{
		eNum = 5;
		goto out;
	}

	strPtr1 = textCopy;
	for (;;)
	{
		strPtr0 = strchr(strPtr1, '\n');
		if (strPtr0 == NULL)
		{
			/*----------------------------------------------------------------*/
			/* There are no more newlines in the string                       */
			/*----------------------------------------------------------------*/
			if (NEUIK_MakeLabel(&newLabel, strPtr0))
			{
				eNum = 4;
				goto out;
			}
			if (NEUIK_Container_AddElement(plot->title, newLabel))
			{
				eNum = 6;
				goto out;
			}
			break;
		} 
		else
		{
			*strPtr0 = '\0';
			if (NEUIK_MakeLabel(&newLabel, strPtr1))
			{
				eNum = 4;
				goto out;
			}
			if (NEUIK_Container_AddElement(plot->title, newLabel))
			{
				eNum = 6;
				goto out;
			}
			strPtr0++;
			strPtr1 = strPtr0;
		}
	}

	neuik_Element_RequestRedraw((NEUIK_Element)plot);
out:
	if (textCopy != NULL) free(textCopy);
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
out2:
	return eNum;
}