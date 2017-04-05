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
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "MainMenu_internal.h"
#include "NEUIK_Window.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_WindowConfig.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"
#include "NEUIK_Event_internal.h"
#include "NEUIK_Element.h"
#include "NEUIK_Element_internal.h"
// #include "NEUIK_PopUp.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Window(void ** wPtr);
// int neuik_Object_Copy__Window(void * dst, const void * src);
int neuik_Object_Free__Window(void * wPtr);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Window_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Window,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Window,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Window
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_Window()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Window";
	static char  * errMsgs[]  = {"",                  // [0] no error
		"NEUIK library must be initialized first.",   // [1]
		"Failed to register `Window` object class .", // [2]
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
		"NEUIK_Window",             // className
		"The NEUIK_Window Object.", // classDescription
		neuik__Set_NEUIK,           // classSet
		NULL,                       // superClass
		&neuik_Window_BaseFuncs,    // baseFuncs
		NULL,                       // classFuncs
		&neuik__Class_Window))      // newClass
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
 *  Name:          neuik_Object_New
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__Window(
	void  ** wPtr)
{
	return NEUIK_NewWindow((NEUIK_Window **)wPtr);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_NewWindow
 *
 *  Description:   Allocates and initializes values for a new Window.
 *
 *  Returns:       NULL if error otherwise, returns a pointer to a valid Window. 
 *
 ******************************************************************************/
int NEUIK_NewWindow(
	NEUIK_Window ** wPtr)
{
	int            eNum       = 0; /* which error to report (if any) */
	NEUIK_Window * w          = NULL;
	NEUIK_Image  * icon       = NULL;
	static char    funcName[] = "NEUIK_NewWindow";
	static char  * errMsgs[]  = {"",             // [0] no error
		"Failure to allocate memory.",           // [1]
		"Failure in NEUIK_NewWindowConfig.",     // [2]
		"Output Argument `wPtr` is NULL.",       // [3]
		"Failure in NEUIK_MakeImage_FromStock.", // [4]
	};

	if (wPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*wPtr) = (NEUIK_Window*) malloc(sizeof(NEUIK_Window));
	w = (*wPtr);
	if (w == NULL)
	{
		eNum = 1;
		goto out;
	}

	neuik_GetObjectBaseOfClass(
		neuik__Set_NEUIK, 
		neuik__Class_Window, 
		NULL,
		&(w->objBase));

	/* initialize pointers to NULL */
	w->win         = NULL;
	w->rend        = NULL;
	w->title       = NULL;
	w->mmenu       = NULL;
	w->cfgPtr      = NULL;
	w->elem        = NULL;
	w->focused     = NULL;
	w->popups      = NULL;
	w->icon        = NULL;

	/* set default values */
	w->posX        = -1;
	w->posY        = -1;
	w->sizeW       = 320;
	w->sizeH       = 240;
	w->shown       = 1;
	w->updateTitle = 0;
	w->updateIcon  = 0;
	w->doRedraw    = 1;

	w->eHT       = NEUIK_NewEventHandlerTable();
	w->eCT       = NEUIK_NewCallbackTable();

	if (!NEUIK_MakeImage_FromStock(&icon, NEUIK_STOCKIMAGE_NEUIK_ICON))
	{
		w->icon = icon;
	}

	if (NEUIK_NewWindowConfig(&w->cfg))
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

void neuik_Window_Configure_capture_segv(
	int sig_num)
{
	static char funcName[] = "NEUIK_Window_Configure";
	static char errMsg[] = 
		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

	NEUIK_RaiseError(funcName, errMsg);
	NEUIK_BacktraceErrors();
	exit(1);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_Window_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_Window_Configure(
	NEUIK_Window  * w,
	const char    * set0,
	...)
{
	int                   ns; /* number of items from sscanf */
	int                   ctr;
	int                   nCtr;
	int                   eNum      = 0; /* which error to report (if any) */
	int                   isBool;
	int                   boolVal   = 0;
	int                   typeMixup;
	char                  buf[4096];
	va_list               args;
	char                * strPtr    = NULL;
	char                * name      = NULL;
	char                * value     = NULL;
	const char          * set       = NULL;
	NEUIK_Color           clr;
	NEUIK_WindowConfig  * wCfg      = NULL;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char          * boolNames[] = {
		"Borderless",
		"Fullscreen",
		"Resizable",
		"Minimize",
		"Maximize",
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	static char          * valueNames[] = {
		"BGColor",
		"AutoResize",
		"AutoResizeW",
		"AutoResizeH",
		"CanResize",
		"CanResizeW",
		"CanResizeH",
		NULL,
	};
	static char           funcName[] = "NEUIK_Window_Configure";
	static char         * errMsgs[] = {"",                        // [ 0] no error
		"Argument `w` does not implement Window class.",          // [ 1]
		"NamedSet.name is NULL, skipping..",                      // [ 2]
		"NamedSet.name is blank, skipping..",                     // [ 3]
		"NamedSet.name type unknown, skipping.",                  // [ 4]
		"`name=value` string is too long.",                       // [ 5]
		"Invalid `name=value` string.",                           // [ 6]
		"AutoResize value is invalid.",                           // [ 7]
		"AutoResizeW value is invalid.",                          // [ 8]
		"AutoResizeH value is invalid.",                          // [ 9]
		"CanResize value is invalid.",                            // [10]
		"CanResizeW value is invalid.",                           // [11]
		"CanResizeH value is invalid.",                           // [12]
		"Window_GetConfig() failed.",                             // [13]
		"Borderless value is invalid.",                           // [14]
		"Resizable value is invalid.",                            // [15]
		"Fullscreen value is invalid.",                           // [16]
		"ValueType name used as BoolType, skipping.",             // [17]
		"BoolType name unknown, skipping.",                       // [18]
		"BoolType name used as ValueType, skipping.",             // [19]
		"BGColor value invalid; should be comma separated RGBA.", // [20]
		"BGColor value invalid; RGBA value range is 0-255."       // [21]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	set = set0;

	wCfg = neuik_Window_GetConfig(w);
	if (wCfg == NULL)
	{
		eNum = 13;
		goto out;
	}

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
		isBool = 0;
		name   = NULL;
		value  = NULL;

		if (set == NULL) break;

		#ifndef NO_NEUIK_SIGNAL_TRAPPING
			signal(SIGSEGV, neuik_Window_Configure_capture_segv);
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
					NEUIK_RaiseError(funcName, errMsgs[6]);
					set = va_arg(args, const char *);
					continue;
				}
				name  = buf;
				value = strPtr;
			}
		}

		if (isBool)
		{
			if (!strcmp("Borderless", name))
			{
				if (wCfg->isBorderless == boolVal) continue;

				/* The previous setting was changed */
				wCfg->isBorderless = boolVal;

				/*------------------------------------------------------------*/
				/* If the window is currently being shown. It will need to be */
				/* destroyed, and recreated with these settings.              */
				/*------------------------------------------------------------*/
				if (w->shown && w->win != NULL) NEUIK_Window_Recreate(w);
			}
			else if (!strcmp("Fullscreen", name))
			{
				if (wCfg->isFullscreen == boolVal) continue;

				/* The previous setting was changed */
				wCfg->isFullscreen = boolVal;

				/*------------------------------------------------------------*/
				/* If the window is currently being shown. Apply settings now */
				/*------------------------------------------------------------*/
				if (w->shown && w->win != NULL)
				{
					/* The window is currently being shown */
					if (boolVal)
					{
						SDL_SetWindowFullscreen(w->win, SDL_WINDOW_FULLSCREEN_DESKTOP);
						SDL_GetWindowSize(w->win, &(w->sizeW), &(w->sizeH));
						SDL_GetWindowPosition(w->win, &(w->posX), &(w->posY));
						/* Force a full redraw of the contained elements */
						if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
					}
					else
					{
						SDL_SetWindowFullscreen(w->win, 0);
						SDL_GetWindowSize(w->win, &(w->sizeW), &(w->sizeH));
						SDL_GetWindowPosition(w->win, &(w->posX), &(w->posY));
						/* Force a full redraw of the contained elements */
						if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
					}
				}
			}
			else if (!strcmp("Resizable", name))
			{
				if (wCfg->isResizable == boolVal) continue;

				/* The previous setting was changed */
				wCfg->isResizable = boolVal;

				/*------------------------------------------------------------*/
				/* If the window is currently being shown. It will need to be */
				/* destroyed, and recreated with these settings.              */
				/*------------------------------------------------------------*/
				if (w->shown && w->win != NULL) NEUIK_Window_Recreate(w);
			}
			else if (!strcmp("Minimize", name))
			{
				if (wCfg->isMinimized == boolVal) continue;

				/* The previous setting was changed */
				wCfg->isMinimized = boolVal;

				/*------------------------------------------------------------*/
				/* If the window is currently being shown. Apply settings now */
				/*------------------------------------------------------------*/
				if (w->shown && w->win != NULL)
				{
					/* The window is currently being shown */
					if (boolVal)
					{
						/* do nothing */
					}
					else
					{
						/* do nothing */
					}
				}
			}
			else if (!strcmp("Maximize", name))
			{
				if (wCfg->isMaximized == boolVal) continue;

				/* The previous setting was changed */
				wCfg->isMaximized = boolVal;

				/*------------------------------------------------------------*/
				/* If the window is currently being shown. Apply settings now */
				/*------------------------------------------------------------*/
				if (w->shown && w->win != NULL)
				{
					/* The window is currently being shown */
					if (boolVal)
					{
						/* do nothing */
						SDL_MaximizeWindow(w->win);
					}
					else
					{
						/* do nothing */
						SDL_RestoreWindow(w->win);
					}
				}
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
					NEUIK_RaiseError(funcName, errMsgs[17]);
				}
				else
				{
					/* An unsupported name was used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[18]);
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
			else if (!strcmp("BGColor", name))
			{
				/*------------------------------------------------------------*/
				/* Check for empty value errors.                              */
				/*------------------------------------------------------------*/
				if (value == NULL)
				{
					NEUIK_RaiseError(funcName, errMsgs[20]);
					continue;
				}
				if (value[0] == '\0')
				{
					NEUIK_RaiseError(funcName, errMsgs[20]);
					continue;
				}

				ns = sscanf(value, "%d,%d,%d,%d", &clr.r, &clr.g, &clr.b, &clr.a);
				/*------------------------------------------------------------*/
				/* Check for EOF, incorrect # of values, & out of range vals. */
				/*------------------------------------------------------------*/
				if (ns == EOF || ns < 4) 
				{
					NEUIK_RaiseError(funcName, errMsgs[20]);
					continue;
				}

				if (clr.r < 0 || clr.r > 255 ||
					clr.g < 0 || clr.g > 255 ||
					clr.b < 0 || clr.b > 255 ||
					clr.a < 0 || clr.a > 255)
				{
					NEUIK_RaiseError(funcName, errMsgs[21]);
					continue;
				}

				wCfg->colorBG = clr;
				w->doRedraw   = 1;
			}
			else if (!strcmp("AutoResize", name))
			{
				/* Set autoResize parameters for both width and height */
				if (!strcmp("any", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ANY;
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[7]);
				}
			}
			else if (!strcmp("AutoResizeW", name))
			{
				if (!strcmp("any", value))
				{
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->autoResizeW = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[8]);
				}
			}
			else if (!strcmp("AutoResizeH", name))
			{
				if (!strcmp("any", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->autoResizeH = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[9]);
				}
			}
			else if (!strcmp("CanResize", name))
			{
				/* Set canResize parameters for both width and height */
				if (!strcmp("any", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ANY;
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[10]);
				}
			}
			else if (!strcmp("CanResizeW", name))
			{
				if (!strcmp("any", value))
				{
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->canResizeW = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[11]);
				}
			}
			else if (!strcmp("CanResizeH", name))
			{
				if (!strcmp("any", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ANY;
				}
				else if (!strcmp("expand", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
				}
				else if (!strcmp("contract", value))
				{
					wCfg->canResizeH = NEUIK_WINDOW_RESIZE_ONLY_CONTRACT;
				}
				else 
				{
					NEUIK_RaiseError(funcName, errMsgs[12]);
				}
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
					NEUIK_RaiseError(funcName, errMsgs[19]);
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
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


int neuik_Object_Free__Window(
	void * wPtr)
{
	return NEUIK_Window_Free((NEUIK_Window *)wPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_Free
 *
 *  Description:   Free all of the resources loaded by the Window.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_Window_Free(
	NEUIK_Window * w) /* (in,out) the window to destroy */
{
	int            eNum       = 0;
	static char    funcName[] = "NEUIK_Window_Free";
	static char  * errMsgs[]  = {"",                     // [0] no error
		"Argument `w` is NULL.",                         // [1]
		"Argument `w` does not implement Window class.", // [2]
	};

	if (w == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 2;
		goto out;
	}


	/*------------------------------------------------------------------------*/
	/* Free all memory that was dynamically allocated for this object         */
	/*------------------------------------------------------------------------*/
	if (w->rend != NULL) 
	{
		SDL_DestroyRenderer(w->rend);
	}
	if (w->win != NULL) 
	{
		SDL_DestroyWindow(w->win);
	}
	if (w->title != NULL)
	{
		free(w->title);
	}

	free(w);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


//int neuik_Window_AddPopup(
//		NEUIK_Window   * w,      /* The Window to add an PopUp element to */
//		NEUIK_PopUp    * pu,     /* The PopUp element to add to the Window */
//		NEUIK_Element    parent) /* The Parent element of the PopUp Element */
//{
//	int           len        = 0;
//	int           ctr        = 0;
//	int           eNum       = 0; /* which error to report (if any) */
//	static char   funcName[] = "neuik_Window_AddPopup";
//	static char * errMsgs[]  = {"",                            // [0] no error
//		"Argument `w` does not implement Window class.",       // [1]
//		"Argument `pu` does not implement Element class.",     // [2]
//		"Failure to allocate memory.",                         // [3]
//		"Failure to reallocate memory.",                       // [4]
//		"PopUp Element failed CheckType().",                   // [5] unused
//		"Argument `parent` does not implement Element class.", // [6]
//		"Element_SetChildPopup failed.",                       // [7]
//	};
//
//
//	if (!neuik_Object_IsClass(w, neuik__Class_Window))
//	{
//		eNum = 1;
//		goto out;
//	}
//	if (!neuik_Object_ImplementsClass(pu, neuik__Class_Element))
//	{
//		eNum = 2;
//		goto out;
//	}
//	// if (NEUIK_CheckType(pu, 5, NEUIK_TYPE_ELEM_POPUP))
//	// {
//	// 	eNum = 2;
//	// 	goto out;
//	// }
//	if (!neuik_Object_ImplementsClass(parent, neuik__Class_Element))
//	{
//		eNum = 6;
//		goto out;
//	}
//
//	if (w->popups == NULL)
//	{
//		/*--------------------------------------------------------------------*/
//		/* This is the first PopUp element to be added, allocate initial      */
//		/* memory. This pointer array will be null terminated.                */
//		/*--------------------------------------------------------------------*/
//		w->popups = (NEUIK_Element *)malloc(2*sizeof(NEUIK_Element));
//		if (w->popups == NULL)
//		{
//			eNum = 3;
//			goto out;
//		}
//		neuik_Element_SetWindowPointer(pu, w);
//		neuik_Element_SetParentPointer(pu, parent);
//		if (neuik_Element_SetChildPopup(parent, pu))
//		{
//			eNum = 7;
//			goto out;
//		}
//		w->popups[0] = pu;
//		w->popups[1] = NULL;
//	}
//	else
//	{
//		/*--------------------------------------------------------------------*/
//		/* This is subsequent UI element, reallocate memory.                  */
//		/* This pointer array will be null terminated.                        */
//		/*--------------------------------------------------------------------*/
//		
//		/* determine the current length */
//		for (ctr = 0;;ctr++)
//		{
//			if (w->popups[ctr] == NULL)
//			{
//				len = 2 + ctr;
//				break;
//			}
//		}
//
//		w->popups = (NEUIK_Element*)realloc(w->popups, len*sizeof(NEUIK_Element));
//		if (w->popups == NULL)
//		{
//			eNum = 4;
//			goto out;
//		}
//		neuik_Element_SetWindowPointer(pu, w);
//		neuik_Element_SetParentPointer(pu, parent);
//		if (neuik_Element_SetChildPopup(parent, pu))
//		{
//			eNum = 7;
//			goto out;
//		}
//		w->popups[ctr]   = pu;
//		w->popups[ctr+1] = NULL;
//	}
//out:
//	if (eNum > 0)
//	{
//		NEUIK_RaiseError(funcName, errMsgs[eNum]);
//		eNum = 1;
//	}
//
//	return eNum;
//}

/*******************************************************************************
 *
 *  Name:          neuik_Window_Recreate
 *
 *  Description:   Destroy and then recreate the Window.
 *
 *                 This function is called if a parameter such as being 
 *                 resizable is changed while the window is visible.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_Recreate(
	NEUIK_Window * w)
{
	int                   eNum       = 0;
	int                   oldX;          /* when recreated, use this window placement */
	int                   oldY;          /* when recreated, use this window placement */
	Uint32                winFlags   = 0;
	NEUIK_WindowConfig  * wCfg       = NULL;
	static char           funcName[] = "NEUIK_Window_Recreate";
	static char         * errMsgs[]  = {"",              // [0] no error
		"SDL_CreateWindow returned NULL.",               // [1]
		"SDL_CreateRenderer returned NULL.",             // [2]
		"Window_GetConfig() failed.",                    // [3]
		"Argument `w` does not implement Window class.", // [4]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 4;
		goto out;
	}

	if (w->win == NULL) return 0; /* Nothing to recreate */

	wCfg = neuik_Window_GetConfig(w);

	if (w->shown)           winFlags |= SDL_WINDOW_SHOWN;
	if (wCfg->isResizable)  winFlags |= SDL_WINDOW_RESIZABLE;
	if (wCfg->isBorderless) winFlags |= SDL_WINDOW_BORDERLESS;

	/*------------------------------------------------------------------------*/
	/* Destroy the old window                                                 */
	/*------------------------------------------------------------------------*/
	SDL_GetWindowPosition(w->win, &oldX, &oldY);
	SDL_DestroyWindow(w->win);

	/*------------------------------------------------------------------------*/
	/* Recreate the window                                                    */
	/*------------------------------------------------------------------------*/
	w->win = SDL_CreateWindow(
		w->title,
		oldX,
		oldY,
		w->sizeW,
		w->sizeH,
		winFlags
	);

	if (w->win == NULL)
	{
		NEUIK_RaiseError(funcName, SDL_GetError());
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create the associated renderer for the window                          */
	/*------------------------------------------------------------------------*/
	w->rend = SDL_CreateRenderer(
		w->win, 
		-1, 
		SDL_RENDERER_ACCELERATED);
	if (w->rend == NULL)
	{
		NEUIK_RaiseError(funcName, SDL_GetError());
		eNum = 2;
		goto out;
	}

	if (w->icon != NULL)
	{
		if (w->icon->image != NULL) SDL_SetWindowIcon(w->win, w->icon->image);
	}

	/* Force a full redraw of the contained elements */
	if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
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
 *  Name:          NEUIK_Window_Create
 *
 *  Description:   Create the Window.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_Create(
	NEUIK_Window * w)
{
	int                   newW;         /* used when autoresizing the window */
	int                   newH;         /* used when autoresizing the window */
	int                   availW;       /* available element width */
	int                   availH;       /* available element height */
	int                   doResize   = 0;
	int                   eNum       = 0;
	Uint32                winFlags   = 0;
	RenderSize            rSize      = {0, 0};
	SDL_Rect              dispBnds;
	NEUIK_WindowConfig  * wCfg       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	static char           funcName[] = "NEUIK_Window_Create";
	static char         * errMsgs[]  = {"",                                   // [0] no error
		"SDL_CreateWindow returned NULL.",                                    // [1]
		"SDL_CreateRenderer returned NULL.",                                  // [2]
		"Window_GetConfig() failed.",                                         // [3]
		"`w->elem` does not implement Element class.",                        // [4]
		"Element_GetMinSize Failed.",                                         // [5]
		"Element_GetConfig returned NULL.",                                   // [6]
		"SDL_GetDisplayBounds() failed.",                                     // [7]
		"Aborting... Errors were already present before attempted creation.", // [8]
	};

	if (NEUIK_HasErrors())
	{
		/*--------------------------------------------------------------------*/
		/* Register the window for event handling purposes                    */
		/*--------------------------------------------------------------------*/
		neuik_RegisterWindow(NULL);
		eNum = 8;
		goto out;
	}

	wCfg = neuik_Window_GetConfig(w);

	if (w->shown)           winFlags |= SDL_WINDOW_SHOWN;
	if (wCfg->isResizable)  winFlags |= SDL_WINDOW_RESIZABLE;
	if (wCfg->isFullscreen) winFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (wCfg->isBorderless) winFlags |= SDL_WINDOW_BORDERLESS;
	if (wCfg->isMinimized)  winFlags |= SDL_WINDOW_MINIMIZED;
	if (wCfg->isMaximized)  winFlags |= SDL_WINDOW_MAXIMIZED;

	if (SDL_GetDisplayBounds(0, &dispBnds) != 0)
	{
		eNum = 7;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Perform initial calculations to determine the required size of the     */
	/* window                                                                 */
	/*------------------------------------------------------------------------*/
	if (w->elem != NULL && NEUIK_Element_IsShown(w->elem))
	{
		if (!neuik_Object_ImplementsClass(w->elem, neuik__Class_Element))
		{
			eNum = 4;
			goto out;
		}

		if (neuik_Element_GetMinSize(w->elem, &rSize))
		{
			eNum = 5;
			goto out;
		}

		eCfg = neuik_Element_GetConfig(w->elem);
		if (eCfg == NULL)
		{
			eNum = 6;
			goto out;
		}

		newW = w->sizeW;
		newH = w->sizeH;
		availW = w->sizeW - (eCfg->PadLeft + eCfg->PadRight);
		availH = w->sizeH - (eCfg->PadTop  + eCfg->PadBottom);

		if (rSize.w > availW || rSize.h > availH)
		{
			if (rSize.w > availW && (
					wCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ANY ||
					wCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ONLY_EXPAND
				))
			{
				doResize = 1;
				newW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
			}
			if (rSize.h > availH && (
					wCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ANY ||
					wCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ONLY_EXPAND
				))
			{
				doResize = 1;
				newH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
			}
		}

		/*--------------------------------------------------------------------*/
		/* Check and see if the window needs to shrink in size                */
		/*--------------------------------------------------------------------*/
		if (rSize.w < availW || rSize.h < availH)
		{
			if (rSize.w < availW && (
					wCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ANY ||
					wCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ONLY_CONTRACT
				))
			{
				doResize = 1;
				newW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
			}
			if (rSize.h < availH && (
					wCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ANY ||
					wCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ONLY_CONTRACT
				))
			{
				doResize = 1;
				newH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
			}
		}

		if (doResize) NEUIK_Window_SetSize(w, newW, newH);
	}

	/*------------------------------------------------------------------------*/
	/* Create the status window                                               */
	/*------------------------------------------------------------------------*/
	w->win = SDL_CreateWindow(
		w->title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w->sizeW,
		w->sizeH,
		winFlags
	);

	if (w->win == NULL)
	{
		NEUIK_RaiseError(funcName, SDL_GetError());
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create the associated renderer for the window                          */
	/*------------------------------------------------------------------------*/
	w->rend = SDL_CreateRenderer(
		w->win, 
		-1, 
		SDL_RENDERER_ACCELERATED);
	if (w->rend == NULL)
	{
		NEUIK_RaiseError(funcName, SDL_GetError());
		eNum = 2;
		goto out;
	}

	if (w->icon != NULL)
	{
		if (w->icon->image != NULL) SDL_SetWindowIcon(w->win, w->icon->image);
	}

	/*------------------------------------------------------------------------*/
	/* Store the initial location of the window                               */
	/*------------------------------------------------------------------------*/
	SDL_GetWindowPosition(w->win, &(w->posX), &(w->posY));

	/*------------------------------------------------------------------------*/
	/* Set the child pointers to this NEUIK_Window                            */
	/*------------------------------------------------------------------------*/
	if (w->mmenu != NULL)
	{
		neuik_MainMenu_SetWindowPointer(w->mmenu, w);
	}
	if (w->elem != NULL)
	{
		neuik_Element_SetWindowPointer(w->elem, w);
	}


	/*------------------------------------------------------------------------*/
	/* Register the window for event handling purposes                        */
	/*------------------------------------------------------------------------*/
	neuik_RegisterWindow(w);

	/* blank out the window so it's clear */
	NEUIK_Window_Redraw(w);

	if (rSize.w != 0 && rSize.h != 0)
	{
		availW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
		availH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
		// printf("Setting Minimum Window Size to: (%d, %d)\n", availW, availH);
		SDL_SetWindowMinimumSize(w->win, availW, availH);
	}

	NEUIK_Window_TriggerCallback(w, NEUIK_CALLBACK_ON_CREATED);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	return eNum;
}

// void 
// 	NEUIK_Window_Calculate(
// 			NEUIK_Window *w);

/*******************************************************************************
 *
 *  Name:          NEUIK_Window_CaptureEvent
 *
 *  Description:   Redraw the window and its contents.
 *
 *  Returns:       1 if the window captures the event, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_CaptureEvent(
	NEUIK_Window * w, 
	SDL_Event    * ev)
{
	int                   evCaputred = 0;
	int                   tempX;
	int                   tempY;
	SDL_Event           * e;
	SDL_KeyboardEvent   * keyEv;
	NEUIK_WindowConfig  * wCfg      = NULL;

	e = (SDL_Event*)(ev);

	wCfg = neuik_Window_GetConfig(w);

	/*------------------------------------------------------------------------*/
	/* Check if the event belongs to this window                              */
	/*------------------------------------------------------------------------*/
	// TODO 
	if (e->type == SDL_WINDOWEVENT)
	{
		// printf("Event for WindowID: %d\n", e->window.windowID);
		switch (e->window.event)
		{
			case SDL_WINDOWEVENT_MAXIMIZED:
				wCfg->isMaximized = 1;
				SDL_GetWindowSize(w->win, &(w->sizeW), &(w->sizeH));
				SDL_GetWindowPosition(w->win, &tempX, &tempY);
				// printf("Maximized: Window Position moved to: (%d, %d)\n", tempX, tempY);
				/* Force a full redraw of the contained elements */
				if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
				break;

			case SDL_WINDOWEVENT_RESTORED:
				wCfg->isMaximized = 0;
				wCfg->isMinimized = 0;
				SDL_GetWindowSize(w->win, &(w->sizeW), &(w->sizeH));
				SDL_GetWindowPosition(w->win, &tempX, &tempY);
				// printf("Restored: Window Position moved to: (%d, %d)\n", tempX, tempY);
				/* Force a full redraw of the contained elements */
				if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
				break;

			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				SDL_GetWindowSize(w->win, &(w->sizeW), &(w->sizeH));
				SDL_GetWindowPosition(w->win, &tempX, &tempY);
				// printf("Resized: Window Position moved to: (%d, %d)\n", tempX, tempY);
				/* Force a full redraw of the contained elements */
				if (w->elem != NULL) neuik_Element_ForceRedraw(w->elem);
				break;

			case SDL_WINDOWEVENT_MOVED:
				SDL_GetWindowPosition(w->win, &(w->posX), &(w->posY));
				// printf("Capture: Window Position moved to: (%d, %d)\n", w->posX, w->posY);
				break;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by a custom eventHandler [BEFORE]       */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Window_EventHandler_CaptureEvent(w, 
		NEUIK_EVENTHANDLER_BEFORE, &evCaputred, ev))
	{
		evCaputred = 0;
	}
	if (evCaputred) goto out;

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the main menu                        */
	/*------------------------------------------------------------------------*/
	if (w->mmenu != NULL)
	{
		evCaputred = NEUIK_MainMenu_CaptureEvent(w->mmenu, ev);
		if (evCaputred)
		{
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Main menu is active but, it didn't capture the event. Look for the */
		/* sorts of events which could cause the main menu to be deselected.  */
		/*--------------------------------------------------------------------*/
		e = (SDL_Event*)ev;
		switch (e->type)
		{
		case SDL_MOUSEBUTTONDOWN:
			NEUIK_MainMenu_Deselect(w->mmenu);
			break;
		case SDL_KEYDOWN:
			keyEv = (SDL_KeyboardEvent*)(e);
			if (keyEv->keysym.sym == SDLK_ESCAPE)
			{
				NEUIK_MainMenu_Deselect(w->mmenu);
			}
			break;
		}
	}

	if (w->eHT.Override != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Check if the event is captured by a custom eventHandler [OVERRIDE] */
		/*--------------------------------------------------------------------*/
		if (NEUIK_Window_EventHandler_CaptureEvent(w, 
			NEUIK_EVENTHANDLER_OVERRIDE, &evCaputred, ev))
		{
			evCaputred = 0;
		}
		if (evCaputred) goto out;
	}
	else if (w->elem != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Check if the event is captured by                                  */
		/*--------------------------------------------------------------------*/
		if (NEUIK_Element_IsShown(w->elem))
		{
			evCaputred = neuik_Element_CaptureEvent(w->elem, ev);
			if (evCaputred)
			{
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by a custom eventHandler [AFTER]        */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Window_EventHandler_CaptureEvent(w, 
		NEUIK_EVENTHANDLER_AFTER, &evCaputred, ev))
	{
		evCaputred = 0;
	}
	if (evCaputred) goto out;
out:
	return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_Redraw
 *
 *  Description:   Redraw the window and its contents.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_Redraw(
	NEUIK_Window  * w)
{
	int                   ctr;
	int                   doResize   = 0;
	int                   rv         = 0;
	int                   eNum       = 0; /* which error to report (if any) */
	int                   newW;           /* used when autoresizing the window */
	int                   newH;           /* used when autoresizing the window */
	int                   availW;         /* available element width */
	int                   availH;         /* available element height */
	int                   minW;           /* minimum element width */
	int                   minH;           /* minimum element height */
	int                   oldMinW;        /* old minimum element width */
	int                   oldMinH;        /* old minimum element height */
	NEUIK_Color         * clr;
	NEUIK_WindowConfig  * aCfg       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Element       * popup      = NULL;
	// NEUIK_PopUp         * popup      = NULL;
	SDL_Rect              dispBnds;
	SDL_Texture         * menuTex    = NULL;
	SDL_Texture         * elemTex    = NULL;
	SDL_Rect              destRect   = {0, 0, 0, 0};  /* destination rectangle */
	RenderSize            rSize      = {0, 0};
	RenderLoc             rLoc       = {0, 0};
	static char           funcName[] = "NEUIK_Window_Redraw";
	static char         * errMsgs[]  = {"",              // [ 0] no error
		"Element_GetConfig returned NULL.",              // [ 1]
		"Element_GetMinSize Failed.",                    // [ 2]
		"Element_Render returned NULL.",                 // [ 3]
		"MainMenu_GetMinSize Failed.",                   // [ 4]
		"MainMenu_Render returned NULL.",                // [ 5]
		"Argument `w` does not implement Window class.", // [ 6]
		"`w->elem` does not implement Element class.",   // [ 7]
		"`popup` does not implement Element class.",     // [ 8]
		"Popup Element_GetMinSize Failed.",              // [ 9]
		"Popup Element_GetConfig returned NULL.",        // [10]
		"Popup Element_Render returned NULL.",           // [11]
		"Popup Element_GetLocation Failed.",             // [12]
		"SDL_GetDisplayBounds() failed.",                // [13]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 6;
		goto out;
	}

	w->doRedraw = 0;

	/* select the correct WindowConfig to use (pointer or internal) */
	if (w->cfgPtr != NULL)
	{
		aCfg = w->cfgPtr;
	}
	else 
	{
		aCfg = w->cfg;
	}

	clr = &(aCfg->colorBG);
	SDL_SetRenderDrawColor(w->rend, clr->r, clr->g, clr->b, 255);
	SDL_RenderClear(w->rend);

	if (SDL_GetDisplayBounds(0, &dispBnds) != 0)
	{
		eNum = 7;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Redraw the contained Element                                           */
	/*------------------------------------------------------------------------*/
	if (w->elem != NULL && NEUIK_Element_IsShown(w->elem))
	{
		if (!neuik_Object_ImplementsClass(w->elem, neuik__Class_Element))
		{
			eNum = 7;
			goto out;
		}

		if (neuik_Element_GetMinSize(w->elem, &rSize))
		{
			eNum = 2;
			goto out;
		}

		eCfg = neuik_Element_GetConfig(w->elem);
		if (eCfg == NULL)
		{
			eNum = 1;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Check and see if the window needs to grow in size                  */
		/*--------------------------------------------------------------------*/
		newW = w->sizeW;
		newH = w->sizeH;
		availW = w->sizeW - (eCfg->PadLeft + eCfg->PadRight);
		availH = w->sizeH - (eCfg->PadTop  + eCfg->PadBottom);

		if (rSize.w > availW || rSize.h > availH)
		{
			if (rSize.w > availW && (
					aCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ANY ||
					aCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ONLY_EXPAND
				))
			{
				doResize = 1;
				newW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
			}
			if (rSize.h > availH && (
					aCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ANY ||
					aCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ONLY_EXPAND
				))
			{
				doResize = 1;
				newH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
			}
		}

		/*--------------------------------------------------------------------*/
		/* Check and see if the window needs to shrink in size                */
		/*--------------------------------------------------------------------*/
		if (rSize.w < availW || rSize.h < availH)
		{
			if (rSize.w < availW && (
					aCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ANY ||
					aCfg->autoResizeW == NEUIK_WINDOW_RESIZE_ONLY_CONTRACT
				))
			{
				doResize = 1;
				newW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
			}
			if (rSize.h < availH && (
					aCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ANY ||
					aCfg->autoResizeH == NEUIK_WINDOW_RESIZE_ONLY_CONTRACT
				))
			{
				doResize = 1;
				newH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
			}
		}

		/*--------------------------------------------------------------------*/
		/* Check and see if the minimum required window size has changed      */
		/*--------------------------------------------------------------------*/
		minW = rSize.w + (eCfg->PadLeft + eCfg->PadRight);
		minH = rSize.h + (eCfg->PadTop  + eCfg->PadBottom);
		SDL_GetWindowMinimumSize(w->win, &oldMinW, &oldMinH);

		if (minW != oldMinW || minH != oldMinH)
		{
			// printf("Redraw: Setting Minimum Window Size to: (%d, %d)\n", minW, minH);
			SDL_SetWindowMinimumSize(w->win, minW, minH);
			// printf("Redraw: Setting Window Position to: (%d, %d)\n", w->posX, w->posY);
			SDL_SetWindowPosition(w->win, w->posX, w->posY);
		}

		if (doResize)
		{
			NEUIK_Window_SetSize(w, newW, newH);
			neuik_Element_ForceRedraw(w->elem);
		}

		/*----------------------------------------------------------------*/
		/* Determine whether the contained element fills the window       */
		/*----------------------------------------------------------------*/
		if (eCfg->HFill || eCfg->VFill)
		{
			if (eCfg->HFill && eCfg->VFill)
			{
				/* The element fills the window vertically and horizonatally */
				rSize.w = w->sizeW - (eCfg->PadLeft + eCfg->PadRight);
				rSize.h = w->sizeH - (eCfg->PadTop  + eCfg->PadBottom);
			}
			else
			{

				if (eCfg->HFill)
				{
					/* The element fills the window only horizonatally */
					rSize.w = w->sizeW - (eCfg->PadLeft + eCfg->PadRight);
				}
				else
				{
					/* The element fills the window only vertically  */
					rSize.h = w->sizeH - (eCfg->PadTop  + eCfg->PadBottom);
				}
			}
		}

		/*--------------------------------------------------------------------*/
		/* Update the stored location before rendering the element. This is   */
		/* necessary as the location of this object will propagate to its     */
		/* child objects.                                                     */
		/*--------------------------------------------------------------------*/
		switch (eCfg->VJustify)
		{
			case NEUIK_VJUSTIFY_TOP:
				rLoc.y = eCfg->PadTop;
				break;
			case NEUIK_VJUSTIFY_CENTER:
				rLoc.y = w->sizeH/2 - (rSize.h/2);
				break;
			case NEUIK_VJUSTIFY_BOTTOM:
				rLoc.y = w->sizeH - (rSize.h + eCfg->PadBottom);
				break;
		}
		switch (eCfg->HJustify)
		{
			case NEUIK_HJUSTIFY_LEFT:
				rLoc.x = eCfg->PadLeft;
				break;
			case NEUIK_HJUSTIFY_CENTER:
				rLoc.x = w->sizeW/2 - (rSize.w/2);
				break;
			case NEUIK_HJUSTIFY_RIGHT:
				rLoc.x = w->sizeW - (rSize.w + eCfg->PadRight);
				break;
		}

		neuik_Element_StoreSizeAndLocation(w->elem, rSize, rLoc);
		if (doResize) neuik_Element_ForceRedraw(w->elem);

		elemTex = neuik_Element_Render(w->elem, &rSize, w->rend);
		if (elemTex != NULL)
		{
			destRect.x = rLoc.x;
			destRect.y = rLoc.y;
			destRect.w = rSize.w;
			destRect.h = rSize.h;
			SDL_RenderCopy(w->rend, elemTex, NULL, &destRect);
		}
		else
		{
			eNum = 3;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Redraw the MainMenu                                                    */
	/*------------------------------------------------------------------------*/
	rLoc.x = 0;
	rLoc.y = 0;
	if (w->mmenu != NULL)
	{
		if (NEUIK_MainMenu_GetSize(w->mmenu, &rSize))
		{
			eNum = 4;
			goto out;
		}

		// ConditionallyDestroyTexture(&menuTex);
		menuTex = NEUIK_RenderMainMenu(w->mmenu, &rSize, w->rend);
		if (menuTex != NULL)
		{
			destRect.x = 0;
			destRect.y = 0;
			destRect.w = rSize.w;
			destRect.h = rSize.h;
			SDL_RenderCopy(w->rend, menuTex, NULL, &destRect);
			NEUIK_MainMenu_StoreSizeAndLocation(w->mmenu, rSize, rLoc);
		}
		else
		{
			eNum = 5;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Redraw all contained popups                                            */
	/*------------------------------------------------------------------------*/
	if (w->popups != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			popup = w->popups[ctr];
			if (popup == NULL) break; /* End of array */
			if (!NEUIK_Element_IsShown(popup)) continue;

			if (!neuik_Object_ImplementsClass(popup, neuik__Class_Element))
			{
				eNum = 8;
				goto out;
			}

			if (neuik_Element_GetMinSize(popup, &rSize))
			{
				eNum = 9;
				goto out;
			}

			eCfg = neuik_Element_GetConfig(popup);
			if (eCfg == NULL)
			{
				eNum = 10;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Check if the popup has enough space to live within the window  */
			/*----------------------------------------------------------------*/
			if (neuik_Element_GetLocation(popup, &rLoc))
			{
				eNum = 12;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Update the stored location before rendering the element. This  */
			/* is necessary as the location of this object will propagate to  */
			/* its child objects.                                             */
			/*----------------------------------------------------------------*/
			// neuik_Element_StoreSizeAndLocation(popup, rSize, rLoc);
			if (doResize) neuik_Element_ForceRedraw(popup);

			elemTex = neuik_Element_Render(popup, &rSize, w->rend);
			if (elemTex != NULL)
			{
				destRect.x = rLoc.x;
				destRect.y = rLoc.y;
				destRect.w = rSize.w;
				destRect.h = rSize.h;
				SDL_RenderCopy(w->rend, elemTex, NULL, &destRect);
			}
			else
			{
				eNum = 11;
				goto out;
			}
		}
	}

	SDL_RenderPresent(w->rend);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return rv;
}


int  NEUIK_Window_NeedsRedraw(
	NEUIK_Window  * w)
{
	return 	w->doRedraw;
}

// }
// void 
// 	NEUIK_Window_Add(
// 			NEUIK_Window *w,
// 			void         *thing);


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_SetShown
 *
 *  Description:   Either set the window to be shown/hidden when created or 
 *                 show/hide an already created window.
 *
 *  Returns:       Nothing.
 *
 ******************************************************************************/
void NEUIK_Window_SetShown(
	NEUIK_Window * w, 
	int            show)
{
	w->shown = 0;
	if (w->win != NULL)
	{
		if (show == 0)
		{
			SDL_HideWindow(w->win);
			w->shown = 0;
		}
		else
		{
			SDL_ShowWindow(w->win);
			w->shown = 1;
		}
	}
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_SetSize
 *
 *  Description:   Set the size to be used for a window that is yet-to-be 
 *                 created or change the size of a previously created window.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int NEUIK_Window_SetSize(
	NEUIK_Window  * w, 
	int             width,
	int             height)
{
	int           eNum = 0;   /* which error to report (if any) */
	static char   funcName[] = "NEUIK_Window_SetSize";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Invalid window width (<=0) supplied.",          // [2]
		"Invalid window height (<=0) supplied.",         // [3]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	if (w != NULL)
	{
		if (width <= 0)
		{
			/* invalid window width */
			eNum = 2;
			goto out;
		}
		w->sizeW = width;
		if (height <= 0)
		{
			/* invalid window height */
			eNum = 3;
			goto out;
		}
		w->sizeH = height;

		if (w->shown && w->win != NULL)
		{
			SDL_SetWindowSize(w->win, width, height);
		}
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
 *  Name:          NEUIK_Window_SetTitle
 *
 *  Description:   Sets the title for a Window.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_SetTitle(
	NEUIK_Window  * w, 
	const char    * title)
{
	size_t        tLen       = 1; /* title length */
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "NEUIK_Window_SetTitle";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Unable to allocate memory.",                    // [2]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	w->doRedraw = 1;

	if (title != NULL)
	{
		if (w->title != NULL) free(w->title);

		tLen += strlen(title);
		w->title = (char*)malloc(tLen*sizeof(char));
		if (w->title == NULL)
		{
			eNum = 2; // Unable to allocate memory
			goto out;
		}
		strcpy(w->title, title);

		/*--------------------------------------------------------------------*/
		/* If the SDL window is already active, change the title now.         */
		/*--------------------------------------------------------------------*/
		if (w->win != NULL) w->updateTitle = 1;
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
 *  Name:          NEUIK_Window_SetIcon
 *
 *  Description:   Sets the icon for a Window.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Window_SetIcon(
	NEUIK_Window * w,
	NEUIK_Image  * img)
{
	int           eNum       = 0;    /* which error to report (if any) */
	static char   funcName[] = "NEUIK_Window_SetElement";
	static char * errMsgs[]  = {"",                       // [0] no error
		"Argument `w` does not implement Window class.",  // [1]
		"Argument `img` is not a valid NEUIK_Image.",     // [2]
		"Failure in `SetWindowPointer`.",                 // [3]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}
	if (!neuik_Object_IsClass(img, neuik__Class_Image))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Element_SetWindowPointer(img, w))
	{
		eNum = 3;
		goto out;
	}
	w->icon = img;

	/*------------------------------------------------------------------------*/
	/* If the SDL window is already active, set the icon now.                 */
	/*------------------------------------------------------------------------*/
	if (w->win != NULL)
	{
		if (w->icon->image != NULL) SDL_SetWindowIcon(w->win, w->icon->image);
	}


out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;

}


int NEUIK_Window_AddMenu(
	NEUIK_Window *w, 
	void         *mmenu)
{
	if (w != NULL && mmenu != NULL)
	{
		w->mmenu = mmenu;
	}

	return 0;
}


int NEUIK_Window_SetMainMenu(
	NEUIK_Window *w, 
	void         *mmenu)
{
	if (w != NULL && mmenu != NULL)
	{
		w->mmenu = mmenu;
	}

	return 0;	
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_SetCallback
 *
 *  Description:   Set the function and arguments for the named callback event.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int  NEUIK_Window_SetCallback(
	NEUIK_Window * w,       /* the window to set the callback for */
	const char   * cbName,  /* the name of the callback to set */
	void         * cbFunc,  /* the function to use for the callback */
	void         * cbArg1,  /* the first argument to pass to the cbFunc */
	void         * cbArg2)  /* the second argument to pass to the cbFunc */
{
	int           eNum       = 0;
	static char   funcName[] = "NEUIK_Window_SetCallback";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Callback Name `cbName` is NULL.",               // [2]
		"Callback Name `cbName` is blank.",              // [3]
		"Callback Name `cbName` unknown.",               // [4]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
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
		if (w->eCT.OnClick) free(w->eCT.OnClick);
		w->eCT.OnClick = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnClicked", cbName))
	{
		if (w->eCT.OnClicked) free(w->eCT.OnClicked);
		w->eCT.OnClicked = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnCreated", cbName))
	{
		if (w->eCT.OnCreated) free(w->eCT.OnCreated);
		w->eCT.OnCreated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnHover", cbName))
	{
		if (w->eCT.OnHover) free(w->eCT.OnHover);
		w->eCT.OnHover = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnMouseEnter", cbName))
	{
		if (w->eCT.OnMouseEnter) free(w->eCT.OnMouseEnter);
		w->eCT.OnMouseEnter = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnMouseLeave", cbName))
	{
		if (w->eCT.OnMouseLeave) free(w->eCT.OnMouseLeave);
		w->eCT.OnMouseLeave = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnSelected", cbName))
	{
		if (w->eCT.OnSelected) free(w->eCT.OnSelected);
		w->eCT.OnSelected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnDeselected", cbName))
	{
		if (w->eCT.OnDeselected) free(w->eCT.OnDeselected);
		w->eCT.OnDeselected = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnActivated", cbName))
	{
		if (w->eCT.OnActivated) free(w->eCT.OnActivated);
		w->eCT.OnActivated = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
	}
	else if (!strcmp("OnTextChanged", cbName))
	{
		if (w->eCT.OnTextChanged) free(w->eCT.OnTextChanged);
		w->eCT.OnTextChanged = NEUIK_NewCallback(cbFunc, cbArg1, cbArg2);
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
 *  Name:          NEUIK_Window_SetBindingCallback
 *
 *  Description:   Set the bindID to be sent when the specified callback is 
 *                 triggered.
 *
 *                 This alternative callback procedure should only be used if 
 *                 the standard `NEUIK_Window_SetCallback` function can not be
 *                 used, like for instance in a binding with another language. 
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Window_SetBindingCallback(
	NEUIK_Window * w,      /* the window to set the callback for */
	const char   * cbName, /* the name of the callback to set */
	unsigned int   bindID) /* A unique number to identify this callback instance */
{
	int                  eNum       = 0;
	static char          funcName[] = "NEUIK_Window_SetBindingCallback";
	static char        * errMsgs[]  = {"",               // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Callback Name `cbName` is NULL.",               // [2]
		"Callback Name `cbName` is blank.",              // [3]
		"Callback Name `cbName` unknown.",               // [4]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
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
		if (w->eCT.OnClick) free(w->eCT.OnClick);
		w->eCT.OnClick = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnClicked", cbName))
	{
		if (w->eCT.OnClicked) free(w->eCT.OnClicked);
		w->eCT.OnClicked = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnCreated", cbName))
	{
		if (w->eCT.OnCreated) free(w->eCT.OnCreated);
		w->eCT.OnCreated = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnHover", cbName))
	{
		if (w->eCT.OnHover) free(w->eCT.OnHover);
		w->eCT.OnHover = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnMouseEnter", cbName))
	{
		if (w->eCT.OnMouseEnter) free(w->eCT.OnMouseEnter);
		w->eCT.OnMouseEnter = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnMouseLeave", cbName))
	{
		if (w->eCT.OnMouseLeave) free(w->eCT.OnMouseLeave);
		w->eCT.OnMouseLeave = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnSelected", cbName))
	{
		if (w->eCT.OnSelected) free(w->eCT.OnSelected);
		w->eCT.OnSelected = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnDeselected", cbName))
	{
		if (w->eCT.OnDeselected) free(w->eCT.OnDeselected);
		w->eCT.OnDeselected = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnActivated", cbName))
	{
		if (w->eCT.OnActivated) free(w->eCT.OnActivated);
		w->eCT.OnActivated = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnDeactivated", cbName))
	{
		if (w->eCT.OnDeactivated) free(w->eCT.OnDeactivated);
		w->eCT.OnDeactivated = NEUIK_NewBindingCallback(bindID);
	}
	else if (!strcmp("OnTextChanged", cbName))
	{
		if (w->eCT.OnTextChanged) free(w->eCT.OnTextChanged);
		w->eCT.OnTextChanged = NEUIK_NewBindingCallback(bindID);
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
 *  Name:          NEUIK_Window_TriggerCallback
 *
 *  Description:   Trigger a callback of the specified type (if not NULL).
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Window_TriggerCallback(
	NEUIK_Window  * w,       /* The window whose callback should be triggered */
	int             cbType)  /* Which callback to trigger */
{
	int           eNum       = 0;
	static char   funcName[] = "NEUIK_Window_TriggerCallback";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Unknown Callback Type `cbType`.",               // [2]
	};


	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	switch (cbType)
	{
		case NEUIK_CALLBACK_ON_CLICK:
			if (w->eCT.OnClick) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnClick, w);
			}
			break;

		case NEUIK_CALLBACK_ON_CLICKED:
			if (w->eCT.OnClicked) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnClicked, w);
			}
			break;

		case NEUIK_CALLBACK_ON_CREATED:
			if (w->eCT.OnCreated) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnCreated, w);
			}
			break;

		case NEUIK_CALLBACK_ON_HOVER:
			if (w->eCT.OnHover) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnHover, w);
			}
			break;

		case NEUIK_CALLBACK_ON_MOUSE_ENTER:
			if (w->eCT.OnMouseEnter) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnMouseEnter, w);
			}
			break;

		case NEUIK_CALLBACK_ON_MOUSE_LEAVE:
			if (w->eCT.OnMouseLeave) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnMouseLeave, w);
			}
			break;

		case NEUIK_CALLBACK_ON_SELECTED:
			if (w->eCT.OnSelected) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnSelected, w);
			}
			break;

		case NEUIK_CALLBACK_ON_DESELECTED:
			if (w->eCT.OnDeselected) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnDeselected, w);
			}
			break;

		case NEUIK_CALLBACK_ON_ACTIVATED:
			if (w->eCT.OnActivated) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnActivated, w);
			}
			break;

		case NEUIK_CALLBACK_ON_TEXT_CHANGED:
			if (w->eCT.OnTextChanged) 
			{
				NEUIK_Callback_Trigger(w->eCT.OnTextChanged, w);
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
 *  Name:          neuik_Window_GetConfig
 *
 *  Description:   Return the pointer to the active WindowConfig.
 *
 *  Returns:       NULL if error; otherwise the pointer to the active config.
 *
 ******************************************************************************/
NEUIK_WindowConfig * neuik_Window_GetConfig(
	NEUIK_Window * w)
{
	int                  eNum       = 0; /* which error to report (if any) */
	NEUIK_WindowConfig * aCfg       = NULL;
	static char          funcName[] = "neuik_Window_GetConfig";
	static char        * errMsgs[]  = {"",               // [0] no error
		"Argument `w` does not implement Window class.", // [1]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	/* select the correct WindowConfig to use (pointer or internal) */
	if (w->cfgPtr != NULL)
	{
		aCfg = w->cfgPtr;
	}
	else 
	{
		aCfg = w->cfg;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return aCfg;
}


/*******************************************************************************
 *
 *  Name:          neuik_Window_TakeFocus
 *
 *  Description:   The element which calls this function will take focus within
 *                 this window. The focused element will get a chance to capture
 *                 events before any other element.
 *
 *  Returns:       Nothing.
 *
 ******************************************************************************/
void neuik_Window_TakeFocus(
	NEUIK_Window * w,
	void         * elem)
{
	NEUIK_ElementBase * eBase;

	if (w != NULL)
	{
		if (w->focused != NULL) neuik_Element_Defocus(w->focused);

		if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
		{
			return;
		}

		eBase->eSt.hasFocus = 1;
		w->focused = elem;
	}
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_SetEventHandler
 *
 *  Description:   Set the function and arguments for the named event handler.
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int  NEUIK_Window_SetEventHandler(
	NEUIK_Window  * w,       /* the window to set the callback for */
	const char    * eHName,  /* the name of the callback to set */
	void          * eHFunc,  /* the function to use for the callback */
	void          * eHArg1,  /* the first argument to pass to the cbFunc */
	void          * eHArg2)  /* the second argument to pass to the cbFunc */
{
	int           eNum       = 0;
	static char   funcName[] = "NEUIK_Window_SetEventHandler";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"EventHandler Name `eHName` is NULL.",           // [2]
		"EventHandler Name `eHName` is blank.",          // [3]
		"EventHandler Name `eHName` unknown.",           // [4]
	};


	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}

	if (eHName == NULL)
	{
		eNum = 2;
		goto out;
	}
	else if (eHName[0] == 0)
	{
		eNum = 3;
		goto out;
	}
	else if (!strcmp("Before", eHName))
	{
		if (w->eHT.Before) free(w->eHT.Before);
		w->eHT.Before = NEUIK_NewEventHandler(eHFunc, eHArg1, eHArg2);
	}
	else if (!strcmp("After", eHName))
	{
		if (w->eHT.After) free(w->eHT.After);
		w->eHT.After = NEUIK_NewEventHandler(eHFunc, eHArg1, eHArg2);
	}
	else if (!strcmp("Override", eHName))
	{
		if (w->eHT.Override) free(w->eHT.Override);
		w->eHT.Override = NEUIK_NewEventHandler(eHFunc, eHArg1, eHArg2);
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
 *  Name:          NEUIK_Window_EventHandler_CaptureEvent
 *
 *  Description:   Check if an eventHandler captures an event (if not NULL).
 *
 *  Returns:       Non-zero if error, 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Window_EventHandler_CaptureEvent(
	NEUIK_Window    * w,        /* The window whose callback should be triggered */
	int               eHType,   /* Which eventHandler to check with an event */
	int             * captured, /* Whether or not the event was captured */
	ptrTo_SDL_Event   ev)       /* The SDL_Event to check */
{
	int           eNum       = 0;
	static char   funcName[] = "NEUIK_Window_EventHandler_CaptureEvent";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `w` does not implement Window class.", // [1]
		"Unknown EventHandler `eHType`.",                // [2]
		"Arg pointer `captured` is NULL.",               // [3]
		"Event capture [BEFORE] failed.",                // [4]
		"Event capture [AFTER] failed.",                 // [5]
		"Event capture [OVERRIDE] failed.",              // [6]
	};


	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}
	else if (captured == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*captured) = 0;

	switch (eHType)
	{
		case NEUIK_EVENTHANDLER_BEFORE:
			if (w->eHT.Before) 
			{
				if (NEUIK_EventHandler_Capture(w->eHT.Before, w, captured, ev))
				{
					/* Error ocurred during event handling */
					eNum = 4;
					goto out;
				}
			}
			break;

		case NEUIK_EVENTHANDLER_AFTER:
			if (w->eHT.After) 
			{
				if (NEUIK_EventHandler_Capture(w->eHT.After, w, captured, ev))
				{
					/* Error ocurred during event handling */
					eNum = 5;
					goto out;
				}
			}
			break;

		case NEUIK_EVENTHANDLER_OVERRIDE:
			if (w->eHT.Override) 
			{
				if (NEUIK_EventHandler_Capture(w->eHT.Override, w, captured, ev))
				{
					/* Error ocurred during event handling */
					eNum = 6;
					goto out;
				}
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
 *  Name:          NEUIK_Window_SetElement
 *
 *  Description:   Set the element contained by this window.
 *
 *  Returns:       Non-zero if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Window_SetElement(
	NEUIK_Window  * w,
	NEUIK_Element   elem)
{
	int           eNum       = 0;    /* which error to report (if any) */
	static char   funcName[] = "NEUIK_Window_SetElement";
	static char * errMsgs[]  = {"",                          // [0] no error
		"Argument `w` does not implement Window class.",     // [1]
		"Argument `elem` does not implement Element class.", // [2]
		"Failure in `SetWindowPointer`.",                    // [3]
	};

	if (!neuik_Object_IsClass(w, neuik__Class_Window))
	{
		eNum = 1;
		goto out;
	}
	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Element_SetWindowPointer(elem, w))
	{
		eNum = 3;
		goto out;
	}
	w->elem = elem;

out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


