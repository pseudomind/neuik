/*******************************************************************************
 * Copyright (c) 2014-2020, Michael Leimon <leimon@gmail.com>
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
#include "NEUIK_Line.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Line(void **);
int neuik_Object_Free__Line(void *);

int neuik_Element_GetMinSize__Line(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Line(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Line_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Line,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Line,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Line_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Line,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Line,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Line
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Line()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Line";
	static char  * errMsgs[]  = {"",                // [0] no error
		"NEUIK library must be initialized first.", // [1]
		"Failed to register `Line` object class .", // [2]
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
		"NEUIK_Line",                     // className
		"A vertical or horizontal line.", // classDescription
		neuik__Set_NEUIK,                 // classSet
		neuik__Class_Element,             // superClass
		&neuik_Line_BaseFuncs,            // baseFuncs
		NULL,                             // classFuncs
		&neuik__Class_Line))              // newClass
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
 *  Name:          neuik_Object_New__Line
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Line(
		void ** linePtr)
{
	int                  eNum       = 0;
	NEUIK_Line         * line       = NULL;
	NEUIK_Element      * sClassPtr  = NULL;
	static NEUIK_Color   dClr       = COLOR_GRAY;
	static char          funcName[] = "neuik_Object_New__Line";
	static char        * errMsgs[]  = {"",                                  // [0] no error
		"Output Argument `linePtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                      // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                         // [3]
		"Failure in function `neuik.NewElement`.",                          // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",                // [5]
		"Argument `linePtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",        // [7]
	};

	if (linePtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*linePtr) = (NEUIK_Line*) malloc(sizeof(NEUIK_Line));
	line = *linePtr;
	if (line == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Line, 
			NULL,
			&(line->objBase)))
	{
		eNum = 3;
		goto out;
	}
	line->color = dClr;

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(line->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(line, &neuik_Line_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(line, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(line, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(line, "hovered"))
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
 *  Name:          neuik_Object_Free__Line
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Line(
	void  * linePtr)
{
	int           eNum       = 0;    /* which error to report (if any) */
	NEUIK_Line  * line       = NULL;
	static char   funcName[] = "neuik_Object_Free__Line";
	static char * errMsgs[]  = {"",                      // [0] no error
		"Argument `linePtr` is NULL.",                   // [1]
		"Argument `linePtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",      // [3]
	};

	if (linePtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(linePtr, neuik__Class_Line))
	{
		eNum = 2;
		goto out;
	}
	line = (NEUIK_Line*)linePtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(line->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(line);
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
 *  Name:          NEUIK_NewHLine
 *
 *  Description:   Create a new horizontal NEUIK_Line.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewHLine(
	NEUIK_Line ** linePtr)
{
	int                 eNum       = 0; /* which error to report (if any) */
	NEUIK_Line        * line       = NULL;
	NEUIK_ElementBase * eBase      = NULL; 
	static char         funcName[] = "NEUIK_NewHLine";
	static char       * errMsgs[]  = {"",                                   // [0] no error
		"Failure in function `neuik_Object_New__Line`.",                    // [1]
		"Argument `linePtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
	};

	if (neuik_Object_New__Line((void**)linePtr))
	{
		eNum = 1;
		goto out;
	}
	line = *linePtr;

	/*------------------------------------------------------------------------*/
	/* Configure the line to be horizontal                                    */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(line, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	eBase->eCfg.HFill = 1;

	line->orientation = 0;
	line->thickness   = 1;
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
 *  Name:          NEUIK_NewVLine
 *
 *  Description:   Create a new vertical NEUIK_Line.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewVLine(
	NEUIK_Line ** linePtr)
{
	int                 eNum       = 0; /* which error to report (if any) */
	NEUIK_Line        * line       = NULL;
	NEUIK_ElementBase * eBase      = NULL; 
	static char         funcName[] = "NEUIK_NewVLine";
	static char       * errMsgs[]  = {"",                                   // [0] no error
		"Failure in function `neuik_Object_New__Line`.",                    // [1]
		"Argument `linePtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
	};

	if (neuik_Object_New__Line((void**)linePtr))
	{
		eNum = 1;
		goto out;
	}
	line = *linePtr;

	/*------------------------------------------------------------------------*/
	/* Configure the line to be horizontal                                    */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(line, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	eBase->eCfg.VFill = 1;

	line->orientation = 1;
	line->thickness   = 1;
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
 *  Name:          neuik_Element_GetMinSize__Line
 *
 *  Description:   Returns the rendered size of a given line.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Line(
	NEUIK_Element    elem,
	RenderSize     * rSize)
{
	int           eNum       = 0;    /* which error to report (if any) */
	NEUIK_Line  * line       = NULL;
	static char   funcName[] = "neuik_Element_GetMinSize__Line";
	static char * errMsgs[]  = {"",              // [0] no error
		"Argument `elem` is not of Line class.", // [1]
		"Invalid line orientation.",             // [2]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(elem, neuik__Class_Line))
	{
		eNum = 1;
		goto out;
	}
	line = (NEUIK_Line*)elem;
	
	if (line->orientation == 0)
	{
		rSize->w = 5;
		rSize->h = line->thickness;
	}
	else if (line->orientation == 1)
	{
		rSize->w = line->thickness;
		rSize->h = 5;
	}
	else
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
 *  Name:          NEUIK_RenderLine
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetMinSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Line(
	NEUIK_Element   elem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                 eNum       = 0; /* which error to report (if any) */
	const NEUIK_Color * lClr       = NULL;
	SDL_Renderer      * rend       = NULL;
	SDL_Rect            rect;
	RenderLoc           rl;
	NEUIK_Line        * line       = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "neuik_Element_Render__Line";
	static char       * errMsgs[]  = {"", // [0] no error
		"Argument `elem` is not of Line class.",                         // [1]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"", // [3]
		"Invalid specified `rSize` (negative values).",                  // [4]
		"Failure in neuik_Element_RedrawBackground().",                  // [5]
		"Invalid line orientation.",                                     // [6]
	};

	if (!neuik_Object_IsClass(elem, neuik__Class_Line))
	{
		eNum = 1;
		goto out;
	}
	line = (NEUIK_Line*)elem;

	if (neuik_Object_GetClassObject(line, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
		goto out;
	}
	if (mock)
	{
		/*--------------------------------------------------------------------*/
		/* This is a mock render operation; don't draw anything...            */
		/*--------------------------------------------------------------------*/
		goto out;
	}

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(elem, rlMod, NULL))
	{
		eNum = 5;
		goto out;
	}
	rl = eBase->eSt.rLoc;

	/* use the specified line color */
	lClr = &(line->color);
	SDL_SetRenderDrawColor(rend, lClr->r, lClr->g, lClr->b, 255);


	if (line->orientation == 0)
	{
		/* HLine */
		if (line->thickness == 1)
		{
			SDL_RenderDrawLine(rend, 
				rl.x,                  rl.y, 
				rl.x + (rSize->w - 1), rl.y); 
		}
		else if (line->thickness > 1)
		{
			rect.x = rl.x;
			rect.y = rl.y;
			rect.w = rSize->w - 1;
			rect.h = line->thickness;
			SDL_RenderFillRect(rend, &rect);
		}
	}
	else if (line->orientation == 1)
	{
		/* VLine */
		if (line->thickness == 1)
		{
			SDL_RenderDrawLine(rend, 
				rl.x, rl.y, 
				rl.x, rl.y + (rSize->h - 1));
		}
		else if (line->thickness > 1)
		{
			rect.x = rl.x;
			rect.y = rl.y;
			rect.w = line->thickness;
			rect.h = rSize->h - 1;
			SDL_RenderFillRect(rend, &rect);
		}
	}
	else
	{
		/* Incorrect orientation */
		eNum = 6;
		goto out;
	}

out:
	if (eBase != NULL)
	{
		if (!mock) eBase->eSt.doRedraw = 0;
	}

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_Line_SetThickness
 *
 *  Description:   Set the vertical spacing parameter of a vertical group.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Line_SetThickness(
	NEUIK_Line  * line,
	int           px)
{
	int            eNum       = 0;    /* which error to report (if any) */
	RenderSize     rSize;
	RenderLoc      rLoc;
	static char    funcName[] = "NEUIK_Line_SetThickness";
	static char  * errMsgs[]  = {"",                        // [0] no error
		"Argument `line` is not of Line class.",            // [1]
		"Argument `px` can not be negative.",               // [2]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
	};

	if (!neuik_Object_IsClass(line, neuik__Class_Line))
	{
		eNum = 1;
		goto out;
	}
	if (px < 0)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no effective change in spacing; don't do anything          */
	/*------------------------------------------------------------------------*/
	if (px == line->thickness) goto out;

	line->thickness = px;
	if (neuik_Element_GetSizeAndLocation(line, &rSize, &rLoc))
	{
		eNum = 3;
		goto out;
	}
	neuik_Element_RequestRedraw(line, rLoc, rSize);
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
 *  Name:          NEUIK_Line_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_Line_Configure(
	NEUIK_Line * line,
	const char * set0,
	...)
{
	int                  ns; /* number of items from sscanf */
	int                  ctr;
	int                  nCtr;
	int                  eNum      = 0; /* which error to report (if any) */
	int                  doRedraw  = FALSE;
	int                  isBool    = FALSE;
	int                  boolVal   = FALSE;
	int                  typeMixup;
	char                 buf[4096];
	RenderSize           rSize;
	RenderLoc            rLoc;
	va_list              args;
	char               * strPtr    = NULL;
	char               * name      = NULL;
	char               * value     = NULL;
	const char         * set       = NULL;
	NEUIK_Color          clr;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char * boolNames[] = {
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	static char * valueNames[] = {
		"Color",
		NULL,
	};
	static char   funcName[] = "NEUIK_Line_Configure";
	static char * errMsgs[]  = {"", // [ 0] no error
		"Argument `line` does not implement Label class.",      // [ 1]
		"`name=value` string is too long.",                     // [ 2]
		"Invalid `name=value` string.",                         // [ 3]
		"ValueType name used as BoolType, skipping.",           // [ 4]
		"BoolType name unknown, skipping.",                     // [ 5]
		"NamedSet.name is NULL, skipping..",                    // [ 6]
		"NamedSet.name is blank, skipping..",                   // [ 7]
		"Color value invalid; should be comma separated RGBA.", // [ 8]
		"Color value invalid; RGBA value range is 0-255.",      // [ 9]
		"Failure in `neuik_Element_GetSizeAndLocation()`.",     // [10]
		"BoolType name used as ValueType, skipping.",           // [11]
		"NamedSet.name type unknown, skipping.",                // [12]
	};

	if (!neuik_Object_IsClass(line, neuik__Class_Line))
	{
		eNum = 1;
		goto out;
	}
	set = set0;

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
		if (ctr > 0)
		{
			/* before starting */
			set = va_arg(args, const char *);
		}

		isBool = FALSE;
		name   = NULL;
		value  = NULL;

		if (set == NULL) break;

		if (strlen(set) > 4095)
		{
			NEUIK_RaiseError(funcName, errMsgs[2]);
			set = va_arg(args, const char *);
			continue;
		}
		else
		{
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
					NEUIK_RaiseError(funcName, errMsgs[3]);
				}

				isBool  = TRUE;
				boolVal = TRUE;
				name    = buf;
				if (buf[0] == '!')
				{
					boolVal = FALSE;
					name    = buf + 1;
				}
				if (boolVal)
				{
					/*--------------------------------------------------------*/
					/* Do nothing; this is to resolve an unused var warning.  */
					/*--------------------------------------------------------*/
				}
			}
			else
			{
				*strPtr = 0;
				strPtr++;
				if (*strPtr == 0)
				{
					/* `name=value` string is missing a value */
					NEUIK_RaiseError(funcName, errMsgs[3]);
					set = va_arg(args, const char *);
					continue;
				}
				name  = buf;
				value = strPtr;
			}
		}

		if (isBool)
		{
			/*----------------------------------------------------------------*/
			/* Check for boolean parameter setting.                           */
			/*----------------------------------------------------------------*/
			/* Bool parameter not found; may be mixup or mistake.             */
			/*----------------------------------------------------------------*/
			typeMixup = FALSE;
			for (nCtr = 0;; nCtr++)
			{
				if (valueNames[nCtr] == NULL) break;

				if (!strcmp(valueNames[nCtr], name))
				{
					typeMixup = TRUE;
					break;
				}
			}

			if (typeMixup)
			{
				/* A value type was mistakenly used as a bool type */
				NEUIK_RaiseError(funcName, errMsgs[4]);
			}
			else
			{
				/* An unsupported name was used as a bool type */
				NEUIK_RaiseError(funcName, errMsgs[5]);
			}
		}
		else
		{
			if (name == NULL)
			{
				NEUIK_RaiseError(funcName, errMsgs[6]);
			}
			else if (name[0] == 0)
			{
				NEUIK_RaiseError(funcName, errMsgs[7]);
			}
			else if (!strcmp("Color", name))
			{
				/*------------------------------------------------------------*/
				/* Check for empty value errors.                              */
				/*------------------------------------------------------------*/
				if (value == NULL)
				{
					NEUIK_RaiseError(funcName, errMsgs[8]);
					continue;
				}
				if (value[0] == '\0')
				{
					NEUIK_RaiseError(funcName, errMsgs[8]);
					continue;
				}

				ns = sscanf(value, "%d,%d,%d,%d", &clr.r, &clr.g, &clr.b, &clr.a);
				/*------------------------------------------------------------*/
				/* Check for EOF, incorrect # of values, & out of range vals. */
				/*------------------------------------------------------------*/
			#ifndef WIN32
				if (ns == EOF || ns < 4)
			#else
				if (ns < 4)
			#endif /* WIN32 */
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
				if (line->color.r == clr.r &&
					line->color.g == clr.g &&
					line->color.b == clr.b &&
					line->color.a == clr.a) continue;

				/* else: The previous setting was changed */
				line->color = clr;
				doRedraw = TRUE;
			}
			else
			{
				typeMixup = FALSE;
				for (nCtr = 0;; nCtr++)
				{
					if (boolNames[nCtr] == NULL) break;

					if (!strcmp(boolNames[nCtr], name))
					{
						typeMixup = TRUE;
						break;
					}
				}

				if (typeMixup)
				{
					/* A bool type was mistakenly used as a value type */
					NEUIK_RaiseError(funcName, errMsgs[11]);
				}
				else
				{
					/* An unsupported name was used as a value type */
					NEUIK_RaiseError(funcName, errMsgs[12]);
				}
			}
		}
	}
	va_end(args);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (doRedraw)
	{
		if (neuik_Element_GetSizeAndLocation(line, &rSize, &rLoc))
		{
			eNum = 10;
			NEUIK_RaiseError(funcName, errMsgs[eNum]);
			eNum = 1;
		}
		else
		{
			neuik_Element_RequestRedraw(line, rLoc, rSize);
		}
	}

	return eNum;
}

