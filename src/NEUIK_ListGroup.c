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
#include "NEUIK_ListGroup.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "NEUIK_Window_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ListGroup(void ** lgPtr);
int neuik_Object_Free__ListGroup(void * lgPtr);

int neuik_Element_GetMinSize__ListGroup(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__ListGroup(NEUIK_Element lgElem, SDL_Event * ev);
SDL_Texture * neuik_Element_Render__ListGroup(NEUIK_Element, RenderSize*, SDL_Renderer*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ListGroup_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__ListGroup,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__ListGroup,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ListGroup_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__ListGroup,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__ListGroup,

	/* CaptureEvent(): Determine if this element caputures a given event */
	neuik_Element_CaptureEvent__ListGroup,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ListGroup
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ListGroup()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_ListGroup";
	static char  * errMsgs[]  = {"",                     // [0] no error
		"NEUIK library must be initialized first.",      // [1]
		"Failed to register `ListGroup` object class .", // [2]
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
		"NEUIK_ListGroup",                                       // className
		"An element container which horizontally groups items.", // classDescription
		neuik__Set_NEUIK,                                        // classSet
		neuik__Class_Container,                                  // superClass
		&neuik_ListGroup_BaseFuncs,                              // baseFuncs
		NULL,                                                    // classFuncs
		&neuik__Class_ListGroup))                                // newClass
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
 *  Name:          neuik_Object_New__ListGroup
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ListGroup(
	void ** lgPtr)
{
	int               eNum        = 0;
	NEUIK_Container * cont        = NULL;
	NEUIK_ListGroup * lg          = NULL;
	NEUIK_Element   * sClassPtr   = NULL;
	NEUIK_Color       bdrClr      = COLOR_GRAY;
	NEUIK_Color       bgSelectClr = COLOR_MBLUE;
	NEUIK_Color       bgOddClr    = COLOR_WHITE;
	NEUIK_Color       bgEvenClr   = COLOR_LWHITE;
	static char       funcName[]  = "neuik_Object_New__ListGroup";
	static char     * errMsgs[]   = {"",                                  // [0] no error
		"Output Argument `lgPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                    // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                       // [3]
		"Failure in function `neuik.NewElement`.",                        // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",              // [5]
		"Argument `lgPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",      // [7]
	};

	if (lgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*lgPtr) = (NEUIK_ListGroup*) malloc(sizeof(NEUIK_ListGroup));
	lg = *lgPtr;
	if (lg == NULL)
	{
		eNum = 2;
		goto out;
	}
	lg->VSpacing      = 0;
	lg->WidthBorder   = 1;           /* thickness of border (px) */
	lg->colorBorder   = bdrClr;      /* color to use for the border */
	lg->colorBGSelect = bgSelectClr; /* color to use for the selected text */
	lg->colorBGOdd    = bgOddClr;    /* background color to use for unselected odd rows */
	lg->colorBGEven   = bgEvenClr;   /* background color to use for unselected even rows */

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_ListGroup, 
			NULL,
			&(lg->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(lg->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(lg, &neuik_ListGroup_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(lg, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_NO_DEFAULT_ADD_SET;
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
 *  Name:          NEUIK_NewListGroup
 *
 *  Description:   Create and return a pointer to a new NEUIK_ListGroup.
 *
 *                 Wrapper function to neuik_Object_New__ListGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewListGroup(
	NEUIK_ListGroup ** lgPtr)
{
	return neuik_Object_New__ListGroup((void**)lgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Free__ListGroup
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ListGroup(
	void * lgPtr)
{
	int               eNum       = 0;    /* which error to report (if any) */
	NEUIK_ListGroup * lg         = NULL;
	static char       funcName[] = "neuik_Object_Free__ListGroup";
	static char     * errMsgs[]  = {"",                // [0] no error
		"Argument `lgPtr` is NULL.",                   // [1]
		"Argument `lgPtr` is not of FlowGroup class.", // [2]
		"Failure in function `neuik_Object_Free`.",    // [3]
	};

	if (lgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	lg = (NEUIK_ListGroup*)lgPtr;

	if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(lg->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(lg);
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
 *  Name:          neuik_Element_GetMinSize__ListGroup
 *
 *  Description:   Returns the minimum rendered size of a given FlowGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ListGroup(
	NEUIK_Element    lgElem,
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
 *  Name:          neuik_Element_Render__ListGroup
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * neuik_Element_Render__ListGroup(
	NEUIK_Element    lgElem,
	RenderSize     * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer   * xRend) /* the external renderer to prepare the texture for */
{
	int                   tempW;
	int                   offLeft;
	int                   offRight;
	int                   offTop;
	int                   offBottom;
	int                   ctr     = 0;
	int                   vctr    = 0; /* valid counter; for elements shown */
	int                   xPos    = 0;
	int                   yPos    = 0;
	int                   elWidth = 0;
	int                   eNum    = 0; /* which error to report (if any) */
	float                 yFree   = 0.0; /* px of space free for vFill elems */
	float                 tScale  = 0.0; /* total vFill scaling factors */
	RenderLoc             rl;
	SDL_Rect              rect;
	RenderSize            rs;
	static RenderSize     rsZero  = {0, 0};
	const NEUIK_Color   * bClr    = NULL; /* border color */
	const NEUIK_Color   * bgClr   = NULL; /* background color */
	SDL_Surface         * surf    = NULL;
	SDL_Renderer        * rend    = NULL;
	SDL_Texture         * tex     = NULL; /* texture */
	NEUIK_Container     * cont    = NULL;
	NEUIK_ElementBase   * eBase   = NULL;
	NEUIK_Element         elem    = NULL;
	NEUIK_ElementConfig * eCfg    = NULL;
	NEUIK_ListGroup     * lg      = NULL;
	static char           funcName[] = "neuik_Element_Render__ListGroup";
	static char         * errMsgs[]  = {"",                                // [0] no error
		"Argument `lgElem` is not of ListGroup class.",                    // [1]
		"Failure in Element_Resize().",                                    // [2]
		"Element_GetConfig returned NULL.",                                // [3]
		"Element_GetMinSize Failed.",                                      // [4]
		"Element_Render returned NULL.",                                   // [5]
		"Invalid specified `rSize` (negative values).",                    // [6]
		"SDL_CreateTextureFromSurface returned NULL.",                     // [7]
		"Argument `lgElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                    // [9]
	};

	if (!neuik_Object_IsClass(lgElem, neuik__Class_ListGroup))
	{
		eNum = 1;
		goto out;
	}
	lg = (NEUIK_ListGroup*)lgElem;

	if (neuik_Object_GetClassObject(lgElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(lgElem, neuik__Class_Container, (void**)&cont))
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
		if (!neuik_Element_NeedsRedraw((NEUIK_Element)lg) && eBase->eSt.texture != NULL) 
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
		if (neuik_Element_Resize((NEUIK_Element)lg, *rSize) != 0)
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
	if (neuik_Element_RedrawBackground(lgElem))
	{
		eNum = 9;
		goto out;
	}
	bgClr = &(lg->colorBGOdd);

	/*------------------------------------------------------------------------*/
	/* Draw the border of the ListGroup.                                      */
	/*------------------------------------------------------------------------*/
	bClr = &(lg->colorBorder);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

	offLeft   = 0;
	offRight  = rSize->w - 1;
	offTop    = 0;
	offBottom = rSize->h - 1;

	/* upper border line */
	SDL_RenderDrawLine(rend, offLeft, offTop, offRight, offTop); 
	/* left border line */
	SDL_RenderDrawLine(rend, offLeft, offTop, offLeft, offBottom); 
	/* right border line */
	SDL_RenderDrawLine(rend, offRight, offTop, offRight, offBottom); 
	/* lower border line */
	SDL_RenderDrawLine(rend, offLeft, offBottom, offRight, offBottom);

	xPos = 2;

	/*------------------------------------------------------------------------*/
	/* Draw the UI elements into the ListGroup                                */
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
				yFree -= lg->VSpacing;
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
			yFree -= rs.h;
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
				yPos += lg->VSpacing;
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
				rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight) - 4;
				/* ^^^ The -4 is for the left/right frame and space near it. */
			}

			/*----------------------------------------------------------------*/
			/* Update the stored location before rendering the element. This  */
			/* is necessary as the location of this object will propagate to  */
			/* its child objects.                                             */
			/*----------------------------------------------------------------*/
			rect.x = xPos + eCfg->PadLeft;

			rect.y = yPos + eCfg->PadTop;
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

	if (eBase == NULL) return NULL;
	return eBase->eSt.texture;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_ListGroup_AddRow
 *
 *  Description:   Adds a row to a ListGroup.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListGroup_AddRow(
	NEUIK_ListGroup * lg, 
	NEUIK_ListRow   * row)
{
	int                 len;
	int                 ctr;
	int                 newInd;            /* index for newly added item */
	int                 eNum       = 0;    /* which error to report (if any) */
	NEUIK_ElementBase * eBase      = NULL;
	NEUIK_Container   * cBase      = NULL;
	static char         funcName[] = "NEUIK_ListGroup_AddRow";
	static char       * errMsgs[]  = {"",                              // [0] no error
		"Argument `lg` is not of ListGroup class.",                    // [1]
		"Argument `lg` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Argument `row` is not of ListRow class.",                     // [3]
		"Failure to allocate memory.",                                 // [4]
		"Failure to reallocate memory.",                               // [5]
	};

	if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(lg, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 2;
		goto out;
	}
	if (!neuik_Object_IsClass(row, neuik__Class_ListRow))
	{
		eNum = 3;
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
			eNum = 4;
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
			eNum = 5;
			goto out;
		}
		newInd = ctr;
	}

	/*------------------------------------------------------------------------*/
	/* Set the Window and Parent Element pointers                             */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(lg, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	if (eBase->eSt.window != NULL)
	{
		neuik_Element_SetWindowPointer(row, eBase->eSt.window);
	}
	neuik_Element_SetParentPointer(row, lg);

	/*------------------------------------------------------------------------*/
	/* Set the odd/even flag of the row.                                      */
	/*------------------------------------------------------------------------*/
	row->isOddRow = 0;
	if ((newInd+1) % 2 == 1) row->isOddRow = 1;

	cBase->elems[newInd]   = row;
	cBase->elems[newInd+1] = NULL; /* NULLptr terminated array */
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
 *  Name:          NEUIK_ListGroup_AddRows
 *
 *  Description:   Add multiple rows to a ListGroup.
 *
 *                 NOTE: the variable # of arguments must be terminated by a 
 *                 NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ListGroup_AddRows(
	NEUIK_ListGroup * lg, 
	NEUIK_ListRow   * row0, 
	...)
{
	int             ctr;
	int             vaOpen = 0;
	int             eNum   = 0; /* which error to report (if any) */
	va_list         args;
	NEUIK_ListRow * row    = NULL; 
	static char     funcName[] = "NEUIK_ListGroup_AddRows";
	static char   * errMsgs[]  = {"",               // [0] no error
		"Argument `lg` is not of ListGroup class.", // [1]
		"Failure in `ListGroup_AddRow()`.",         // [2]
	};

	if (!neuik_Object_IsClass(lg, neuik__Class_ListGroup))
	{
		eNum = 1;
		goto out;
	}

	va_start(args, row0);
	vaOpen = 1;

	row = row0;
	for (ctr = 0;; ctr++)
	{
		if (row == NULL) break;

		if (NEUIK_ListGroup_AddRow(lg, row))
		{
			eNum = 2;
			goto out;
		}

		/* before starting */
		row = va_arg(args, NEUIK_Element);
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
 *  Name:          neuik_Element_CaptureEvent__ListGroup
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__ListGroup(
	NEUIK_Element   lgElem, 
	SDL_Event     * ev)
{
	int                 ctr         = 0;
	int                 indSelect   = 0;
	int                 wasSelected = 0;
	neuik_EventState    evCaputred  = NEUIK_EVENTSTATE_NOT_CAPTURED;
	NEUIK_Element       elem        = NULL;
	NEUIK_ElementBase * eBase       = NULL;
	NEUIK_Container   * cBase       = NULL;
	SDL_KeyboardEvent * keyEv       = NULL;

	if (neuik_Object_GetClassObject_NoError(
		lgElem, neuik__Class_Container, (void**)&cBase)) goto out;

	if (neuik_Object_GetClassObject_NoError(
		lgElem, neuik__Class_Element, (void**)&eBase)) goto out;

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by one of the contained rows.          */
	/*------------------------------------------------------------------------*/
	if (cBase->elems != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			elem = cBase->elems[ctr];
			if (elem == NULL) break;

			if (!NEUIK_Element_IsShown(elem)) continue;

			wasSelected = NEUIK_ListRow_IsSelected(elem);
			evCaputred = neuik_Element_CaptureEvent(elem, ev);
			if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
			{
				goto out;
			}
			else if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
			{
				if (!wasSelected && NEUIK_ListRow_IsSelected(elem))
				{
					indSelect = ctr;
					/*--------------------------------------------------------*/
					/* This event just caused this row to be selected.        */
					/* Deselect the other rows.                               */
					/*--------------------------------------------------------*/
					for (ctr = 0;; ctr++)
					{
						elem = cBase->elems[ctr];
						if (elem == NULL) break;
						if (ctr == indSelect) continue;

						NEUIK_ListRow_SetSelected(elem, 0);
					}
				}

				neuik_Element_SetActive(lgElem, 1);
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the ListGroup itself.                */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_IsActive(lgElem))
	{
		switch (ev->type)
		{
		case SDL_KEYDOWN:
			keyEv = (SDL_KeyboardEvent*)(ev);
			switch (keyEv->keysym.sym)
			{
			case SDLK_UP:
				/*------------------------------------------------------------*/
				/* Determine the where the first selected item is             */
				/*------------------------------------------------------------*/
				for (ctr = 0;; ctr++)
				{
					elem = cBase->elems[ctr];
					if (elem == NULL) break;

					if (!NEUIK_Element_IsShown(elem)) continue;
					if (NEUIK_ListRow_IsSelected(elem))
					{
						indSelect = ctr;
						break;
					}
				}

				if (indSelect > 0)
				{
					NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 0);
					indSelect--;
					NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 1);
					neuik_Window_TakeFocus(eBase->eSt.window, cBase->elems[indSelect]);
				}
				break;
			case SDLK_DOWN:
				/*------------------------------------------------------------*/
				/* Determine the where the first selected item is             */
				/*------------------------------------------------------------*/
				for (ctr = 0;; ctr++)
				{
					elem = cBase->elems[ctr];
					if (elem == NULL) break;

					if (!NEUIK_Element_IsShown(elem)) continue;
					if (NEUIK_ListRow_IsSelected(elem))
					{
						indSelect = ctr;
						break;
					}
				}

				if (cBase->elems[indSelect+1] != NULL)
				{
					NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 0);
					indSelect++;
					NEUIK_ListRow_SetSelected(cBase->elems[indSelect], 1);
					neuik_Window_TakeFocus(eBase->eSt.window, cBase->elems[indSelect]);
				}
				break;
			}
		}		
	}
out:
	return evCaputred;
}
