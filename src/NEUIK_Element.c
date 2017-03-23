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
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "NEUIK_colors.h"
#include "NEUIK_error.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Event.h"
#include "NEUIK_Window.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Element(void ** elemPtr);
int neuik_Object_Free__Element(void * elemPtr);

int neuik_NewElement(NEUIK_Element ** elemPtr);
int neuik_Element_Free(NEUIK_Element ** elemPtr);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Element_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Element,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Element,
};

NEUIK_ElementBackground neuik_default_ElementBackground = {
	NEUIK_BGSTYLE_SOLID, /* style to use when element is unselected */
	NEUIK_BGSTYLE_SOLID, /* style to use when element is selected */
	NEUIK_BGSTYLE_SOLID, /* style to use when element is hovered */
	COLOR_LLGRAY,        /* solid color to use under normal condtions */
	COLOR_LLGRAY,        /* solid color to use when selected */
	COLOR_LLGRAY,        /* solid color to use being hovered over */
	'v',                 /* direction to use for the gradient (`v` or `h`) */
	NULL,                /* color gradient to use under normal condtions */
	NULL,                /* color gradient to use when selected */
	NULL,                /* color gradient to use being hovered over */
};

NEUIK_ElementConfig neuik_default_ElementConfig = {
	 1.0,                  /* Scale Factor : 0 = Doesn't stretch; other value does */
	 1.0,                  /* Scale Factor : 0 = Doesn't stretch; other value does */
	 0,                    /* Element fills Vertically   : 1 = true; 0 = false */
	 0,                    /* Element fills Horizontally : 1 = true; 0 = false */
	NEUIK_VJUSTIFY_CENTER, /* Vertical   justification */
	NEUIK_HJUSTIFY_CENTER, /* Horizontal justification */
	 0,                    /* Pad the top of the element with transparent space */
	 0,                    /* Pad the bottom of the element with transparent space */
	 0,                    /* Pad the left of the element with transparent space */
	 0,                    /* Pad the right of the element with transparent space */
	-1,                    /* Minimum Width */
	-1,                    /* Maximum Width */
	-1,                    /* Minimum Height */
	-1,                    /* Maximum Height */
	 1,                    /* Element is being shown */
};

NEUIK_ElementState neuik_default_ElementState = {
	1,                       /* Element needs to be redrawn */
	0,                       /* Element does not have Window focus */
	0,                       /* Element does not require an alpha blending */
	0,                       /* Element is not active by default */
	NEUIK_FOCUSSTATE_NORMAL, /* Element is unselected */
	NULL,                    /* (NEUIK_Window *) containing window */
	NULL,                    /* (NEUIK_Element)  parent element */
	NULL,                    /* (NEUIK_Element)  popup element */
	NULL,                    /* (SDL_Texture *)  rendered texture */
	NULL,                    /* (SDL_Surface *)  surface for this element */
	NULL,                    /* (SDL_Renderer *) renderer for this surface */
	NULL,                    /* (SDL_Renderer *) last used extRenderer */
	{0, 0},                  /* render size */
	{-1, -1},                /* old render size */
	{0, 0},                  /* render loc  */
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Element
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_Element()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Element";
	static char  * errMsgs[]  = {"",                   // [0] no error
		"NEUIK library must be initialized first.",    // [1]
		"Failed to register `Element` object class .", // [2]
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
		"NEUIK_Element",                 // className
		"The basic NEUIK_Object Class.", // classDescription
		neuik__Set_NEUIK,                // classSet
		NULL,                            // superClass
		&neuik_Element_BaseFuncs,        // baseFuncs
		NULL,                            // classFuncs XXXXX
		&neuik__Class_Element))          // newClass
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
 *  Name:          neuik_NewElement
 *
 *  Description:   Allocate memory and set default values for Element.
 *
 *                 An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__Element(
	void ** elemPtr)
{
	int                 eNum       = 0;
	NEUIK_ElementBase * elem       = NULL;
	static char         funcName[] = "neuik_NewElement";
	static char       * errMsgs[]  = {"",           // [0] no error
		"Output Argument `elemPtr` is NULL.",       // [1]
		"Failure to allocate memory.",              // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.", // [3]
	};

	if (elemPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*elemPtr) = (NEUIK_Element*) malloc(sizeof(NEUIK_ElementBase));
	elem = (NEUIK_ElementBase*)(*elemPtr);
	if (elem == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Allocation successful; set default values.                             */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Element, 
			NULL,
			&(elem->objBase)))
	{
		eNum = 3;
		goto out;
	}

	elem->eFT  = NULL; 
	elem->eCfg = neuik_default_ElementConfig;
	elem->eSt  = neuik_default_ElementState;
	elem->eBg  = neuik_default_ElementBackground;
	elem->eCT  = NEUIK_NewCallbackTable();
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


int neuik_Element_SetFuncTable(
	NEUIK_Element             elem, 
	NEUIK_Element_FuncTable * eFT)
{
	int                 eNum       = 0;
	NEUIK_ElementBase * eBase      = NULL; 
	static char         funcName[] = "neuik_Element_SetFuncTable";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Argument `eFT` is NULL.",                                       // [2]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}
	if (eFT == NULL)
	{
		eNum = 2;
		goto out;
	}

	eBase->eFT = eFT;
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
 *  Name:          neuik_Element_Free
 *
 *  Description:   Free memory allocated for this object and NULL out pointer.
 *
 *                 An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__Element(
	void * elemPtr)
{
	int                 eNum       = 0;
	NEUIK_ElementBase * eBase      = NULL; 
	static char         funcName[] = "neuik_Element_Free";
	static char       * errMsgs[]  = {"",                                   // [0] no error
		"Argument `elemPtr` is NULL.",                                      // [1]
		"Argument `elemPtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
	};

	if (elemPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(elemPtr, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	free(eBase);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


NEUIK_ElementConfig * neuik_Element_GetConfig(NEUIK_Element elem)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return NULL;
	}

	return &(eBase->eCfg);
}

NEUIK_ElementConfig neuik_GetDefaultElementConfig()
{
	return neuik_default_ElementConfig;
}

void neuik_Element_Configure_capture_segv(
	int sig_num)
{
	static char funcName[] = "NEUIK_Element_Configure";
	static char errMsg[] = 
		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

	NEUIK_RaiseError(funcName, errMsg);
	NEUIK_BacktraceErrors();
	exit(1);
}

/* This list of named sets must be terminated by a NULL pointer */
int NEUIK_Element_Configure(
	NEUIK_Element    elem,
	const char     * set0,
	...)
{
	int                    ctr;
	int                    nCtr;
	int                    isBool;
	int                    boolVal    = 0;
	int                    doRedraw   = 0;
	int                    typeMixup;
	va_list                args;
	char                 * strPtr     = NULL;
	char                 * name       = NULL;
	char                 * value      = NULL;
	const char           * set        = NULL;
	char                   buf[4096];
	NEUIK_ElementBase    * eBase      = NULL; 
	NEUIK_ElementConfig  * eCfg       = NULL;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char          * boolNames[] = {
		"FillAll",
		"HFill",
		"VFill",
		"Show",
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	static char          * valueNames[] = {
		"HScale",
		"VScale",
		"HJustify",
		"VJustify",
		"PadLeft",
		"PadRight",
		"PadTop",
		"PadBottom",
		"PadAll",
		NULL,
	};
	static char            funcName[] = "NEUIK_Element_Configure";
	static char          * errMsgs[] = {"",                                 // [ 0] no error
		"Argument `elemPtr` caused `neuik_Object_GetClassObject` to fail.", // [ 1]
		"NamedSet.name is NULL, skipping.",                                 // [ 2]
		"NamedSet.name is blank, skipping.",                                // [ 3]
		"NamedSet.name type unknown, skipping.",                            // [ 4]
		"`name=value` string is too long.",                                 // [ 5]
		"Set string is empty.",                                             // [ 6]
		"HJustify value is invalid.",                                       // [ 7]
		"VJustify value is invalid.",                                       // [ 8]
		"BoolType name unknown, skipping.",                                 // [ 9]
		"Invalid `name=value` string.",                                     // [10]
		"ValueType name used as BoolType, skipping.",                       // [11]
		"BoolType name used as ValueType, skipping.",                       // [12]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
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

		#ifndef NO_NEUIK_SIGNAL_TRAPPING
			signal(SIGSEGV, neuik_Element_Configure_capture_segv);
		#endif

		if (strlen(set) > 4095)
		{
			#ifndef NO_NEUIK_SIGNAL_TRAPPING
				signal(SIGSEGV, NULL);
			#endif
			NEUIK_RaiseError(funcName, errMsgs[5]);
			set = va_arg(args, const char *);
			continue;
		}
		else
		{
			#ifndef NO_NEUIK_SIGNAL_TRAPPING
				signal(SIGSEGV, NULL);
			#endif
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

		if (isBool)
		{
			if (!strcmp("VFill", name))
			{
				eCfg->VFill = boolVal;
				doRedraw = 1;
			}
			else if (!strcmp("HFill", name))
			{
				eCfg->HFill = boolVal;
				doRedraw = 1;
			}
			else if (!strcmp("FillAll", name))
			{
				eCfg->HFill = boolVal;
				eCfg->VFill = boolVal;
				doRedraw = 1;
			}
			else if (!strcmp("Show", name))
			{
				eCfg->Show = boolVal;
				doRedraw = 1;
			}
			else
			{
				typeMixup = 0;
				for (nCtr = 0;; nCtr++)
				{
					if (valueNames[nCtr] == NULL) break;

					if (!strcmp(valueNames[nCtr], name))
					{
						typeMixup = 1;
						break;
					}
				}

				if (typeMixup)
				{
					/* A value type was mistakenly used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[11]);
				}
				else
				{
					/* An unsupported name was used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[9]);
				}
			}
		}
		else
		{
			if (name == NULL)
			{
				NEUIK_RaiseError(funcName, errMsgs[2]);
			}
			else if (name[0] == 0)
			{
				NEUIK_RaiseError(funcName, errMsgs[3]);
			}
			else if (!strcmp("VScale", name))
			{
				eCfg->VScale = atof(value);
				doRedraw = 1;
			}
			else if (!strcmp("HScale", name))
			{
				eCfg->HScale = atof(value);
				doRedraw = 1;
			}
			else if (!strcmp("HJustify", name))
			{
				if (!strcmp("left", value))
				{
					eCfg->HJustify = NEUIK_HJUSTIFY_LEFT;
					doRedraw = 1;
				}
				else if (!strcmp("center", value))
				{
					eCfg->HJustify = NEUIK_HJUSTIFY_CENTER;
					doRedraw = 1;
				}
				else if (!strcmp("right", value))
				{
					eCfg->HJustify = NEUIK_HJUSTIFY_RIGHT;
					doRedraw = 1;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[7]);
				}
			}
			else if (!strcmp("VJustify", name))
			{
				if (!strcmp("top", value))
				{
					eCfg->VJustify = NEUIK_VJUSTIFY_TOP;
					doRedraw = 1;
				}
				else if (!strcmp("center", value))
				{
					eCfg->VJustify = NEUIK_VJUSTIFY_CENTER;
					doRedraw = 1;
				}
				else if (!strcmp("bottom", value))
				{
					eCfg->VJustify = NEUIK_VJUSTIFY_BOTTOM;
					doRedraw = 1;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[8]);
				}
			}
			else if (!strcmp("PadLeft", name))
			{
				eCfg->PadLeft = atoi(value);
				doRedraw = 1;
			}
			else if (!strcmp("PadRight", name))
			{
				eCfg->PadRight = atoi(value);
				doRedraw = 1;
			}
			else if (!strcmp("PadTop", name))
			{
				eCfg->PadTop = atoi(value);
				doRedraw = 1;
			}
			else if (!strcmp("PadBottom", name))
			{
				eCfg->PadBottom = atoi(value);
				doRedraw = 1;
			}
			else if (!strcmp("PadAll", name))
			{
				eCfg->PadLeft   = atoi(value);
				eCfg->PadRight  = eCfg->PadLeft;
				eCfg->PadTop    = eCfg->PadLeft;
				eCfg->PadBottom = eCfg->PadLeft;
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
		}

		/* before starting */
		set = va_arg(args, const char *);
	}
	va_end(args);

	if (doRedraw) neuik_Element_RequestRedraw(elem);

	return 0;
}


NEUIK_ElementState neuik_GetDefaultElementState()
{
	return neuik_default_ElementState;
}

void neuik_SetDefaultElementConfig(NEUIK_ElementConfig eCfg)
{
	neuik_default_ElementConfig = eCfg;
}


int neuik_Element_GetMinSize(
	NEUIK_Element    elem, 
	RenderSize     * rSize)
{
	int                  eNum       = 0;
	NEUIK_ElementBase  * eBase      = NULL;
	static int           nRecurse   = 0; /* number of times recursively called */
	static char          funcName[] = "neuik_Element_GetMinSize";
	static char        * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Element Function Table is NULL (missing or not set).",          // [2]
		"Failure in implementation of function `GetMinSize`.",           // [3]
	};

	nRecurse++;
	if (nRecurse > NEUIK_MAX_RECURSION)
	{
		/*--------------------------------------------------------------------*/
		/* This is likely a case of appears to be runaway recursion; report   */
		/* an error to the user.                                              */
		/*--------------------------------------------------------------------*/
		neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
		goto out;
	}

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (eBase->eFT == NULL)
	{
		eNum = 2;
		goto out;
	}

	if ((eBase->eFT->GetMinSize)(elem, rSize))
	{
		if (neuik_HasFatalError())
		{
			eNum = 1;
			goto out2;
		}
		eNum = 3;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
out2:
	nRecurse--;

	return eNum;
}

int neuik_Element_GetLocation(
	NEUIK_Element   elem, 
	RenderLoc     * rLoc)
{
	int                  eNum      = 0;
	NEUIK_ElementBase  * eBase     = NULL;
	static char          funcName[] = "neuik_Element_GetLocation";
	static char        * errMsgs[] = {"",                                // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	(*rLoc) = eBase->eSt.rLoc;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


int 
	NEUIK_Element_SetSize(NEUIK_Element elem, RenderSize * rSize);


SDL_Texture * neuik_Element_Render(
	NEUIK_Element    elem, 
	RenderSize     * rSize, 
	SDL_Renderer   * xRend)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return NULL;
	}
	if (eBase->eFT == NULL) return NULL;
	if (eBase->eFT->Render == NULL) return NULL;

	return (eBase->eFT->Render)(elem, rSize, xRend);
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent    (virtual-function)
 *
 *  Description:   Pass an SDL_Event to an object and see if it was captured.
 *
 *                 This operation of this function may be redefined by a 
 *                 Element subclass.
 *
 *  Returns:       1 if the event was captured, 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent(
	NEUIK_Element   elem, 
	SDL_Event     * ev)
{
	neuik_EventState     captured = 0;
	neuik_EventState  (* funcImp) (NEUIK_Element, SDL_Event*);
	NEUIK_ElementBase  * eBase;

	/*------------------------------------------------------------------------*/
	/* Try the standard element implementation                                */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return 0;
	}

	if (eBase->eFT != NULL)
	{
		if (eBase->eFT->CaptureEvent != NULL)
		{
			captured = (eBase->eFT->CaptureEvent)(elem, ev);
			goto out;
		}
	}

	// if (eBase->eFT == NULL) return 0;
	// if (eBase->eFT->CaptureEvent == NULL) return 0;

	/*------------------------------------------------------------------------*/
	/* ELSE: Try and use an implemented virtual function (if implemented)     */
	/*------------------------------------------------------------------------*/
	funcImp = neuik_VirtualFunc_GetImplementation(
		neuik_Element_vfunc_CaptureEvent, elem);
	if (funcImp != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* A virtual reimplementation is availible for this function          */
		/*--------------------------------------------------------------------*/
		captured = (*funcImp)(elem, ev);
	}


	/*----------------------------------------------------------------*/
	/* If an element does not have a capture event function, it can   */
	/* not capture events.                                            */
	/*----------------------------------------------------------------*/

	// return (eBase->eFT->CaptureEvent)(elem, ev);
out:
	return captured;
}


void neuik_Element_StoreSizeAndLocation(
	NEUIK_Element elem, 
	RenderSize    rSize, 
	RenderLoc     rLoc)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return;
	}

	eBase->eSt.rSize = rSize;
	eBase->eSt.rLoc  = rLoc;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorGradient
 *
 *  Description:   Set the specified background style to a color gradient.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorGradient(
	NEUIK_Element   elem,
	const char    * styleName,
	char            direction,
	const char    * colorStop0,
	...)
{
	int                   ns; /* number of items from sscanf */
	int                   ctr;
	int                   eNum       = 0;
	int                   nStops     = 1;
	NEUIK_Color           clr;
	float                 frac;
	char                  buf[4096];
	va_list               args;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ColorStop   *** cstops     = NULL; /* pointer to the active colorstops */
	NEUIK_ColorStop     * cs;
	const char          * cs_str     = NULL;
	static char           funcName[] = "NEUIK_Element_SetBackgroundColorGradient";
	static char         * errMsgs[]  = {"",                               // [ 0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.",  // [ 1]
		"Argument `styleName` is NULL.",                                  // [ 2]
		"Argument `styleName` is blank.",                                 // [ 3]
		"Argument `styleName` has unexpected value.",                     // [ 4]
		"Failure in function `neuik_Element_RequestRedraw`.",             // [ 5]
		"Argument `direction` has unexpected value.",                     // [ 6]
		"`colorStop` string is too long.",                                // [ 7]
		"`colorStop` string invalid; should be comma separated RGBAF.",   // [ 8]
		"`colorStop` string invalid; RGBA value range is 0-255."          // [ 9]
		"`colorStop` string invalid; fraction value range is 0.0-1.0."    // [10]
		"Failed to allocate memory.",                                     // [11]
		"Failed to reallocate memory.",                                   // [12]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Read in the styleName and set the pointer to the appropriate style     */
	/*------------------------------------------------------------------------*/
	if (styleName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (styleName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("normal", styleName))
	{
		eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_GRADIENT;
		cstops = &(eBase->eBg.gradient_normal);
	}
	else if (!strcmp("selected", styleName))
	{
		eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_GRADIENT;
		cstops = &(eBase->eBg.gradient_selected);
	}
	else if (!strcmp("hovered", styleName))
	{
		eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_GRADIENT;
		cstops = &(eBase->eBg.gradient_hover);
	}
	else
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Free and NULL out the currently allocated ColorStops (if allocated).   */
	/*------------------------------------------------------------------------*/
	if (*cstops != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			if ((*cstops)[ctr] == NULL)
			{
				break;
			}
			free((*cstops)[ctr]);
		}
		free(*cstops);
		(*cstops) = NULL;
	}

	/*------------------------------------------------------------------------*/
	/* Set the gradient direction.                                            */
	/*------------------------------------------------------------------------*/
	switch (direction)
	{
		case 'h':
		case 'v':
			eBase->eBg.gradient_dirn = direction;
			break;
		default:
			/* Unsupported gradient direction */
			eNum = 6;
			goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Iterate over the list of colorstops.                                   */
	/*------------------------------------------------------------------------*/
	cs_str = colorStop0;

	va_start(args, colorStop0);

	for (ctr = 0;; ctr++)
	{
		if (cs_str == NULL)
		{
			(*cstops)[ctr] = NULL;
			break;
		}

		// #ifndef NO_NEUIK_SIGNAL_TRAPPING
		// 	signal(SIGSEGV, neuik_Element_Configure_capture_segv);
		// #endif

		if (strlen(cs_str) > 4095)
		{
			// #ifndef NO_NEUIK_SIGNAL_TRAPPING
			// 	signal(SIGSEGV, NULL);
			// #endif
			eNum = 7;
			goto out;
		}
		// #ifndef NO_NEUIK_SIGNAL_TRAPPING
		// 	signal(SIGSEGV, NULL);
		// #endif
		strcpy(buf, cs_str);
		/*--------------------------------------------------------------------*/
		/* Check for empty value errors.                                      */
		/*--------------------------------------------------------------------*/
		if (buf[0] == '\0')
		{
			NEUIK_RaiseError(funcName, errMsgs[8]);
			continue;
		}

		ns = sscanf(buf, "%d,%d,%d,%d,%f", &clr.r, &clr.g, &clr.b, &clr.a, &frac);
		/*--------------------------------------------------------------------*/
		/* Check for EOF, incorrect # of values, & out of range vals.         */
		/*--------------------------------------------------------------------*/
		if (ns == EOF || ns < 5)
		{
			NEUIK_RaiseError(funcName, errMsgs[8]);
			continue;
		}
		if (clr.r < 0 || clr.r > 255 ||
			clr.g < 0 || clr.g > 255 ||
			clr.b < 0 || clr.b > 255 ||
			clr.a < 0 || clr.a > 255)
		{
			NEUIK_RaiseError(funcName, errMsgs[9]);
			continue;
		}
		if (frac < 0.0 || frac > 1.0)
		{
			NEUIK_RaiseError(funcName, errMsgs[10]);
			continue;
		}

		/*--------------------------------------------------------------------*/
		/* Allocate/Reallocate memory for the colorStop array                 */
		/*--------------------------------------------------------------------*/
		if (*cstops != NULL)
		{
			/*----------------------------------------------------------------*/
			/* Reallocate memory for the colorStop array                      */
			/*----------------------------------------------------------------*/
			(*cstops) = (NEUIK_ColorStop **)realloc((*cstops),
				(nStops + 1)*sizeof(NEUIK_ColorStop*));
			if ((*cstops) == NULL)
			{
				eNum = 12;
				goto out;
			}
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* Allocate memory for the colorStop array                        */
			/*----------------------------------------------------------------*/
			(*cstops) = (NEUIK_ColorStop **)malloc(2*sizeof(NEUIK_ColorStop*));
			if ((*cstops) == NULL)
			{
				eNum = 11;
				goto out;
			}
		}

		/*--------------------------------------------------------------------*/
		/* Allocate memory for this new colorStop                             */
		/*--------------------------------------------------------------------*/
		(*cstops)[nStops - 1] = (NEUIK_ColorStop *)malloc(sizeof(NEUIK_ColorStop));
		cs = (*cstops)[nStops - 1];
		if (cs == NULL)
		{
			eNum = 11;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Set the value of the new ColorStop                                 */
		/*--------------------------------------------------------------------*/
		cs->color.r = clr.r;
		cs->color.g = clr.g;
		cs->color.b = clr.b;
		cs->color.a = clr.a;
		cs->frac    = frac;

		/* before starting */
		cs_str = va_arg(args, const char *);
		nStops++;
	}
	va_end(args);

	if (neuik_Element_RequestRedraw(elem))
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


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorSolid
 *
 *  Description:   Set the specified background style to a solid color.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorSolid(
	NEUIK_Element   elem,
	const char    * styleName,
	unsigned char   r,
	unsigned char   g,
	unsigned char   b,
	unsigned char   a)
{
	int                 eNum       = 0;
	NEUIK_ElementBase * eBase      = NULL;
	NEUIK_Color       * aClr       = NULL;
	static char         funcName[] = "NEUIK_Element_SetBackgroundColorSolid";
	static char       * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Argument `styleName` is NULL.",                                 // [2]
		"Argument `styleName` is blank.",                                // [3]
		"Argument `styleName` has unexpected value.",                    // [4]
		"Failure in function `neuik_Element_RequestRedraw`.",            // [5]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (styleName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (styleName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("normal", styleName))
	{
		eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_SOLID;
		aClr = &(eBase->eBg.solid_normal);
	}
	else if (!strcmp("selected", styleName))
	{
		eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_SOLID;
		aClr = &(eBase->eBg.solid_selected);
	}
	else if (!strcmp("hovered", styleName))
	{
		eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_SOLID;
		aClr = &(eBase->eBg.solid_hover);
	}
	else
	{
		eNum = 4;
		goto out;
	}

	aClr->r = r;
	aClr->g = g;
	aClr->b = b;
	aClr->a = a;
	if (neuik_Element_RequestRedraw(elem))
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


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBackgroundColorTransparent
 *
 *  Description:   Set the specified background style to transparent.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBackgroundColorTransparent(
	NEUIK_Element   elem,
	const char    * styleName)
{
	int                 eNum       = 0;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "NEUIK_Element_SetBackgroundColorTransparent";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Argument `styleName` is NULL.",                                 // [2]
		"Argument `styleName` is blank.",                                // [3]
		"Argument `styleName` has unexpected value.",                    // [4]
		"Failure in function `neuik_Element_RequestRedraw`.",            // [5]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (styleName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (styleName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("normal", styleName))
	{
		if (eBase->eBg.bgstyle_normal != NEUIK_BGSTYLE_TRANSPARENT)
		{
			eBase->eBg.bgstyle_normal = NEUIK_BGSTYLE_TRANSPARENT;
			if (neuik_Element_RequestRedraw(elem))
			{
				eNum = 5;
				goto out;
			}
		}
	}
	else if (!strcmp("selected", styleName))
	{
		if (eBase->eBg.bgstyle_selected != NEUIK_BGSTYLE_TRANSPARENT)
		{
			eBase->eBg.bgstyle_selected = NEUIK_BGSTYLE_TRANSPARENT;
			if (neuik_Element_RequestRedraw(elem))
			{
				eNum = 5;
				goto out;
			}
		}
	}
	else if (!strcmp("hovered", styleName))
	{
		if (eBase->eBg.bgstyle_hover != NEUIK_BGSTYLE_TRANSPARENT)
		{
			eBase->eBg.bgstyle_hover = NEUIK_BGSTYLE_TRANSPARENT;
			if (neuik_Element_RequestRedraw(elem))
			{
				eNum = 5;
				goto out;
			}
		}
	}
	else
	{
		eNum = 4;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetCallback
 *
 *  Description:   Set the function and arguments for the named callback event.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetCallback(
	NEUIK_Element    elem,    /* the element to se the callback for */
	const char     * cbName,  /* the name of the callback to set */
	void           * cbFunc,  /* the function to use for the callback */
	void           * cbArg1,  /* the first argument to pass to the cbFunc */
	void           * cbArg2)  /* the second argument to pass to the cbFunc */
{
	int                  eNum       = 0;
	NEUIK_ElementBase  * eBase      = NULL;
	static char          funcName[] = "NEUIK_Element_SetCallback";
	static char        * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Callback Name `cbName` is NULL.",                               // [2]
		"Callback Name `cbName` is blank.",                              // [3]
		"Callback Name `cbName` unknown.",                               // [4]
	};


	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (cbName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (cbName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("OnClick", cbName))
	{
		if (eBase->eCT.OnClick) free(eBase->eCT.OnClick);
		eBase->eCT.OnClick = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnClicked", cbName))
	{
		if (eBase->eCT.OnClicked) free(eBase->eCT.OnClicked);
		eBase->eCT.OnClicked = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnHover", cbName))
	{
		if (eBase->eCT.OnHover) free(eBase->eCT.OnHover);
		eBase->eCT.OnHover = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnMouseEnter", cbName))
	{
		if (eBase->eCT.OnMouseEnter) free(eBase->eCT.OnMouseEnter);
		eBase->eCT.OnMouseEnter = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnMouseLeave", cbName))
	{
		if (eBase->eCT.OnMouseLeave) free(eBase->eCT.OnMouseLeave);
		eBase->eCT.OnMouseLeave = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnSelected", cbName))
	{
		if (eBase->eCT.OnSelected) free(eBase->eCT.OnSelected);
		eBase->eCT.OnSelected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnDeselected", cbName))
	{
		if (eBase->eCT.OnDeselected) free(eBase->eCT.OnDeselected);
		eBase->eCT.OnDeselected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnActivated", cbName))
	{
		if (eBase->eCT.OnActivated) free(eBase->eCT.OnActivated);
		eBase->eCT.OnActivated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnDeactivated", cbName))
	{
		if (eBase->eCT.OnDeactivated) free(eBase->eCT.OnDeactivated);
		eBase->eCT.OnDeactivated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnTextChanged", cbName))
	{
		if (eBase->eCT.OnTextChanged) free(eBase->eCT.OnTextChanged);
		eBase->eCT.OnTextChanged = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else
	{
		eNum = 4;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_SetBindingCallback
 *
 *  Description:   Set the bindID to be sent when the specified callback is 
 *                 triggered.
 *
 *                 This alternative callback procedure should only be used if 
 *                 the standard `NEUIK_Element_SetCallback` function can not be
 *                 used, like for instance in a binding with another language. 
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_SetBindingCallback(
	NEUIK_Element   elem,   /* the element to se the callback for */
	const char    * cbName, /* the name of the callback to set */
	unsigned int    bindID) /* A unique number to identify this callback instance */
{
	int                  eNum       = 0;
	NEUIK_ElementBase  * eBase      = NULL;
	static char          funcName[] = "NEUIK_Element_SetBindingCallback";
	static char        * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Callback Name `cbName` is NULL.",                               // [2]
		"Callback Name `cbName` is blank.",                              // [3]
		"Callback Name `cbName` unknown.",                               // [4]
	};


	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (cbName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (cbName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("OnClick", cbName))
	{
		if (eBase->eCT.OnClick) free(eBase->eCT.OnClick);
		eBase->eCT.OnClick = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnClicked", cbName))
	{
		if (eBase->eCT.OnClicked) free(eBase->eCT.OnClicked);
		eBase->eCT.OnClicked = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnHover", cbName))
	{
		if (eBase->eCT.OnHover) free(eBase->eCT.OnHover);
		eBase->eCT.OnHover = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnMouseEnter", cbName))
	{
		if (eBase->eCT.OnMouseEnter) free(eBase->eCT.OnMouseEnter);
		eBase->eCT.OnMouseEnter = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnMouseLeave", cbName))
	{
		if (eBase->eCT.OnMouseLeave) free(eBase->eCT.OnMouseLeave);
		eBase->eCT.OnMouseLeave = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnSelected", cbName))
	{
		if (eBase->eCT.OnSelected) free(eBase->eCT.OnSelected);
		eBase->eCT.OnSelected = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnDeselected", cbName))
	{
		if (eBase->eCT.OnDeselected) free(eBase->eCT.OnDeselected);
		eBase->eCT.OnDeselected = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnActivated", cbName))
	{
		if (eBase->eCT.OnActivated) free(eBase->eCT.OnActivated);
		eBase->eCT.OnActivated = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnDeactivated", cbName))
	{
		if (eBase->eCT.OnDeactivated) free(eBase->eCT.OnDeactivated);
		eBase->eCT.OnDeactivated = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnTextChanged", cbName))
	{
		if (eBase->eCT.OnTextChanged) free(eBase->eCT.OnTextChanged);
		eBase->eCT.OnTextChanged = NEUIK_NewBindingCallback(bindID);
	}
	else
	{
		eNum = 4;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_TriggerCallback
 *
 *  Description:   Trigger a callback of the specified type (if not NULL).
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int  neuik_Element_TriggerCallback(
	NEUIK_Element      elem,    /* The element whose callback should be triggered */
	neuik_CallbackEnum cbType)  /* Which callback to trigger */
{
	int                  eNum       = 0;
	NEUIK_ElementBase  * eBase      = NULL;
	static char          funcName[] = "neuik_Element_TriggerCallback";
	static char        * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Unknown Callback Type `cbType`.",                               // [2]
	};


	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	switch (cbType)
	{
		case NEUIK_CALLBACK_ON_CLICK:
			if (eBase->eCT.OnClick) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnClick, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_CLICKED:
			if (eBase->eCT.OnClicked) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnClicked, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_CREATED:
			if (eBase->eCT.OnCreated) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnCreated, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_HOVER:
			if (eBase->eCT.OnHover) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnHover, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_MOUSE_ENTER:
			if (eBase->eCT.OnMouseEnter) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnMouseEnter, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_MOUSE_LEAVE:
			if (eBase->eCT.OnMouseLeave) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnMouseLeave, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_SELECTED:
			if (eBase->eCT.OnSelected) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnSelected, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_DESELECTED:
			if (eBase->eCT.OnDeselected) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnDeselected, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_ACTIVATED:
			if (eBase->eCT.OnActivated) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnActivated, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_DEACTIVATED:
			if (eBase->eCT.OnDeactivated) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnDeactivated, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_TEXT_CHANGED:
			if (eBase->eCT.OnTextChanged) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnTextChanged, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_EXPANDED:
			if (eBase->eCT.OnExpanded) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnExpanded, eBase->eSt.window);
			}
			break;

		case NEUIK_CALLBACK_ON_COLLAPSED:
			if (eBase->eCT.OnCollapsed) 
			{
				NEUIK_Callback_Trigger(eBase->eCT.OnCollapsed, eBase->eSt.window);
			}
			break;



		default:
			eNum = 2;
			goto out;
			break;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer    (virtual-function)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation of this function may be redefined by a 
 *                 Element subclass.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_SetWindowPointer(
	NEUIK_Element   elem,
 	void          * win)
{
	int                  eNum       = 0;
	NEUIK_ElementBase  * eBase      = NULL;
	int               (* funcImp) (NEUIK_Element, void*);
	static char          funcName[] = "neuik_Element_SetWindowPointer";
	static char        * errMsgs[]  = {"",                               // [0] no error
		"Argument `elem` does not implement Element class.",             // [1]
		"Argument `win` does not implement Window class.",               // [2]
		"Failure in virtual-function implementation.",                   // [3]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [4]
	};

	funcImp = neuik_VirtualFunc_GetImplementation(
		neuik_Element_vfunc_SetWindowPointer, elem);
	if (funcImp != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* A virtual reimplementation is availible for this function          */
		/*--------------------------------------------------------------------*/
		// printf("vfunc is valid...\n");
		if ((*funcImp)(elem, win))
		{
			eNum = 3;
		}
		goto out;
	}
	/*------------------------------------------------------------------------*/
	/* ELSE: Fall back to standard element_SetWindowPointer operation         */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
	{
		eNum = 1;
		printf("elem failed check\n");
		goto out;
	}

	if (!neuik_Object_ImplementsClass(win, neuik__Class_Window))
	{
		eNum = 2;
		printf("win failed check\n");
		goto out;
	}

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	eBase->eSt.window = win;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


void neuik_Element_SetParentPointer(
	NEUIK_Element    elem, 
	void           * parent)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return;
	}

	eBase->eSt.parent = parent;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_ForceRedraw
 *
 *  Description:   This function marks the element as one which needs to be 
 *                 redrawn.  This message will propagate upwards through its
 *                 parent elements until it has reached the top. Finally, the
 *                 parent window is also marked as needing a redraw.
 *
 *                 This function unsets the internal old element size, causing
 *                 the element to resize and redraw itself. Only use this 
 *                 function if neuik_Element_RequestRedraw is failing to cause
 *                 a redraw of the element.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_ForceRedraw(
	NEUIK_Element elem)
{
	NEUIK_ElementBase  * eBase;
	static RenderSize    redrawSz = {-1, -1};
	static char          funcName[] = "neuik_Element_ForceRedraw";
	static char          errMsg[] = 
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.";

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		NEUIK_RaiseError(funcName, errMsg);
		return 1;
	}

	/*------------------------------------------------------------------------*/
	/* Setting the old size to (-1, -1), will always cause resize->redraw     */
	/* since the element will think it has changed size.                      */
	/*------------------------------------------------------------------------*/
	eBase->eSt.rSizeOld = redrawSz;

	eBase->eSt.doRedraw = 1;
	if (eBase->eSt.parent != NULL)
	{
		neuik_Element_ForceRedraw(eBase->eSt.parent);
	}
	else
	{
		/* notify the parent window that it will probably need to be redrawn */
		((NEUIK_Window *)(eBase->eSt.window))->doRedraw = 1;
	}

	/* No errors*/
	return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_RequestRedraw
 *
 *  Description:   This function marks the element as one which needs to be 
 *                 redrawn.  This message will propagate upwards through its
 *                 parent elements until it has reached the top. Finally, the
 *                 parent window is also marked as needing a redraw.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_RequestRedraw(
	NEUIK_Element elem)
{
	NEUIK_ElementBase  * eBase;
	NEUIK_Window       * win;
	static char          funcName[] = "neuik_Element_RequestRedraw";
	static char          errMsg[] = 
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.";

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		NEUIK_RaiseError(funcName, errMsg);
		return 1;
	}

	eBase->eSt.doRedraw = 1;
	if (eBase->eSt.parent != NULL)
	{
		neuik_Element_RequestRedraw(eBase->eSt.parent);
	}
	else
	{
		/* notify the parent window that it will probably need to be redrawn */
		win = (NEUIK_Window*)(eBase->eSt.window);
		if (win != NULL) win->doRedraw = 1;
	}

	/* No errors*/
	return 0;
}


int neuik_Element_NeedsRedraw(
	NEUIK_Element elem)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return 0;
	}

	return 	eBase->eSt.doRedraw;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_RedrawBackgroundGradient
 *
 *  Description:   Renders a color gradient using the specified ColorStops.
 *
 *                 Vertical gradients start at the top and go down from there.
 *                 Horizontal gradients start at the left and go right from there.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_RedrawBackgroundGradient(
	NEUIK_Element      elem,
	NEUIK_ColorStop ** cs)
{
	int                 ctr;
	int                 gCtr;             /* gradient counter */
	int                 nClrs;
	int                 eNum      = 0;    /* which error to report (if any) */
	int                 clrR;
	int                 clrG;
	int                 clrB;
	int                 clrA;
	int                 clrFound;
	char                dirn;            /* Direction of the gradient 'v' or 'h' */
	float               lastFrac  = -1.0;
	float               frac;
	float               fracDelta;        /* fraction between ColorStop 1 & 2 */
	float               fracStart = 0.0;  /* fraction at ColorStop 1 */
	float               fracEnd   = 1.0;  /* fraction at ColorStop 2 */
	RenderSize          rSize;            /* Size of the element background to fill */
	NEUIK_ElementBase * eBase     = NULL;
	SDL_Renderer      * rend      = NULL;
	colorDeltas       * deltaPP   = NULL;
	colorDeltas       * clrDelta;
	NEUIK_Color       * clr;
	static char         funcName[] = "neuik_Element_RedawBackgroundGradient";
	static char       * errMsgs[] = {"",                                 // [0] no error
		"Pointer to ColorStops is NULL.",                                // [1]
		"Unsupported gradient direction.",                               // [2]
		"Invalid RenderSize supplied.",                                  // [3]
		"Unable to create RGB surface.",                                 // [4]
		"SDL_CreateTextureFromSurface failed.",                          // [5]
		"ColorStops array is empty.",                                    // [6]
		"Invalid ColorStop fraction (<0 or >1).",                        // [7]
		"ColorStops array fractions not in ascending order.",            // [8]
		"Failure to allocate memory.",                                   // [9]
		"Failed to create software renderer.",                           // [10]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [11]
	};

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 11;
		goto out;
	}

	rSize.w = eBase->eSt.rSize.w;
	rSize.h = eBase->eSt.rSize.h;

	rend = eBase->eSt.rend;
	dirn = eBase->eBg.gradient_dirn;

	/*------------------------------------------------------------------------*/
	/* Check for easily issues before attempting to render the gradient       */
	/*------------------------------------------------------------------------*/
	if (cs == NULL)
	{
		eNum = 1;
		goto out;
	}
	else if (*cs == NULL)
	{
		eNum = 6;
		goto out;
	}
	if (dirn != 'v' && dirn != 'h')
	{
		eNum = 2;
		goto out;
	}
	if (rSize.w <= 0 || rSize.h <= 0)
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Count the number of color stops and check that the color stop          */
	/* fractions are in increasing order                                      */
	/*------------------------------------------------------------------------*/
	for (nClrs = 0;; nClrs++)
	{
		if (cs[nClrs] == NULL) break; /* this is the number of ColorStops */
		if (cs[nClrs]->frac < 0.0 || cs[nClrs]->frac > 1.0)
		{
			printf("cs[%d]->frac = %f\n", nClrs, cs[nClrs]->frac);
			eNum = 7;
			goto out;
		}
		else if (cs[nClrs]->frac < lastFrac)
		{
			eNum = 8;
			goto out;
		}
		else
		{
			lastFrac = cs[nClrs]->frac;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for delta-per-px array and calculate the ColorStop     */
	/* delta-per-px values.                                                   */
	/*------------------------------------------------------------------------*/
	if (nClrs > 1)
	{
		deltaPP = (colorDeltas *)malloc((nClrs - 1)*sizeof(colorDeltas));
		if (deltaPP == NULL)
		{
			eNum = 9;
			goto out;
		}
		for (ctr = 0; ctr < nClrs-1; ctr++)
		{
			deltaPP[ctr].r = (cs[ctr+1]->color).r - (cs[ctr]->color).r;
			deltaPP[ctr].g = (cs[ctr+1]->color).g - (cs[ctr]->color).g;
			deltaPP[ctr].b = (cs[ctr+1]->color).b - (cs[ctr]->color).b;
			deltaPP[ctr].a = (cs[ctr+1]->color).a - (cs[ctr]->color).a;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Fill in the colors of the gradient                                     */
	/*------------------------------------------------------------------------*/
	if (nClrs == 1)
	{
		/*--------------------------------------------------------------------*/
		/* A single color; this will just be a filled rectangle               */
		/*--------------------------------------------------------------------*/
		clr = &(cs[0]->color);
		SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
		SDL_RenderClear(rend);
	}
	else if (dirn == 'v')
	{
		/*--------------------------------------------------------------------*/
		/* Draw a vertical gradient                                           */
		/*--------------------------------------------------------------------*/
		for (gCtr = 0; gCtr < rSize.h; gCtr++)
		{
			/* calculate the fractional position within the gradient */
			frac = (float)(gCtr+1)/(float)(rSize.h);


			/* determine which ColorStops/colorDeltas should be used */
			fracStart = cs[0]->frac;
			clr       = &(cs[0]->color);
			clrDelta  = NULL;
			clrFound  = 0;
			for (ctr = 0;;ctr++)
			{
				if (cs[ctr] == NULL) break;

				if (frac < cs[ctr]->frac)
				{
					/* apply delta from this clr */
					fracEnd  = cs[ctr]->frac;
					clrFound = 1;
					break;
				}

				clr      = &(cs[ctr]->color);
				clrDelta = &(deltaPP[ctr]);
			}

			if (!clrFound)
			{
				/* line is beyond the final ColorStop; use that color */
				clrDelta = NULL;
			}

			/* calculate and set the color for this gradient line */
			if (clrDelta != NULL)
			{
				/* between two ColorStops, blend the color */
				fracDelta = (frac - fracStart)/(fracEnd - fracStart);
				clrR = clr->r + (int)((clrDelta->r)*fracDelta);
				clrG = clr->g + (int)((clrDelta->g)*fracDelta);
				clrB = clr->b + (int)((clrDelta->b)*fracDelta);
				clrA = clr->a + (int)((clrDelta->a)*fracDelta);
				SDL_SetRenderDrawColor(rend, clrR, clrG, clrB, clrA);
			}
			else
			{
				/* not between two ColorStops, use a single color */
				SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, clr->a);
			}

			SDL_RenderDrawLine(rend, 0, gCtr, rSize.w - 1, gCtr);
		}
	}
	else if (dirn == 'h')
	{
		/*--------------------------------------------------------------------*/
		/* Draw a horizontal gradient                                         */
		/*--------------------------------------------------------------------*/
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (deltaPP != NULL) free(deltaPP);

	return eNum;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_RedrawBackground
 *
 *  Description:   Redraw the element background to the stored renderer.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_RedrawBackground(
	NEUIK_Element elem)
{
	NEUIK_ElementBase    * eBase          = NULL;
	SDL_Renderer         * rend           = NULL;
	enum neuik_bgstyle     bgstyle;               /* active background style */
	NEUIK_Color          * color_solid    = NULL; /* pointer to active solid color */
	NEUIK_ColorStop    *** color_gradient = NULL; /* color gradient to use under normal condtions */
	RenderSize             rSize;                 /* Size of the element background to fill */
	static char            funcName[] = "neuik_Element_RedrawBackground";
	static char            errMsg[]   =
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.";

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		NEUIK_RaiseError(funcName, errMsg);
		return 1;
	}

	rSize.w = eBase->eSt.rSize.w;
	rSize.h = eBase->eSt.rSize.h;

	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Identify both the background style to use as well as the color(s) to   */
	/* use to render the background.                                          */
	/*------------------------------------------------------------------------*/
	switch (eBase->eSt.focusstate)
	{
		case NEUIK_FOCUSSTATE_NORMAL:
			bgstyle = eBase->eBg.bgstyle_normal;
			switch (bgstyle)
			{
				case NEUIK_BGSTYLE_SOLID:
					color_solid = &(eBase->eBg.solid_normal);
					break;
				case NEUIK_BGSTYLE_GRADIENT:
					color_gradient = &(eBase->eBg.gradient_normal);
					break;
				case NEUIK_BGSTYLE_TRANSPARENT:
					break;
			}
			break;
		case NEUIK_FOCUSSTATE_SELECTED:
			bgstyle = eBase->eBg.bgstyle_selected;
			switch (bgstyle)
			{
				case NEUIK_BGSTYLE_SOLID:
					color_solid = &(eBase->eBg.solid_selected);
					break;
				case NEUIK_BGSTYLE_GRADIENT:
					color_gradient = &(eBase->eBg.gradient_selected);
					break;
				case NEUIK_BGSTYLE_TRANSPARENT:
					break;
			}
			break;
		case NEUIK_FOCUSSTATE_HOVERED:
			bgstyle = eBase->eBg.bgstyle_hover;
			switch (bgstyle)
			{
				case NEUIK_BGSTYLE_SOLID:
					color_solid = &(eBase->eBg.solid_hover);
					break;
				case NEUIK_BGSTYLE_GRADIENT:
					color_gradient = &(eBase->eBg.gradient_hover);
					break;
				case NEUIK_BGSTYLE_TRANSPARENT:
					break;
			}
			break;
	}

	/*------------------------------------------------------------------------*/
	/* Render the background.                                                 */
	/*------------------------------------------------------------------------*/
	switch (bgstyle)
	{
		case NEUIK_BGSTYLE_SOLID:
			/*----------------------------------------------------------------*/
			/* Fill the entire surface background with a solid color.         */
			/*----------------------------------------------------------------*/
			SDL_SetRenderDrawColor(rend,
				color_solid->r, color_solid->g, color_solid->b, color_solid->a);
			SDL_RenderClear(rend);
			break;
		case NEUIK_BGSTYLE_GRADIENT:
			neuik_Element_RedrawBackgroundGradient(elem, *color_gradient);
			break;
		case NEUIK_BGSTYLE_TRANSPARENT:
			/*----------------------------------------------------------------*/
			/* Fill the entire surface background with a transparent color.   */
			/*----------------------------------------------------------------*/
			SDL_SetRenderDrawColor(rend, 255, 255, 255, 0);
			SDL_RenderClear(rend);
			break;
	}

	/* No errors*/
	return 0;
}


int neuik_Element_Resize(
	NEUIK_Element elem, 
	RenderSize    rSize)
{
	int                 eNum = 0; /* which error to report (if any) */
	NEUIK_ElementBase * eBase;
	Uint32              rmask;
	Uint32              gmask;
	Uint32              bmask;
	Uint32              amask;
	static char         funcName[] = "neuik_Element_Resize";
	static char       * errMsgs[] = {"",                                 // [0] no error
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [1]
		"Failed to create RGB surface.",                                 // [2]
		"Failed to create software renderer.",                           // [3]
	};

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xFF000000;
		gmask = 0x00FF0000;
		bmask = 0x0000FF00;
		amask = 0x000000FF;
	#else
		rmask = 0x000000FF;
		gmask = 0x0000FF00;
		bmask = 0x00FF0000;
		amask = 0xFF000000;
	#endif

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (eBase->eSt.surf != NULL) SDL_FreeSurface(eBase->eSt.surf);
	if (eBase->eSt.rend != NULL) SDL_DestroyRenderer(eBase->eSt.rend);

	eBase->eSt.surf = SDL_CreateRGBSurface(0, 
		rSize.w, rSize.h, 32, rmask, gmask, bmask, amask);
	if (eBase->eSt.surf == NULL)
	{
		eNum = 2;
		goto out;
	}

	eBase->eSt.rend = SDL_CreateSoftwareRenderer(eBase->eSt.surf);
	if (eBase->eSt.rend == NULL)
	{
		eNum = 3;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


int neuik_Element_SetChildPopup(
	NEUIK_Element  parent, 
	NEUIK_Element  pu)
{
	int                  eNum       = 0; /* which error to report (if any) */
	NEUIK_ElementBase  * eBase      = NULL;
	static char          funcName[] = "neuik_Element_SetChildPopup";
	static char        * errMsgs[]  = {"",                                 // [0] no error
		"Argument `pu` does not implement Element class.",                 // [1]
		"Argument `parent` caused `neuik_Object_GetClassObject` to fail.", // [2]
	};

	if (!neuik_Object_ImplementsClass(pu, neuik__Class_Element))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(parent, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	eBase->eSt.popup = pu;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Defocus__Container (virtual-function)
 *
 *  Description:   Call Element defocus function.
 *
 ******************************************************************************/
void neuik_Element_Defocus(
	NEUIK_Element  elem)
{
	NEUIK_ElementBase * eBase;

	if (!neuik_Object_IsNEUIKObject_NoError(elem))
	{
		/* The object may have been freed/corrupted; ignore defocus call */
		return;
	}

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return;
	}

	eBase->eSt.hasFocus = 0;

	/*------------------------------------------------------------------------*/
	/* Check to see if this element may contain other elements. If so,        */
	/* recursively defocus these items.                                       */
	/*------------------------------------------------------------------------*/
	if (eBase->eFT->Defocus != NULL)
	{
		(eBase->eFT->Defocus)(elem);
	}
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Element_IsShown    (virtual-function)
 *
 *  Description:   This function reports whether or not an element is currently
 *                 being shown.
 *
 *                 This operation of this function may be redefined by a 
 *                 Element subclass.
 *
 *  Returns:       1 if element is shown, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Element_IsShown(
	NEUIK_Element  elem)
{
	NEUIK_ElementBase  * eBase;
	int               (*funcImp) (NEUIK_Element);

	funcImp = neuik_VirtualFunc_GetImplementation(
		neuik_Element_vfunc_IsShown, elem);
	if (funcImp != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* A virtual reimplementation is availible for this function          */
		/*--------------------------------------------------------------------*/
		return (*funcImp)(elem);
	}
	/*------------------------------------------------------------------------*/
	/* ELSE: Fall back to standard Element_IsShown operation                  */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		return 0;
	}

	return eBase->eCfg.Show;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetActive
 *
 *  Description:   Set the `isActive` parameter of an element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_SetActive(
	NEUIK_Element elem,
	int           isActive)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase)) return;

	eBase->eSt.isActive = isActive;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_IsActive
 *
 *  Description:   Return the `isActive` parameter of an element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
int neuik_Element_IsActive(
	NEUIK_Element elem)
{
	NEUIK_ElementBase * eBase;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase)) return 0;

	return eBase->eSt.isActive;
}
