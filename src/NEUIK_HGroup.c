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
 
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_HGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__HGroup(void ** hgPtr);
int neuik_Object_Free__HGroup(void * hgPtr);

int neuik_Element_GetMinSize__HGroup(NEUIK_Element, RenderSize*);
SDL_Texture * neuik_Element_Render__HGroup(NEUIK_Element, RenderSize*, SDL_Renderer*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_HGroup_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__HGroup,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__HGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_HGroup_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__HGroup,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__HGroup,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_HGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_HGroup()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_HGroup";
	static char  * errMsgs[]  = {"",                  // [0] no error
		"NEUIK library must be initialized first.",   // [1]
		"Failed to register `HGroup` object class .", // [2]
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
		"NEUIK_HGroup",                                          // className
		"An element container which horizontally groups items.", // classDescription
		neuik__Set_NEUIK,                                        // classSet
		neuik__Class_Container,                                  // superClass
		&neuik_HGroup_BaseFuncs,                                 // baseFuncs
		NULL,                                                    // classFuncs
		&neuik__Class_HGroup))                                   // newClass
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
 *  Name:          neuik_Object_New__HGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__HGroup(
		void ** hgPtr)
{
	int               eNum       = 0;
	NEUIK_Container * cont       = NULL;
	NEUIK_HGroup    * hg         = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__HGroup";
	static char     * errMsgs[]  = {"",                                   // [0] no error
		"Output Argument `hgPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                    // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
		"Failure in function `neuik.NewElement`.",                        // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",              // [5]
		"Argument `hgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",      // [7]
	};

	if (hgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*hgPtr) = (NEUIK_HGroup*) malloc(sizeof(NEUIK_HGroup));
	hg = *hgPtr;
	if (hg == NULL)
	{
		eNum = 2;
		goto out;
	}

	/* Allocation successful */
	hg->HSpacing = 1;
	hg->isActive = 0;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_HGroup, 
			NULL,
			&(hg->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(hg->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(hg, &neuik_HGroup_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(hg, neuik__Class_Container, (void**)&cont))
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
 *  Name:          NEUIK_NewHGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_HGroup.
 *
 *                 Wrapper function to neuik_Object_New__HGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewHGroup(
	NEUIK_HGroup ** hgPtr)
{
	return neuik_Object_New__HGroup((void**)hgPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_HGroup_SetHSpacing
 *
 *  Description:   Set the horizontal spacing parameter of a horizontal group.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_HGroup_SetHSpacing(
	NEUIK_HGroup  * hg,
	int             spacing)
{
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_HGroup_SetHSpacing";
	static char  * errMsgs[]  = {"",               // [0] no error
		"Argument `hg` is not of VGroup class.",   // [1]
		"Argument `spacing` can not be negative.", // [2]
	};

	if (!neuik_Object_IsClass(hg, neuik__Class_HGroup))
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
	if (spacing == hg->HSpacing) goto out;

	hg->HSpacing = spacing;
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
 *  Name:          neuik_Object_Free__HGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__HGroup(
	void * hgPtr)
{
	int            eNum       = 0;    /* which error to report (if any) */
	NEUIK_HGroup * hg         = NULL;
	static char    funcName[] = "neuik_Object_Free__HGroup";
	static char  * errMsgs[]  = {"",                // [0] no error
		"Argument `hgPtr` is NULL.",                // [1]
		"Argument `hgPtr` is not of HGroup class.", // [2]
		"Failure in function `neuik_Object_Free`.", // [3]
	};

	if (hgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(hgPtr, neuik__Class_HGroup))
	{
		eNum = 2;
		goto out;
	}
	hg = (NEUIK_HGroup*)hgPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(hg->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(hg);
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
 *  Name:          neuik_Element_GetMinSize__HGroup
 *
 *  Description:   Returns the rendered size of a given HGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__HGroup(
	NEUIK_Element    hgElem,
	RenderSize     * rSize)
{
	int                   tempH;
	int                   ctr        = 0;
	int                   vctr       = 0;   /* valid counter; for elements shown */
	int                   maxMinW    = 0;   /* largest min-width of all HFill elements */
	int                   thisMinW   = 0;   /* this HFill element's min-width */
	int                   eNum       = 0;   /* which error to report (if any) */
	float                 thisW      = 0.0;
	RenderSize            rs;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_HGroup        * hg         = NULL;
	static char           funcName[] = "neuik_Element_GetMinSize__HGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `hgElem` is not of HGroup class.",                       // [1]
		"Element_GetMinSize Failed.",                                      // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Argument `hgElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
	};

	rSize->w = 0;
	rSize->h = 0;

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceding                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(hgElem, neuik__Class_HGroup))
	{
		eNum = 1;
		goto out;
	}
	hg = (NEUIK_HGroup*)hgElem;

	if (neuik_Object_GetClassObject(hgElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Object_GetClassObject(hgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	if (cont->elems == NULL) {
		/* there are no UI elements contained by this HGroup */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the (maximum) height required by any one of the elements.    */
	/*                                                                        */
	/*    and                                                                 */
	/*                                                                        */
	/* Find the largest minimum width of all the horizontally filling items.  */
	/*------------------------------------------------------------------------*/
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

		if (neuik_Element_GetMinSize(elem, &rs) != 0)
		{
			eNum = 2;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Get the (maximum) height required by any one of the elements       */
		/*--------------------------------------------------------------------*/
		tempH = rs.h + (eCfg->PadTop + eCfg->PadBottom);
		if (tempH > rSize->h)
		{
			rSize->h = tempH;
		}

		/*--------------------------------------------------------------------*/
		/* Get the largest min-width of all the horizontally filling items    */
		/*--------------------------------------------------------------------*/
		if (eCfg->HFill)
		{
			/* This element is fills space horizontally */
			thisMinW = rs.w;

			if (thisMinW > maxMinW)
			{
				maxMinW = thisMinW;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Determine the required horizontal width                                */
	/*------------------------------------------------------------------------*/
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
			/* subsequent UI element is valid, add Horizontal Spacing */
			thisW += (float)(hg->HSpacing);
		}

		if (neuik_Element_GetMinSize(elem, &rs))
		{
			eNum = 2;
			goto out;
		}

		if (eCfg->HFill)
		{
			/* This element is fills space horizontally */
			thisW += (eCfg->HScale)*(float)(maxMinW);
		}
		else
		{
			thisW += (float)(rs.w);
		}
		thisW += (float)(eCfg->PadLeft + eCfg->PadRight);
	}

	rSize->w = (int)(thisW);
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
 *  Name:          neuik_Element_Render__HGroup
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * neuik_Element_Render__HGroup(
	NEUIK_Element    hgElem,
	RenderSize     * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer   * xRend) /* the external renderer to prepare the texture for */
{
	RenderSize            rs;
	RenderLoc             rl;
	SDL_Rect              rect;
	int                   tempH;
	int                   ctr        = 0;
	int                   vctr       = 0;    /* valid counter; for elements shown */
	int                   yPos       = 0;
	int                   elHeight   = 0;
	int                   eNum       = 0;    /* which error to report (if any) */
	float                 hFillPx    = 0.0;
	float                 xPos       = 0.0;
	float                 xSize      = 0.0;
	float                 xFree      = 0.0;  /* px of space for hFill elems */
	float                 tScale     = 0.0;  /* total vFill scaling factors */
	SDL_Texture         * tex        = NULL; /* texture */
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_HGroup        * hg         = NULL;
	SDL_Renderer        * rend       = NULL;
	static RenderSize     rsZero     = {0, 0};
	static char           funcName[] = "neuik_Element_Render__HGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `hgElem` is not of HGroup class.",                       // [1]
		"Failure in Element_Resize().",                                    // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Element_GetMinSize Failed.",                                      // [4]
		"Element_Render returned NULL.",                                   // [5]
		"Invalid specified `rSize` (negative values).",                    // [6]
		"SDL_CreateTextureFromSurface returned NULL.",                     // [7]
		"Argument `hgElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                    // [9]
	};

	if (!neuik_Object_IsClass(hgElem, neuik__Class_HGroup))
	{
		eNum = 1;
		goto out;
	}
	hg = (NEUIK_HGroup*)hgElem;

	if (neuik_Object_GetClassObject(hgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(hgElem, neuik__Class_Container, (void**)&cont))
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
		if (!neuik_Element_NeedsRedraw((NEUIK_Element)hg) && eBase->eSt.texture != NULL) 
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
	xFree = (float)(rSize->w); /* free X-px: start with the full width and deduct as used */

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
		if (neuik_Element_Resize((NEUIK_Element)hg, *rSize) != 0)
		{
			eNum = 2;
			goto out;
		}
	}
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(hgElem))
	{
		eNum = 9;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Draw the UI elements into the HGroup                                   */
	/*------------------------------------------------------------------------*/
	if (cont->elems != NULL)
	{
		/*-------------------------------------------------------*/
		/* Determine the (maximum) height of any of the elements */
		/*-------------------------------------------------------*/
		elHeight = rSize->h;
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
				/* subsequent UI element is valid, deduct Vertical Spacing */
				xFree -= hg->HSpacing;
			}

			if (neuik_Element_GetMinSize(elem, &rs))
			{
				eNum = 4;
				goto out;
			}

			tempH = rs.h + eCfg->PadTop + eCfg->PadBottom;
			if (tempH > elHeight)
			{
				elHeight = tempH;
			}
			xFree -= rs.w;
		}

		if (vctr == 0)
		{
			goto out2;
		}

		/*-------------------------------------------------------------*/
		/* Check if there are any elements which can fill horizontally */
		/*-------------------------------------------------------------*/
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

			if (eCfg->HFill)
			{
				/* This element is fills space horizontally */
				tScale += eCfg->HScale;
				rs = rsZero; /* (0,0); use default calculated size */
				if (neuik_Element_GetMinSize(elem, &rs))
				{
					eNum = 4;
					goto out;
				}
				xFree += rs.w;
			}
		}

		/* calculate the number of horizontal px per 1.0 of HScaling */
		hFillPx = (float)((int)((float)(xFree) / tScale));

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
				xPos += hg->HSpacing;
			}

			/*----------------------------------------------------------------*/
			/* Start with the default calculated element size                 */
			/*----------------------------------------------------------------*/
			if (neuik_Element_GetMinSize(elem, &rs))
			{
				eNum = 4;
				goto out;
			}
			xSize = (float)(rs.w);

			/*----------------------------------------------------------------*/
			/* Check for and apply if necessary Horizontal and Veritcal fill  */
			/*----------------------------------------------------------------*/
			if (eCfg->VFill)
			{
				/* This element is configured to fill space vertically */
				rs.h = elHeight - (eCfg->PadTop + eCfg->PadBottom);
			}
			if (eCfg->HFill)
			{
				/* This element is configured to fill space horizontally */
				xSize = hFillPx * (eCfg->HScale) - (eCfg->PadLeft + eCfg->PadRight);
				rs.w  = (int)(xSize);
			}

			/*----------------------------------------------------------------*/
			/* Update the stored location before rendering the element. This  */
			/* is necessary as the location of this object will propagate to  */
			/* its child objects.                                             */
			/*----------------------------------------------------------------*/
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
							rect.y = (rSize->h -
								(eCfg->PadTop + eCfg->PadBottom))/2 -
								(rs.h/2);
							break;
						case NEUIK_VJUSTIFY_BOTTOM:
							rect.y = rSize->h - (rs.h + eCfg->PadBottom);
							break;
					}
					break;
				case NEUIK_VJUSTIFY_TOP:
					rect.y = yPos + eCfg->PadTop;
					break;
				case NEUIK_VJUSTIFY_CENTER:
					rect.y = (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 -
						(rs.h/2);
					break;
				case NEUIK_VJUSTIFY_BOTTOM:
					rect.y = rSize->h - (rs.h + eCfg->PadBottom);
					break;
			}

			rect.x = (int)(xPos + eCfg->PadLeft);
			rect.w = rs.w;
			rect.h = rs.h;
			rl.x = (eBase->eSt.rLoc).x + rect.x;
			rl.y = (eBase->eSt.rLoc).y + rect.y;
			neuik_Element_StoreSizeAndLocation(elem, rs, rl);

			tex = neuik_Element_Render(elem, &rs, rend);
			if (tex == NULL)
			{
				eNum = 5;
				goto out;
			}

			SDL_RenderCopy(rend, tex, NULL, &rect);

			xPos += xSize + (eCfg->PadLeft + eCfg->PadRight) ;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Present all changes and create a texture from this surface             */
	/*------------------------------------------------------------------------*/
out2:
	ConditionallyDestroyTexture((SDL_Texture **)&(eBase->eSt.texture));
	SDL_RenderPresent(eBase->eSt.rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, eBase->eSt.surf);
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

