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
#include "NEUIK_VGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__VGroup(void ** vgPtr);
int neuik_Object_Free__VGroup(void * vgPtr);

int neuik_Element_GetMinSize__VGroup(NEUIK_Element, RenderSize*);
int neuik_Element_Render__VGroup(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_VGroup_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__VGroup,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__VGroup,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_VGroup_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__VGroup,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__VGroup,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_VGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_VGroup()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_VGroup";
	static char  * errMsgs[]  = {"",                  // [0] no error
		"NEUIK library must be initialized first.",   // [1]
		"Failed to register `VGroup` object class .", // [2]
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
		"NEUIK_VGroup",                                        // className
		"An element container which vertically groups items.", // classDescription
		neuik__Set_NEUIK,                                      // classSet
		neuik__Class_Container,                                // superClass
		&neuik_VGroup_BaseFuncs,                               // baseFuncs
		NULL,                                                  // classFuncs
		&neuik__Class_VGroup))                                 // newClass
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
 *  Name:          neuik_Object_New__VGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__VGroup(
		void ** vgPtr)
{
	int               eNum       = 0;
	NEUIK_Container * cont       = NULL;
	NEUIK_VGroup    * vg         = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__VGroup";
	static char     * errMsgs[]  = {"",                                   // [0] no error
		"Output Argument `vgPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                    // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
		"Failure in function `neuik.NewElement`.",                        // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",              // [5]
		"Argument `vgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",      // [7]
	};

	if (vgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*vgPtr) = (NEUIK_VGroup*) malloc(sizeof(NEUIK_VGroup));
	vg = *vgPtr;
	if (vg == NULL)
	{
		eNum = 2;
		goto out;
	}

	/* Allocation successful */
	vg->VSpacing = 1;
	vg->isActive = 0;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_VGroup, 
			NULL,
			&(vg->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(vg->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(vg, &neuik_VGroup_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(vg, neuik__Class_Container, (void**)&cont))
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
 *  Name:          NEUIK_NewVGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_VGroup.
 *
 *                 Wrapper function to neuik_Object_New__VGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewVGroup(
	NEUIK_VGroup ** vgPtr)
{
	return neuik_Object_New__VGroup((void**)vgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_VGroup_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__VGroup(
	void  * vgPtr)
{
	int            eNum       = 0;    /* which error to report (if any) */
	NEUIK_VGroup * vg         = NULL;
	static char    funcName[] = "neuik_Object_Free__VGroup";
	static char  * errMsgs[]  = {"",                   // [0] no error
		"Argument `vgPtr` is NULL.",                   // [1]
		"Argument `vgPtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",    // [3]
	};

	if (vgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(vgPtr, neuik__Class_VGroup))
	{
		eNum = 2;
		goto out;
	}
	vg = (NEUIK_VGroup*)vgPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(vg->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(vg);
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
 *  Name:          NEUIK_VGroup_SetVSpacing
 *
 *  Description:   Set the vertical spacing parameter of a vertical group.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_VGroup_SetVSpacing(
	NEUIK_VGroup  * vg,
	int             spacing)
{
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_VGroup_SetVSpacing";
	static char  * errMsgs[]  = {"",               // [0] no error
		"Argument `vg` is not of VGroup class.",   // [1]
		"Argument `spacing` can not be negative.", // [2]
	};

	if (!neuik_Object_IsClass(vg, neuik__Class_VGroup))
	{
		eNum = 1;
		goto out;
	}
	if (spacing < 0)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no effective change in spacing; don't do anything          */
	/*------------------------------------------------------------------------*/
	if (spacing == vg->VSpacing) goto out;

	vg->VSpacing = spacing;
	// neuik_Element_RequestRedraw(vg);

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
 *  Name:          neuik_Element_GetMinSize__VGroup
 *
 *  Description:   Returns the rendered size of a given VGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__VGroup(
	NEUIK_Element    vgElem,
	RenderSize     * rSize)
{
	int                   tempW;
	int                   ctr        = 0;
	int                   vctr       = 0;   /* valid counter; for elements shown */
	int                   maxMinH    = 0;   /* largest min-height of all VFill elements */
	int                   thisMinH   = 0;   /* this VFill element's min-height */
	int                   eNum       = 0; /* which error to report (if any) */
	float                 thisH      = 0.0;

	int                    nAlloc     = 0;
	int                  * elemsShown = NULL; // Free upon returning.
	RenderSize           * elemsMinSz = NULL; // Free upon returning.
	NEUIK_ElementConfig ** elemsCfg   = NULL; // Free upon returning.

	RenderSize          * rs;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_VGroup        * vg         = NULL;
	static char           funcName[] = "neuik_Element_GetMinSize__VGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `vgElem` is not of VGroup class.",                       // [1]
		"Element_GetMinSize Failed.",                                      // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Argument `vgElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
		"Failure to allocate memory.",                                     // [5]
		"Unexpected NULL... Investigate.",                                 // [6]
	};

	rSize->w = 0;
	rSize->h = 0;

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceding                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(vgElem, neuik__Class_VGroup))
	{
		eNum = 1;
		goto out;
	}
	vg = (NEUIK_VGroup*)vgElem;

	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	if (cont->elems == NULL) {
		/* there are no UI elements contained by this VGroup */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the number of elements within the container.                 */
	/*------------------------------------------------------------------------*/
	for (ctr = 0;; ctr++)
	{
		elem = cont->elems[ctr];
		if (elem == NULL) break;
	}
	nAlloc = ctr;

	/*------------------------------------------------------------------------*/
	/* Allocate memory for lists of contained element properties.             */
	/*------------------------------------------------------------------------*/
	elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
	if (elemsCfg == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsShown = malloc(nAlloc*sizeof(int));
	if (elemsShown == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
	if (elemsMinSz == NULL)
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Store the current properties for the contained elements.               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		elem = cont->elems[ctr];
		if (elem == NULL)
		{
			eNum = 6;
			goto out;
		}

		elemsShown[ctr] = NEUIK_Element_IsShown(elem);
		if (!elemsShown[ctr]) continue;

		elemsCfg[ctr] = neuik_Element_GetConfig(elem);
		if (elemsCfg[ctr] == NULL)
		{
			eNum = 3;
			goto out;
		}

		if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
		{
			eNum = 2;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Determine the (maximum) width required by any one of the elements.     */
	/*                                                                        */
	/*    and                                                                 */
	/*                                                                        */
	/* Find the largest minimum height of all the vertically filling items.   */
	/*------------------------------------------------------------------------*/
	vctr = 0;
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		if (!elemsShown[ctr]) continue; /* this elem isn't shown */
		vctr++;

		eCfg = elemsCfg[ctr];
		rs   = &elemsMinSz[ctr];

		/*--------------------------------------------------------------------*/
		/* Get the (maximum) width required by any one of the elements        */
		/*--------------------------------------------------------------------*/
		tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
		if (tempW > rSize->w)
		{
			rSize->w = tempW;
		}

		/*--------------------------------------------------------------------*/
		/* Get the largest min-height of all the vertically filling items     */
		/*--------------------------------------------------------------------*/
		if (eCfg->VFill)
		{
			/* This element is fills space vertically */
			thisMinH = rs->h;

			if (thisMinH > maxMinH)
			{
				maxMinH = thisMinH;
			}
		}
	}

	if (vctr == 0) goto out;

	/*------------------------------------------------------------------------*/
	/* Determine the required vertical height.                                */
	/*------------------------------------------------------------------------*/
	vctr = 0;
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		if (!elemsShown[ctr]) continue; /* this elem isn't shown */
		vctr++;

		eCfg = elemsCfg[ctr];
		rs   = &elemsMinSz[ctr];

		if (vctr > 0)
		{
			/* subsequent UI element is valid, add Horizontal Spacing */
			thisH += (float)(vg->VSpacing);
		}

		if (eCfg->VFill)
		{
			/* This element is fills space horizontally */
			thisH += (eCfg->VScale)*(float)(maxMinH);
		}
		else
		{
			thisH += (float)(rs->h);
		}
		thisH += (float)(eCfg->PadTop + eCfg->PadBottom);
	}

	rSize->h = (int)(thisH);
out:
	if (elemsCfg != NULL)   free(elemsCfg);
	if (elemsShown != NULL) free(elemsShown);
	if (elemsMinSz != NULL) free(elemsMinSz);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__VGroup
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__VGroup(
	NEUIK_Element   vgElem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                    nAlloc        = 0;
	int                    tempH         = 0;
	int                    tempW         = 0;
	int                    ctr           = 0;
	int                    yPos          = 0;
	int                    yFree         = 0; // px of space free for vFill elems
	int                    dH            = 0; // Change in height [px]
	int                    eNum          = 0; // which error to report (if any)
	int                    nVFill        = 0; // number of rows which can VFill
	int                    reqResizeH    = 0; // required resize height
	int                    vfillRowsMinH = 0; // min height for all vFill rows
	int                    vfillMaxMinH  = 0; // largest minimum row height 
	                                          // among vertically filling rows.
	int                  * allHFill      = NULL; // Free upon returning; 
	                                             // Cols fills vertically? (per col)
	int                  * allVFill      = NULL; // Free upon returning; 
	                                             // Row fills vertically? (per row)
	int                  * allMaxMinH    = NULL; // Free upon returning; 
	                                             // The max min width (per row)
	int                    maxMinW       = 0;    // The max min width
	int                  * rendRowH      = NULL; // Free upon returning; 
	                                             // Rendered row height (per row)
	int                  * elemsShown    = NULL; // Free upon returning.
	RenderSize           * elemsMinSz    = NULL; // Free upon returning.
	NEUIK_ElementConfig ** elemsCfg      = NULL; // Free upon returning.
	RenderLoc              rl            = {0, 0};
	RenderLoc              rlRel         = {0, 0}; /* renderloc relative to parent */
	SDL_Rect               rect          = {0, 0, 0, 0};
	static RenderSize      rsZero        = {0, 0};
	RenderSize             rsMin         = {0, 0};
	RenderSize           * rs            = NULL;
	SDL_Renderer         * rend          = NULL;
	NEUIK_Container      * cont          = NULL;
	NEUIK_ElementBase    * eBase         = NULL;
	NEUIK_Element          elem          = NULL;
	NEUIK_ElementConfig  * eCfg          = NULL;
	NEUIK_VGroup         * vg      = NULL;
	neuik_MaskMap        * maskMap       = NULL; /* FREE upon return */
	enum neuik_bgstyle     bgStyle;
	static char            funcName[]    = "neuik_Element_Render__VGroup";
	static char          * errMsgs[]     = {"", // [0] no error
		"Argument `vgElem` is not of VGroup class.",                       // [1]
		"Argument `vgElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Invalid specified `rSize` (negative values).",                    // [3]
		"Failure in `neuik_Element_GetCurrentBGStyle()`.",                 // [4]
		"Failure in `neuik_MakeMaskMap()`",                                // [5]
		"Failure in `neuik_Window_FillTranspMaskFromLoc()`",               // [6]
		"Failure in neuik_Element_RedrawBackground().",                    // [7]
		"Failure to allocate memory.",                                     // [8]
		"Element_GetConfig returned NULL.",                                // [9]
		"Element_GetMinSize Failed.",                                      // [10]
		"Failure in `neuik_Element_Render()`",                             // [11]
	};

	if (!neuik_Object_IsClass(vgElem, neuik__Class_VGroup))
	{
		eNum = 1;
		goto out;
	}
	vg = (NEUIK_VGroup*)vgElem;

	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 3;
		goto out;
	}
	yFree = (float)(rSize->h); 
	/* ^^^ free Y-px: start with the full ht. and deduct as used */

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (!mock)
	{
		if (neuik_Element_GetCurrentBGStyle(vgElem, &bgStyle))
		{
			eNum = 4;
			goto out;
		}
		if (bgStyle != NEUIK_BGSTYLE_TRANSPARENT)
		{
			/*----------------------------------------------------------------*/
			/* Create a MaskMap an mark off the trasnparent pixels.           */
			/*----------------------------------------------------------------*/
			if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
			{
				eNum = 5;
				goto out;
			}

			rl = eBase->eSt.rLoc;
			if (neuik_Window_FillTranspMaskFromLoc(
					eBase->eSt.window, maskMap, rl.x, rl.y))
			{
				eNum = 6;
				goto out;
			}

			if (neuik_Element_RedrawBackground(vgElem, rlMod, maskMap))
			{
				eNum = 7;
				goto out;
			}
		}
	}

	if (cont->elems == NULL)
	{
		/* No elements are contained; don't do any more work here. */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for the calculated maximum minimum values, the         */
	/* VFill/HFill flags, and for the rendered row/column heights/widths.     */
	/*------------------------------------------------------------------------*/
	for (ctr = 0;; ctr++)
	{
		elem = (NEUIK_Element)cont->elems[ctr];
		if (elem == NULL) break;
	}
	nAlloc = ctr;

	allMaxMinH = malloc(nAlloc*sizeof(int));
	if (allMaxMinH == NULL)
	{
		eNum = 8;
		goto out;
	}
	allHFill = malloc(nAlloc*sizeof(int));
	if (allHFill == NULL)
	{
		eNum = 8;
		goto out;
	}
	allVFill = malloc(nAlloc*sizeof(int));
	if (allVFill == NULL)
	{
		eNum = 8;
		goto out;
	}
	rendRowH = malloc(nAlloc*sizeof(int));
	if (rendRowH == NULL)
	{
		eNum = 8;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Zero out the initial maximum minimum values and HFill/VFill flags.     */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		maxMinW         = 0;
		allHFill[ctr]   = 0;
		allMaxMinH[ctr] = 0;
		allVFill[ctr]   = 0;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for lists of contained element properties.             */
	/*------------------------------------------------------------------------*/
	elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
	if (elemsCfg == NULL)
	{
		eNum = 8;
		goto out;
	}
	elemsShown = malloc(nAlloc*sizeof(int));
	if (elemsShown == NULL)
	{
		eNum = 8;
		goto out;
	}
	elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
	if (elemsMinSz == NULL)
	{
		eNum = 8;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Store the current properties for the contained elements.               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0;; ctr++)
	{
		elem = (NEUIK_Element)cont->elems[ctr];
		if (elem == NULL) break;

		elemsShown[ctr] = NEUIK_Element_IsShown(elem);
		if (!elemsShown[ctr]) continue;

		elemsCfg[ctr] = neuik_Element_GetConfig(elem);
		if (elemsCfg[ctr] == NULL)
		{
			eNum = 9;
			goto out;
		}

		elemsMinSz[ctr] = rsZero;
		if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
		{
			eNum = 10;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the maximum minimum widths required for all of the columns.  */
	/* (i.e., for each column of elements, determine the maximum value of the */
	/* minimum widths required (among elements in the column)).               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0;; ctr++)
	{
		elem = (NEUIK_Element)cont->elems[ctr];
		if (elem == NULL) break;

		if (!elemsShown[ctr]) continue; /* this elem isn't shown */

		eCfg = elemsCfg[ctr];
		rs   = &elemsMinSz[ctr];

		tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
		if (tempW > maxMinW)
		{
			maxMinW = tempW;
		}

		allMaxMinH[ctr] = rs->h + (eCfg->PadTop + eCfg->PadBottom);

		/*--------------------------------------------------------------------*/
		/* Check if the element can fill horizontally and if so, mark the     */
		/* whole vgroup as one that can fill horizontally.                    */
		/*--------------------------------------------------------------------*/
		if (eCfg->HFill)
		{
			allHFill[ctr] = 1;
		}
		/*--------------------------------------------------------------------*/
		/* Check if the element can fill vertically and if so, mark the whole */
		/* vgroup as one that can fill vertically.                            */
		/*--------------------------------------------------------------------*/
		if (eCfg->VFill)
		{
			allVFill[ctr] = 1;
		}
	}

	/*========================================================================*/
	/* Calculation of rendered row heights (accounts for VFill).              */
	/*========================================================================*/
	/* Determine the required minimum height and the total number of rows     */
	/* which can fill vertically.                                             */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		rsMin.h += allMaxMinH[ctr];
		nVFill += allVFill[ctr];
	}
	if (nAlloc > 1)
	{
		rsMin.h += vg->VSpacing*(nAlloc - 1);
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the space occupied by all VFill rows and determine the value */
	/* of the largest minimum row height among vertically filling rows.       */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		if (allVFill[ctr])
		{
			vfillRowsMinH += allMaxMinH[ctr];
			if (vfillMaxMinH < allMaxMinH[ctr])
			{
				vfillMaxMinH = allMaxMinH[ctr];
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the amount of currently unused vertical space (beyond min).  */
	/*------------------------------------------------------------------------*/
	yFree = rSize->h - rsMin.h;

	/*------------------------------------------------------------------------*/
	/* Check if there is enough unused vertical space to bring all VFill rows */
	/* to the same height.                                                    */
	/*------------------------------------------------------------------------*/
	reqResizeH = nVFill*vfillMaxMinH - vfillRowsMinH; // required resize height
	if (yFree >= reqResizeH)
	{
		/*--------------------------------------------------------------------*/
		/* There is enough space; get all VFill rows to the same size first.  */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < nAlloc; ctr++)
		{
			rendRowH[ctr] = allMaxMinH[ctr];
			if (allVFill[ctr])
			{
				rendRowH[ctr] = vfillMaxMinH;
			}
		}

		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining vSpace between the vFill rows.         */
		/*--------------------------------------------------------------------*/
		yFree -= reqResizeH;

		dH = (int)(floor( (float)(yFree)/(float)(nVFill) ));
		if (dH > 0)
		{
			/*----------------------------------------------------------------*/
			/* Increase the height of vFill rows all by the same quantity.    */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < nAlloc; ctr++)
			{
				if (allVFill[ctr])
				{
					rendRowH[ctr] += dH;
					yFree -= dH;
				}
			}
		}

		if (yFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining vSpace to the vFill one pixel at a    */
			/* time (starting from the top row and moving down).              */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < nAlloc; ctr++)
			{
				if (allVFill[ctr])
				{
					rendRowH[ctr] += 1;
					yFree -= 1;
					if (yFree == 0)
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining vSpace between the vFill rows.         */
		/*--------------------------------------------------------------------*/
		while (yFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining vSpace to the vFill one pixel at a    */
			/* time (starting from the top row and moving down).              */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < nAlloc; ctr++)
			{
				if (allVFill[ctr] && rendRowH[ctr] < vfillMaxMinH)
				{
					rendRowH[ctr] += 1;
					yFree -= 1;
					if (yFree == 0)
					{
						break;
					}
				}
			}
		}
	}

	/*========================================================================*/
	/* Render and place the child elements                                    */
	/*========================================================================*/
	yPos = 0;
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		if (ctr > 0)
		{
			yPos += rendRowH[ctr-1] + vg->VSpacing;
		}
		if (!elemsShown[ctr]) continue; /* this elem isn't shown */

		elem = cont->elems[ctr];
		if (!neuik_Element_NeedsRedraw(elem)) continue;

		eCfg = elemsCfg[ctr];
		rs   = &elemsMinSz[ctr];

		tempH = rendRowH[ctr];

		/*--------------------------------------------------------------------*/
		/* Check for and apply if necessary Horizontal and Vertical fill.     */
		/*--------------------------------------------------------------------*/
		if (allHFill[ctr])
		{
			rs->w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
		}
		if (allVFill[ctr])
		{
			rs->h = tempH - (eCfg->PadTop + eCfg->PadBottom);
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
						rect.x = (rSize->w/2) - (rs->w/2);
						break;
					case NEUIK_HJUSTIFY_RIGHT:
						rect.x = rSize->w - (rs->w + eCfg->PadRight);
						break;
				}
				break;
			case NEUIK_HJUSTIFY_LEFT:
				rect.x = eCfg->PadLeft;
				break;
			case NEUIK_HJUSTIFY_CENTER:
				rect.x = (rSize->w/2) - (rs->w/2);
				break;
			case NEUIK_HJUSTIFY_RIGHT:
				rect.x = rSize->w - (rs->w + eCfg->PadRight);
				break;
		}
		switch (eCfg->VJustify)
		{
			case NEUIK_VJUSTIFY_DEFAULT:
				switch (cont->VJustify)
				{
					case NEUIK_VJUSTIFY_TOP:
						rect.y = yPos + eCfg->PadTop;
						break;
					case NEUIK_VJUSTIFY_CENTER:
					case NEUIK_VJUSTIFY_DEFAULT:
						rect.y = (yPos + rendRowH[ctr]/2) - (tempH/2);
						break;
					case NEUIK_VJUSTIFY_BOTTOM:
						rect.y = (yPos + rendRowH[ctr]) - 
							(rs->h + eCfg->PadBottom);
						break;
				}
				break;
			case NEUIK_VJUSTIFY_TOP:
				rect.y = yPos + eCfg->PadTop;
				break;
			case NEUIK_VJUSTIFY_CENTER:
				rect.y = (yPos + rendRowH[ctr]/2) - (tempH/2);
				break;
			case NEUIK_VJUSTIFY_BOTTOM:
				rect.y = (yPos + rendRowH[ctr]) - 
					(rs->h + eCfg->PadBottom);
				break;
		}

		rect.w = rs->w;
		rect.h = rendRowH[ctr];
		rl.x = (eBase->eSt.rLoc).x + rect.x;
		rl.y = (eBase->eSt.rLoc).y + rect.y;
		rlRel.x = rect.x;
		rlRel.y = rect.y;
		neuik_Element_StoreSizeAndLocation(elem, *rs, rl, rlRel);

		if (neuik_Element_Render(elem, rs, rlMod, rend, mock))
		{
			eNum = 11;
			goto out;
		}
	}
out:
	if (!mock) eBase->eSt.doRedraw = 0;
	if (maskMap != NULL) neuik_Object_Free(maskMap);

	if (elemsCfg   != NULL) free(elemsCfg);
	if (elemsShown != NULL) free(elemsShown);
	if (elemsMinSz != NULL) free(elemsMinSz);
	if (allMaxMinH != NULL) free(allMaxMinH);
	if (rendRowH   != NULL) free(rendRowH);
	if (allHFill   != NULL) free(allHFill);
	if (allVFill   != NULL) free(allVFill);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}
