/*******************************************************************************
 * Copyright (c) 2014-2019, Michael Leimon <leimon@gmail.com>
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
#include <stdarg.h>
#include <stdlib.h>
 
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Stack.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "NEUIK_Container_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Stack(void ** stkPtr);
int neuik_Object_Free__Stack(void * stkPtr);

int neuik_Element_CaptureEvent__Stack(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_GetMinSize__Stack(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Stack(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Stack_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Stack,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Stack,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Stack_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Stack,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Stack,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Stack
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Stack()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Stack";
	static char  * errMsgs[]  = {"",                 // [0] no error
		"NEUIK library must be initialized first.",  // [1]
		"Failed to register `Stack` object class .", // [2]
		"Failed to register `Virtual Function`.",    // [3]
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
		"Stack",                                                     // className
		"A multi-element container which shows only one at a time.", // classDescription
		neuik__Set_NEUIK,                                            // classSet
		neuik__Class_Container,                                      // superClass
		&neuik_Stack_BaseFuncs,                                      // baseFuncs
		NULL,                                                        // classFuncs
		&neuik__Class_Stack))                                        // newClass
	{
		eNum = 2;
		goto out;
	}

	if (neuik_VirtualFunc_RegisterImplementation(
		&neuik_Element_vfunc_CaptureEvent,
		neuik__Class_Stack,
		neuik_Element_CaptureEvent__Stack))
	{
		eNum = 3;
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
 *  Name:          neuik_Object_New__Stack
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Stack(
	void ** objPtr)
{
	int               eNum       = 0;
	NEUIK_Container * cont       = NULL;
	NEUIK_Stack     * stk        = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__Stack";
	static char     * errMsgs[]  = {"",                                  // [0] no error
		"Output Argument `fPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                   // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                      // [3]
		"Failure in function `neuik.NewElement`.",                       // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",             // [5]
		"Argument `fPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",     // [7]
	};

	if (objPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*objPtr) = (NEUIK_Stack*) malloc(sizeof(NEUIK_Stack));
	stk = *objPtr;
	if (stk == NULL)
	{
		eNum = 2;
		goto out;
	}
	stk->elemActive = NULL;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Stack, 
			NULL,
			&(stk->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(stk->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(stk, &neuik_Stack_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(stk, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_MULTI;
	cont->shownIfEmpty = 1;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(stk, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(stk, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(stk, "hovered"))
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
 *  Name:          neuik_Object_Free__Stack
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Stack(
	void * stkPtr)
{
	int           eNum       = 0;    /* which error to report (if any) */
	NEUIK_Stack * stk        = NULL;
	static char   funcName[] = "neuik_Object_Free__Stack";
	static char * errMsgs[]  = {"",                 // [0] no error
		"Argument `fPtr` is NULL.",                 // [1]
		"Argument `fPtr` is not of Frame class.",   // [2]
		"Failure in function `neuik_Object_Free`.", // [3]
	};

	if (stkPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	stk = (NEUIK_Stack*)stkPtr;

	if (!neuik_Object_IsClass(stk, neuik__Class_Stack))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(stk->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(stk);
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
 *  Name:          NEUIK_NewStack
 *
 *  Description:   Create and return a pointer to a new NEUIK_Stack.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
int NEUIK_NewStack(
	NEUIK_Stack ** fPtr)
{
	return neuik_Object_New__Stack((void**)fPtr);
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__Stack
 *
 *  Description:   Returns the rendered size of a given Stack. The minimum 
 *                 required size for a stack is the largest mimimum width 
 *                 required by any one contained element (shown/active or not)
 *                 and the largest minimum height required by any one contained
 *                 element (shown/active or not).
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Stack(
	NEUIK_Element    stkElem, 
	RenderSize     * rSize)
{
	int                   tempW;
	int                   tempH;
	int                   eNum      = 0;   /* which error to report (if any) */
	int                   ctr       = 0;
	RenderSize            rs        = {0, 0};
	NEUIK_Container     * cont      = NULL;
	NEUIK_Element         elem      = NULL;
	NEUIK_ElementConfig * eCfg      = NULL;
	static char          funcName[] = "neuik_Element_GetMinSize__Stack";
	static char         * errMsgs[] = {"",                                  // [0] no error
		"Argument `stkElem` is not of Stack class.",                        // [1]
		"Argument `stkElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Element_GetConfig returned NULL.",                                 // [3]
		"Failure in neuik_Element_GetSize.",                                // [4]
	};

	(*rSize) = rs;

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceding                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(stkElem, neuik__Class_Stack))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(stkElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (cont->elems == NULL) goto out;
	/* ^^^ there are no UI elements contained by this Stack */

	/*------------------------------------------------------------------------*/
	/* Determine the (maximum) width & height required by any of the elements */
	/*------------------------------------------------------------------------*/
	for (ctr = 0;; ctr++)
	{
		elem = cont->elems[ctr];
		if (elem == NULL) break;

		eCfg = neuik_Element_GetConfig(elem);
		if (eCfg == NULL)
		{
			eNum = 3;
			goto out;
		}

		if (neuik_Element_GetMinSize(elem, &rs) != 0)
		{
			eNum = 4;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Get the (maximum) width required by any one of the elements        */
		/*--------------------------------------------------------------------*/
		tempW = rs.w + (eCfg->PadLeft + eCfg->PadRight);
		if (tempW > rSize->w)
		{
			rSize->w = tempW;
		}

		/*--------------------------------------------------------------------*/
		/* Get the (maximum) height required by any one of the elements       */
		/*--------------------------------------------------------------------*/
		tempH = rs.h + (eCfg->PadTop + eCfg->PadBottom);
		if (tempH > rSize->h)
		{
			rSize->h = tempH;
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
 *  Name:          neuik_Element_Render__Stack
 *
 *  Description:   Renders a NEUIK_Stack as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Stack(
	NEUIK_Element   stkElem, 
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf, /* the external surface (used for transp. bg) */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                      ctr      = 0;
	int                      eNum     = 0; /* which error to report (if any) */
	int                      elemIncl;
	RenderLoc                rl;
	RenderLoc                rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Rect                 rect;
	RenderSize               rs;
	SDL_Renderer           * rend       = NULL;
	NEUIK_Stack            * stk        = NULL;
	NEUIK_Container        * cont       = NULL;
	NEUIK_Element            elem       = NULL;
	NEUIK_ElementBase      * eBase      = NULL;
	NEUIK_ElementConfig    * eCfg       = NULL;
	static char              funcName[] = "neuik_Element_Render__Stack";
	static char            * errMsgs[]  = {"",                              // [0] no error
		"Argument `stkElem` is not of Stack class.",                        // [1]
		"Argument `stkElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Call to Element_GetMinSize failed.",                               // [3]
		"Invalid specified `rSize` (negative values).",                     // [4]
		"Failure in `neuik_Element_Render().",                              // [5]
		"Active element not contained by this stack.",                      // [6]
		"Element_GetConfig returned NULL.",                                 // [7]
		"Failure in neuik_Element_RedrawBackground().",                     // [8]
	};

	if (!neuik_Object_IsClass(stkElem, neuik__Class_Stack))
	{
		eNum = 1;
		goto out;
	}
	stk = (NEUIK_Stack *)stkElem;

	if (neuik_Object_GetClassObject(stkElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Object_GetClassObject(stkElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
		goto out;
	}

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (!mock)
	{
		if (neuik_Element_RedrawBackground(stkElem, xSurf, rlMod, NULL))
		{
			eNum = 8;
			goto out;
		}
	}
	rl = eBase->eSt.rLoc;

	/*------------------------------------------------------------------------*/
	/* Draw the currently shown UI element onto the Stack                     */
	/*------------------------------------------------------------------------*/
	if (cont->elems == NULL) goto out; /* stack contains no elements */
	if (stk->elemActive == NULL) goto out; /* no active stack element */

	/*------------------------------------------------------------------------*/
	/* Verify that the current active stack element is within the stack.      */
	/*------------------------------------------------------------------------*/
	elemIncl = 0;
	for (ctr = 0;; ctr++)
	{
		elem = cont->elems[ctr];
		if (elem == NULL) break;

		if (elem == stk->elemActive)
		{
			elemIncl = 1;
			break;
		}
	}

	if (elemIncl == 0) 
	{
		/* Error: the currently active element is not contained by this stack */
		eNum = 6;
		goto out;
	}

	elem = stk->elemActive;
	if (!NEUIK_Element_IsShown(elem)) goto out; /* active elem not shown */


	/*------------------------------------------------------------------------*/
	/* Render and place the currently active stack element                    */
	/*------------------------------------------------------------------------*/
	eCfg = neuik_Element_GetConfig(elem);
	if (eCfg == NULL)
	{
		eNum = 7;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start with the default calculated element size                         */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_GetMinSize(elem, &rs))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check for and apply if necessary Horizontal and Veritcal fill          */
	/*------------------------------------------------------------------------*/
	if (eCfg->HFill)
	{
		/* This element is configured to fill space horizontally */
		rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
	}
	if (eCfg->VFill)
	{
		/* This element is configured to fill space vertically */
		rs.h = rSize->h - (eCfg->PadTop + eCfg->PadBottom);
	}



	/*------------------------------------------------------------------------*/
	/* Update the stored location before rendering the element. This is       */
	/* necessary as the location of this object will propagate to its         */
	/* child objects.                                                         */
	/*------------------------------------------------------------------------*/
	rect.x = rl.x;
	rect.y = rl.y;
	rect.w = rs.w;
	rect.h = rs.h;

	switch (eCfg->HJustify)
	{
		case NEUIK_HJUSTIFY_DEFAULT:
			switch (cont->HJustify)
			{
				case NEUIK_HJUSTIFY_LEFT:
					rect.x += eCfg->PadLeft;
					break;
				case NEUIK_HJUSTIFY_CENTER:
				case NEUIK_HJUSTIFY_DEFAULT:
					rect.x += rSize->w/2 - (rs.w/2);
					break;
				case NEUIK_HJUSTIFY_RIGHT:
					rect.x += rSize->w - (rs.w + eCfg->PadRight);
					break;
			}
			break;
		case NEUIK_HJUSTIFY_LEFT:
			rect.x += eCfg->PadLeft;
			break;
		case NEUIK_HJUSTIFY_CENTER:
			rect.x += rSize->w/2 - (rs.w/2);
			break;
		case NEUIK_HJUSTIFY_RIGHT:
			rect.x += rSize->w - (rs.w + eCfg->PadRight);
			break;
	}

	switch (eCfg->VJustify)
	{
		case NEUIK_VJUSTIFY_DEFAULT:
			switch (cont->VJustify)
			{
				case NEUIK_VJUSTIFY_TOP:
					rect.y += eCfg->PadTop;
					break;
				case NEUIK_VJUSTIFY_CENTER:
				case NEUIK_VJUSTIFY_DEFAULT:
					rect.y += (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 -
						(rs.h/2);
					break;
				case NEUIK_VJUSTIFY_BOTTOM:
					rect.y += rSize->h - (rs.h + eCfg->PadBottom);
					break;
			}
			break;
		case NEUIK_VJUSTIFY_TOP:
			rect.y += eCfg->PadTop;
			break;
		case NEUIK_VJUSTIFY_CENTER:
			rect.y += (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 - (rs.h/2);
			break;
		case NEUIK_VJUSTIFY_BOTTOM:
			rect.y += rSize->h - (rs.h + eCfg->PadBottom);
			break;
	}

	rl.x = rect.x;
	rl.y = rect.y;
	rlRel.x = rect.x;
	rlRel.y = rect.y;
	neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

	if (neuik_Element_Render(elem, &rs, rlMod, rend, xSurf, mock))
	{
		eNum = 5;
		goto out;
	}
out:
	if (!mock) eBase->eSt.doRedraw = 0;

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__Stack
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_CaptureEvent__Stack(
	NEUIK_Element   stkElem, 
	SDL_Event     * ev)
{
	int             evCaputred = 0;
	NEUIK_Stack   * stk        = NULL;
	NEUIK_Element   elem       = NULL;

	if (neuik_Object_GetClassObject_NoError(
		stkElem, neuik__Class_Stack, (void**)&stk)) goto out;

	elem = stk->elemActive;
	if (elem == NULL) goto out;
	if (!NEUIK_Element_IsShown(elem)) goto out;

	evCaputred = neuik_Element_CaptureEvent(elem, ev);
	if (evCaputred) neuik_Element_SetActive(stkElem, 1);
out:
	return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Stack_SetActiveElement
 *
 *  Description:   Set the active element for this Stack. This element must have
 *                 been previously added to this stack (using `AddElements()`),
 *                 before being able to set it as active.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
int NEUIK_Stack_SetActiveElement(
	NEUIK_Stack   * stk,
	NEUIK_Element   elem)
{
	int               elemIncl;
	int               ctr        = 0;
	int               eNum       = 0; /* which error to report (if any) */
	NEUIK_Container * cont       = NULL;
	NEUIK_Element   * elemPtr;
	static char       funcName[] = "NEUIK_Stack_SetActiveElement";
	static char     * errMsgs[]  = {"",                                 // [0] no error
		"Argument `stk` is not of Stack class.",                        // [1]
		"Argument `elem` does not implement Element class.",            // [2]
		"Active element not within this stack.",                        // [3]
		"Argument `stk` caused `neuik_Object_GetClassObject` to fail.", // [4]
		"Failure in `neuik_Container_RequestFullRedraw()`.",            // [5]
	};

	if (!neuik_Object_IsClass(stk, neuik__Class_Stack))
	{
		eNum = 1;
		goto out;
	}
	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
	{
		eNum = 2;
		goto out;
	}
	if (neuik_Object_GetClassObject(stk, neuik__Class_Container, (void**)&cont))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Verify that the current active stack element is within the stack.      */
	/*------------------------------------------------------------------------*/
	elemIncl = 0;
	for (ctr = 0;; ctr++)
	{
		elemPtr = cont->elems[ctr];
		if (elemPtr == NULL) break;

		if (elem == elemPtr)
		{
			elemIncl = 1;
			break;
		}
	}

	if (!elemIncl) 
	{
		/* Error: the currently active element is not contained by this stack */
		eNum = 3;
		goto out;
	}

	stk->elemActive = elem;
	if (neuik_Container_RequestFullRedraw(stk))
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

