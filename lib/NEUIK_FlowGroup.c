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
#include <stdlib.h>
 
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_FlowGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__FlowGroup(void ** fgPtr);
int neuik_Object_Free__FlowGroup(void * fgPtr);

int neuik_Element_GetMinSize__FlowGroup(NEUIK_Element, RenderSize*);
int neuik_Element_Render__FlowGroup(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_FlowGroup_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__FlowGroup,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__FlowGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_FlowGroup_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__FlowGroup,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__FlowGroup,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_FlowGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_FlowGroup()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_FlowGroup";
	static char  * errMsgs[]  = {"",                     // [0] no error
		"NEUIK library must be initialized first.",      // [1]
		"Failed to register `FlowGroup` object class .", // [2]
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
		"NEUIK_FlowGroup",                                       // className
		"An element container which horizontally groups items.", // classDescription
		neuik__Set_NEUIK,                                        // classSet
		neuik__Class_Container,                                  // superClass
		&neuik_FlowGroup_BaseFuncs,                              // baseFuncs
		NULL,                                                    // classFuncs
		&neuik__Class_FlowGroup))                                // newClass
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
 *  Name:          neuik_Object_New__FlowGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__FlowGroup(
		void ** fgPtr)
{
	int               eNum       = 0;
	NEUIK_Container * cont       = NULL;
	NEUIK_FlowGroup * fg         = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__FlowGroup";
	static char     * errMsgs[]  = {"",                                   // [0] no error
		"Output Argument `fgPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                    // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
		"Failure in function `neuik.NewElement`.",                        // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",              // [5]
		"Argument `fgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",      // [7]
	};

	if (fgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*fgPtr) = (NEUIK_FlowGroup*) malloc(sizeof(NEUIK_FlowGroup));
	fg = *fgPtr;
	if (fg == NULL)
	{
		eNum = 2;
		goto out;
	}
	fg->HSpacing   = 1;
	fg->VSpacing   = 1;
	fg->FillFirst  = NEUIK_FLOWGROUP_FILLDIRN_LEFT_TO_RIGHT;
	fg->FillSecond = NEUIK_FLOWGROUP_FILLDIRN_TOP_TO_BOTTOM;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_FlowGroup, 
			NULL,
			&(fg->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(fg->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(fg, &neuik_FlowGroup_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(fg, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_MULTI;
	cont->shownIfEmpty = 0;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "hovered"))
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
 *  Name:          NEUIK_NewFlowGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_FlowGroup.
 *
 *                 Wrapper function to neuik_Object_New__FlowGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewFlowGroup(
	NEUIK_FlowGroup ** fgPtr)
{
	return neuik_Object_New__FlowGroup((void**)fgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Free__FlowGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__FlowGroup(
	void * fgPtr)
{
	int               eNum       = 0;    /* which error to report (if any) */
	NEUIK_FlowGroup * fg         = NULL;
	static char       funcName[] = "neuik_Object_Free__FlowGroup";
	static char     * errMsgs[]  = {"",                // [0] no error
		"Argument `fgPtr` is NULL.",                   // [1]
		"Argument `fgPtr` is not of FlowGroup class.", // [2]
		"Failure in function `neuik_Object_Free`.",    // [3]
	};

	if (fgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(fgPtr, neuik__Class_FlowGroup))
	{
		eNum = 2;
		goto out;
	}
	fg = (NEUIK_FlowGroup*)fgPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(fg->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(fg);
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
 *  Name:          neuik_Element_GetMinSize__FlowGroup
 *
 *  Description:   Returns the minimum rendered size of a given FlowGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__FlowGroup(
	NEUIK_Element    fgElem,
	RenderSize     * rSize)
{
	if (rSize != NULL)
	{
		rSize->w = 1;
		rSize->h = 1;
	}
	return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__FlowGroup
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__FlowGroup(
	NEUIK_Element   fgElem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
{
	int                    tempH;
	int                    elemCount;
	int                    nextInd;
	int                    finalInd;
	int                    ctr        = 0;
	int                    vctr       = 0;    /* valid counter; for elements shown */
	int                    yPos       = 0;
	int                    elHeight   = 0;
	int                    eNum       = 0;    /* which error to report (if any) */
	int                  * eShown     = NULL;
	float                  xPos       = 0.0;
	float                  xSize      = 0.0;
	float                  xFree      = 0.0;  /* px of space for hFill elems */
	NEUIK_ElementConfig  * eCfg       = NULL;
	NEUIK_ElementConfig ** eCfgs      = NULL;
	NEUIK_Element          elem       = NULL;
	NEUIK_Container      * cont       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_FlowGroup      * fg         = NULL;
	SDL_Renderer         * rend       = NULL;
	SDL_Rect               rect;
	RenderLoc              rl;
	RenderLoc              rlRel      = {0, 0}; /* renderloc relative to parent */
	RenderSize             rs;
	RenderSize           * eSizes     = NULL;
	static char            funcName[] = "neuik_Element_Render__FlowGroup";
	static char          * errMsgs[]  = {"",                               // [ 0] no error
		"Argument `fgElem` is not of FlowGroup class.",                    // [ 1]
		"", // [ 2]
		"Element_GetConfig returned NULL.",                                // [ 3]
		"Element_GetMinSize Failed.",                                      // [ 4]
		"Failure in `neuik_Element_Render()`",                             // [ 5]
		"Invalid specified `rSize` (negative values).",                    // [ 6]
		"", // [ 7]
		"Argument `fgElem` caused `neuik_Object_GetClassObject` to fail.", // [ 8]
		"Vertical fill preference not specified.",                         // [ 9]
		"Horizontal fill preference not specified.",                       // [10]
		"Failure in `NEUIK_Container_GetElementCount`."                    // [11]
		"Failed to allocate memory"                                        // [12]
		"Invalid (negative) number of contained elements.",                // [13]
		"Failure in neuik_Element_RedrawBackground().",                    // [14]
	};

	if (!neuik_Object_IsClass(fgElem, neuik__Class_FlowGroup))
	{
		eNum = 1;
		goto out;
	}
	fg = (NEUIK_FlowGroup*)fgElem;

	if (neuik_Object_GetClassObject(fgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(fgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 8;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check for invalid combinations of fill orders                          */
	/*------------------------------------------------------------------------*/
	if ((fg->FillFirst  == NEUIK_FLOWGROUP_FILLDIRN_LEFT_TO_RIGHT ||
		 fg->FillSecond == NEUIK_FLOWGROUP_FILLDIRN_LEFT_TO_RIGHT) && 
		(fg->FillFirst  == NEUIK_FLOWGROUP_FILLDIRN_RIGHT_TO_LEFT ||
		 fg->FillSecond == NEUIK_FLOWGROUP_FILLDIRN_RIGHT_TO_LEFT))
	{
		/*--------------------------------------------------------------------*/
		/* Vertical fill preference not specified                             */
		/*--------------------------------------------------------------------*/
		eNum = 9;
		goto out;
	}
	if ((fg->FillFirst  == NEUIK_FLOWGROUP_FILLDIRN_TOP_TO_BOTTOM ||
		 fg->FillSecond == NEUIK_FLOWGROUP_FILLDIRN_TOP_TO_BOTTOM) && 
		(fg->FillFirst  == NEUIK_FLOWGROUP_FILLDIRN_BOTTOM_TO_TOP ||
		 fg->FillSecond == NEUIK_FLOWGROUP_FILLDIRN_BOTTOM_TO_TOP))
	{
		/*--------------------------------------------------------------------*/
		/* Horizontal fill preference not specified                           */
		/*--------------------------------------------------------------------*/
		eNum = 10;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 6;
		goto out;
	}

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(fgElem, xSurf, rlMod, NULL))
	{
		eNum = 14;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the minimum size used by the flow fill preferenes            */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Container_GetElementCount(fg, &elemCount))
	{
		eNum = 11;
		goto out;
	}

	/*========================================================================*/
	/* Draw the UI elements into the FlowGroup                                */
	/*========================================================================*/
	if (elemCount == 0) goto out2; /* No elements */
	else if (elemCount < 0) 
	{
		eNum = 13;
		goto out;
	}
	else if (elemCount == 1)
	{
		elem = cont->elems[0];
		if (!NEUIK_Element_IsShown(elem)) goto out2;

		eCfg = neuik_Element_GetConfig(elem);
		if (eCfg == NULL)
		{
			eNum = 3;
			goto out;
		}
		if (neuik_Element_GetMinSize(elem, &rs))
		{
			eNum = 4;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Update the stored location before rendering the element. This is   */
		/* necessary as the location of this object will propagate to its     */
		/* child objects.                                                     */
		/*--------------------------------------------------------------------*/
		rect.x = (int)(xPos + eCfg->PadLeft);
		rect.y = (int)(yPos + eCfg->PadTop);
		rect.w = rs.w;
		rect.h = rs.h;
		rl.x = (eBase->eSt.rLoc).x + rect.x;
		rl.y = (eBase->eSt.rLoc).y + rect.y;
		rlRel.x = rect.x;
		rlRel.y = rect.y;
		neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

		if (neuik_Element_Render(elem, &rs, rlMod, rend, xSurf))
		{
			eNum = 5;
			goto out;
		}

		goto out2;
	}
	/*------------------------------------------------------------------------*/
	/* ELSE: Render the elements according to the specified fill order        */
	/*------------------------------------------------------------------------*/
	eSizes = (RenderSize*)malloc(elemCount*sizeof(RenderSize));
	if (eSizes == NULL)
	{
		eNum = 12;
		goto out;
	}
	eShown = (int*)malloc(elemCount*sizeof(int));
	if (eShown == NULL)
	{
		eNum = 12;
		goto out;
	}
	eCfgs = (NEUIK_ElementConfig**)malloc(elemCount*sizeof(NEUIK_ElementConfig*));
	if (eShown == NULL)
	{
		eNum = 12;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Extract the RenderSizes, IsShown states, and ElementConfigs for the    */
	/* contained elements.                                                    */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < elemCount; ctr++)
	{
		elem = cont->elems[ctr];

		eShown[ctr] = NEUIK_Element_IsShown(elem);
		eCfgs[ctr] = neuik_Element_GetConfig(elem);
		if (eCfgs[ctr] == NULL)
		{
			eNum = 3;
			goto out;
		}
		if (neuik_Element_GetMinSize(elem, &(eSizes[ctr])))
		{
			eNum = 4;
			goto out;
		}
	}


	if (fg->FillFirst  == NEUIK_FLOWGROUP_FILLDIRN_LEFT_TO_RIGHT &&
		fg->FillSecond == NEUIK_FLOWGROUP_FILLDIRN_TOP_TO_BOTTOM)
	{
		/*--------------------------------------------------------------------*/
		/* Continue looping over elements until they have all been placed.    */
		/*--------------------------------------------------------------------*/
		yPos  = 0;
		for (nextInd = 0; nextInd < elemCount;)
		{
			if (cont->elems[nextInd] == NULL) break;

			vctr  = 0;        /* valid element counter */
			xPos  = 0;
			xFree = (float)(rSize->w); /* free X-px: start @ full width; deduct as used */
			finalInd = nextInd;
			
			/*----------------------------------------------------------------*/
			/* Determine how many elements will fit in the first row and what */
			/* the maximum height used by any of these elements.              */
			/*----------------------------------------------------------------*/
			for (ctr = nextInd; ctr < elemCount; ctr++)
			{
				finalInd++;
				elem = cont->elems[ctr];
				if (elem == NULL) break;

				if (!eShown[ctr]) continue;
				vctr += 1;

				if (vctr > 0)
				{
					/* subsequent UI element is valid, deduct Horiz. Spacing */
					xFree -= fg->HSpacing;
				}

				eCfg = eCfgs[ctr];
				rs   = eSizes[ctr];

				tempH = rs.h + eCfg->PadTop + eCfg->PadBottom;
				if (tempH > elHeight)
				{
					elHeight = tempH;
				}
				xFree -= rs.w;

				if (xFree < 0)
				{
					/*--------------------------------------------------------*/
					/* If the row only contained a single element, then the   */
					/* element can't get any smaller, just continue.          */
					/*--------------------------------------------------------*/
					/* Alternatively, push the element to the next row.       */
					/*--------------------------------------------------------*/
					if (vctr > 1) finalInd--;
					break;
				}
			}

			/*----------------------------------------------------------------*/
			/* Render and place the child elements                            */
			/*----------------------------------------------------------------*/
			vctr  = 0;        /* valid element counter */
			for (ctr = nextInd; ctr < finalInd; ctr++)
			{
				nextInd++;
				elem = cont->elems[ctr];
				if (elem == NULL) 
				{
					nextInd--;
					break;
				}

				if (!eShown[ctr]) continue;
				vctr += 1;

				if (vctr > 0)
				{
					/* add horizontal spacing between subsequent elements */
					xPos += fg->HSpacing;
				}

				eCfg = eCfgs[ctr];
				rs   = eSizes[ctr];
				xSize = (float)(rs.w);

				/*------------------------------------------------------------*/
				/* Update the stored location before rendering the element.   */
				/* This is necessary as the location of this object will      */
				/* propagate to its child objects.                            */
				/*------------------------------------------------------------*/
				rect.x = (int)(xPos) + eCfg->PadLeft;
				rect.y = yPos + eCfg->PadTop;
				rect.w = rs.w;
				rect.h = rs.h;
				rl.x = (eBase->eSt.rLoc).x + rect.x;
				rl.y = (eBase->eSt.rLoc).y + rect.y;
				rlRel.x = rect.x;
				rlRel.y = rect.y;
				neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

				if (neuik_Element_Render(elem, &rs, rlMod, rend, xSurf))
				{
					eNum = 5;
					goto out;
				}

				xPos += xSize + (eCfg->PadLeft + eCfg->PadRight) ;
			}

			yPos += elHeight + fg->VSpacing;
		}
	}

out2:
	eBase->eSt.doRedraw = 0;
out:
	/*------------------------------------------------------------------------*/
	/* Free any dynamically allocated memory                                  */
	/*------------------------------------------------------------------------*/
	if (eSizes != NULL) free(eSizes);
	if (eShown != NULL) free(eShown);
	if (eCfgs  != NULL) free(eCfgs);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

