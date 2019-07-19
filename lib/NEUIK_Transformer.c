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
#include "NEUIK_Transformer.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;
extern int neuik__Report_Debug;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Transformer(void ** tPtr);
int neuik_Object_Free__Transformer(void * tPtr);

int neuik__getTransformedMinSize__Transformer(NEUIK_Element, RenderSize*);
int neuik_Element_GetMinSize__Transformer(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__Transformer(
	NEUIK_Element, SDL_Event*);
int neuik_Element_Render__Transformer(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


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
	neuik_Element_CaptureEvent__Transformer,

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
	int                   doRedraw   = 0;
	int                   typeMixup;
	va_list               args;
	char                * strPtr     = NULL;
	char                * name       = NULL;
	char                * value      = NULL;
	const char          * set        = NULL;
	char                  buf[4096];
	RenderSize            rSize      = {0, 0};
	RenderLoc             rLoc       = {0, 0};;
	NEUIK_ElementBase   * eBase      = NULL;
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
		"Failure in `neuik_Element_GetSizeAndLocation()`.",               // [13]
	};

	if (neuik_Object_GetClassObject(trans, neuik__Class_Element, (void**)&eBase))
	{
		NEUIK_RaiseError(funcName, errMsgs[1]);
		return 1;
	}

	set = set0;

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
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

				name    = buf;
				if (buf[0] == '!')
				{
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

	if (doRedraw)
	{
		if (neuik_Element_GetSizeAndLocation(trans, &rSize, &rLoc))
		{
			NEUIK_RaiseError(funcName, errMsgs[13]);
			return 1;
		}
		neuik_Element_RequestRedraw(trans, rLoc, rSize);
	}

	return 0;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__Transformer
 *
 *  Description:   Returns the rendered size of a given transformer.
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
 *  Name:          neuik__getTransformedMinSize__Transformer
 *
 *  Description:   Returns the rendered size of transformed element.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik__getTransformedMinSize__Transformer(
	NEUIK_Element   tElem, 
	RenderSize    * rSize)
{
	int                   eNum       = 0;    /* which error to report (if any) */
	RenderSize            rs         = {0, 0};
	NEUIK_Element         elem       = NULL;
	NEUIK_Transformer   * trans      = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	static char           funcName[] = "neuik__getTransformedMinSize__Transformer";
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

	rSize->w = rs.w;
	rSize->h = rs.h;

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
		rSize->w += (eCfg->PadLeft + eCfg->PadRight);
		rSize->h += (eCfg->PadTop + eCfg->PadBottom);
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
		rs.w += (eCfg->PadTop + eCfg->PadBottom);
		rs.h += (eCfg->PadLeft + eCfg->PadRight);

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
int neuik_Element_Render__Transformer(
	NEUIK_Element   tElem, 
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                   eNum       = 0; /* which error to report (if any) */
	RenderLoc             rl         = {0, 0};
	RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Surface         * surf       = NULL;
	SDL_Renderer        * rend       = NULL;
	RenderSize            rs         = {0, 0};
	RenderSize            rsOrig     = {0, 0}; /* the unrotated render size */
	RenderLoc             rlModNext  = {0, 0}; /* to pass to containd elem */
	SDL_Rect              destRect   = {0, 0, 0, 0};  /* destination rectangle */
	SDL_Texture         * tex        = NULL;
	NEUIK_Container     * cont       = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Transformer   * trans      = NULL;
	neuik_MaskMap       * maskMap    = NULL; /* FREE upon return */
	enum neuik_bgstyle    bgStyle;
	static char           funcName[] = "neuik_Element_Render__Transformer";
	static char         * errMsgs[]  = {"",                               // [0] no error
		"Argument `tElem` is not of Transformer class.",                  // [1]
		"Argument `tElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Call to Element_GetMinSize failed.",                             // [3]
		"Invalid specified `rSize` (negative values).",                   // [4]
		"Failure in Element_Resize().",                                   // [5]
		"Element_GetConfig returned NULL.",                               // [6]
		"Failure in `neuik_Element_RenderRotate()`",                      // [7]
		"Failure in `SDL_CreateTextureFromSurface()`.",                   // [8]
		"Failure in `neuik_Element_RedrawBackground()`.",                 // [9]
		"Failure in `neuik_Element_GetCurrentBGStyle()`.",                // [10]
		"Failure in `neuik_MakeMaskMap()`",                               // [11]
		"Failure in `neuik_Window_FillTranspMaskFromLoc()`",              // [12]
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

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
		goto out;
	}


	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (!mock)
	{
		if (neuik_Element_GetCurrentBGStyle(tElem, &bgStyle))
		{
			eNum = 10;
			goto out;
		}
		if (bgStyle != NEUIK_BGSTYLE_TRANSPARENT)
		{
			/*----------------------------------------------------------------*/
			/* Create a MaskMap an mark off the trasnparent pixels.           */
			/*----------------------------------------------------------------*/
			if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
			{
				eNum = 11;
				goto out;
			}

			rl = eBase->eSt.rLoc;
			if (neuik_Window_FillTranspMaskFromLoc(
					eBase->eSt.window, maskMap, rl.x, rl.y))
			{
				eNum = 12;
				goto out;
			}

			if (neuik_Element_RedrawBackground(tElem, rlMod, maskMap))
			{
				eNum = 9;
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Render the contained Element                                           */
	/*------------------------------------------------------------------------*/
	if (cont->elems == NULL) goto out;
	elem = cont->elems[0];

	if (elem == NULL) goto out;

	if (!NEUIK_Element_IsShown(elem)) goto out;

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
		if (neuik_Element_GetMinSize__Transformer(tElem, &rs))
		{
			eNum = 3;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Check for and apply rotation if necessary.                         */
		/*--------------------------------------------------------------------*/
		if (trans->rotation ==    0.0 || 
			trans->rotation ==  180.0 || 
			trans->rotation == -180.0 ||
			trans->rotation ==  360.0 ||
			trans->rotation == -360.0)
		{
			/*----------------------------------------------------------------*/
			/* This resulting element minimum size will be the same as the    */
			/* normal unrotated element.                                      */
			/*----------------------------------------------------------------*/
			if (eCfg->HFill)
			{
				/* The element fills the window horizontally */
				rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
			}
			if (eCfg->VFill)
			{
				/* The element fills the window vertically  */
				rs.h = rSize->h - (eCfg->PadTop  + eCfg->PadBottom);
			}
		}
		else if (
			trans->rotation ==  90.0  ||
			trans->rotation == -90.0  ||
			trans->rotation ==  270.0 ||
			trans->rotation == -270.0)
		{
			/*----------------------------------------------------------------*/
			/* This resulting element minimum size will have the width and    */
			/* height swapped compared to its values in the unrotated state.  */
			/*----------------------------------------------------------------*/
			if (eCfg->HFill)
			{
				/* The element fills the window horizontally */
				rs.w = rSize->h - (eCfg->PadTop + eCfg->PadBottom);
			}
			if (eCfg->VFill)
			{
				/* The element fills the window vertically  */
				rs.h = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
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

	/*------------------------------------------------------------------------*/
	/* Check for and apply rotation if necessary.                             */
	/*------------------------------------------------------------------------*/
	if (trans->rotation ==    0.0 ||
		trans->rotation ==  360.0 ||
		trans->rotation == -360.0)
	{
		/*--------------------------------------------------------------------*/
		/* No effective rotation                                              */
		/*--------------------------------------------------------------------*/
		rsOrig = rs;
		destRect.x = eCfg->PadLeft;
		destRect.y = eCfg->PadTop;
		destRect.w = rs.w;
		destRect.h = rs.h;
	}
	else if (trans->rotation ==  180.0 || 
		     trans->rotation == -180.0)
	{
		/*--------------------------------------------------------------------*/
		/* Rotated by 180 degrees (turned upside-down)                        */
		/*--------------------------------------------------------------------*/
		rsOrig = rs;
		destRect.x = eCfg->PadRight;
		destRect.y = eCfg->PadBottom;
		destRect.w = rs.w;
		destRect.h = rs.h;
	}
	else if (trans->rotation ==   90.0 || 
		     trans->rotation == -270.0)
	{
		/*--------------------------------------------------------------------*/
		/* Rotated by 90 degrees (resting on its right side).                 */
		/*--------------------------------------------------------------------*/
		rsOrig.w = rs.h;
		rsOrig.h = rs.w;
		destRect.x = eCfg->PadBottom;
		destRect.y = eCfg->PadLeft;
		destRect.w = rs.h;
		destRect.h = rs.w;
	}
	else if (trans->rotation ==  270.0 || 
		     trans->rotation ==  -90.0)
	{
		/*--------------------------------------------------------------------*/
		/* Rotated by 270 degrees (resting on its left side).                 */
		/*--------------------------------------------------------------------*/
		rsOrig.w = rs.h;
		rsOrig.h = rs.w;
		destRect.x = eCfg->PadTop;
		destRect.y = eCfg->PadRight;
		destRect.w = rs.h;
		destRect.h = rs.w;
	}

	rl.x = (eBase->eSt.rLoc).x + destRect.x;
	rl.y = (eBase->eSt.rLoc).y + destRect.y;
	rlRel.x = destRect.x;
	rlRel.y = destRect.y;

	neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

	/*------------------------------------------------------------------------*/
	/* Calculate an updated RenderLoc modifier.                               */
	/*------------------------------------------------------------------------*/
	rlModNext.x = -eBase->eSt.rLoc.x;
	rlModNext.y = -eBase->eSt.rLoc.y;

	if (rlMod != NULL)
	{
		rlModNext.x += rlMod->x;
		rlModNext.y += rlMod->y;
	}

	if (neuik_Element_NeedsRedraw(elem))
	{
		/*--------------------------------------------------------------------*/
		/* Check to see if the requested draw size of the element has changed */
		/*--------------------------------------------------------------------*/
		if (eBase->eSt.rSize.w != eBase->eSt.rSizeOld.w  ||
			eBase->eSt.rSize.h != eBase->eSt.rSizeOld.h)
		{
			/*----------------------------------------------------------------*/
			/* This will create a new SDL_Surface & SDL_Renderer; also it     */
			/* will free old ones if they are allocated.                      */
			/*----------------------------------------------------------------*/
			if (neuik_Element_Resize(trans, *rSize) != 0)
			{
				eNum = 5;
				goto out;
			}
		}
		surf = eBase->eSt.surf;
		rend = eBase->eSt.rend;

		if (neuik_Element_RenderRotate(
				elem, &rsOrig, &rlModNext, rend, mock, trans->rotation))
		{
			eNum = 7;
			goto out;
		}
		SDL_RenderPresent(rend);

		tex = SDL_CreateTextureFromSurface(xRend, surf);
		if (tex == NULL)
		{
			eNum = 8;
			goto out;
		}

		rl = eBase->eSt.rLoc;

		destRect.x = rl.x;
		destRect.y = rl.y;
		destRect.w = eBase->eSt.rSize.w;
		destRect.h = eBase->eSt.rSize.h;

		if (!mock)
		{
			SDL_RenderCopy(xRend, tex, NULL, &destRect);
		}
	}
out:
	if (!mock) eBase->eSt.doRedraw = 0;
	if (maskMap != NULL) neuik_Object_Free(maskMap);

	ConditionallyDestroyTexture(&tex);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__Transformer
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__Transformer(
	NEUIK_Element   tElem, 
	SDL_Event     * ev)
{
	int                    ctr        = 0;
	int                    transform  = 0;
	float                  xFrac      = 0.0;
	float                  yFrac      = 0.0;
	neuik_EventState       evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
	NEUIK_Element          elem;
	RenderLoc              eLoc;
	RenderLoc              evPos;
	RenderSize             eSz;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_Container      * cBase      = NULL;
	NEUIK_Transformer    * trans      = NULL;
	SDL_MouseMotionEvent * mouseMotEv;
	SDL_MouseButtonEvent * mouseButEv;
	SDL_Event            * evActive   = NULL;
	SDL_Event              evTr; /* Transformed mouse event */

	NEUIK_ElementBase    * childEBase      = NULL;

	if (!neuik_Object_IsClass(tElem, neuik__Class_Transformer))
	{
		goto out;
	}
	trans = (NEUIK_Transformer *)tElem;

	if (neuik_Object_GetClassObject_NoError(
		tElem, neuik__Class_Element, (void**)&eBase)) goto out;

	if (neuik_Object_GetClassObject_NoError(
		tElem, neuik__Class_Container, (void**)&cBase)) goto out;

	/*------------------------------------------------------------------------*/
	/* Check if there is a mouse event which needs to be transformed.         */
	/*------------------------------------------------------------------------*/
	eLoc = eBase->eSt.rLoc;
	eSz  = eBase->eSt.rSize;

	switch (ev->type)
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		mouseButEv = (SDL_MouseButtonEvent*)(ev);
		if (mouseButEv->y >= eLoc.y && mouseButEv->y <= eLoc.y + eSz.h)
		{
			if (mouseButEv->x >= eLoc.x && mouseButEv->x <= eLoc.x + eSz.w)
			{
				/* This mouse button action is within the transformer */
				transform = 1;
				evTr.button.type      = mouseButEv->type;
				evTr.button.timestamp = mouseButEv->timestamp;
				evTr.button.windowID  = mouseButEv->windowID;
				evTr.button.which     = mouseButEv->which;
				evTr.button.button    = mouseButEv->button;
				evTr.button.state     = mouseButEv->state;
				evTr.button.clicks    = mouseButEv->clicks;
				evTr.button.x         = mouseButEv->x;
				evTr.button.y         = mouseButEv->y;
				evPos.x = mouseButEv->x;
				evPos.y = mouseButEv->y;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mouseMotEv = (SDL_MouseMotionEvent*)(ev);

		if (mouseMotEv->y >= eLoc.y && mouseMotEv->y <= eLoc.y + eSz.h)
		{
			if (mouseMotEv->x >= eLoc.x && mouseMotEv->x <= eLoc.x + eSz.w)
			{
				/* This mouse motion is within the transformer */
				transform = 1;
				evTr.motion.type      = mouseMotEv->type;
				evTr.motion.timestamp = mouseMotEv->timestamp;
				evTr.motion.windowID  = mouseMotEv->windowID;
				evTr.motion.which     = mouseMotEv->which;
				evTr.motion.x         = mouseMotEv->x;
				evTr.motion.y         = mouseMotEv->y;
				evTr.motion.xrel      = mouseMotEv->xrel;
				evTr.motion.yrel      = mouseMotEv->yrel;
				evPos.x = mouseMotEv->x;
				evPos.y = mouseMotEv->y;
			}
		}
		break;
	}

	evActive = ev;
	if (transform)
	{
		/*--------------------------------------------------------------------*/
		/* Apply the appropriate transformation(s) to the mouse event.        */
		/*--------------------------------------------------------------------*/
		evActive = &evTr;
		if (neuik__Report_Debug)
		{
			printf("eLoc = [%d, %d]\n", eLoc.x, eLoc.y);
			printf("eSz = [%d, %d]\n", eSz.w, eSz.h);
			printf("evPos0 = [%d, %d]\n", evPos.x, evPos.y);
		}
		
		/*--------------------------------------------------------------------*/
		/* Apply scaling transformation (if necessary).                       */
		/*--------------------------------------------------------------------*/
		#pragma message("[TODO] `neuik_Element_CaptureEvent__Transformer` Scaling Transform")

		/*--------------------------------------------------------------------*/
		/* Apply rotation transformation (if necessary).                      */
		/*--------------------------------------------------------------------*/
		if (trans->rotation ==    0.0 || 
			trans->rotation ==  360.0 ||
			trans->rotation == -360.0)
		{
			/*----------------------------------------------------------------*/
			/* This rotation results in an unrotated element. Do nothing.     */
			/*----------------------------------------------------------------*/
		}
		else if (
			trans->rotation ==  180.0 || 
			trans->rotation == -180.0)
		{
			/*----------------------------------------------------------------*/
			/* The x & y-axis positions are flipped within this transformer.  */
			/*----------------------------------------------------------------*/
			evPos.x = eLoc.x + (eLoc.x + eSz.w) - evPos.x;
			evPos.y = eLoc.y + (eLoc.y + eSz.h) - evPos.y;
		}
		else if (
			trans->rotation ==  90.0  ||
			trans->rotation == -270.0)
		{
			// yFrac = 1.0 - ((float)(eLoc.y) + (float)(eSz.w) - (float)(evPos.y))/(float)(eSz.w);
			// xFrac = 1.0 + ((float)(eLoc.x) - (float)(evPos.x))/(float)(eSz.w);

			// evPos.x = eLoc.x + (int)((float)(eSz.h)*yFrac);
			// evPos.y = eLoc.y + (int)((float)(eSz.w)*xFrac); // correct

			yFrac = (float)(evPos.y - eLoc.y)/(float)(eSz.h);
			xFrac = ((float)(evPos.x) - (float)(eLoc.x))/(float)(eSz.w);


			// yFrac = 1.0 - ((float)(eLoc.y) + (float)(eSz.h) - (float)(evPos.y))/(float)(eSz.h);
			// xFrac = 1.0 + ((float)(eLoc.x) - (float)(evPos.x))/(float)(eSz.w);

			evPos.x = eLoc.x + (int)((float)(eSz.w)*(yFrac));
			evPos.y = eLoc.y + (int)((float)(eSz.h)*(1.0 - xFrac)); // correct

			if (neuik__Report_Debug)
			{
				printf("evPosf = [%d, %d]\n", evPos.x, evPos.y);
			}
		}
		else if (
			trans->rotation == -90.0  ||
			trans->rotation ==  270.0)
		{
			/*--------------------------------------------------------------------*/
			/* This resulting element minimum size will have the width and height */
			/* swapped compared to its values in the unrotated state              */
			/*--------------------------------------------------------------------*/
			// rs.w = rSize->h;
			// rs.h = rSize->w;
			// (*rSize) = rs;
		}

		/*--------------------------------------------------------------------*/
		/* Store the transformed mouse position into the transformed event.   */
		/*--------------------------------------------------------------------*/
		switch (ev->type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			/* This mouse button action is within the transformer */
			evTr.button.x = evPos.x;
			evTr.button.y = evPos.y;
			break;
		case SDL_MOUSEMOTION:
			evTr.motion.x = evPos.x;
			evTr.motion.y = evPos.y;
			break;
		}
	}

	if (cBase->elems != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			elem = cBase->elems[ctr];
			if (elem == NULL) break;

			if (!NEUIK_Element_IsShown(elem)) continue;

			if (neuik_Object_GetClassObject_NoError(
				elem, neuik__Class_Element, (void**)&childEBase)) goto out;
			eLoc = childEBase->eSt.rLoc;
			eSz  = childEBase->eSt.rSize;

			// if (neuik__Report_Debug)
			// {
			// 	printf("childELoc = [%d, %d]\n", eLoc.x, eLoc.y);
			// 	printf("childESz = [%d, %d]\n", eSz.w, eSz.h);
			// }

			evCaputred = neuik_Element_CaptureEvent(elem, evActive);
			if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
			{
				goto out;
			}
			if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
			{
				neuik_Element_SetActive(tElem, 1);
				goto out;
			}
		}
	}
out:
	return evCaputred;
}
