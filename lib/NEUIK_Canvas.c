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
#include "NEUIK_Canvas.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Canvas(void **);
int neuik_Object_Free__Canvas(void *);

int neuik_Element_GetMinSize__Canvas(NEUIK_Element, RenderSize*);
SDL_Texture * neuik_Element_Render__Canvas(NEUIK_Element, RenderSize*, SDL_Renderer*, SDL_Surface*);


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Canvas_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Canvas,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Canvas,
};

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Canvas_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Canvas,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Canvas,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Canvas
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Canvas()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Canvas";
	static char  * errMsgs[]  = {"",                  // [0] no error
		"NEUIK library must be initialized first.",   // [1]
		"Failed to register `Canvas` object class .", // [2]
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
		"NEUIK_Canvas",                      // className
		"An element which can be drawn to.", // classDescription
		neuik__Set_NEUIK,                    // classSet
		neuik__Class_Element,                // superClass
		&neuik_Canvas_BaseFuncs,             // baseFuncs
		NULL,                                // classFuncs
		&neuik__Class_Canvas))               // newClass
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
 *  Name:          neuik_Object_New__Canvas
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Canvas(
		void ** cnvsPtr)
{
	int             eNum       = 0;
	NEUIK_Canvas  * cnvs       = NULL;
	NEUIK_Element * sClassPtr  = NULL;
	char          * dFontName; /* don't free this value */
	static char     funcName[] = "neuik_Object_New__Canvas";
	static char   * errMsgs[]  = {"",                                       // [0] no error
		"Output Argument `cnvPtr` is NULL.",                                // [1]
		"Failure to allocate memory.",                                      // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                         // [3]
		"Failure in function `neuik.NewElement`.",                          // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",                // [5]
		"Argument `cnvsPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",        // [7]
		"Failure in `NEUIK_GetDefaultFontSet()`.",                          // [8]
		"Failure in `String_Duplicate()`.",                                 // [9]
		"Failure in `NEUIK_FontSet_GetFont()`.",                            // [10]
	};

	if (cnvsPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*cnvsPtr) = (NEUIK_Canvas*) malloc(sizeof(NEUIK_Canvas));
	cnvs = *cnvsPtr;
	if (cnvs == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Canvas, 
			NULL,
			&(cnvs->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(cnvs->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(cnvs, &neuik_Canvas_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	cnvs->fontSet    = NULL; /* NEUIK_FontSet */
	cnvs->fontName   = NULL; /* font name for the TTF_Font */
	cnvs->fontSize   = 11;   /* point size to use for the TTF_Font */
	cnvs->fontBold   = 0;    /* (bool) use bold style */
	cnvs->fontItalic = 0;    /* (bool) use italic style */
	cnvs->draw_x     = 0;
	cnvs->draw_y     = 0;
	cnvs->draw_clr_r = 0;
	cnvs->draw_clr_g = 0;
	cnvs->draw_clr_b = 0;
	cnvs->draw_clr_a = 0;
	
	cnvs->ops = malloc(100*sizeof(neuik_canvas_op));
	if (cnvs->ops == NULL)
	{
		eNum = 2;
		goto out;
	}

	cnvs->ops_allocated = 100;
	cnvs->ops_used      = 0;

	/* Look for the first default font that is supported */
	cnvs->fontSet = NEUIK_GetDefaultFontSet(&dFontName);
	if (cnvs->fontSet == NULL)
	{
		eNum = 8;
		goto out;
	}

	String_Duplicate(&cnvs->fontName, dFontName);
	if (cnvs->fontName == NULL)
	{
		eNum = 9;
		goto out;
	}

	/* Finally attempt to load the font */
	if (NEUIK_FontSet_GetFont(cnvs->fontSet, cnvs->fontSize,
		cnvs->fontBold, cnvs->fontItalic) == NULL)
	{
		eNum = 10;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(cnvs, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cnvs, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cnvs, "hovered"))
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
 *  Name:          neuik_Object_Free__Canvas
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Canvas(
	void  * cnvsPtr)
{
	int            eNum       = 0;    /* which error to report (if any) */
	NEUIK_Canvas * cnvs       = NULL;
	static char    funcName[] = "neuik_Object_Free__Canvas";
	static char  * errMsgs[]  = {"",                     // [0] no error
		"Argument `cnvsPtr` is NULL.",                   // [1]
		"Argument `cnvsPtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",      // [3]
	};

	if (cnvsPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(cnvsPtr, neuik__Class_Canvas))
	{
		eNum = 2;
		goto out;
	}
	cnvs = (NEUIK_Canvas*)cnvsPtr;

	if (cnvs->ops != NULL) free(cnvs->ops);

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(cnvs->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(cnvs);
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
 *  Name:          NEUIK_NewCanvas
 *
 *  Description:   Create a new NEUIK_Canvas.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewCanvas(
	NEUIK_Canvas ** cnvsPtr)
{
	int                 eNum       = 0; /* which error to report (if any) */
	NEUIK_Canvas      * cnvs       = NULL;
	NEUIK_ElementBase * eBase      = NULL; 
	static char         funcName[] = "NEUIK_NewCanvas";
	static char       * errMsgs[]  = {"",                                   // [0] no error
		"Failure in function `neuik_Object_New__Canvas`.",                  // [1]
		"Argument `cnvsPtr` caused `neuik_Object_GetClassObject` to fail.", // [2]
	};

	if (neuik_Object_New__Canvas((void**)cnvsPtr))
	{
		eNum = 1;
		goto out;
	}
	cnvs = *cnvsPtr;

	/*------------------------------------------------------------------------*/
	/* Configure the cnvs to be horizontal                                    */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(cnvs, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	eBase->eCfg.HFill = 1;
	eBase->eCfg.VFill = 1;
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
 *  Name:          neuik_Element_GetMinSize__Canvas
 *
 *  Description:   Returns the rendered size of a given canvas.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Canvas(
	NEUIK_Element   elem,
	RenderSize    * rSize)
{
	if (rSize != NULL)
	{
		rSize->w = 1;
		rSize->h = 1;
	}
	return 0;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_RenderCanvas
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
SDL_Texture * neuik_Element_Render__Canvas(
	NEUIK_Element   elem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf) /* the external surface (used for transp. bg) */
{
	int                 eNum       = 0; /* which error to report (if any) */
	int                 ctr;            /* loop iteration counter */
	int                 textW;
	int                 textH;
	const NEUIK_Color * lClr       = NULL;
	SDL_Surface       * surf       = NULL;
	SDL_Surface       * text_surf  = NULL;
	SDL_Renderer      * rend       = NULL;
	TTF_Font          * font       = NULL;
	SDL_Rect            rect;
	SDL_Color           color;
	neuik_canvas_op   * op;
	NEUIK_Canvas      * cnvs       = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "neuik_Element_Render__Canvas";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `elem` is not of Canvas class.",                       // [1]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"neuik_Element_GetMinSize__Canvas failed.",                      // [3]
		"Invalid cnvs orientation.",                                     // [4]
		"SDL_CreateTextureFromSurface returned NULL.",                   // [5]
		"Failure in neuik_Element_RedrawBackground().",                  // [6]
		"Failure in SDL_SetRenderDrawColor().",                          // [7]
		"Failure in SDL_RenderDrawPoint().",                             // [8]
		"Failure in SDL_RenderDrawLine().",                              // [9]
		"Failure in SDL_RenderClear().",                                 // [10]
		"Failure in `TTF_RenderText_Blended()`.",                        // [11]
		"FontSet_GetFont returned NULL.",                                // [12]
	};

	if (!neuik_Object_IsClass(elem, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}
	cnvs = (NEUIK_Canvas*)elem;

	if (neuik_Object_GetClassObject(cnvs, neuik__Class_Element, (void**)&eBase))
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
		if (!neuik_Element_NeedsRedraw(cnvs) && eBase->eSt.texture != NULL) 
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
		if (neuik_Element_GetMinSize__Canvas(cnvs, rSize))
		{
			eNum = 1;
			goto out;
		}
	}
	else if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 6;
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
		if (neuik_Element_Resize(cnvs, *rSize) != 0)
		{
			eNum = 3;
			goto out;
		}
	}
	surf = eBase->eSt.surf;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(elem, xSurf))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Redraw the canvas as specified by the associated draw functions.       */
	/*------------------------------------------------------------------------*/
	if (cnvs->ops_used > 0)
	{
		for (ctr = 0; ctr < cnvs->ops_used; ctr++)
		{
			op = &cnvs->ops[ctr];
			switch (op->op) {
				case NEUIK_CANVAS_OP_MOVETO:
					cnvs->draw_x = op->op_moveto.x;
					cnvs->draw_y = op->op_moveto.y;
					break;
				case NEUIK_CANVAS_OP_SETDRAWCOLOR:
					cnvs->draw_clr_r = op->op_setdrawcolor.r;
					cnvs->draw_clr_g = op->op_setdrawcolor.g;
					cnvs->draw_clr_b = op->op_setdrawcolor.b;
					cnvs->draw_clr_a = op->op_setdrawcolor.a;

					if (SDL_SetRenderDrawColor(rend, 
						cnvs->draw_clr_r,
						cnvs->draw_clr_g,
						cnvs->draw_clr_b,
						cnvs->draw_clr_a))
					{
						eNum = 7;
						goto out;
					}
					break;
				case NEUIK_CANVAS_OP_DRAWPOINT:
					if (SDL_RenderDrawPoint(rend, cnvs->draw_x, cnvs->draw_y))
					{
						eNum = 8;
						goto out;
					}
					break;
				case NEUIK_CANVAS_OP_DRAWLINE:
					if (SDL_RenderDrawLine(rend,
						cnvs->draw_x, 
						cnvs->draw_y,
						op->op_drawline.x, 
						op->op_drawline.y))
					{
						eNum = 9;
						goto out;
					}
					/* Update the position of the draw point */
					cnvs->draw_x = op->op_drawline.x;
					cnvs->draw_y = op->op_drawline.y;
					break;

				case NEUIK_CANVAS_OP_DRAWTEXT:
					#pragma message("Implement NEUIK_CANVAS_OP_DRAWTEXT")
					color.r = cnvs->draw_clr_r;
					color.g = cnvs->draw_clr_g;
					color.b = cnvs->draw_clr_b;
					color.a = cnvs->draw_clr_a;

					if (font == NULL)
					{
						font = NEUIK_FontSet_GetFont(cnvs->fontSet, 
							cnvs->fontSize, cnvs->fontBold, cnvs->fontItalic);
						if (font == NULL) 
						{
							eNum = 12;
							goto out;

						}
					}
					TTF_SizeText(font, op->op_drawtext.text, &textW, &textH);

					text_surf = TTF_RenderText_Blended(font, 
						op->op_drawtext.text, color);
					if (text_surf == NULL)
					{
						eNum = 11;
						goto out;
					}

					rect.x = cnvs->draw_x;
					rect.y = cnvs->draw_y;
					rect.w = textW;
					rect.h = textH;
					SDL_BlitSurface(text_surf, NULL, surf, &rect);

					/* free the text surface before continuing */
					if (text_surf != NULL) SDL_FreeSurface(text_surf);
					text_surf = NULL;
					break;

				case NEUIK_CANVAS_OP_DRAWTEXTLARGE:
					#pragma message("Implement NEUIK_CANVAS_OP_DRAWTEXTLARGE")
					color.r = cnvs->draw_clr_r;
					color.g = cnvs->draw_clr_g;
					color.b = cnvs->draw_clr_b;
					color.a = cnvs->draw_clr_a;
					break;
				case NEUIK_CANVAS_OP_SETTEXTSIZE:
					cnvs->text_size = op->op_settextsize.size;
					font = NEUIK_FontSet_GetFont(cnvs->fontSet, cnvs->fontSize,
						cnvs->fontBold, cnvs->fontItalic);
					if (font == NULL) 
					{
						eNum = 12;
						goto out;

					}
					break;
				case NEUIK_CANVAS_OP_FILL:
					if (SDL_RenderClear(rend))
					{
						eNum = 10;
						goto out;
					}
					break;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Copy the text onto the renderer and update it                          */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 5;
		goto out;
	}
	eBase->eSt.doRedraw = 0;
out:
	if (text_surf != NULL) SDL_FreeSurface(text_surf);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (eBase == NULL) return NULL;
	return eBase->eSt.texture;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Canvas_Clear
 *
 *  Description:   Clear the draw operation buffer for this canvas.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_Clear(
	NEUIK_Canvas * cnvs)
{
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_Canvas_Clear";
	static char  * errMsgs[]  = {"",               // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no operations were there; don't do anything                */
	/*------------------------------------------------------------------------*/
	if (cnvs->ops_used == 0) goto out;

	cnvs->ops_used = 0;
	neuik_Element_RequestRedraw(cnvs);

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
 *  Name:          NEUIK_Canvas_MoveTo
 *
 *  Description:   Move the location of the draw point.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_MoveTo(
	NEUIK_Canvas * cnvs,
	unsigned int   x,
	unsigned int   y)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_MoveTo";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	op.op = NEUIK_CANVAS_OP_MOVETO;
	op.op_moveto.x = x;
	op.op_moveto.y = y;

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_SetDrawColor
 *
 *  Description:   Set the active draw color of the canvas.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_SetDrawColor(
	NEUIK_Canvas * cnvs,
	unsigned char  r,
	unsigned char  g,
	unsigned char  b,
	unsigned char  a)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_SetDrawColor";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	op.op = NEUIK_CANVAS_OP_SETDRAWCOLOR;
	op.op_setdrawcolor.r = r;
	op.op_setdrawcolor.g = g;
	op.op_setdrawcolor.b = b;
	op.op_setdrawcolor.a = a;

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_SetTextSize
 *
 *  Description:   Set the text size to use for drawing new text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_SetTextSize(
	NEUIK_Canvas * cnvs,
	unsigned int   size)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_SetTextSize";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	op.op = NEUIK_CANVAS_OP_SETTEXTSIZE;
	op.op_settextsize.size = size;

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_DrawPoint
 *
 *  Description:   Draw a point under the current draw location.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_DrawPoint(
	NEUIK_Canvas * cnvs)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_DrawPoint";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}
	op.op = NEUIK_CANVAS_OP_DRAWPOINT;
	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_DrawLine
 *
 *  Description:   Draw a line from the current draw point to the specified draw
 *                 point. This will also move the draw point to the final draw
 *                 point of the line.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_DrawLine(
	NEUIK_Canvas * cnvs,
	unsigned int   x,
	unsigned int   y)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_DrawLine";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	op.op = NEUIK_CANVAS_OP_DRAWLINE;
	op.op_drawline.x = x;
	op.op_drawline.y = y;

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_DrawText
 *
 *  Description:   Draw some text with it's top left corner at the current draw
 *                 point.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_DrawText(
	NEUIK_Canvas * cnvs,
	const char   * text)
{
	int               eNum       = 0;    /* which error to report (if any) */
	size_t            textLen;
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_DrawText";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Arugment `text` is NULL.",                // [2]
		"Failure to reallocate memory.",           // [3]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	if (text == NULL)
	{
		eNum = 2;
		goto out;
	}

	textLen = strlen(text);

	if (textLen <= 20)
	{
		op.op = NEUIK_CANVAS_OP_DRAWTEXT;
		strcpy(op.op_drawtext.text, text);
	}
	else
	{
		op.op = NEUIK_CANVAS_OP_DRAWTEXTLARGE;
	}

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 3;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
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
 *  Name:          NEUIK_Canvas_Fill
 *
 *  Description:   Fill the canvas with the current draw color.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Canvas_Fill(
	NEUIK_Canvas * cnvs)
{
	int               eNum       = 0;    /* which error to report (if any) */
	neuik_canvas_op   op;
	static char       funcName[] = "NEUIK_Canvas_Fill";
	static char     * errMsgs[]  = {"",            // [0] no error
		"Argument `cnvs` is not of Canvas class.", // [1]
		"Failure to reallocate memory.",           // [2]
	};

	if (!neuik_Object_IsClass(cnvs, neuik__Class_Canvas))
	{
		eNum = 1;
		goto out;
	}

	op.op = NEUIK_CANVAS_OP_FILL;

	if (cnvs->ops_used >= cnvs->ops_allocated)
	{
		cnvs->ops_allocated += 50;
		cnvs->ops = realloc(cnvs->ops, 
			cnvs->ops_allocated*sizeof(neuik_canvas_op));
		if (cnvs->ops == NULL)
		{
			eNum = 2;
			goto out;
		}
	}
	cnvs->ops[cnvs->ops_used++] = op;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}
