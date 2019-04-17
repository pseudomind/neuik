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
#include "NEUIK_Frame.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Frame(void ** fPtr);
int neuik_Object_Free__Frame(void * fPtr);

int neuik_Element_GetMinSize__Frame(NEUIK_Element, RenderSize*);
SDL_Texture * neuik_Element_Render__Frame(NEUIK_Element, RenderSize*, SDL_Renderer*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Frame_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Frame,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Frame,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Frame_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Frame,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Frame,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Frame
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Frame()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Frame";
	static char  * errMsgs[]  = {"",                 // [0] no error
		"NEUIK library must be initialized first.",  // [1]
		"Failed to register `Frame` object class .", // [2]
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
		"Frame",                                           // className
		"A single element container boxes in an element.", // classDescription
		neuik__Set_NEUIK,                                  // classSet
		neuik__Class_Container,                            // superClass
		&neuik_Frame_BaseFuncs,                            // baseFuncs
		NULL,                                              // classFuncs
		&neuik__Class_Frame))                              // newClass
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
 *  Name:          neuik_Object_New__Frame
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Frame(
	void ** fPtr)
{
	int               eNum       = 0;
	NEUIK_Color       dClr       = COLOR_GRAY;
	NEUIK_Container * cont       = NULL;
	NEUIK_Frame     * frame      = NULL;
	NEUIK_Element   * sClassPtr  = NULL;
	static char       funcName[] = "neuik_Object_New__Frame";
	static char     * errMsgs[]  = {"",                                  // [0] no error
		"Output Argument `fPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                   // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                      // [3]
		"Failure in function `neuik.NewElement`.",                       // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",             // [5]
		"Argument `fPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",     // [7]
	};

	if (fPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*fPtr) = (NEUIK_Frame*) malloc(sizeof(NEUIK_Frame));
	frame = *fPtr;
	if (frame == NULL)
	{
		eNum = 2;
		goto out;
	}
	frame->thickness = 1;     /* thickness of frame (px) */
	frame->color     = dClr;  /* color to use for the frame */

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Frame, 
			NULL,
			&(frame->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(frame->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(frame, &neuik_Frame_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(frame, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_SINGLE;
	cont->shownIfEmpty = 1;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(frame, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(frame, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(frame, "hovered"))
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
 *  Name:          neuik_Object_Free__Frame
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Frame(
	void * fPtr)
{
	int           eNum       = 0;    /* which error to report (if any) */
	NEUIK_Frame * frame      = NULL;
	static char   funcName[] = "neuik_Object_Free__Frame";
	static char * errMsgs[]  = {"",                 // [0] no error
		"Argument `fPtr` is NULL.",                 // [1]
		"Argument `fPtr` is not of Frame class.",   // [2]
		"Failure in function `neuik_Object_Free`.", // [3]
	};

	if (fPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(fPtr, neuik__Class_Frame))
	{
		eNum = 2;
		goto out;
	}
	frame = (NEUIK_Frame*)fPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(frame->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(frame);
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
 *  Name:          NEUIK_NewFrame
 *
 *  Description:   Create and return a pointer to a new NEUIK_Frame.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
int NEUIK_NewFrame(
	NEUIK_Frame ** fPtr)
{
	return neuik_Object_New__Frame((void**)fPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__Frame
 *
 *  Description:   Returns the rendered size of a given button.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Frame(
	NEUIK_Element   fElem, 
	RenderSize    * rSize)
{
	int                   eNum       = 0;    /* which error to report (if any) */
	RenderSize            rs         = {0, 0};
	NEUIK_Element         elem       = NULL;
	NEUIK_Frame         * frame      = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	static char           funcName[] = "neuik_Element_GetMinSize__Frame";
	static char         * errMsgs[]  = {"",                               // [0] no error
		"Argument `fElem` is not of Frame class.",                        // [1]
		"Argument `fElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Element_GetConfig returned NULL.",                               // [3]
		"Failure in neuik_Element_GetSize.",                              // [4]
	};

	(*rSize) = rs;

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(fElem, neuik__Class_Frame))
	{
		eNum = 1;
		goto out;
	}
	frame = (NEUIK_Frame *)fElem;

	if (neuik_Object_GetClassObject(fElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (cont->elems == NULL)
	{
		/* This frame does not contain an element, just make it a small box */
		rSize->w = 20;
		rSize->h = 20;
		goto out;
	}
	else if (cont->elems[0] == NULL)
	{
		/* This frame does not contain an element, just make it a small box */
		rSize->w = 20;
		rSize->h = 20;
		goto out;
	}
	else
	{
		elem = cont->elems[0];
		eCfg = neuik_Element_GetConfig(elem);
		if (eCfg == NULL)
		{
			eNum = 3;
			goto out;
		}

		if (!NEUIK_Element_IsShown(elem))
		{
			/* This frame does a hidden element, just make it a small box */
			rSize->w = 20;
			rSize->h = 20;
			goto out;
		}
		if (neuik_Element_GetMinSize(elem, &rs) != 0)
		{
			eNum = 4;
			goto out;
		}

		rSize->w = rs.w + (2 * frame->thickness) + eCfg->PadLeft + eCfg->PadRight;
		rSize->h = rs.h + (2 * frame->thickness) + eCfg->PadTop  + eCfg->PadBottom;
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
 *  Name:          neuik_Element_Render__Frame
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * neuik_Element_Render__Frame(
	NEUIK_Element    fElem, 
	RenderSize     * rSize, 
	SDL_Renderer   * xRend)
{
	int                   eNum       = 0; /* which error to report (if any) */
	int                   offLeft    = 0;
	int                   offRight   = 0;
	int                   offTop     = 0;
	int                   offBottom  = 0;
	RenderLoc             rl         = {0, 0};
	const NEUIK_Color   * bClr       = NULL; /* border color */
	SDL_Surface         * surf       = NULL;
	SDL_Renderer        * rend       = NULL;
	RenderSize            rs         = {0, 0};
	SDL_Texture         * tex        = NULL; /* texture */
	SDL_Rect              destRect   = {0, 0, 0, 0};  /* destination rectangle */
	NEUIK_Container     * cont       = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Frame         * frame      = NULL;
	static char           funcName[] = "neuik_Element_Render__Frame";
	static char         * errMsgs[]  = {"",                               // [0] no error
		"Argument `fElem` is not of Frame class.",                        // [1]
		"Argument `fElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Call to Element_GetMinSize failed.",                             // [3]
		"Invalid specified `rSize` (negative values).",                   // [4]
		"Failure in Element_Resize().",                                   // [5]
		"Element_GetConfig returned NULL.",                               // [6]
		"neuik_Element_Render returned NULL.",                            // [7]
		"SDL_CreateTextureFromSurface returned NULL.",                    // [8]
		"Failure in neuik_Element_RedrawBackground().",                   // [9]
	};

	if (!neuik_Object_IsClass(fElem, neuik__Class_Frame))
	{
		eNum = 1;
		goto out;
	}
	frame = (NEUIK_Frame *)fElem;

	if (neuik_Object_GetClassObject(fElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Object_GetClassObject(fElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w == eBase->eSt.rSizeOld.w  &&
		eBase->eSt.rSize.h == eBase->eSt.rSizeOld.h)
	{
		if (!neuik_Element_NeedsRedraw(frame) && eBase->eSt.texture != NULL) 
		{
			(*rSize) = eBase->eSt.rSize;
			return eBase->eSt.texture;
		}
	}

	/*------------------------------------------------------*/
	/* Calculate the required size of the resultant texture */
	/*------------------------------------------------------*/
	if (rSize->w == 0 && rSize->h == 0)
	{
		if (neuik_Element_GetMinSize__Frame(frame, rSize))
		{
			eNum = 3;
			goto out;
		}
	}
	else if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
		goto out;
	}

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
		if (neuik_Element_Resize(frame, *rSize) != 0)
		{
			eNum = 5;
			goto out;
		}
	}
	surf = eBase->eSt.surf;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(fElem))
	{
		eNum = 9;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Draw the border of the frame.                                          */
	/*------------------------------------------------------------------------*/
	bClr = &(frame->color);
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


	/*------------------------------------------------------------------------*/
	/* Render the contained Element                                           */
	/*------------------------------------------------------------------------*/
	if (cont->elems == NULL) goto out2;
	elem = cont->elems[0];

	if (elem == NULL) goto out2;

	/* If this frame has a hidden element, just make it a small box */
	if (!NEUIK_Element_IsShown(elem)) goto out2;

	/*------------------------------------------------------------------------*/
	/* Determine whether the contained element fills the window               */
	/*------------------------------------------------------------------------*/
	eCfg = neuik_Element_GetConfig(elem);
	if (eCfg == NULL)
	{
		eNum = 6;
		goto out;
	}

	if (eCfg->HFill || eCfg->VFill)
	{
		if (eCfg->HFill && eCfg->VFill)
		{
			/* The element fills the window vertically and horizonatally */
			rs.w = rSize->w - (2*frame->thickness + eCfg->PadLeft + eCfg->PadRight);
			rs.h = rSize->h - (2*frame->thickness + eCfg->PadTop  + eCfg->PadBottom);
		}
		else
		{
			if (neuik_Element_GetMinSize(elem, &rs))
			{
				eNum = 3;
				goto out;
			}

			if (eCfg->HFill)
			{
				/* The element fills the window only horizonatally */
				rs.w = rSize->w - (2*frame->thickness + eCfg->PadLeft + eCfg->PadRight);
			}
			else
			{
				/* The element fills the window only vertically  */
				rs.h = rSize->h - (2*frame->thickness + eCfg->PadTop  + eCfg->PadBottom);
			}
		}
	}
	else
	{
		if (neuik_Element_GetMinSize(elem, &rs))
		{
			eNum = 3;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Update the stored location before rendering the element. This is       */
	/* necessary as the location of this object will propagate to its child   */
	/* objects.                                                               */
	/*------------------------------------------------------------------------*/
	switch (eCfg->HJustify)
	{
		case NEUIK_HJUSTIFY_DEFAULT:
			switch (cont->HJustify)
			{
				case NEUIK_HJUSTIFY_LEFT:
					destRect.x = frame->thickness + eCfg->PadLeft;
					break;
				case NEUIK_HJUSTIFY_CENTER:
				case NEUIK_HJUSTIFY_DEFAULT:
					destRect.x = rSize->w/2 - (rs.w/2);
					break;
				case NEUIK_HJUSTIFY_RIGHT:
					destRect.x =  rSize->w - (rs.w + frame->thickness + eCfg->PadRight);
					break;
			}
			break;
		case NEUIK_HJUSTIFY_LEFT:
			destRect.x = frame->thickness + eCfg->PadLeft;
			break;
		case NEUIK_HJUSTIFY_CENTER:
			destRect.x = rSize->w/2 - (rs.w/2);
			break;
		case NEUIK_HJUSTIFY_RIGHT:
			destRect.x =  rSize->w - (rs.w + frame->thickness + eCfg->PadRight);
			break;
	}

	switch (eCfg->VJustify)
	{
		case NEUIK_VJUSTIFY_DEFAULT:
			switch (cont->VJustify)
			{
				case NEUIK_VJUSTIFY_TOP:
					// rect.y = eCfg->PadTop;
					destRect.y = frame->thickness + eCfg->PadTop;
					break;
				case NEUIK_VJUSTIFY_CENTER:
				case NEUIK_VJUSTIFY_DEFAULT:
					destRect.y = rSize->h/2 - (rs.h/2);
					break;
				case NEUIK_VJUSTIFY_BOTTOM:
					destRect.y = rSize->h - (rs.h + frame->thickness + eCfg->PadBottom);
					break;
			}
			break;
		case NEUIK_VJUSTIFY_TOP:
			// rect.y = eCfg->PadTop;
			destRect.y = frame->thickness + eCfg->PadTop;
			break;
		case NEUIK_VJUSTIFY_CENTER:
			destRect.y = rSize->h/2 - (rs.h/2);
			break;
		case NEUIK_VJUSTIFY_BOTTOM:
			destRect.y = rSize->h - (rs.h + frame->thickness + eCfg->PadBottom);
			break;
	}

	destRect.w = rs.w;
	destRect.h = rs.h;
	rl.x = (eBase->eSt.rLoc).x + destRect.x;
	rl.y = (eBase->eSt.rLoc).y + destRect.y;

	neuik_Element_StoreSizeAndLocation(elem, rs, rl);

	tex = neuik_Element_Render(elem, &rs, rend);
	if (tex != NULL)
	{
		SDL_RenderCopy(rend, tex, NULL, &destRect);
	}
	else
	{
		eNum = 7;
		goto out;
	}


out2:
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 8;
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

