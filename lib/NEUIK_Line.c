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
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*);


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
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
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

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(elem, xSurf, rlMod, NULL))
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
	eBase->eSt.doRedraw = 0;

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
	static char    funcName[] = "NEUIK_Line_SetThickness";
	static char  * errMsgs[]  = {"",             // [0] no error
		"Argument `line` is not of Line class.", // [1]
		"Argument `px` can not be negative.",    // [2]
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
	neuik_Element_RequestRedraw(line);

out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

