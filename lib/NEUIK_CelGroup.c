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
#include "NEUIK_CelGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__CelGroup(void ** cgPtr);
int neuik_Object_Free__CelGroup(void * cgPtr);

int neuik_Element_CaptureEvent__CelGroup(NEUIK_Element cont, SDL_Event * ev);
int neuik_Element_GetMinSize__CelGroup(NEUIK_Element, RenderSize*);
int neuik_Element_Render__CelGroup(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_CelGroup_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__CelGroup,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__CelGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_CelGroup_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__CelGroup,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__CelGroup,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_CelGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_CelGroup()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_CelGroup";
	static char  * errMsgs[]  = {"",                    // [0] no error
		"NEUIK library must be initialized first.",     // [1]
		"Failed to register `CelGroup` object class .", // [2]
		"Failed to register `Virtual Function`.",       // [3]
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
		"CelGroup",                                   // className
		"A multi-element container which shows "
		"all elements layered on top of each other.", // classDescription
		neuik__Set_NEUIK,                             // classSet
		neuik__Class_Container,                       // superClass
		&neuik_CelGroup_BaseFuncs,                    // baseFuncs
		NULL,                                         // classFuncs
		&neuik__Class_CelGroup))                      // newClass
	{
		eNum = 2;
		goto out;
	}

	if (neuik_VirtualFunc_RegisterImplementation(
		&neuik_Element_vfunc_CaptureEvent,
		neuik__Class_CelGroup,
		neuik_Element_CaptureEvent__CelGroup))
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
 *  Name:          neuik_Object_New__CelGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__CelGroup(
	void ** objPtr)
{
	int               eNum       = 0;
	NEUIK_Container * cont       = NULL;
	NEUIK_CelGroup  * cg         = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__CelGroup";
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
	(*objPtr) = (NEUIK_CelGroup*) malloc(sizeof(NEUIK_CelGroup));
	cg = *objPtr;
	if (cg == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_CelGroup, 
			NULL,
			&(cg->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(cg->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(cg, &neuik_CelGroup_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(cg, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_MULTI;
	cont->shownIfEmpty = 1;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(cg, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cg, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cg, "hovered"))
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
 *  Name:          neuik_Object_Free__CelGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__CelGroup(
	void * cgPtr)
{
	int              eNum       = 0;    /* which error to report (if any) */
	NEUIK_CelGroup * cg         = NULL;
	static char      funcName[] = "neuik_Object_Free__CelGroup";
	static char    * errMsgs[]  = {"",                 // [0] no error
		"Argument `fPtr` is NULL.",                 // [1]
		"Argument `fPtr` is not of Frame class.",   // [2]
		"Failure in function `neuik_Object_Free`.", // [3]
	};

	if (cgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	cg = (NEUIK_CelGroup*)cgPtr;

	if (!neuik_Object_IsClass(cg, neuik__Class_CelGroup))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(cg->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(cg);
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
 *  Name:          NEUIK_NewCelGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_CelGroup.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
int NEUIK_NewCelGroup(
	NEUIK_CelGroup ** fPtr)
{
	return neuik_Object_New__CelGroup((void**)fPtr);
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__CelGroup
 *
 *  Description:   Returns the rendered size of a given CelGroup. The minimum 
 *                 required size for a CelGroup is the largest mimimum width 
 *                 required by any one contained element and the largest minimum
 *                 height required by any one contained element.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__CelGroup(
	NEUIK_Element   cgElem, 
	RenderSize    * rSize)
{
	int                   tempW;
	int                   tempH;
	int                   eNum      = 0;   /* which error to report (if any) */
	int                   ctr       = 0;
	RenderSize            rs        = {0, 0};
	NEUIK_Container     * cont      = NULL;
	NEUIK_Element         elem      = NULL;
	NEUIK_ElementConfig * eCfg      = NULL;
	static char          funcName[] = "neuik_Element_GetMinSize__CelGroup";
	static char         * errMsgs[] = {"",                                 // [0] no error
		"Argument `cgElem` is not of CelGroup class.",                     // [1]
		"Argument `cgElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Failure in neuik_Element_GetSize.",                               // [4]
	};

	(*rSize) = rs;

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceding                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(cgElem, neuik__Class_CelGroup))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(cgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (cont->elems == NULL) goto out;
	/* ^^^ there are no UI elements contained by this CelGroup */

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
 *  Name:          neuik_Element_Render__CelGroup
 *
 *  Description:   Renders a NEUIK_CelGroup as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__CelGroup(
	NEUIK_Element   cgElem, 
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* The external renderer to prepare the texture for */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                   ctr        = 0;
	int                   eNum       = 0; /* which error to report (if any) */
	RenderLoc             rl;
	RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Rect              rect;
	RenderSize            rs;
	SDL_Renderer        * rend       = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	static char           funcName[] = "neuik_Element_Render__CelGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `cgElem` is not of CelGroup class.",                     // [1]
		"Argument `cgElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Call to Element_GetMinSize failed.",                              // [3]
		"Invalid specified `rSize` (negative values).",                    // [4]
		"Failure in neuik_Element_RedrawBackground().",                    // [5]
		"Element_GetConfig returned NULL.",                                // [6]
	};

	if (!neuik_Object_IsClass(cgElem, neuik__Class_CelGroup))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(cgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Object_GetClassObject(cgElem, neuik__Class_Container, (void**)&cont))
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
		if (neuik_Element_RedrawBackground(cgElem, rlMod, NULL))
		{
			eNum = 5;
			goto out;
		}
	}
	rl = eBase->eSt.rLoc;

	/*------------------------------------------------------------------------*/
	/* Draw the currently shown UI element onto the CelGroup                  */
	/*------------------------------------------------------------------------*/
	if (cont->elems == NULL)
	{
		goto out; /* CelGroup contains no elements */
	}

	for (ctr = 0;; ctr++)
	{
		elem = (NEUIK_Element)cont->elems[ctr];
		if (elem == NULL) break;

		/*--------------------------------------------------------------------*/
		/* Render and place the currently active stack element                */
		/*--------------------------------------------------------------------*/
		eCfg = neuik_Element_GetConfig(elem);
		if (eCfg == NULL)
		{
			eNum = 6;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Start with the default calculated element size                     */
		/*--------------------------------------------------------------------*/
		if (neuik_Element_GetMinSize(elem, &rs))
		{
			eNum = 3;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Check for and apply if necessary Horizontal and Veritcal fill      */
		/*--------------------------------------------------------------------*/
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

		/*--------------------------------------------------------------------*/
		/* Update the stored location before rendering the element. This is   */
		/* necessary as the location of this object will propagate to its     */
		/* child objects.                                                     */
		/*--------------------------------------------------------------------*/
		switch (eCfg->HJustify)
		{
			case NEUIK_HJUSTIFY_DEFAULT:
				switch (cont->HJustify)
				{
					case NEUIK_HJUSTIFY_LEFT:
						rect.x = eCfg->PadLeft;
						break;
					case NEUIK_HJUSTIFY_CENTER:
					case NEUIK_HJUSTIFY_DEFAULT:
						rect.x = rSize->w/2 - (rs.w/2);
						break;
					case NEUIK_HJUSTIFY_RIGHT:
						rect.x = rSize->w - (rs.w + eCfg->PadRight);
						break;
				}
				break;
			case NEUIK_HJUSTIFY_LEFT:
				rect.x = eCfg->PadLeft;
				break;
			case NEUIK_HJUSTIFY_CENTER:
				rect.x = rSize->w/2 - (rs.w/2);
				break;
			case NEUIK_HJUSTIFY_RIGHT:
				rect.x = rSize->w - (rs.w + eCfg->PadRight);
				break;
		}

		switch (eCfg->VJustify)
		{
			case NEUIK_VJUSTIFY_DEFAULT:
				switch (cont->VJustify)
				{
					case NEUIK_VJUSTIFY_TOP:
						rect.y = eCfg->PadTop;
						break;
					case NEUIK_VJUSTIFY_CENTER:
					case NEUIK_VJUSTIFY_DEFAULT:
						rect.y = (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 -
							(rs.h/2);
						break;
					case NEUIK_VJUSTIFY_BOTTOM:
						rect.y = rSize->h - (rs.h + eCfg->PadBottom);
						break;
				}
				break;
			case NEUIK_VJUSTIFY_TOP:
				rect.y = eCfg->PadTop;
				break;
			case NEUIK_VJUSTIFY_CENTER:
				rect.y = (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 - (rs.h/2);
				break;
			case NEUIK_VJUSTIFY_BOTTOM:
				rect.y = rSize->h - (rs.h + eCfg->PadBottom);
				break;
		}

		rect.w = rs.w;
		rect.h = rs.h;
		rl.x = (eBase->eSt.rLoc).x + rect.x;
		rl.y = (eBase->eSt.rLoc).y + rect.y;
		rlRel.x = rect.x;
		rlRel.y = rect.y;
		neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

		if (neuik_Element_Render(elem, &rs, rlMod, rend, mock))
		{
			eNum = 5;
			goto out;
		}
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
 *  Name:          neuik_Element_CaptureEvent__CelGroup
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *                 Since elements within CelGroup are placed one layer on top of
 *                 the other, the final elements will appear on top and as such
 *                 evalualtion of event capturing should happen in reverse 
 *                 element order.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_CaptureEvent__CelGroup(
	NEUIK_Element   cgElem, 
	SDL_Event     * ev)
{
	int                ctr;
	neuik_EventState   evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
	NEUIK_Element      elem;
	NEUIK_Container  * cBase;

	if (neuik_Object_GetClassObject_NoError(
		cgElem, neuik__Class_Container, (void**)&cBase)) goto out;

	if (NEUIK_Container_GetElementCount(cgElem, &ctr)) goto out;
	ctr--;

	if (cBase->elems != NULL)
	{
		for (;ctr >= 0; ctr--)
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
				neuik_Element_SetActive(cgElem, 1);
				goto out;
			}
		}
	}
out:
	return evCaputred;
}