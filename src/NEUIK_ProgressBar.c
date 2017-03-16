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
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_ProgressBar.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ProgressBar(void ** wPtr);
int neuik_Object_Free__ProgressBar(void ** wPtr);
int neuik_Element_GetMinSize__ProgressBar(NEUIK_Element, RenderSize*);
int neuik_Element_CaptureEvent__ProgressBar(NEUIK_Element, SDL_Event*);
SDL_Texture * neuik_Element_Render__ProgressBar(NEUIK_Element, RenderSize*, SDL_Renderer*);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ProgressBar_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__ProgressBar,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__ProgressBar,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_ProgressBar_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__ProgressBar,

	/* Render(): Redraw the element */
	neuik_Element_Render__ProgressBar,

	/* CaptureEvent(): Determine if this element caputures a given event */
	neuik_Element_CaptureEvent__ProgressBar,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ProgressBar
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ProgressBar()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_ProgressBar";
	static char  * errMsgs[]  = {"",                       // [0] no error
		"NEUIK library must be initialized first.",        // [1]
		"Failed to register `ProgressBar` object class .", // [2]
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
		"ProgressBar",                             // className
		"A GUI which displays activity progress.", // classDescription
		neuik__Set_NEUIK,                          // classSet
		neuik__Class_Element,                      // superClass
		&neuik_ProgressBar_BaseFuncs,              // baseFuncs
		NULL,                                      // classFuncs
		&neuik__Class_ProgressBar))                // newClass
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
 *  Name:          neuik_Object_New__ProgressBar
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ProgressBar(
	void ** pbPtr)
{
	int                 eNum       = 0; /* which error to report (if any) */
	NEUIK_ProgressBar * pb         = NULL;
	NEUIK_Element     * sClassPtr  = NULL;
	NEUIK_Color  		bgClr      = COLOR_LGRAY;

	static char         funcName[] = "neuik_Object_New__ProgressBar";
	static char       * errMsgs[]  = {"",                      // [0] no error
		"Failure to allocate memory.",                         // [1]
		"Failure in NEUIK_NewProgressBarConfig.",              // [2]
		"Output Argument `pbPtr` is NULL.",                    // [3]
		"Failure in function `neuik_Object_New`.",             // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",   // [5]
		"Failure in `neuik_GetObjectBaseOfClass`.",            // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorSolid`.", // [7]
	};

	if (pbPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*pbPtr) = (NEUIK_ProgressBar*) malloc(sizeof(NEUIK_ProgressBar));
	pb = *pbPtr;
	if (pb == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_ProgressBar, 
			NULL,
			&(pb->objBase)))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(pb->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_ProgressBar_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	/* Allocation successful */
	pb->selected     = 0;
	pb->wasSelected  = 0;
	pb->isActive     = 0;
	pb->clickOrigin  = 0;
	pb->needsRedraw  = 1;
	pb->cfg          = NULL;
	pb->cfgPtr       = NULL;


	if (NEUIK_NewProgressBarConfig(&pb->cfg))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the new ProgressBar text contents                                  */
	/*------------------------------------------------------------------------*/
	pb->frac = 0.0;
	if (pb->cfg->decimalPlaces == 0)
	{
		strcpy(pb->fracText, "0%");
	}
	else
	{
		strcpy(pb->fracText, "0.0%");
	}

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorSolid(pb, "normal",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorSolid(pb, "selected",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorSolid(pb, "hovered",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
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
 *  Name:          neuik_Object_Free__ProgressBar
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__ProgressBar(
	void  ** objPtr)  /* [out] the object to free */
{
	int                 eNum       = 0; /* which error to report (if any) */
	NEUIK_ProgressBar * pb         = NULL;
	static char         funcName[] = "neuik_Object_Free__ProgressBar";
	static char       * errMsgs[]  = {"",                 // [0] no error
		"Argument `btnPtr` is not of Button class.", // [1]
		"Failure in function `neuik_Object_Free`.",  // [2]
		"Argument `btnPtr` is NULL.",                // [3]
	};

	if (objPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	if (!neuik_Object_IsClass(*objPtr, neuik__Class_ProgressBar))
	{
		eNum = 1;
		goto out;
	}
	pb = (NEUIK_ProgressBar*)(*objPtr);

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(&(pb->objBase.superClassObj)))
	{
		eNum = 2;
		goto out;
	}
	if(neuik_Object_Free((void**)&(pb->cfg)))
	{
		eNum = 2;
		goto out;
	}

	free(pb);
	(*objPtr) = NULL;
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
 *  Name:          neuik_Element_GetMinSize__ProgressBar
 *
 *  Description:   Returns the rendered size of a given ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__ProgressBar(
	NEUIK_Element    elem,
	RenderSize     * rSize)
{
	int                       tW;
	int                       tH;
	int                       eNum = 0;    /* which error to report (if any) */
	TTF_Font                * font = NULL;
	NEUIK_ProgressBar       * pb   = NULL;
	NEUIK_ProgressBarConfig * aCfg = NULL; /* the active ProgressBar config */
	static char               funcName[] = "neuik_Element_GetMinSize__ProgressBar";
	static char * errMsgs[] = {"",                      // [0] no error
		"Argument `elem` is not of ProgressBar class.", // [1]
		"ProgressBarConfig* is NULL.",                  // [2]
		"ProgressBarConfig->FontSet is NULL.",          // [3]
		"FontSet_GetFont returned NULL.",               // [4]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(elem, neuik__Class_ProgressBar))
	{
		eNum = 1;
		goto out;
	}
	pb = (NEUIK_ProgressBar*)elem;
	
	/* select the correct ProgressBar config to use (pointer or internal) */
	if (pb->cfgPtr != NULL)
	{
		aCfg = pb->cfgPtr;
	}
	else 
	{
		aCfg = pb->cfg;
	}

	if (aCfg == NULL)
	{
		eNum = 2;
		goto out;
	} 

	if (aCfg->fontSet == NULL)
	{
		eNum = 3;
		goto out;
	}

	font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize, 
		aCfg->fontBold, aCfg->fontItalic);
	if (font == NULL) 
	{
		eNum = 4;
		goto out;
	}

	if (strlen(pb->fracText) > 0)
	{
		/* this ProgressBar contains text */
		TTF_SizeText(font, pb->fracText, &tW, &tH);

	}
	else
	{
		/* this ProgressBar does not contain text */
		TTF_SizeText(font, " ", &tW, &tH);
	}

	rSize->w = tW + aCfg->fontEmWidth;
	rSize->h = (int)(1.5 * (float)TTF_FontHeight(font));
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
 *  Name:          NEUIK_NewProgressBar
 *
 *  Description:   Create a new NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewProgressBar(
	NEUIK_ProgressBar ** pbPtr)  /* [out] The newly created object. */
{
	return neuik_Object_New__ProgressBar((void **)pbPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ProgressBar_GetFraction
 *
 *  Description:   Return the current fraction of an NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ProgressBar_GetFraction(
	NEUIK_ProgressBar  * pb,
	double             * frac)
{
	int           eNum = 0;    /* which error to report (if any) */
	static char   funcName[] = "NEUIK_ProgressBar_GetFraction";
	static char * errMsgs[] = {"",                    // [0] no error
		"Argument `pb` is not of ProgressBar class.", // [1]
	};

	if (!neuik_Object_IsClass(pb, neuik__Class_ProgressBar))
	{
		eNum = 1;
		goto out;
	}

	(*frac) = pb->frac;
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
 *  Name:          NEUIK_ProgressBar_SetFraction
 *
 *  Description:   Update the fraction of a NEUIK_ProgressBar.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ProgressBar_SetFraction(
		NEUIK_ProgressBar * pb,
		double              frac)
{
	int                       eNum = 0;    /* which error to report (if any) */
	NEUIK_ProgressBarConfig * aCfg = NULL; /* the active ProgressBar config */
	static char               funcName[] = "NEUIK_ProgressBar_SetFraction";
	static char             * errMsgs[] = {"",        // [0] no error
		"Argument `pb` is not of ProgressBar class.", // [1]
	};

	if (!neuik_Object_IsClass(pb, neuik__Class_ProgressBar))
	{
		eNum = 1;
		goto out;
	}

	if (frac != pb->frac)
	{
		/*--------------------------------------------------------------------*/
		/* ProgressBar fraction value has changed, updated text and request a */
		/* redraw.                                                            */
		/*--------------------------------------------------------------------*/
		pb->frac = frac;

		/*--------------------------------------------------------------------*/
		/* select the correct ProgressBar config to use (pointer or internal) */
		/*--------------------------------------------------------------------*/
		if (pb->cfgPtr != NULL)
		{
			aCfg = pb->cfgPtr;
		}
		else 
		{
			aCfg = pb->cfg;
		}

		if (aCfg->decimalPlaces == 0)
		{
			sprintf(pb->fracText, "%d%%", (unsigned int)(100.0*pb->frac));
		}
		else
		{
			sprintf(pb->fracText, "%.2f%%", 100.0*pb->frac);
		}

		neuik_Element_RequestRedraw(pb);
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
 *  Name:          neuik_Element_Render__ProgressBar
 *
 *  Description:   Renders a single ProgressBar as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * neuik_Element_Render__ProgressBar(
	NEUIK_Element    elem,
	RenderSize     * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer   * xRend) /* the external renderer to prepare the texture for */
{
	const NEUIK_Color        * fgClr  = NULL;
	const NEUIK_Color        * bgClr  = NULL;
	const NEUIK_Color        * bClr   = NULL; /* border color */
	static SDL_Color           tClr   = COLOR_TRANSP;
	SDL_Surface              * surf   = NULL;
	SDL_Renderer             * rend   = NULL;
	int                        progW  = 0; /* pixel width of entire shadable region */
	int                        shadeW = 0; /* width of shaded progress bar region */
	int                        textW  = 0;
	int                        textH  = 0;
	int                        eNum   = 0; /* which error to report (if any) */
	SDL_Texture              * gTex   = NULL; /* gradient progress texture */
	SDL_Texture              * tTex   = NULL; /* text texture */
	TTF_Font                 * font   = NULL;
	SDL_Rect                   rect;
	RenderSize                 shadeSize;
	NEUIK_ProgressBar        * pb     = NULL;
	NEUIK_ElementBase        * eBase  = NULL;
	int                        ctr;
	int                        gCtr;             /* gradient counter */
	int                        nClrs;
	int                        clrR;
	int                        clrG;
	int                        clrB;
	int                        clrFound;
	float                      lastFrac  = -1.0;
	float                      frac;
	float                      fracDelta;        /* fraction between ColorStop 1 & 2 */
	float                      fracStart = 0.0;  /* fraction at ColorStop 1 */
	float                      fracEnd   = 1.0;  /* fraction at ColorStop 2 */
	colorDeltas              * deltaPP   = NULL;
	colorDeltas              * clrDelta;
	NEUIK_Color              * clr;
	NEUIK_ColorStop         ** cs;
	NEUIK_ProgressBarConfig  * aCfg = NULL; /* the active ProgressBar config */
	static char                funcName[] = "neuik_Element_Render__ProgressBar";
	static char              * errMsgs[] = {"",                           // [0] no error
		"Argument `elem` is not of ProgressBar class.",                  // [1]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Invalid specified `rSize` (negative values).",                  // [3]
		"Failure in Element_Resize().",                                  // [4]
		"Invalid ColorStop fraction (<0 or >1).",                        // [5]
		"ColorStops array fractions not in ascending order.",            // [6]
		"ColorStops array is empty.",                                    // [7]
		"Failure to allocate memory.",                                   // [8]
		"FontSet_GetFont returned NULL.",                                // [9]
		"RenderText returned NULL.",                                     // [10]
		"SDL_CreateTextureFromSurface returned NULL.",                   // [11]
		"Failure in `neuik_Element_RedrawBackground()`.",                // [12]
	};

	if (!neuik_Object_IsClass(elem, neuik__Class_ProgressBar))
	{
		eNum = 1;
		goto out;
	}
	pb = (NEUIK_ProgressBar*)elem;

	if (neuik_Object_GetClassObject(pb, neuik__Class_Element, (void**)&eBase))
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
		if (!neuik_Element_NeedsRedraw(pb) && eBase->eSt.texture != NULL) 
		{
			(*rSize) = eBase->eSt.rSize;
			return eBase->eSt.texture;
		}
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 3;
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
		if (neuik_Element_Resize(pb, *rSize) != 0)
		{
			eNum = 4;
			goto out;
		}
	}
	surf = eBase->eSt.surf;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* select the correct ProgressBar config to use (pointer or internal)     */
	/*------------------------------------------------------------------------*/
	if (pb->cfgPtr != NULL)
	{
		aCfg = pb->cfgPtr;
	}
	else 
	{
		aCfg = pb->cfg;
	}

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(elem))
	{
		eNum = 12;
		goto out;
	}
	bgClr = &(aCfg->bgColor);
	fgClr = &(aCfg->fgColor);

	/*------------------------------------------------------------------------*/
	/* Draw the color representation of the progress bar progress             */
	/*------------------------------------------------------------------------*/
	if (pb->frac > 0.0)
	{
		progW  = (rSize->w - 2) - 1;
		shadeW = 2 + (int)(pb->frac * (double)(progW));
		rect.x = 1;
		rect.y = 0;
		rect.w = shadeW;
		rect.h = (rSize->h - 1);

		shadeSize.w = shadeW;
		shadeSize.h = rect.h;

		/*--------------------------------------------------------------------*/
		/* TODO: when the opportunity presents itself, the rest of the code   */
		/* in this block should be replaced by a fixed call to RenderGradient */
		/* however for now, I will leave duplicate code here since it works.  */
		/*--------------------------------------------------------------------*/

		// gTex = NEUIK_RenderGradient(aCfg->gradCS, 'v', rend, shadeSize);
		// if (gTex == NULL)
		// {
		// 	eNum = 3;
		// 	goto out;
		// }

		// SDL_QueryTexture(gTex, &testUint32, &access, &testW, &testH);

		// srcRect.x = 0;
		// srcRect.y = 0;
		// srcRect.w = rect.w;
		// srcRect.h = rect.h;
		// SDL_RenderCopy(rend, gTex, NULL, &rect);
		// // SDL_RenderCopy(rend, gTex, &srcRect, &rect);
		// //SDL_RenderCopy(rend, gTex, NULL, NULL);

		cs = aCfg->gradCS;

		/*------------------------------------------------------------------------*/
		/* Count the number of color stops and check that the color stop          */
		/* fractions are in increasing order                                      */
		/*------------------------------------------------------------------------*/
		for (nClrs = 0;; nClrs++)
		{
			if (cs[nClrs] == NULL) break; /* this is the number of ColorStops */
			if (cs[nClrs]->frac < 0.0 || cs[nClrs]->frac > 1.0)
			{
				eNum = 5;
				goto out;
			}
			else if (cs[nClrs]->frac < lastFrac)
			{
				eNum = 6;
				goto out;
			}
			else
			{
				lastFrac = cs[nClrs]->frac;
			}
		}
		if (nClrs == 0)
		{
			eNum = 7;
			goto out;
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
				eNum = 8;
				goto out;
			}
			for (ctr = 0; ctr < nClrs-1; ctr++)
			{
				deltaPP[ctr].r = (cs[ctr+1]->color).r - (cs[ctr]->color).r;
				deltaPP[ctr].g = (cs[ctr+1]->color).g - (cs[ctr]->color).g;
				deltaPP[ctr].b = (cs[ctr+1]->color).b - (cs[ctr]->color).b;
			}
		}

		/*--------------------------------------------------------------------*/
		/* Draw a vertical gradient                                           */
		/*--------------------------------------------------------------------*/
		for (gCtr = 0; gCtr < shadeSize.h; gCtr++)
		{
			/* calculate the fractional position within the gradient */
			frac = (float)(gCtr+1)/(float)(shadeSize.h);


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
				SDL_SetRenderDrawColor(rend, clrR, clrG, clrB, 255);
			}
			else
			{
				/* not between two ColorStops, use a single color */
				SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, 255);
			}

			SDL_RenderDrawLine(rend, 0, gCtr, shadeSize.w - 1, gCtr);
		}
	}

	/*------------------------------------------------------------------------*/
	/* Trim off the rounded sections of the ProgressBar using a transparent   */
	/* color                                                                  */
	/*------------------------------------------------------------------------*/
	SDL_SetColorKey(surf, SDL_TRUE, 
		SDL_MapRGB(surf->format, tClr.r, tClr.g, tClr.b));
	SDL_SetRenderDrawColor(rend, tClr.r, tClr.g, tClr.b, 255);

	/* Apply transparent pixels to (round off) the upper-left corner */
	SDL_RenderDrawPoint(rend, 0, 0);
	SDL_RenderDrawPoint(rend, 0, 1);
	SDL_RenderDrawPoint(rend, 1, 0);

	/* Apply transparent pixels to (round off) the lower-left corner */
	SDL_RenderDrawPoint(rend, 0, rSize->h - 1);
	SDL_RenderDrawPoint(rend, 0, rSize->h - 2);
	SDL_RenderDrawPoint(rend, 1, rSize->h - 1);

	/* Apply transparent pixels to (round off) the upper-right corner */
	SDL_RenderDrawPoint(rend, rSize->w - 1, 0);
	SDL_RenderDrawPoint(rend, rSize->w - 1, 1);
	SDL_RenderDrawPoint(rend, rSize->w - 2, 0);

	/* Apply transparent pixels to (round off) the lower-right corner */
	SDL_RenderDrawPoint(rend, rSize->w - 1, rSize->h - 1);
	SDL_RenderDrawPoint(rend, rSize->w - 1, rSize->h - 2);
	SDL_RenderDrawPoint(rend, rSize->w - 2, rSize->h - 1);

	/*------------------------------------------------------------------------*/
	/* Draw the border around the ProgressBar.                                */
	/*------------------------------------------------------------------------*/
	bClr = &(aCfg->borderColor);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

	/* Draw upper-left corner border pixels */
	SDL_RenderDrawPoint(rend, 1, 1);
	SDL_RenderDrawPoint(rend, 1, 2);
	SDL_RenderDrawPoint(rend, 2, 1);

	/* Draw lower-left corner border pixels */
	SDL_RenderDrawPoint(rend, 1, rSize->h - 2);
	SDL_RenderDrawPoint(rend, 1, rSize->h - 3);
	SDL_RenderDrawPoint(rend, 2, rSize->h - 2);

	/* Draw upper-right corner border pixels */
	SDL_RenderDrawPoint(rend, rSize->w - 2, 1);
	SDL_RenderDrawPoint(rend, rSize->w - 2, 2);
	SDL_RenderDrawPoint(rend, rSize->w - 3, 1);

	/* Draw upper-right corner border pixels */
	SDL_RenderDrawPoint(rend, rSize->w - 2, rSize->h - 2);
	SDL_RenderDrawPoint(rend, rSize->w - 2, rSize->h - 3);
	SDL_RenderDrawPoint(rend, rSize->w - 3, rSize->h - 2);


	/* upper border line */
	SDL_RenderDrawLine(rend, 2, 0, rSize->w - 3, 0); 
	/* left border line */
	SDL_RenderDrawLine(rend, 0, 2, 0, rSize->h - 3); 
	/* right border line */
	SDL_RenderDrawLine(rend, rSize->w - 1, 2, rSize->w - 1, rSize->h - 3); 

	/* lower border line */
	bClr = &(aCfg->borderColorDark);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
	SDL_RenderDrawLine(rend, 2, rSize->h - 1, rSize->w - 3, rSize->h - 1);

	/*------------------------------------------------------------------------*/
	/* Render the ProgressBar text                                            */
	/*------------------------------------------------------------------------*/
	if (strlen(pb->fracText) > 0)
	{
		font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
			aCfg->fontBold, aCfg->fontItalic);
		if (font == NULL) 
		{
			eNum = 9;
			goto out;
		}

		tTex = NEUIK_RenderText(pb->fracText, font, *fgClr, rend, &textW, &textH);
		if (tTex == NULL)
		{
			eNum = 10;
			goto out;
		}

		switch (eBase->eCfg.HJustify)
		{
			case NEUIK_HJUSTIFY_LEFT:
				rect.x = 6;
				rect.y = (int) ((float)(rSize->h - textH)/2.0);
				rect.w = textW;
				rect.h = 1.1*textH;
				break;

			case NEUIK_HJUSTIFY_CENTER:
				rect.x = (int) ((float)(rSize->w - textW)/2.0);
				rect.y = (int) ((float)(rSize->h - textH)/2.0);
				rect.w = textW;
				rect.h = 1.1*textH;
				break;

			case NEUIK_HJUSTIFY_RIGHT:
				rect.x = (int) (rSize->w - textW - 6);
				rect.y = (int) ((float)(rSize->h - textH)/2.0);
				rect.w = textW;
				rect.h = 1.1*textH;
				break;
		}

		SDL_RenderCopy(rend, tTex, NULL, &rect);
	}

	/*------------------------------------------------------------------------*/
	/* Copy the text onto the renderer and update it                          */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 11;
		goto out;
	}
	eBase->eSt.doRedraw = 0;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	ConditionallyDestroyTexture(&tTex);
	ConditionallyDestroyTexture(&gTex);
	if (deltaPP != NULL) free(deltaPP);
	
	if (eBase == NULL) return NULL;
	return eBase->eSt.texture;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__ProgressBar
 *
 *  Description:   Check to see if this event is captured by the element.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
int neuik_Element_CaptureEvent__ProgressBar(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                    evCaputred = 0;
	NEUIK_ProgressBar    * pb         = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	SDL_Event            * e;
	SDL_MouseMotionEvent * mouseMotEv;
	SDL_MouseButtonEvent * mouseButEv;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		/* not the right type of object */
		goto out;
	}
	pb = (NEUIK_ProgressBar *)elem;

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (mouseclick/mousemotion).   */
	/*------------------------------------------------------------------------*/
	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_MOUSEBUTTONDOWN:
		mouseButEv = (SDL_MouseButtonEvent*)(e);
		if (mouseButEv->y >= eBase->eSt.rLoc.y && mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
		{
			if (mouseButEv->x >= eBase->eSt.rLoc.x && mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
			{
				/* This mouse click originated within this ProgressBar */
				pb->clickOrigin = 1;
				pb->selected    = 1;
				pb->wasSelected = 1;
				neuik_Window_TakeFocus(eBase->eSt.window, pb);
				neuik_Element_RequestRedraw(pb);
				neuik_Element_TriggerCallback(pb, NEUIK_CALLBACK_ON_CLICK);
				evCaputred       = 1;
				goto out;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mouseButEv = (SDL_MouseButtonEvent*)(e);
		if (pb->clickOrigin)
		{
			if (mouseButEv->y >= eBase->eSt.rLoc.y && 
				mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseButEv->x >= eBase->eSt.rLoc.x && 
					mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* cursor is still within the ProgressBar, activate cbFunc */
					neuik_Element_TriggerCallback(pb, NEUIK_CALLBACK_ON_CLICKED);
				}
			}
			pb->selected    = 0;
			pb->wasSelected = 0;
			pb->clickOrigin = 0;
			neuik_Element_RequestRedraw(pb);
			evCaputred      = 1;
			goto out;
		}
		break;

	case SDL_MOUSEMOTION:
		mouseMotEv = (SDL_MouseMotionEvent*)(e);

		if (pb->clickOrigin)
		{
			/*----------------------------------------------------------------*/
			/* The mouse was initially clicked within the ProgressBar. If the */
			/* user moves the cursor out of the ProgressBar area, deselect    */
			/* it.                                                            */
			/*----------------------------------------------------------------*/
			pb->selected = 0;
			if (mouseMotEv->y >= eBase->eSt.rLoc.y && 
				mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseMotEv->x >= eBase->eSt.rLoc.x && 
					mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					pb->selected = 1;
				}
			}

			if (pb->wasSelected != pb->selected)
			{
				neuik_Element_RequestRedraw(pb);
			}
			pb->wasSelected = pb->selected;
			evCaputred = 1;
			goto out;
		}

		break;
	}

out:
	return evCaputred;
}

