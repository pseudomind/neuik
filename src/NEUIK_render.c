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

#include "NEUIK_colors.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"


void ConditionallyDestroyTexture(
	SDL_Texture **tex) /* The target texture will be freed and nulled out */
{
	if (tex != NULL)
	{
		if ((*tex) != NULL)
		{
			SDL_DestroyTexture(*tex);
			(*tex) = NULL;
		}
	}
}


/*******************************************************************************
 *
 *  Name:          RenderArrowDown
 *
 *  Description:   Renders an arrow pointing down as a SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * NEUIK_RenderArrowDown(
	NEUIK_Color     color,
	SDL_Renderer  * xRend,
	RenderSize      rSize)
{
	int                 ctr;
	int                 eNum       = 0;    /* which error to report (if any) */
	int                 slopeAdj;
	float               slope;
	unsigned int        encPixClr;
	SDL_Surface       * iSurf      = NULL;
	SDL_Renderer      * iRend      = NULL;
	SDL_Texture       * rvTex      = NULL;
	static SDL_Color    tClr       = COLOR_TRANSP;
	static char         funcName[] = "NEUIK_RenderArrowDown";
	static char       * errMsgs[]  = {"",              // [0] no error
		"Failed to create RGB surface.",               // [1]
		"Failed to create software renderer.",         // [2]
		"SDL_CreateTextureFromSurface returned NULL.", // [3]
	};

	iSurf = SDL_CreateRGBSurface(0, rSize.w, rSize.h, 32, 0, 0, 0, 0);
	if (iSurf == NULL)
	{
		eNum = 1;
		goto out;
	}

	iRend = SDL_CreateSoftwareRenderer(iSurf);
	if (iRend == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Fill the background of the Label using a transparent color             */
	/*------------------------------------------------------------------------*/
	SDL_SetRenderDrawColor(iRend, tClr.r, tClr.g, tClr.b, 255);
	SDL_RenderClear(iRend);

	encPixClr = SDL_MapRGB(iSurf->format, tClr.r, tClr.g, tClr.b);
	SDL_SetColorKey(iSurf, SDL_TRUE, encPixClr);

	/*------------------------------------------------------------------------*/
	/* Draw the triangular down arrow                                         */
	/*------------------------------------------------------------------------*/
	SDL_SetRenderDrawColor(iRend, color.r, color.g, color.b, 255);

	/* dw/dh */
	slope = (((float)rSize.w - 1)/2.0)  /  ((float)rSize.h - 1);

	for (ctr = 0; ctr < rSize.h; ctr++)
	{
		slopeAdj  = slope * ((float)(ctr));
		SDL_RenderDrawLine(iRend, slopeAdj, ctr, rSize.w - (1 + slopeAdj), ctr);
	}
	SDL_RenderDrawPoint(iRend, rSize.w/2, rSize.h - 1);

	/*------------------------------------------------------------------------*/
	/* Present the renderer and update the texture                            */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(iRend);
	rvTex = SDL_CreateTextureFromSurface(xRend, iSurf);
	if (rvTex == NULL)
	{
		eNum = 3;
		goto out;
	}

out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (iSurf != NULL) SDL_FreeSurface(iSurf);

	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          RenderText
 *
 *  Description:   Renders a string of text as a SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * NEUIK_RenderText(
		const char    *textStr, 
		TTF_Font      *font, 
		NEUIK_Color    textColor,
		SDL_Renderer  *renderer, 
		int           *rvW,      /* [out] best width for resulting surface */
		int           *rvH)      /* [out] best height for resulting surface */
{
	SDL_Color      color;
	SDL_Surface  * surf  = NULL;
	SDL_Texture  * rvTex = NULL;
	int            eNum  = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_RenderText";
	static char  * errMsgs[] = {"",              // [0] no error
		"Pointer to Font is NULL.",              // [1]
		"Failed to Render Text.",                // [2]
		"SDL_CreateTextureFromSurface failed.",  // [3]
	};


	if (font == NULL)
	{
		eNum = 1;
		goto out;
	}

	color.r = textColor.r;
	color.g = textColor.g;
	color.b = textColor.b;
	color.a = textColor.a;

	TTF_SizeText(font, textStr, rvW, rvH);

	surf = TTF_RenderText_Blended(font, textStr, color);
	if (surf != NULL)
	{
		rvTex = SDL_CreateTextureFromSurface(renderer, surf);
		if (rvTex == NULL)
		{
			eNum = 3;
			goto out;
		}
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
	}
	if (surf != NULL) SDL_FreeSurface(surf);


	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          RenderTextAsSurface
 *
 *  Description:   Renders a string of text as a SDL_Surface*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Surface*.
 *
 ******************************************************************************/
SDL_Surface * NEUIK_RenderTextAsSurface(
		const char    *textStr, 
		TTF_Font      *font, 
		NEUIK_Color    textColor,
		SDL_Renderer  *renderer, 
		int           *rvW,      /* [out] best width for resulting surface */
		int           *rvH)      /* [out] best height for resulting surface */
{
	SDL_Color      color;
	SDL_Surface  * surf  = NULL;
	int            eNum  = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_RenderTextAsSurface";
	static char  * errMsgs[] = {"",              // [0] no error
		"Pointer to Font is NULL.",              // [1]
		"Failed to Render Text.",                // [2]
		"SDL_CreateTextureFromSurface failed.",  // [3]
	};

	if (font == NULL)
	{
		eNum = 1;
		goto out;
	}

	color.r = textColor.r;
	color.g = textColor.g;
	color.b = textColor.b;
	color.a = textColor.a;

	TTF_SizeText(font, textStr, rvW, rvH);

	surf = TTF_RenderText_Blended(font, textStr, color);
	if (surf == NULL)
	{
		eNum = 2;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	return surf;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_RenderGradient
 *
 *  Description:   Renders a color gradient using the specified ColorStops.
 *
 *                 Vertical gradients start at the top and go down from there.
 *                 Horizontal gradients start at the left and go right from there.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * NEUIK_RenderGradient(
		NEUIK_ColorStop  ** cs,       /* [in] ColorStops which define the gradient */
		                              /*  this array should be NULL-ptr terminated */
		char                dirn,     /* [in] Direction of the gradient 'v' or 'h' */
		SDL_Renderer      * renderer, /* [in] Renderer to make the texture for */
		RenderSize          rSize)    /* [in] Desired size of the gradient */
{
	int             ctr;
	int             gCtr;             /* gradient counter */
	int             nClrs;
	int             eNum      = 0;    /* which error to report (if any) */
	int             clrR;
	int             clrG;
	int             clrB;
	int             clrFound;
	float           lastFrac  = -1.0;
	float           frac;
	float           fracDelta;        /* fraction between ColorStop 1 & 2 */
	float           fracStart = 0.0;  /* fraction at ColorStop 1 */
	float           fracEnd   = 1.0;  /* fraction at ColorStop 2 */
	SDL_Renderer  * rend      = NULL;
	SDL_Surface   * surf      = NULL;
	SDL_Texture   * rvTex     = NULL;
	colorDeltas   * deltaPP   = NULL;
	colorDeltas   * clrDelta;
	NEUIK_Color   * clr;
	static char     funcName[] = "NEUIK_RenderGradient";
	static char   * errMsgs[] = {"",                          // [0] no error
		"Pointer to ColorStops is NULL.",                     // [1]
		"Unsupported gradient direction.",                    // [2]
		"Invalid RenderSize supplied.",                       // [3]
		"Unable to create RGB surface.",                      // [4]
		"SDL_CreateTextureFromSurface failed.",               // [5]
		"ColorStops array is empty.",                         // [6]
		"Invalid ColorStop fraction (<0 or >1).",             // [7]
		"ColorStops array fractions not in ascending order.", // [8]
		"Failure to allocate memory.",                        // [9]
		"Failed to create software renderer.",                // [10]
	};


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
		}
	}


	surf = SDL_CreateRGBSurface(0, rSize.w, rSize.h, 32, 0, 0, 0, 0);
	if (surf == NULL)
	{
		eNum = 4;
		goto out;
	}

	rend = SDL_CreateSoftwareRenderer(surf);
	if (rend == NULL)
	{
		eNum = 10;
		goto out;
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
		SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, 255);
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
				SDL_SetRenderDrawColor(rend, clrR, clrG, clrB, 255);
			}
			else
			{
				/* not between two ColorStops, use a single color */
				SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, 255);
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


	rvTex = SDL_CreateTextureFromSurface(renderer, surf);
	if (rvTex == NULL)
	{
		eNum = 5;
		goto out;
	}
	// SDL_SetTextureBlendMode(rvTex, SDL_BLENDMODE_NONE);

out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (rend    != NULL) SDL_DestroyRenderer(rend);
	if (surf    != NULL) SDL_FreeSurface(surf);
	if (deltaPP != NULL) free(deltaPP);

	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          RenderText_Solid
 *
 *  Description:   Renders a string of text as a SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture* NEUIK_RenderText_Solid(
		const char    *textStr, 
		TTF_Font      *font, 
		NEUIK_Color    textColor,
		SDL_Renderer  *renderer, 
		int           *rvW,      /* [out] best width for resulting surface */
		int           *rvH)      /* [out] best height for resulting surface */
{
	int           eNum = 0; /* which error to report (if any) */
	SDL_Color     color;
	SDL_Surface * surf  = NULL;
	SDL_Texture * rvTex = NULL;
	static char   funcName[] = "NEUIK_RenderText_Solid";
	static char * errMsgs[] = {"",               // [0] no error
		"Pointer to Font is NULL.",              // [1]
		"Failed to Render Text.",                // [2]
		"SDL_CreateTextureFromSurface failed.",  // [3]
	};


	if (font == NULL)
	{
		eNum = 1;
		goto out;
	}

	color.r = textColor.r;
	color.g = textColor.g;
	color.b = textColor.b;
	color.a = textColor.a;

	TTF_SizeText(font, textStr, rvW, rvH);

	surf = TTF_RenderText_Solid(font, textStr, color);
	if (surf != NULL)
	{
		rvTex = SDL_CreateTextureFromSurface(renderer, surf);
		if (rvTex == NULL)
		{
			eNum = 3;
			goto out;
		}
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
	}
	if (surf != NULL) SDL_FreeSurface(surf);


	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          RenderTextSolid
 *
 *  Description:   Renders a string of text as a SDL_Texture* (doesn't blend).
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture* NEUIK_RenderTextSolid(
		const char    *textStr, 
		TTF_Font      *font, 
		NEUIK_Color    textColor,
		SDL_Renderer  *renderer, 
		int           *rvW,      /* [out] best width for resulting surface */
		int           *rvH)      /* [out] best height for resulting surface */
{
	int           eNum = 0; /* which error to report (if any) */
	SDL_Color     color;
	SDL_Surface * surf  = NULL;
	SDL_Texture * rvTex = NULL;
	static char   funcName[] = "NEUIK_RenderTextSolid";
	static char * errMsgs[] = {"",               // [0] no error
		"Pointer to Font is NULL.",              // [1]
		"Failed to Render Text.",                // [2]
		"SDL_CreateTextureFromSurface failed.",  // [3]
	};


	if (font == NULL)
	{
		eNum = 1;
		goto out;
	}

	color.r = textColor.r;
	color.g = textColor.g;
	color.b = textColor.b;
	color.a = textColor.a;

	TTF_SizeText(font, textStr, rvW, rvH);

	surf = TTF_RenderText_Solid(font, textStr, color);
	if (surf != NULL)
	{
		rvTex = SDL_CreateTextureFromSurface(renderer, surf);
		if (rvTex == NULL)
		{
			eNum = 3;
			goto out;
		}
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
	}
	if (surf != NULL) SDL_FreeSurface(surf);


	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          String_Duplicate
 *
 *  Description:   Allocates and copies the source string to dest string.
 *
 *  Returns:       Sets dest string pointer to NULL if error.
 *
 ******************************************************************************/
void String_Duplicate(
		char        **dst, 
		const char   *src)
{
	int sLen = 1;
	if (dst == NULL) return; /* unreportable failure */
	if (src == NULL)
	{
		*dst = NULL;
	}
	else
	{
		sLen += strlen(src);
	}

	(*dst) = (char*)malloc(sLen*sizeof(char));
	if ((*dst) == NULL) return; /* unable to allocate memory */

	if (src != NULL) strcpy((*dst), src);
}

