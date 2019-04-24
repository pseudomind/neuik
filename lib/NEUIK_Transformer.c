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
#include "NEUIK_Transformer.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Transformer(void ** tPtr);
int neuik_Object_Free__Transformer(void * tPtr);

int neuik_Element_GetMinSize__Transformer(NEUIK_Element, RenderSize*);
SDL_Texture * neuik_Element_Render__Transformer(NEUIK_Element, RenderSize*, SDL_Renderer*, SDL_Surface*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Transformer_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Transformer,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Transformer,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Transformer_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Transformer,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Transformer,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Transformer
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Transformer()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Transformer";
	static char  * errMsgs[]  = {"",                       // [0] no error
		"NEUIK library must be initialized first.",        // [1]
		"Failed to register `Transformer` object class .", // [2]
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
		"Transformer",                                // className
		"A single element container which can "
		"rotate and/or scale its contained element.", // classDescription
		neuik__Set_NEUIK,                             // classSet
		neuik__Class_Container,                       // superClass
		&neuik_Transformer_BaseFuncs,                 // baseFuncs
		NULL,                                         // classFuncs
		&neuik__Class_Transformer))                   // newClass
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
 *  Name:          neuik_Object_New__Transformer
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Transformer(
	void ** tPtr)
{
	int                 eNum       = 0;
	NEUIK_Container   * cont       = NULL;
	NEUIK_Transformer * trans      = NULL;
	NEUIK_Element     * sClassPtr  = NULL;
	static char         funcName[] = "neuik_Object_New__Transformer";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Output Argument `tPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                   // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                      // [3]
		"Failure in function `neuik.NewElement`.",                       // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",             // [5]
		"Argument `tPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",     // [7]
	};

	if (tPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*tPtr) = (NEUIK_Transformer*) malloc(sizeof(NEUIK_Transformer));
	trans = *tPtr;
	if (trans == NULL)
	{
		eNum = 2;
		goto out;
	}
	trans->rotation = 0.0;
	trans->scaling  = 1.0;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Transformer, 
			NULL,
			&(trans->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(trans->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(trans, &neuik_Transformer_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(trans, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_SINGLE;
	cont->shownIfEmpty = 0;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(trans, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(trans, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(trans, "hovered"))
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
 *  Name:          neuik_Object_Free__Transformer
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Transformer(
	void * tPtr)
{
	int                 eNum       = 0;    /* which error to report (if any) */
	NEUIK_Transformer * trans      = NULL;
	static char         funcName[] = "neuik_Object_Free__Transformer";
	static char       * errMsgs[]  = {"",               // [0] no error
		"Argument `tPtr` is NULL.",                     // [1]
		"Argument `tPtr` is not of Transformer class.", // [2]
		"Failure in function `neuik_Object_Free`.",     // [3]
	};

	if (tPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(tPtr, neuik__Class_Transformer))
	{
		eNum = 2;
		goto out;
	}
	trans = (NEUIK_Transformer*)tPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(trans->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(trans);
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
 *  Name:          NEUIK_NewTransformer
 *
 *  Description:   Create and return a pointer to a new NEUIK_Transformer.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
int NEUIK_NewTransformer(
	NEUIK_Transformer ** tPtr)
{
	return neuik_Object_New__Transformer((void**)tPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Transformer_Configure
 *
 *  Description:   Configure one or more settings for a transformer.
 *
 *                 NOTE: This list of settings must be terminated by a NULL
 *                 pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Transformer_Configure(
	NEUIK_Transformer * trans,
	const char        * set0,
	...)
{
	int                   ctr;
	int                   nCtr;
	int                   isBool;
	int                   boolVal    = 0;
	int                   doRedraw   = 0;
	int                   typeMixup;
	va_list               args;
	char                * strPtr     = NULL;
	char                * name       = NULL;
	char                * value      = NULL;
	const char          * set        = NULL;
	char                  buf[4096];
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char         * boolNames[] = {
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	// static char         * valueNames[] = {
	// 	"Rotation",
	// 	"Scaling",
	// 	NULL,
	// };
	static char           funcName[] = "NEUIK_Transformer_Configure";
	static char         * errMsgs[] = {"",                                // [ 0] no error
		"Argument `trans` caused `neuik_Object_GetClassObject` to fail.", // [ 1]
		"NamedSet.name is NULL, skipping.",                               // [ 2]
		"NamedSet.name is blank, skipping.",                              // [ 3]
		"NamedSet.name type unknown, skipping.",                          // [ 4]
		"`name=value` string is too long.",                               // [ 5]
		"Set string is empty.",                                           // [ 6]
		"HJustify value is invalid.",                                     // [ 7]
		"VJustify value is invalid.",                                     // [ 8]
		"BoolType name unknown, skipping.",                               // [ 9]
		"Invalid `name=value` string.",                                   // [10]
		"ValueType name used as BoolType, skipping.",                     // [11]
		"BoolType name used as ValueType, skipping.",                     // [12]
	};

	if (neuik_Object_GetClassObject(trans, neuik__Class_Element, (void**)&eBase))
	{
		NEUIK_RaiseError(funcName, errMsgs[1]);
		return 1;
	}

	set = set0;

	eCfg = neuik_Element_GetConfig(eBase);

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
		isBool = 0;
		name   = NULL;
		value  = NULL;

		if (set == NULL) break;

		// #ifndef NO_NEUIK_SIGNAL_TRAPPING
		// 	signal(SIGSEGV, neuik_Element_Configure_capture_segv);
		// #endif

		if (strlen(set) > 4095)
		{
			// #ifndef NO_NEUIK_SIGNAL_TRAPPING
			// 	signal(SIGSEGV, NULL);
			// #endif
			NEUIK_RaiseError(funcName, errMsgs[5]);
			set = va_arg(args, const char *);
			continue;
		}
		else
		{
			// #ifndef NO_NEUIK_SIGNAL_TRAPPING
			// 	signal(SIGSEGV, NULL);
			// #endif
			strcpy(buf, set);
			/* Find the equals and set it to '\0' */
			strPtr = strchr(buf, '=');
			if (strPtr == NULL)
			{
				/*------------------------------------------------------------*/
				/* Bool type configuration (or a mistake)                     */
				/*------------------------------------------------------------*/
				if (buf[0] == 0)
				{
					NEUIK_RaiseError(funcName, errMsgs[6]);
				}

				isBool  = 1;
				boolVal = 1;
				name    = buf;
				if (buf[0] == '!')
				{
					boolVal = 0;
					name    = buf + 1;
				}
			}
			else
			{
				*strPtr = 0;
				strPtr++;
				if (*strPtr == 0)
				{
					/* `name=value` string is missing a value */
					NEUIK_RaiseError(funcName, errMsgs[10]);
					set = va_arg(args, const char *);
					continue;
				}
				name  = buf;
				value = strPtr;
			}
		}

		if (name == NULL)
		{
			NEUIK_RaiseError(funcName, errMsgs[2]);
		}
		else if (name[0] == 0)
		{
			NEUIK_RaiseError(funcName, errMsgs[3]);
		}
		else if (!strcmp("Rotation", name))
		{
			trans->rotation = (double)(atof(value));
			doRedraw = 1;
		}
		else if (!strcmp("Scaling", name))
		{
			trans->scaling = (double)(atof(value));
			doRedraw = 1;
		}
		else
		{
			typeMixup = 0;
			for (nCtr = 0;; nCtr++)
			{
				if (boolNames[nCtr] == NULL) break;

				if (!strcmp(boolNames[nCtr], name))
				{
					typeMixup = 1;
					break;
				}
			}

			if (typeMixup)
			{
				/* A bool type was mistakenly used as a value type */
				NEUIK_RaiseError(funcName, errMsgs[12]);
			}
			else
			{
				/* An unsupported name was used as a value type */
				NEUIK_RaiseError(funcName, errMsgs[4]);
			}
		}

		/* before starting */
		set = va_arg(args, const char *);
	}
	va_end(args);

	if (doRedraw) neuik_Element_RequestRedraw(trans);

	return 0;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__Transformer
 *
 *  Description:   Returns the rendered size of a given button.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Transformer(
	NEUIK_Element   tElem, 
	RenderSize    * rSize)
{
	int                   eNum       = 0;    /* which error to report (if any) */
	RenderSize            rs         = {0, 0};
	NEUIK_Element         elem       = NULL;
	NEUIK_Transformer   * trans      = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	static char           funcName[] = "neuik_Element_GetMinSize__Transformer";
	static char         * errMsgs[]  = {"",                               // [0] no error
		"Argument `tElem` is not of Transformer class.",                  // [1]
		"Argument `tElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Element_GetConfig returned NULL.",                               // [3]
		"Failure in neuik_Element_GetSize.",                              // [4]
	};

	(*rSize) = rs;

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(tElem, neuik__Class_Transformer))
	{
		eNum = 1;
		goto out;
	}
	trans = (NEUIK_Transformer *)tElem;

	if (neuik_Object_GetClassObject(tElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 2;
		goto out;
	}

	if (cont->elems == NULL)
	{
		/* This trans does not contain an element */
		goto out;
	}
	else if (cont->elems[0] == NULL)
	{
		/* This trans does not contain an element */
		goto out;
	}
	/*------------------------------------------------------------------------*/
	/* ELSE: The transformer does contain an element.                         */
	/*------------------------------------------------------------------------*/
	elem = cont->elems[0];
	eCfg = neuik_Element_GetConfig(elem);
	if (eCfg == NULL)
	{
		eNum = 3;
		goto out;
	}

	if (!NEUIK_Element_IsShown(elem))
	{
		/* If the contained element is hidden, then hide the transformer */
		goto out;
	}
	if (neuik_Element_GetMinSize(elem, &rs) != 0)
	{
		eNum = 4;
		goto out;
	}

	rSize->w = rs.w + eCfg->PadLeft + eCfg->PadRight;
	rSize->h = rs.h + eCfg->PadTop  + eCfg->PadBottom;

	/*------------------------------------------------------------------------*/
	/* Check for and apply rotation if necessary.                             */
	/*------------------------------------------------------------------------*/
	if (trans->rotation ==    0.0 || 
		trans->rotation ==  180.0 || 
		trans->rotation == -180.0 ||
		trans->rotation ==  360.0 ||
		trans->rotation == -360.0)
	{
		/*--------------------------------------------------------------------*/
		/* This resulting element minimum size will be the same as the normal */
		/* unrotated element.                                                 */
		/*--------------------------------------------------------------------*/
	}
	else if (
		trans->rotation ==  90.0  ||
		trans->rotation == -90.0  ||
		trans->rotation ==  270.0 ||
		trans->rotation == -270.0)
	{
		/*--------------------------------------------------------------------*/
		/* This resulting element minimum size will have the width and height */
		/* swapped compared to its values in the unrotated state              */
		/*--------------------------------------------------------------------*/
		rs.w = rSize->h;
		rs.h = rSize->w;
		(*rSize) = rs;
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
 *  Name:          neuik_Element_Render__Transformer
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
SDL_Texture * neuik_Element_Render__Transformer(
	NEUIK_Element   tElem, 
	RenderSize    * rSize, 
	SDL_Renderer  * xRend,
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
{
	int                   eNum       = 0; /* which error to report (if any) */
	RenderLoc             rl         = {0, 0};
	RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Surface         * surf       = NULL;
	SDL_Renderer        * rend       = NULL;
	RenderSize            rs         = {0, 0};
	SDL_Texture         * tex        = NULL; /* texture */
	SDL_Rect              destRect   = {0, 0, 0, 0};  /* destination rectangle */
	NEUIK_Container     * cont       = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Transformer   * trans      = NULL;
	static char           funcName[] = "neuik_Element_Render__Transformer";
	static char         * errMsgs[]  = {"",                               // [0] no error
		"Argument `tElem` is not of Transformer class.",                  // [1]
		"Argument `tElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Call to Element_GetMinSize failed.",                             // [3]
		"Invalid specified `rSize` (negative values).",                   // [4]
		"Failure in Element_Resize().",                                   // [5]
		"Element_GetConfig returned NULL.",                               // [6]
		"neuik_Element_Render returned NULL.",                            // [7]
		"SDL_CreateTextureFromSurface returned NULL.",                    // [8]
		"Failure in neuik_Element_RedrawBackground().",                   // [9]
	};

	if (!neuik_Object_IsClass(tElem, neuik__Class_Transformer))
	{
		eNum = 1;
		goto out;
	}
	trans = (NEUIK_Transformer *)tElem;

	if (neuik_Object_GetClassObject(tElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Object_GetClassObject(tElem, neuik__Class_Container, (void**)&cont))
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
		if (!neuik_Element_NeedsRedraw(trans) && eBase->eSt.texture != NULL) 
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
		if (neuik_Element_GetMinSize__Transformer(trans, rSize))
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
		if (neuik_Element_Resize(trans, *rSize) != 0)
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
	if (neuik_Element_RedrawBackground(tElem, xSurf))
	{
		eNum = 9;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Render the contained Element                                           */
	/*------------------------------------------------------------------------*/
	if (cont->elems == NULL) goto out2;
	elem = cont->elems[0];

	if (elem == NULL) goto out2;

	/* If this trans has a hidden element, just make it a small box */
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
			/* The element fills the window vertically and horizontally */
			rs.w = rSize->w - eCfg->PadLeft + eCfg->PadRight;
			rs.h = rSize->h - eCfg->PadTop  + eCfg->PadBottom;
		}
		else
		{
			if (neuik_Element_GetMinSize__Transformer(tElem, &rs))
			{
				eNum = 3;
				goto out;
			}

			if (eCfg->HFill)
			{
				/* The element fills the window only horizontally */
				rs.w = rSize->w - eCfg->PadLeft + eCfg->PadRight;
			}
			else
			{
				/* The element fills the window only vertically  */
				rs.h = rSize->h - eCfg->PadTop  + eCfg->PadBottom;
			}
		}
	}
	else
	{
		if (neuik_Element_GetMinSize__Transformer(tElem, &rs))
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
					destRect.x = eCfg->PadLeft;
					break;
				case NEUIK_HJUSTIFY_CENTER:
				case NEUIK_HJUSTIFY_DEFAULT:
					destRect.x = rSize->w/2 - (rs.w/2);
					break;
				case NEUIK_HJUSTIFY_RIGHT:
					destRect.x =  rSize->w - (rs.w + eCfg->PadRight);
					break;
			}
			break;
		case NEUIK_HJUSTIFY_LEFT:
			destRect.x = eCfg->PadLeft;
			break;
		case NEUIK_HJUSTIFY_CENTER:
			destRect.x = rSize->w/2 - (rs.w/2);
			break;
		case NEUIK_HJUSTIFY_RIGHT:
			destRect.x =  rSize->w - (rs.w + eCfg->PadRight);
			break;
	}

	switch (eCfg->VJustify)
	{
		case NEUIK_VJUSTIFY_DEFAULT:
			switch (cont->VJustify)
			{
				case NEUIK_VJUSTIFY_TOP:
					destRect.y = eCfg->PadTop;
					break;
				case NEUIK_VJUSTIFY_CENTER:
				case NEUIK_VJUSTIFY_DEFAULT:
					destRect.y = rSize->h/2 - (rs.h/2);
					break;
				case NEUIK_VJUSTIFY_BOTTOM:
					destRect.y = rSize->h - (rs.h + eCfg->PadBottom);
					break;
			}
			break;
		case NEUIK_VJUSTIFY_TOP:
			destRect.y = eCfg->PadTop;
			break;
		case NEUIK_VJUSTIFY_CENTER:
			destRect.y = rSize->h/2 - (rs.h/2);
			break;
		case NEUIK_VJUSTIFY_BOTTOM:
			destRect.y = rSize->h - (rs.h + eCfg->PadBottom);
			break;
	}

	destRect.w = rs.w;
	destRect.h = rs.h;
	rl.x = (eBase->eSt.rLoc).x + destRect.x;
	rl.y = (eBase->eSt.rLoc).y + destRect.y;
	rlRel.x = destRect.x;
	rlRel.y = destRect.y;

	if (neuik_Element_GetMinSize(elem, &rs))
	{
		eNum = 3;
		goto out;
	}

	neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

	tex = neuik_Element_RenderRotate(elem, &rs, rend, surf, trans->rotation);
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
