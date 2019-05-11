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
SDL_Texture * neuik_Element_Render__VGroup(NEUIK_Element, RenderSize*, SDL_Renderer*, SDL_Surface*);

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
SDL_Texture * neuik_Element_Render__VGroup(
	NEUIK_Element   vgElem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
{
	int                   tempW   = 0;
	int                   ctr     = 0;
	int                   vctr    = 0; /* valid counter; for elements shown */
	int                   xPos    = 0;
	int                   yPos    = 0;
	int                   elWidth = 0;
	int                   ySize   = 0;
	int                   eNum    = 0; /* which error to report (if any) */
	float                 vFillPx = 0.0;
	float                 yFree   = 0.0; /* px of space free for vFill elems */
	float                 tScale  = 0.0; /* total vFill scaling factors */
	RenderLoc             rl      = {0, 0};
	RenderLoc             rlRel   = {0, 0}; /* renderloc relative to parent */
	SDL_Rect              rect    = {0, 0, 0, 0};
	RenderSize            rs      = {0, 0};
	static RenderSize     rsZero  = {0, 0};
	SDL_Surface         * surf    = NULL;
	SDL_Renderer        * rend    = NULL;
	SDL_Texture         * tex     = NULL; /* texture */
	NEUIK_Container     * cont    = NULL;
	NEUIK_ElementBase   * eBase   = NULL;
	NEUIK_Element         elem    = NULL;
	NEUIK_ElementConfig * eCfg    = NULL;
	NEUIK_VGroup        * vg      = NULL;
	static char           funcName[] = "neuik_Element_Render__VGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `vgElem` is not of VGroup class.",                       // [1]
		"Failure in Element_Resize().",                                    // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Element_GetMinSize Failed.",                                      // [4]
		"Element_Render returned NULL.",                                   // [5]
		"Invalid specified `rSize` (negative values).",                    // [6]
		"SDL_CreateTextureFromSurface returned NULL.",                     // [7]
		"Argument `vgElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                    // [9]
	};

	if (!neuik_Object_IsClass(vgElem, neuik__Class_VGroup))
	{
		eNum = 1;
		goto out;
	}
	vg = (NEUIK_VGroup*)vgElem;

	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(vgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 8;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w == eBase->eSt.rSizeOld.w  &&
		eBase->eSt.rSize.h == eBase->eSt.rSizeOld.h)
	{
		if (!neuik_Element_NeedsRedraw((NEUIK_Element)vg) && eBase->eSt.texture != NULL) 
		{
			(*rSize) = eBase->eSt.rSize;
			return eBase->eSt.texture;
		}
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 6;
		goto out;
	}
	yFree = (float)(rSize->h); /* free Y-px: start with the full ht. and deduct as used */

	/*------------------------------------------------------------------------*/
	/* Check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w != eBase->eSt.rSizeOld.w  ||
		eBase->eSt.rSize.h != eBase->eSt.rSizeOld.h)
	{
		/*--------------------------------------------------------------------*/
		/* This will create a new SDL_Surface & SDL_Renderer; also it will    */
		/* free old ones if they are allocated.                               */
		/*--------------------------------------------------------------------*/
		if (neuik_Element_Resize((NEUIK_Element)vg, *rSize) != 0)
		{
			eNum = 2;
			goto out;
		}
	}
	surf = eBase->eSt.surf;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(vgElem, xSurf))
	{
		eNum = 9;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Draw the UI elements into the VGroup                                   */
	/*------------------------------------------------------------------------*/
	if (cont->elems != NULL)
	{
		/*------------------------------------------------------*/
		/* Determine the (maximum) width of any of the elements */
		/*------------------------------------------------------*/
		vctr    = 0;
		elWidth = rSize->w;
		for (ctr = 0;; ctr++)
		{
			elem = (NEUIK_Element)cont->elems[ctr];
			if (elem == NULL) break;

			eCfg = neuik_Element_GetConfig(elem);
			if (eCfg == NULL)
			{
				eNum = 3;
				goto out;
			}

			if (!NEUIK_Element_IsShown(elem)) continue;
			vctr++;

			if (vctr > 0)
			{
				/* subsequent UI element is valid, deduct Vertical Spacing */
				yFree -= vg->VSpacing;
			}

			if (neuik_Element_GetMinSize(elem, &rs))
			{
				eNum = 4;
				goto out;
			}

			tempW = rs.w + eCfg->PadLeft + eCfg->PadRight;
			if (tempW > elWidth)
			{
				elWidth = tempW;
			}
			yFree -= rs.h + eCfg->PadTop + eCfg->PadBottom;
		}

		if (vctr == 0)
		{
			goto out2;
		}

		/*-----------------------------------------------------------*/
		/* Check if there are any elements which can fill vertically */
		/*-----------------------------------------------------------*/
		for (ctr = 0;; ctr++)
		{
			elem = (NEUIK_Element)cont->elems[ctr];
			if (elem == NULL) break;

			eCfg = neuik_Element_GetConfig(elem);
			if (eCfg == NULL)
			{
				eNum = 3;
				goto out;
			}

			if (!NEUIK_Element_IsShown(elem)) continue;

			if (eCfg->VFill)
			{
				/* This element is fills space vertically */
				tScale += eCfg->VScale;
				rs = rsZero; /* (0,0); use default calculated size */
				if (neuik_Element_GetMinSize(elem, &rs))
				{
					eNum = 4;
					goto out;
				}
				yFree += rs.h;
			}
		}

		/* calculate the number of vertical px per 1.0 of VScaling */
		vFillPx = (float)((int)(yFree/tScale));

		/*--------------------------------------------------------------------*/
		/* Render and place the child elements                                */
		/*--------------------------------------------------------------------*/
		vctr = 0;
		for (ctr = 0;; ctr++)
		{
			elem = (NEUIK_Element)cont->elems[ctr];
			if (elem == NULL) break;

			eCfg = neuik_Element_GetConfig(elem);
			if (eCfg == NULL)
			{
				eNum = 3;
				goto out;
			}

			if (!NEUIK_Element_IsShown(elem)) continue;
			vctr++;

			if (vctr > 0)
			{
				/* add vertical spacing between subsequent elements */
				yPos += vg->VSpacing;
			}

			/*----------------------------------------------------------------*/
			/* Start with the default calculated element size                 */
			/*----------------------------------------------------------------*/
			if (neuik_Element_GetMinSize(elem, &rs))
			{
				eNum = 4;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Check for and apply if necessary Horizontal and Veritcal fill  */
			/*----------------------------------------------------------------*/
			if (eCfg->HFill)
			{
				/* This element is configured to fill space horizontally */
				rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
			}
			if (eCfg->VFill)
			{
				/* This element is configured to fill space vertically */
				ySize = (int)(vFillPx * (eCfg->VScale)) - (eCfg->PadTop + eCfg->PadBottom);
				rs.h  = ySize;
			}

			/*----------------------------------------------------------------*/
			/* Update the stored location before rendering the element. This  */
			/* is necessary as the location of this object will propagate to  */
			/* its child objects.                                             */
			/*----------------------------------------------------------------*/
			switch (eCfg->HJustify)
			{
				case NEUIK_HJUSTIFY_DEFAULT:
					switch (cont->HJustify)
					{
						case NEUIK_HJUSTIFY_LEFT:
							rect.x = xPos + eCfg->PadLeft;
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
					rect.x = xPos + eCfg->PadLeft;
					break;
				case NEUIK_HJUSTIFY_CENTER:
					rect.x = rSize->w/2 - (rs.w/2);
					break;
				case NEUIK_HJUSTIFY_RIGHT:
					rect.x = rSize->w - (rs.w + eCfg->PadRight);
					break;
			}

			rect.y = yPos + eCfg->PadTop;
			rect.w = rs.w;
			rect.h = rs.h;
			rl.x = (eBase->eSt.rLoc).x + rect.x;
			rl.y = (eBase->eSt.rLoc).y + rect.y;
			rlRel.x = rect.x;
			rlRel.y = rect.y;
			neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

			tex = neuik_Element_Render(elem, &rs, rend, surf);
			if (tex == NULL)
			{
				eNum = 5;
				goto out;
			}

			SDL_RenderCopy(rend, tex, NULL, &rect);

			yPos += rs.h + (eCfg->PadTop + eCfg->PadBottom) ;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Present all changes and create a texture from this surface             */
	/*------------------------------------------------------------------------*/
out2:
	ConditionallyDestroyTexture((SDL_Texture **)&(eBase->eSt.texture));
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 7;
		goto out;
	}

	eBase->eSt.doRedraw = 0;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eBase->eSt.texture;
}

