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
#include "NEUIK_Plot.h"
#include "NEUIK_Plot2D.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;
extern int neuik__Report_Debug;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Plot2D(void ** vgPtr);
int neuik_Object_Free__Plot2D(void * vgPtr);

int neuik_Element_GetMinSize__Plot2D(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Plot2D(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Plot2D_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Plot2D,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Plot2D,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Plot2D_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Plot2D,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Plot2D,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Plot2D
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Plot2D()
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_RegisterClass_Plot2D";
	static char * errMsgs[]  = {"",                  // [0] no error
		"NEUIK library must be initialized first.",   // [1]
		"Failed to register `Plot2D` object class .", // [2]
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
		"NEUIK_Plot2D",                                           // className
		"An plot element which displays data in two dimensions.", // classDescription
		neuik__Set_NEUIK,                                         // classSet
		neuik__Class_Plot,                                        // superClass
		&neuik_Plot2D_BaseFuncs,                                  // baseFuncs
		NULL,                                                     // classFuncs
		&neuik__Class_Plot2D))                                    // newClass
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
 *  Name:          neuik_Object_New__Plot2D
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Plot2D(
	void ** pltPtr)
{
	int             eNum       = 0;
	NEUIK_Plot    * plot       = NULL;
	NEUIK_Plot2D  * plot2d     = NULL;
	NEUIK_Element * sClassPtr  = NULL;
	static char     funcName[] = "neuik_Object_New__Plot2D";
	static char   * errMsgs[]  = {"",                                      // [0] no error
		"Output Argument `pltPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                     // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                        // [3]
		"Failure in function `neuik.NewElement`.",                         // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",               // [5]
		"Argument `pltPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in function `NEUIK_Container_AddElement()`.",             // [7]
		"Failure in function `NEUIK_NewCanvas()`.",                        // [8]
		"Failure in function `NEUIK_Element_Configure()`.",                // [9]
	};

	if (pltPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*pltPtr) = (NEUIK_Plot2D*) malloc(sizeof(NEUIK_Plot2D));
	plot2d = *pltPtr;
	if (plot2d == NULL)
	{
		eNum = 2;
		goto out;
	}
	/* Allocation successful */

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Plot2D, 
			NULL,
			&(plot2d->objBase)))
	{
		eNum = 3;
		goto out;
	}

	if (NEUIK_NewCanvas(&plot2d->drawing_background))
	{
		eNum = 8;
		goto out;
	}
	if (NEUIK_Element_Configure(plot2d->drawing_background, "FillAll", NULL))
	{
		eNum = 9;
		goto out;
	}

	if (NEUIK_NewCanvas(&plot2d->drawing_ticmarks))
	{
		eNum = 8;
		goto out;
	}
	if (NEUIK_Element_Configure(plot2d->drawing_ticmarks, "FillAll", NULL))
	{
		eNum = 9;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(plot2d->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Plot, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(plot2d, &neuik_Plot2D_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 6;
		goto out;
	}
	if (NEUIK_Container_AddElement(plot->drawing, plot2d->drawing_background))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Container_AddElement(plot->drawing, plot2d->drawing_ticmarks))
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
 *  Name:          NEUIK_NewPlot2D
 *
 *  Description:   Create and return a pointer to a new NEUIK_Plot2D.
 *
 *                 Wrapper function to neuik_Object_New__Plot2D.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewPlot2D(
	NEUIK_Plot2D ** pltPtr)
{
	return neuik_Object_New__Plot2D((void**)pltPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Plot2D(
	void  * pltPtr)
{
	int            eNum       = 0;    /* which error to report (if any) */
	NEUIK_Plot2D * plt        = NULL;
	static char    funcName[] = "neuik_Object_Free__Plot2D";
	static char  * errMsgs[]  = {"",                    // [0] no error
		"Argument `pltPtr` is NULL.",                   // [1]
		"Argument `pltPtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",     // [3]
	};

	if (pltPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(pltPtr, neuik__Class_Plot2D))
	{
		eNum = 2;
		goto out;
	}
	plt = (NEUIK_Plot2D*)pltPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(plt->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(plt);
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
 *  Name:          neuik_Element_GetMinSize__Plot2D
 *
 *  Description:   Returns the rendered size of a given Plot2D.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Plot2D(
	NEUIK_Element   pltElem,
	RenderSize    * rSize)
{
	int            eNum       = 0;   /* which error to report (if any) */
	NEUIK_Plot2D * plt        = NULL;
	NEUIK_Plot   * plot       = NULL;
	static char    funcName[] = "neuik_Element_GetMinSize__Plot2D";
	static char  * errMsgs[]  = {"",                                        // [0] no error
		"Argument `pltElem` is not of Plot2D class.",                       // [1]
		"Argument `pltElem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Failure in `neuik_Element_GetSize()`.",                            // [3]
	};

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceding                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(pltElem, neuik__Class_Plot2D))
	{
		eNum = 1;
		goto out;
	}
	plt = (NEUIK_Plot2D*)pltElem;

	if (neuik_Object_GetClassObject(plt, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 2;
		goto out;
	}

	if (neuik_Element_GetMinSize(plot->visual, rSize) != 0)
	{
		eNum = 3;
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
 *  Name:          neuik_Element_Render__Plot2D
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Plot2D(
	NEUIK_Element   pltElem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
{
	int                   eNum       = 0; /* which error to report (if any) */
	RenderLoc             rl         = {0, 0};
	RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Rect              rect       = {0, 0, 0, 0};
	RenderSize            rs         = {0, 0};
	RenderSize            dwg_rs;
	SDL_Surface         * surf       = NULL;
	SDL_Renderer        * rend       = NULL;
	NEUIK_Plot          * plot       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementBase   * dwg_eBase  = NULL;
	NEUIK_Element         elem       = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Plot2D        * plt        = NULL;
	NEUIK_Canvas        * dwg;               /* pointer to active drawing (don't free) */
	static char           funcName[] = "neuik_Element_Render__Plot2D";
	static char         * errMsgs[]  = {"",                                 // [0] no error
		"Argument `pltElem` is not of Plot2D class.",                       // [1]
		"", // [2]
		"Element_GetConfig returned NULL.",                                 // [3]
		"Element_GetMinSize Failed.",                                       // [4]
		"Failure in `neuik_Element_Render()`",                              // [5]
		"Invalid specified `rSize` (negative values).",                     // [6]
		"", // [7]
		"Argument `pltElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                     // [9]
	};

	if (!neuik_Object_IsClass(pltElem, neuik__Class_Plot2D))
	{
		eNum = 1;
		goto out;
	}
	plt = (NEUIK_Plot2D*)pltElem;

	if (neuik_Object_GetClassObject(pltElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(pltElem, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 8;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 6;
		goto out;
	}

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(pltElem, xSurf, rlMod, NULL))
	{
		eNum = 9;
		goto out;
	}
	rl = eBase->eSt.rLoc;

	elem = plot->visual;

	/*------------------------------------------------------------------------*/
	/* Render and place the currently active stack element                    */
	/*------------------------------------------------------------------------*/
	eCfg = neuik_Element_GetConfig(elem);
	if (eCfg == NULL)
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start with the default calculated element size                         */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_GetMinSize(elem, &rs))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check for and apply if necessary Horizontal and Veritcal fill          */
	/*------------------------------------------------------------------------*/
	if (eCfg->HFill)
	{
		/* This element is configured to fill space horizontally */
		rs.w = rSize->w - (eCfg->PadLeft + eCfg->PadRight);
	}
	if (eCfg->VFill)
	{
		/* This element is configured to fill space vertically */
		rs.h = rSize->h - (eCfg->PadTop + eCfg->PadBottom);
	}

	/*------------------------------------------------------------------------*/
	/* Update the stored location before rendering the element. This is       */
	/* necessary as the location of this object will propagate to its         */
	/* child objects.                                                         */
	/*------------------------------------------------------------------------*/
	switch (eCfg->HJustify)
	{
		case NEUIK_HJUSTIFY_LEFT:
			rect.x = eCfg->PadLeft;
			break;
		case NEUIK_HJUSTIFY_DEFAULT:
		case NEUIK_HJUSTIFY_CENTER:
			rect.x = rSize->w/2 - (rs.w/2);
			break;
		case NEUIK_HJUSTIFY_RIGHT:
			rect.x = rSize->w - (rs.w + eCfg->PadRight);
			break;
	}

	switch (eCfg->VJustify)
	{
		case NEUIK_VJUSTIFY_TOP:
			rect.y = eCfg->PadTop;
			break;
		case NEUIK_VJUSTIFY_DEFAULT:
		case NEUIK_VJUSTIFY_CENTER:
			rect.y = (rSize->h - (eCfg->PadTop + eCfg->PadBottom))/2 - (rs.h/2);
			break;
		case NEUIK_VJUSTIFY_BOTTOM:
			rect.y = rSize->h - (rs.h + eCfg->PadBottom);
			break;
	}

	rect.w = rs.w;
	rect.h = rs.h;
	rl.x = (eBase->eSt.rLoc).x + rect.x;
	rl.y = (eBase->eSt.rLoc).y + rect.y;
	rlRel.x = rect.x;
	rlRel.y = rect.y;
	neuik_Element_StoreSizeAndLocation(elem, rs, rl, rlRel);

	if (neuik_Element_Render(elem, &rs, rlMod, rend, surf))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* At this point, the size of the plot drawing area should be known. Now  */
	/* we can do the actual drawing to the draw area and then re-render the   */
	/* superclass plot element.                                               */
	/*------------------------------------------------------------------------*/
	dwg = plt->drawing_background;
	if (neuik_Object_GetClassObject(dwg, neuik__Class_Element, (void**)&dwg_eBase))
	{
		eNum = 8;
		goto out;
	}
	dwg_rs = dwg_eBase->eSt.rSize;
	if (neuik__Report_Debug)
	{
		printf("Drawing rs [w:%d, h:%d]\n", dwg_rs.w, dwg_rs.h);
	}

	/*------------------------------------------------------------------------*/
	/* Draw in the labels for the y-axis/x-axis tic marks                     */
	/*------------------------------------------------------------------------*/
	dwg = plt->drawing_ticmarks;
	NEUIK_Canvas_SetDrawColor(dwg, 0, 0, 0, 255); /* dwg ticmark label color */
	NEUIK_Canvas_SetTextSize(dwg, 10);        /* set size of drawn text */

	NEUIK_Canvas_MoveTo(dwg, 2, 2);
	NEUIK_Canvas_DrawText(dwg, "100.0"); /* draw text at location */
	NEUIK_Canvas_MoveTo(dwg, 2, 150);
	NEUIK_Canvas_DrawText(dwg, "50.0");  /* draw text at location */
	NEUIK_Canvas_MoveTo(dwg, 2, 300);
	NEUIK_Canvas_DrawText(dwg, "0.0");   /* draw text at location */

	/* draw y-axis vert line */ 
	NEUIK_Canvas_MoveTo(dwg, 40, 5);
	NEUIK_Canvas_DrawLine(dwg, 40, (dwg_rs.h-(1+5)));

	#pragma message("Draw in the labels for the y-axis tic marks")
	#pragma message("Draw in the labels for the x-axis tic marks")

	/*------------------------------------------------------------------------*/
	/* Fill the background with white and draw the outside border             */
	/*------------------------------------------------------------------------*/
	dwg = plt->drawing_background;
	NEUIK_Canvas_SetDrawColor(dwg, 255, 255, 255, 255); /* dwg bg color */
	NEUIK_Canvas_Fill(dwg);
	NEUIK_Canvas_SetDrawColor(dwg, 150, 150, 150, 255); /* dwg border color */
	NEUIK_Canvas_MoveTo(dwg, 0, 0);
	NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), 0);
	NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), (dwg_rs.h-1));
	NEUIK_Canvas_DrawLine(dwg, 0, (dwg_rs.h-1));
	NEUIK_Canvas_DrawLine(dwg, 0, 0);
out:
	eBase->eSt.doRedraw = 0;

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

