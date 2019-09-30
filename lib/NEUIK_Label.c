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
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "NEUIK_neuik.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Label.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__Label(void ** lblPtr);
int neuik_Object_Free__Label(void * lblPtr);
int neuik_Element_GetMinSize__Label(NEUIK_Element, RenderSize*);
int neuik_Element_Render__Label(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, int);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_Label_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__Label,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__Label,

	/* CaptureEvent(): Determine if this element caputures a given event */
	NULL,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_Label_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__Label,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__Label,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_Label
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_Label()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_Label";
	static char  * errMsgs[]  = {"",                 // [0] no error
		"NEUIK library must be initialized first.",  // [1]
		"Failed to register `Label` object class .", // [2]
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
		"NEUIK_Label",                      // className
		"A GUI label which contains text.", // classDescription
		neuik__Set_NEUIK,                   // classSet
		neuik__Class_Element,               // superClass
		&neuik_Label_BaseFuncs,             // baseFuncs
		NULL,                               // classFuncs
		&neuik__Class_Label))               // newClass
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
 *  Name:          neuik_Object_New__Label
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__Label(
	void ** lblPtr)
{
	int             eNum       = 0; /* which error to report (if any) */
	NEUIK_Label   * lbl        = NULL;
	NEUIK_Element * sClassPtr  = NULL;
	static char     funcName[] = "neuik_Object_New__Label";
	static char   * errMsgs[]  = {"",                                // [0] no error
		"Failure to allocate memory.",                               // [1]
		"Failure in NEUIK_NewLabelConfig.",                          // [2]
		"Output Argument `lblPtr` is NULL.",                         // [3]
		"Failure in function `neuik_Object_New`.",                   // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",         // [5]
		"Failure in `neuik_GetObjectBaseOfClass`.",                  // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.", // [7]
	};

	if (lblPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*lblPtr) = (NEUIK_Label *)malloc(sizeof(NEUIK_Label));
	lbl = *lblPtr;
	if (lbl == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_Label, 
			NULL,
			&(lbl->objBase)))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(lbl->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_Label_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	/* Allocation successful */
	lbl->cfg         = NULL;
	lbl->cfgPtr      = NULL;
	lbl->text        = NULL;
	lbl->needsRedraw = 1;

	if (NEUIK_NewLabelConfig(&lbl->cfg))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(lbl, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(lbl, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(lbl, "hovered"))
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
 *  Name:          neuik_Object_Free__Label
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__Label(
	void * lblPtr)  /* [out] the label to free */
{
	int           eNum       = 0; /* which error to report (if any) */
	NEUIK_Label * lbl        = NULL;
	static char   funcName[] = "neuik_Object_Free__Label";
	static char * errMsgs[]  = {"",                 // [0] no error
		"Argument `lblPtr` is not of Label class.", // [1]
		"Failure in function `neuik_Object_Free`.", // [2]
		"Argument `lblPtr` is NULL.",               // [3]
	};

	if (lblPtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	if (!neuik_Object_IsClass(lblPtr, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}
	lbl = (NEUIK_Label*)lblPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(lbl->objBase.superClassObj))
	{
		eNum = 2;
		goto out;
	}
	if(lbl->text != NULL) free(lbl->text);
	if(neuik_Object_Free((void**)lbl->cfg))
	{
		eNum = 2;
		goto out;
	}
	free(lbl);
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
 *  Name:          NEUIK_MakeLabel
 *
 *  Description:   Create a new NEUIK_Label and assign text to it.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeLabel(
	NEUIK_Label ** lblPtr,  /* [out] The newly created NEUIK_Label. */
	const char   * text)    /* [in]  Initial label text. */
{
	size_t        sLen       = 1;
	int           eNum       = 0; /* which error to report (if any) */
	NEUIK_Label * lbl        = NULL;
	static char   funcName[] = "NEUIK_MakeLabel";
	static char * errMsgs[]  = {"",                       // [0] no error
		"Failure in function `neuik_Object_New__Label`.", // [1]
		"Failure to allocate memory.",                    // [2]
	};

	if (neuik_Object_New__Label((void**)lblPtr))
	{
		eNum = 1;
		goto out;
	}
	lbl = *lblPtr;

	/*------------------------------------------------------------------------*/
	/* Set the new Label text contents                                        */
	/*------------------------------------------------------------------------*/
	if (text == NULL){
		/* label will contain no text */
		lbl->text = NULL;
	}
	else if (text[0] == '\0')
	{
		/* label will contain no text */
		lbl->text = NULL;
	}
	else
	{
		sLen += strlen(text);
		lbl->text = (char*)malloc(sLen*sizeof(char));
		if (lbl->text == NULL) {
			eNum = 2;
			goto out;
		}
		/* Allocation successful */
		strcpy(lbl->text, text);
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
 *  Name:          NEUIK_NewLabel
 *
 *  Description:   Create a new NEUIK_Label with no text.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewLabel(
	NEUIK_Label ** lblPtr) /* [out] The newly created NEUIK_Label. */
{
	return neuik_Object_New__Label((void**)lblPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Label_GetMinSize
 *
 *  Description:   Returns the rendered size of a given Label.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__Label(
	NEUIK_Element    lblElem,
	RenderSize     * rSize)
{
	int                       tW;
	int                       tH;
	int                       eNum       = 0;    /* which error to report (if any) */
	TTF_Font                * font       = NULL;
	NEUIK_Label             * label      = NULL;
	const NEUIK_LabelConfig * aCfg       = NULL; /* the active Label config */
	static char               funcName[] = "neuik_Element_GetMinSize__Label";
	static char             * errMsgs[]  = {"",      // [0] no error
		"Argument `lblElem` is not of Label class.", // [1]
		"LabelConfig* is NULL.",                     // [2]
		"LabelConfig->FontSet is NULL.",             // [3]
		"FontSet_GetFont returned NULL.",            // [4]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(lblElem, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}
	label = (NEUIK_Label *)(lblElem);
	
	/* select the correct Label config to use (pointer or internal) */
	if (label->cfgPtr != NULL)
	{
		aCfg = label->cfgPtr;
	}
	else 
	{
		aCfg = label->cfg;
	}

	if (aCfg == NULL)
	{
		rSize->w = -2;
		rSize->h = -2;

		eNum = 2;
		goto out;
	} 

	if (aCfg->fontSet == NULL)
	{
		rSize->w = -3;
		rSize->h = -3;
		eNum = 3;
		goto out;
	}

	font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
		aCfg->fontBold, aCfg->fontItalic);
	if (font == NULL) 
	{
		rSize->w = -4;
		rSize->h = -4;

		eNum = 4;
		goto out;
	}

	if (label->text != NULL)
	{
		/* this Label contains text */
		TTF_SizeText(font, label->text, &tW, &tH);

	}
	else
	{
		/* this Label does not contain text */
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
 *  Name:          NEUIK_Label_SetText
 *
 *  Description:   Update the text in a NEUIK_Label.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Label_SetText(
	NEUIK_Label * label,
	const char  * text)
{
	RenderSize     rSize;
	RenderLoc      rLoc;
	size_t         sLen = 1;
	int            eNum = 0; /* which error to report (if any) */
	static char    funcName[] = "NEUIK_Label_SetText";
	static char  * errMsgs[] = {"",                         // [0] no error
		"Argument `label` is not of Label class.",          // [1]
		"Failure to allocate memory.",                      // [2]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
	};

	if (!neuik_Object_IsClass(label, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Conditionally free Label text before setting the new contents          */
	/*------------------------------------------------------------------------*/
	if (label->text != NULL) {
		free(label->text);
	}

	/*------------------------------------------------------------------------*/
	/* Set the new Label text contents                                        */
	/*------------------------------------------------------------------------*/
	if (text == NULL){
		/* Label will contain no text */
		label->text = NULL;
	}
	else if (text[0] == '\0')
	{
		/* Label will contain no text */
		label->text = NULL;
	}
	else
	{
		sLen += strlen(text);
		label->text = (char*)malloc(sLen*sizeof(char));
		if (label->text == NULL) {
			eNum = 2;
			goto out;
		}
		/* Allocation successful */
		strcpy(label->text, text);
	}

	if (neuik_Element_GetSizeAndLocation(label, &rSize, &rLoc))
	{
		eNum = 3;
		goto out;
	}
	neuik_Element_RequestRedraw(label, rLoc, rSize);
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
 *  Name:          NEUIK_Label_GetText
 *
 *  Description:   Get a pointer to the text in a NEUIK_Label.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
const char * NEUIK_Label_GetText(
	NEUIK_Label * label)
{
	int            eNum       = 0; /* which error to report (if any) */
	const char   * rvPtr      = NULL;
	static char    emptyStr[] = "";
	static char    funcName[] = "NEUIK_Label_GetText";
	static char  * errMsgs[]  = {"",               // [0] no error
		"Argument `label` is not of Label class.", // [1]
	};

	if (!neuik_Object_IsClass(label, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the new Label text contents                                        */
	/*------------------------------------------------------------------------*/
	if (label->text == NULL){
		/* Label will contain no text */
		rvPtr = emptyStr;
	}
	else
	{
		rvPtr = label->text;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		rvPtr = NULL;
	}

	return rvPtr;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__Label
 *
 *  Description:   Renders a single Label as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__Label(
	NEUIK_Element   elem, 
	RenderSize    * rSize, /* [in] the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	const NEUIK_Color       * fgClr      = NULL;
	SDL_Renderer            * rend       = NULL;
	SDL_Texture             * tTex       = NULL; /* text texture */
	SDL_Rect                  rect;
	int                       textW      = 0;
	int                       textH      = 0;
	int                       eNum       = 0; /* which error to report (if any) */
	NEUIK_Label             * label      = NULL;
	TTF_Font                * font       = NULL;
	NEUIK_ElementBase       * eBase      = NULL;
	RenderLoc                 rl;
	RenderLoc                 rlAdj;             /* loc. including adjustments */
	const NEUIK_LabelConfig * aCfg       = NULL; /* the active Label config */
	static char               funcName[] = "neuik_Element_Render__Label";
	static char             * errMsgs[]  = {"",                          // [0] no error
		"Argument `elem` is not of Label class.",                        // [1]
		"", // [2]
		"Failed to Render Text.",                                        // [3]
		"FontSet_GetFont returned NULL.",                                // [4]
		"RenderText returned NULL.",                                     // [5]
		"", // [6]
		"Invalid specified `rSize` (negative values).",                  // [7]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                  // [9]
	};

	if (!neuik_Object_IsClass(elem, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}
	label = (NEUIK_Label *)elem;

	if (neuik_Object_GetClassObject(elem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 7;
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
	/* select the correct Label config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = label->cfg;
	if (label->cfgPtr != NULL)
	{
		aCfg = label->cfgPtr;
	}

	/*------------------------------------------------------------------------*/
	/* Set the appropriate foreground (text) color                            */
	/*------------------------------------------------------------------------*/
	fgClr = &(aCfg->fgColor);

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(elem, rlMod, NULL))
	{
		eNum = 9;
		goto out;
	}

	rl = eBase->eSt.rLoc;
	rlAdj = rl;
	if (rlMod != NULL)
	{
		rlAdj.x += rlMod->x;
		rlAdj.y += rlMod->y;
	}

	/*------------------------------------------------------------------------*/
	/* Render the Label text                                                  */
	/*------------------------------------------------------------------------*/
	if (label->text != NULL)
	{
		if (label->text[0] != '\0')
		{
			font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
				aCfg->fontBold, aCfg->fontItalic);
			if (font == NULL) 
			{
				eNum = 4;
				goto out;

			}

			tTex = NEUIK_RenderText(
				label->text, font, *fgClr, rend, &textW, &textH);
			if (tTex == NULL)
			{
				eNum = 5;
				goto out;
			}

			rect.x = rlAdj.x;
			rect.y = rlAdj.y;
			rect.w = textW;
			rect.h = textH;

			switch (eBase->eCfg.HJustify)
			{
				case NEUIK_HJUSTIFY_LEFT:
					rect.x += 6;
					rect.y += (int) ((float)(rSize->h - textH)/2.0);
					break;

				case NEUIK_HJUSTIFY_CENTER:
				case NEUIK_HJUSTIFY_DEFAULT:
					rect.x += (int) ((float)(rSize->w - textW)/2.0);
					rect.y += (int) ((float)(rSize->h - textH)/2.0);
					break;

				case NEUIK_HJUSTIFY_RIGHT:
					rect.x += (int) (rSize->w - textW - 6);
					rect.y += (int) ((float)(rSize->h - textH)/2.0);
					break;
			}

			SDL_RenderCopy(rend, tTex, NULL, &rect);
		}
	}
out:
	if (!mock) eBase->eSt.doRedraw = 0;

	ConditionallyDestroyTexture(&tTex);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

void neuik_Label_Configure_capture_segv(
	int sig_num)
{
	static char funcName[] = "NEUIK_Label_Configure";
	static char errMsg[] = 
		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

	NEUIK_RaiseError(funcName, errMsg);
	NEUIK_BacktraceErrors();
	exit(1);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_Label_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_Label_Configure(
	NEUIK_Label * lbl,
	const char  * set0,
	...)
{
	int                  ns; /* number of items from sscanf */
	int                  ctr;
	int                  nCtr;
	int                  eNum      = 0; /* which error to report (if any) */
	int                  doRedraw  = 0;
	int                  isBool;
	int                  boolVal   = 0;
	int                  typeMixup;
	int                  fontSize;
	int                  fontEmWidth;
	char                 buf[4096];
	RenderSize           rSize;
	RenderLoc            rLoc;
	va_list              args;
	char               * strPtr    = NULL;
	char               * name      = NULL;
	char               * value     = NULL;
	const char         * set       = NULL;
	NEUIK_Color          clr;
	NEUIK_LabelConfig  * cfg       = NULL;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char          * boolNames[] = {
		"FontBold",
		"FontItalic",
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	static char          * valueNames[] = {
		"FontEmWidth",
		"FontSize",
		"FontColor",
		NULL,
	};
	static char           funcName[] = "NEUIK_Label_Configure";
	static char         * errMsgs[]  = {"",                               // [ 0] no error
		"Argument `lbl` does not implement Label class.",                 // [ 1]
		"`name=value` string is too long.",                               // [ 2]
		"Invalid `name=value` string.",                                   // [ 3]
		"ValueType name used as BoolType, skipping.",                     // [ 4]
		"BoolType name unknown, skipping.",                               // [ 5]
		"NamedSet.name is NULL, skipping..",                              // [ 6]
		"NamedSet.name is blank, skipping..",                             // [ 7]
		"FontColor value invalid; should be comma separated RGBA.",       // [ 8]
		"FontColor value invalid; RGBA value range is 0-255.",            // [ 9]
		"Failure in `neuik_Element_GetSizeAndLocation()`.",               // [10]
		"FontEmWidth value is invalid; must be int.",                     // [11]
		"FontSize value is invalid; must be int.",                        // [12]
		"BoolType name used as ValueType, skipping.",                     // [13]
		"NamedSet.name type unknown, skipping.",                          // [14]
	};

	if (!neuik_Object_IsClass(lbl, neuik__Class_Label))
	{
		eNum = 1;
		goto out;
	}
	set = set0;

	/*------------------------------------------------------------------------*/
	/* select the correct button config to use (pointer or internal)          */
	/*------------------------------------------------------------------------*/
	cfg = lbl->cfg;
	if (lbl->cfgPtr != NULL)
	{
		cfg = lbl->cfgPtr;
	}

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
		if (ctr > 0)
		{
			/* before starting */
			set = va_arg(args, const char *);
		}

		isBool = 0;
		name   = NULL;
		value  = NULL;

		if (set == NULL) break;

		#ifndef NO_NEUIK_SIGNAL_TRAPPING
			signal(SIGSEGV, neuik_Label_Configure_capture_segv);
		#endif

		if (strlen(set) > 4095)
		{
			#ifndef NO_NEUIK_SIGNAL_TRAPPING
				signal(SIGSEGV, NULL);
			#endif
			NEUIK_RaiseError(funcName, errMsgs[2]);
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
					NEUIK_RaiseError(funcName, errMsgs[3]);
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
			if (!strcmp("FontBold", name))
			{
				if (cfg->fontBold == boolVal) continue;

				/* else: The previous setting was changed */
				cfg->fontBold = boolVal;
				doRedraw = 1;
			}
			else if (!strcmp("FontItalic", name))
			{
				if (cfg->fontItalic == boolVal) continue;

				/* else: The previous setting was changed */
				cfg->fontItalic = boolVal;
				doRedraw = 1;
			}
			else 
			{
				/*------------------------------------------------------------*/
				/* Bool parameter not found; may be mixup or mistake .        */
				/*------------------------------------------------------------*/
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
					NEUIK_RaiseError(funcName, errMsgs[4]);
				}
				else
				{
					/* An unsupported name was used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[5]);
				}
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
			else if (!strcmp("FontColor", name))
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
				if (cfg->fgColor.r == clr.r &&
					cfg->fgColor.g == clr.g &&
					cfg->fgColor.b == clr.b &&
					cfg->fgColor.a == clr.a) continue;

				/* else: The previous setting was changed */
				cfg->fgColor = clr;
				doRedraw = 1;
			}
			else if (!strcmp("FontSize", name))
			{
				ns = sscanf(value, "%d", &fontSize);
				/*------------------------------------------------------------*/
				/* Check for EOF, incorrect # of values, & out of range vals. */
				/*------------------------------------------------------------*/
			#ifndef WIN32
				if (ns == EOF || ns < 1)
			#else
				if (ns < 1)
			#endif /* WIN32 */
				{
					NEUIK_RaiseError(funcName, errMsgs[12]);
					continue;
				}
				if (cfg->fontSize == fontSize) continue;

				/* else: The previous setting was changed */
				cfg->fontSize = fontSize;
				doRedraw = 1;
			}
			else if (!strcmp("FontEmWidth", name))
			{
				ns = sscanf(value, "%d", &fontEmWidth);
				/*------------------------------------------------------------*/
				/* Check for EOF, incorrect # of values, & out of range vals. */
				/*------------------------------------------------------------*/
			#ifndef WIN32
				if (ns == EOF || ns < 1)
			#else
				if (ns < 1)
			#endif /* WIN32 */
				{
					NEUIK_RaiseError(funcName, errMsgs[11]);
					continue;
				}
				if (cfg->fontEmWidth == fontEmWidth) continue;

				/* else: The previous setting was changed */
				cfg->fontEmWidth = fontEmWidth;
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
					NEUIK_RaiseError(funcName, errMsgs[13]);
				}
				else
				{
					/* An unsupported name was used as a value type */
					NEUIK_RaiseError(funcName, errMsgs[14]);
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
		if (neuik_Element_GetSizeAndLocation(lbl, &rSize, &rLoc))
		{
			eNum = 10;
			NEUIK_RaiseError(funcName, errMsgs[eNum]);
			eNum = 1;
		}
		else
		{
			neuik_Element_RequestRedraw(lbl, rLoc, rSize);
		}
	}

	return eNum;
}
