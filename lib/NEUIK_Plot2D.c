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
#include "NEUIK_Plot.h"
#include "NEUIK_Plot2D.h"
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
int neuik_Object_New__Plot2D(void ** vgPtr);
int neuik_Object_Free__Plot2D(void * vgPtr);

int neuik_Element_GetMinSize__Plot2D(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Plot2D(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

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
	int                    eNum       = 0;
	int                    ctr        = 0;
	NEUIK_Plot           * plot       = NULL;
	NEUIK_Plot2D         * plot2d     = NULL;
	neuik_PlotDataConfig * dataCfg    = NULL;
	NEUIK_Element        * sClassPtr  = NULL;
	static char            funcName[] = "neuik_Object_New__Plot2D";
	static char          * errMsgs[]  = {"", // [0] no error
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

	/*------------------------------------------------------------------------*/
	/* Allocate memory for tracking DataSets.                                 */
	/*------------------------------------------------------------------------*/
	plot->data_sets = malloc(5*sizeof(NEUIK_Object));
	if (plot->data_sets == NULL)
	{
		eNum = 2;
		goto out;
	}
	plot->data_configs = malloc(5*sizeof(neuik_PlotDataConfig));
	if (plot->data_configs == NULL)
	{
		eNum = 2;
		goto out;
	}
	plot->n_allocated = 5;
	plot->n_used = 0;

	for (ctr = plot->n_used; ctr < plot->n_allocated; ctr++)
	{
		dataCfg = &(plot->data_configs[ctr]);
		dataCfg->uniqueName = NULL;
		dataCfg->label      = NULL;
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
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	int                   eNum       = 0; /* which error to report (if any) */
	RenderLoc             rl;
	RenderLoc             rlRel      = {0, 0}; /* renderloc relative to parent */
	SDL_Rect              rect       = {0, 0, 0, 0};
	RenderSize            rs         = {0, 0};
	RenderSize            dwg_rs;
	SDL_Renderer        * rend       = NULL;
	NEUIK_Plot          * plot       = NULL;
	NEUIK_ElementBase   * eBase      = NULL;
	NEUIK_ElementBase   * dwg_eBase  = NULL;
	NEUIK_ElementConfig * eCfg       = NULL;
	NEUIK_Plot2D        * plt        = NULL;
	NEUIK_Canvas        * dwg;               /* pointer to active drawing (don't free) */
	neuik_MaskMap       * maskMap    = NULL; /* FREE upon return */
	enum neuik_bgstyle    bgStyle;
	static char           funcName[] = "neuik_Element_Render__Plot2D";
	static char         * errMsgs[]  = {"", // [0] no error
		"Argument `pltElem` is not of Plot2D class.",                       // [1]
		"Failure in `neuik_Element_GetCurrentBGStyle()`.",                  // [2]
		"Element_GetConfig returned NULL.",                                 // [3]
		"Element_GetMinSize Failed.",                                       // [4]
		"Failure in `neuik_Element_Render()`",                              // [5]
		"Invalid specified `rSize` (negative values).",                     // [6]
		"Failure in `neuik_MakeMaskMap()`",                                 // [7]
		"Argument `pltElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                     // [9]
		"Failure in `neuik_Window_FillTranspMaskFromLoc()`",                // [10]
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
	if (!mock)
	{
		if (neuik_Element_GetCurrentBGStyle(pltElem, &bgStyle))
		{
			eNum = 2;
			goto out;
		}
		if (bgStyle != NEUIK_BGSTYLE_TRANSPARENT)
		{
			/*----------------------------------------------------------------*/
			/* Create a MaskMap an mark off the trasnparent pixels.           */
			/*----------------------------------------------------------------*/
			if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
			{
				eNum = 7;
				goto out;
			}

			rl = eBase->eSt.rLoc;
			if (neuik_Window_FillTranspMaskFromLoc(
					eBase->eSt.window, maskMap, rl.x, rl.y))
			{
				eNum = 10;
				goto out;
			}

			if (neuik_Element_RedrawBackground(pltElem, rlMod, maskMap))
			{
				eNum = 9;
				goto out;
			}
		}
	}
	rl = eBase->eSt.rLoc;

	/*------------------------------------------------------------------------*/
	/* Render and place the currently active stack element                    */
	/*------------------------------------------------------------------------*/
	eCfg = neuik_Element_GetConfig(plot->visual);
	if (eCfg == NULL)
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Start with the default calculated element size                         */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_GetMinSize(plot->visual, &rs))
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
	neuik_Element_StoreSizeAndLocation(plot->visual, rs, rl, rlRel);
	/*------------------------------------------------------------------------*/
	/* The following render operation will result in a calculated size for    */
	/* plot drawing area.                                                     */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_Render(plot->visual, &rs, rlMod, rend, TRUE))
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
	NEUIK_Canvas_Clear(dwg);
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
	NEUIK_Canvas_Clear(dwg);
	NEUIK_Canvas_SetDrawColor(dwg, 255, 255, 255, 255); /* dwg bg color */
	NEUIK_Canvas_Fill(dwg);
	NEUIK_Canvas_SetDrawColor(dwg, 150, 150, 150, 255); /* dwg border color */
	NEUIK_Canvas_MoveTo(dwg, 0, 0);
	NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), 0);
	NEUIK_Canvas_DrawLine(dwg, (dwg_rs.w-1), (dwg_rs.h-1));
	NEUIK_Canvas_DrawLine(dwg, 0, (dwg_rs.h-1));
	NEUIK_Canvas_DrawLine(dwg, 0, 0);

	/*------------------------------------------------------------------------*/
	/* Finally, have the entire visual redraw itself. It will only redraw the */
	/* drawing portion and with the correct sizing.                           */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_NeedsRedraw(plot->visual))
	{
		if (neuik_Element_Render(plot->visual, &rs, rlMod, rend, mock))
		{
			eNum = 5;
			goto out;
		}
	}
out:
	if (eBase != NULL)
	{
		if (!mock) eBase->eSt.doRedraw = 0;
	}
	if (maskMap != NULL) neuik_Object_Free(maskMap);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Plot2D_UpdateAxesRanges
 *
 *  Description:   Update the stored X/Y-Axis ranges.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Plot2D_UpdateAxesRanges(
	NEUIK_Plot2D * plot2d)
{
	int             eNum       = 0; /* which error to report (if any) */
	int             boundsSet  = FALSE;
	unsigned int    uCtr       = 0;
	double          xMin       = 0.0;
	double          xMax       = 0.0;
	double          yMin       = 0.0;
	double          yMax       = 0.0;
	double          xMin64     = 0.0;
	double          xMax64     = 0.0;
	double          yMin64     = 0.0;
	double          yMax64     = 0.0;
	double          xRangeMin  = 0.0;
	double          xRangeMax  = 0.0;
	double          yRangeMin  = 0.0;
	double          yRangeMax  = 0.0;
	double          xAxisRange = 0.0;
	double          yAxisRange = 0.0;
	NEUIK_Plot     * plot      = NULL;
	NEUIK_PlotData * data      = NULL;
	static char      funcName[] = "neuik_Plot2D_UpdateAxesRanges";
	static char    * errMsgs[]  = {"", // [0] no error
		"Argument `plot2d` is not of Plot2D class.",                       // [1]
		"Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Unsupported `precision` used within included PlotData.",          // [3]
	};

	/*------------------------------------------------------------------------*/
	/* Check for errors before continuing.                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(plot2d, neuik__Class_Plot2D))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 2;
		goto out;
	}

	if (plot->x_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED &&
		plot->y_range_cfg == NEUIK_PLOTRANGECONFIG_SPECIFIED)
	{
		/*--------------------------------------------------------------------*/
		/* The plot range for both axes has been manuallys specified. There   */
		/* is no need to autocalculate a range here; return now.              */
		/*--------------------------------------------------------------------*/
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the maximum X-Y range of values from all data sets.          */
	/*------------------------------------------------------------------------*/
	for (uCtr = 0; uCtr < plot->n_used; uCtr++)
	{
		data = (NEUIK_PlotData*)(plot->data_sets[uCtr]);
		if (!data->boundsSet) continue;

		/*--------------------------------------------------------------------*/
		/* Extract the bounds for a particular PlotData set as doubles.       */
		/*--------------------------------------------------------------------*/
		switch (data->precision)
		{
			case 32:
				xMin64 = (double)(data->bounds_32.x_min);
				xMax64 = (double)(data->bounds_32.x_max);
				yMin64 = (double)(data->bounds_32.y_min);
				yMax64 = (double)(data->bounds_32.y_max);
				break;
			case 64:
				xMin64 = data->bounds_64.x_min;
				xMax64 = data->bounds_64.x_max;
				yMin64 = data->bounds_64.y_min;
				yMax64 = data->bounds_64.y_max;
				break;
			default:
				/*------------------------------------------------------------*/
				/* Unsupported floating point precision.                      */
				/*------------------------------------------------------------*/
				eNum = 3;
				goto out;
				break;
		}

		/*--------------------------------------------------------------------*/
		/* Update the overall X-Y ranges of values from all data sets.        */
		/*--------------------------------------------------------------------*/
		if (!boundsSet)
		{
			xMin = xMin64;
			xMax = xMax64;
			yMin = yMin64;
			yMax = yMax64;
		}
		else
		{
			if (xMin64 < xMin)
			{
				xMin = xMin64;
			}
			if (xMax64 > xMax)
			{
				xMax = xMax64;
			}
			if (yMin64 < yMin)
			{
				yMin = yMin64;
			}
			if (yMax64 > yMax)
			{
				yMax = yMax64;
			}
		}

		boundsSet = TRUE;
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the X bounds to use for the overall plot.                    */
	/*------------------------------------------------------------------------*/
	xAxisRange = xMax - xMin;

	if (xAxisRange == 0.0)
	{
		xRangeMin = xMin - 1;
		xRangeMax = xMin + 1;
	}
	else if (xAxisRange < 0.5)
	{
		xRangeMin = floor(xMin);
		xRangeMax = xRangeMin + 1;
	}
	else if (xAxisRange < 1.0)
	{
		xRangeMin = floor(xMin);
		xRangeMax = xRangeMin + 2;
	}
	else if (xAxisRange < 10.0)
	{
		xRangeMin = floor(xMin - fmod(xMin, 10));
		if (xMax < xRangeMin + 10)
		{
			xRangeMax = xRangeMin + 10;
		}
		xRangeMax = xRangeMin + 12;
	}
	else if (xAxisRange < 100.0)
	{
		xRangeMin = floor(xMin - fmod(xMin, 100));
		if (xMax < xRangeMin + 100)
		{
			xRangeMax = xRangeMin + 100;
		}
		xRangeMax = xRangeMin + 120;
	}
	else
	{
		printf("neuik_Plot2D_UpdateAxesRanges(): xAxisRange unhandled!!!\n");
		#pragma message ("[TODO] Improve calculation of xAxisRange")
	}
	plot->x_range_min = xRangeMin;
	plot->x_range_max = xRangeMax;


	/*------------------------------------------------------------------------*/
	/* Calculate the Y bounds to use for the overall plot.                    */
	/*------------------------------------------------------------------------*/
	yAxisRange = yMax - yMin;

	if (yAxisRange == 0.0)
	{
		yRangeMin = yMin - 1;
		yRangeMax = yMin + 1;
	}
	else if (yAxisRange < 0.5)
	{
		yRangeMin = floor(yMin);
		yRangeMax = yRangeMin + 1;
	}
	else if (yAxisRange < 1.0)
	{
		yRangeMin = floor(yMin);
		yRangeMax = yRangeMin + 2;
	}
	else if (yAxisRange < 10.0)
	{
		yRangeMin = floor(yMin - fmod(yMin, 10));
		if (yMax < yRangeMin + 10)
		{
			yRangeMax = yRangeMin + 10;
		}
		yRangeMax = yRangeMin + 12;
	}
	else if (yAxisRange < 100.0)
	{
		yRangeMin = floor(yMin - fmod(yMin, 100));
		if (yMax < yRangeMin + 100)
		{
			yRangeMax = yRangeMin + 100;
		}
		yRangeMax = yRangeMin + 120;
	}
	else
	{
		printf("neuik_Plot2D_UpdateAxesRanges(): yAxisRange unhandled!!!\n");
		#pragma message ("[TODO] Improve calculation of yAxisRange")
	}
	plot->x_range_min = yRangeMin;
	plot->x_range_max = yRangeMax;
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
 *  Name:          NEUIK_Plot2D_AddPlotData
 *
 *  Description:   Add the specified PlotData to this plot.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Plot2D_AddPlotData(
	NEUIK_Plot2D   * plot2d,
	NEUIK_PlotData * data,
	const char     * label)
{
	int                   eNum = 0; /* which error to report (if any) */
	int                   sLen = 0;
	unsigned int          uCtr = 0;
	neuik_PlotDataConfig * dataCfg    = NULL;
	NEUIK_Plot           * plot       = NULL;
	RenderSize             rSize;
	RenderLoc              rLoc;
	static char            funcName[] = "NEUIK_Plot2D_AddPlotData";
	static char          * errMsgs[]  = {"", // [0] no error
		"Argument `plot2d` is not of Plot2D class.",                       // [1]
		"Argument `plot2d` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Argument `data` is not of PlotData class.",                       // [3]
		"Failure to reallocate memory.",                                   // [4]
		"PlotData `uniqueName` already in use within this Plot.",          // [5]
		"Failure to allocate memory.",                                     // [6]
		"Failure in `neuik_Plot2D_UpdateAxesRanges()`.",                   // [7]
		"Failure in `neuik_Element_GetSizeAndLocation()`.",                // [8]
	};

	/*------------------------------------------------------------------------*/
	/* Check for errors before continuing.                                    */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(plot2d, neuik__Class_Plot2D))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(plot2d, neuik__Class_Plot, (void**)&plot))
	{
		eNum = 2;
		goto out;
	}

	if (!neuik_Object_IsClass(data, neuik__Class_PlotData))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check to see if the DataSet slots need to be reallocated.              */
	/*------------------------------------------------------------------------*/
	if (plot->n_used >= plot->n_allocated)
	{
		/*--------------------------------------------------------------------*/
		/* More space will be needed for tracking DataSets; reallocate.       */
		/*--------------------------------------------------------------------*/
		plot->data_sets = realloc(plot->data_sets, 
			(plot->n_allocated+5)*sizeof(NEUIK_Object));
		if (plot->data_sets == NULL)
		{
			eNum = 4;
			goto out;
		}
		plot->data_configs = realloc(plot->data_configs,
			(plot->n_allocated+5)*sizeof(neuik_PlotDataConfig));
		if (plot->data_configs == NULL)
		{
			eNum = 4;
			goto out;
		}
		plot->n_allocated += 5;

		for (uCtr = plot->n_used; uCtr < plot->n_allocated; uCtr++)
		{
			dataCfg = &(plot->data_configs[uCtr]);
			dataCfg->uniqueName = NULL;
			dataCfg->label      = NULL;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Make sure the uniqueName for this PlotData isn't already in use within */
	/* this Plot.                                                             */
	/*------------------------------------------------------------------------*/
	for (uCtr = 0; uCtr < plot->n_used; uCtr++)
	{
		if (!strcmp(plot->data_configs[uCtr].uniqueName, data->uniqueName))
		{
			eNum = 5;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Add the PlotData to the first available slot.                          */
	/*------------------------------------------------------------------------*/
	plot->data_sets[plot->n_used] = data;

	dataCfg = &(plot->data_configs[plot->n_used]);
	if (dataCfg->uniqueName != NULL)
	{
		free(dataCfg->uniqueName);
		dataCfg->uniqueName = NULL;
	}
	if (dataCfg->label != NULL)
	{
		free(dataCfg->label);
		dataCfg->label = NULL;
	}

	sLen = strlen(data->uniqueName);
	dataCfg->uniqueName = malloc((1+sLen)*sizeof(char));
	if (dataCfg->uniqueName == NULL)
	{
		eNum = 6;
		goto out;
	}

	sLen = strlen(label);
	dataCfg->label = malloc((1+sLen)*sizeof(char));
	if (dataCfg->label == NULL)
	{
		eNum = 6;
		goto out;
	}

	if (neuik_Plot2D_UpdateAxesRanges(plot2d))
	{
		eNum = 7;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Request a redraw of the old size at old location. This will make sure  */
	/* the content is erased (in case the new content is smaller).            */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_GetSizeAndLocation(plot2d, &rSize, &rLoc))
	{
		eNum = 8;
		goto out;
	}
	neuik_Element_RequestRedraw(plot2d, rLoc, rSize);

	/*PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP*/
	#pragma message("There should be double-linkage between Plot2D and PlotData.")
	/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	/* This is so changes in plot data can trigger redraws of the Plot2D and  */
	/* also, so removal of the PlotData from curve, can remove the linkage    */
	/* from the PlotData side.                                                */
	/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

