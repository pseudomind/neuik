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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "NEUIK_defs.h"
#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_platform.h"
#include "NEUIK_TextEdit.h"
#include "NEUIK_Element_internal.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

#define CURSORPAN_TEXT_INSERTED   0
#define CURSORPAN_TEXT_DELTETED   1
#define CURSORPAN_TEXT_ADD_REMOVE 2
#define CURSORPAN_MOVE_BACK       3
#define CURSORPAN_MOVE_FORWARD    4

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__TextEdit(void ** tePtr);
int neuik_Object_Free__TextEdit(void * tePtr);

int neuik_Element_GetMinSize__TextEdit(NEUIK_Element, RenderSize*);
neuik_EventState neuik_Element_CaptureEvent__TextEdit(
	NEUIK_Element, SDL_Event*);
int neuik_Element_Render__TextEdit(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*, int);
void neuik_Element_Defocus__TextEdit(NEUIK_Element);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_TextEdit_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__TextEdit,

	/* Render(): Redraw the element  element  */
	neuik_Element_Render__TextEdit,

	/* CaptureEvent(): Determine if this element caputures a given event */
	neuik_Element_CaptureEvent__TextEdit,

	/* Defocus(): This function will be called when an element looses focus */
	neuik_Element_Defocus__TextEdit,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_TextEdit_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__TextEdit,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__TextEdit,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_TextEdit
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_TextEdit()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_TextEdit";
	static char  * errMsgs[]  = {"",                    // [0] no error
		"NEUIK library must be initialized first.",     // [1]
		"Failed to register `TextEdit` object class .", // [2]
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
		"NEUIK_TextEdit",             // className
		"An editible GUI text field.", // classDescription
		neuik__Set_NEUIK,              // classSet
		neuik__Class_Element,          // superClass
		&neuik_TextEdit_BaseFuncs,     // baseFuncs
		NULL,                          // classFuncs
		&neuik__Class_TextEdit))       // newClass
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
 *  Name:          neuik_Object_New__TextEdit
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__TextEdit(
	void ** tePtr)
{
	int                sLen       = 1;
	int                eNum       = 0; /* which error to report (if any) */
	NEUIK_Element    * sClassPtr  = NULL;
	NEUIK_TextEdit   * te         = NULL;
	NEUIK_Color        bgClr      = COLOR_WHITE;
	static char        funcName[] = "neuik_Object_New__TextEdit";
	static char      * errMsgs[]  = {"",                       // [0] no error
		"Failure to allocate memory.",                         // [1]
		"Failure in NEUIK_NewTextEditConfig.",                 // [2]
		"Output Argument `tePtr` is NULL.",                    // [3]
		"Failure in function `neuik_Object_New`.",             // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",   // [5]
		"Failure in `neuik_GetObjectBaseOfClass`.",            // [6]
		"Failure in function `neuik_NewTextBlock`.",           // [7]
		"Failure in `NEUIK_Element_SetBackgroundColorSolid`.", // [8]
	};

	if (tePtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	(*tePtr) = (NEUIK_TextEdit*) malloc(sizeof(NEUIK_TextEdit));
	te = *tePtr;
	if (te == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_TextEdit, 
			NULL,
			&(te->objBase)))
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(te->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Element, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(*sClassPtr, &neuik_TextEdit_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_NewTextBlock(&te->textBlk, 0, 0))
	{
		eNum = 7;
		goto out;
	}

	/* allocate for a minimum of 50 char (larger if it starts with more) */
	sLen = 50;
	te->text = (char*)malloc(sLen*sizeof(char));
	if (te->text == NULL) {
		eNum = 1;
		goto out;
	}
	/* Allocation successful */
	te->text[0] = '\0';
	te->textLen = 0;
	te->textAllocSize = sLen;

	/*------------------------------------------------------------------------*/
	/* All allocations successful                                             */
	/*------------------------------------------------------------------------*/
	te->cursorLine         = 0;
	te->cursorPos          = 0;
	te->cursorX            = 0;
	te->selected           = 0;
	te->wasSelected        = 0;
	te->highlightIsSet     = 0;
	te->highlightBeginPos  = 0;
	te->highlightBeginLine = 0;
	te->highlightStartPos  = 0;
	te->highlightStartLine = 0;
	te->highlightEndPos    = 0;
	te->highlightEndLine   = 0;
	te->panX               = 0;
	te->panCursor          = 0;
	te->isActive           = 0;
	te->clickOrigin        = -1;
	te->clickHeld          = 0;
	te->needsRedraw        = 1;
    te->timeLastClick      = 0;
	te->cfg                = NULL; 
	te->cfgPtr             = NULL; 
	te->textSurf           = NULL;
	te->textTex            = NULL;
	te->textRend           = NULL;

	if (NEUIK_NewTextEditConfig(&te->cfg))
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorSolid(te, "normal",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
	{
		eNum = 8;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorSolid(te, "selected",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
	{
		eNum = 8;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorSolid(te, "hovered",
		bgClr.r, bgClr.g, bgClr.b, bgClr.a))
	{
		eNum = 8;
		goto out;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		/* free any dynamically allocated memory */
		if (te != NULL)
		{
			if (te->text != NULL) free(te->text);
			free(te);
		}
		te = NULL;
		eNum = 1;
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewTextEdit
 *
 *  Description:   Wrapper function for the Object_New function.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewTextEdit(
	NEUIK_TextEdit ** tePtr)
{
	return neuik_Object_New__TextEdit((void**)tePtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeTextEdit
 *
 *  Description:   Create a new NEUIK_TextEdit and assign text to it.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeTextEdit(
	NEUIK_TextEdit ** tePtr, /* [out] The newly created NEUIK_TextEdit. */
	const char      * text)  /* [in]  Initial TextEdit text. */
{
	size_t           sLen       = 1;
	int              eNum       = 0; /* which error to report (if any) */
	NEUIK_TextEdit * te         = NULL;
	static char      funcName[] = "NEUIK_MakeTextEdit";
	static char    * errMsgs[]  = {"",                       // [0] no error
		"Failure in function `neuik_Object_New__TextEdit`.", // [1]
		"Failure to allocate memory.",                       // [2]
		"Failure to reallocate memory.",                     // [3]
		"Failure in function `neuik_TextBlock_SetText`.",    // [4]
	};

	if (neuik_Object_New__TextEdit((void**)tePtr))
	{
		eNum = 1;
		goto out;
	}
	te = *tePtr;

	if (text == NULL)
	{
		/* textEntry will contain no text */
		if (te->text != NULL) te->text[0] = '\0';
		goto out;
	}
	else if (text[0] == '\0')
	{
		/* textEntry will contain no text */
		if (te->text != NULL) te->text[0] = '\0';
		goto out;
	}
	else
	{
		te->textLen  = strlen(text);	
		sLen        += te->textLen;	
	}

	/*------------------------------------------------------------------------*/
	/* Make sure the allocated text buffer is large enough to store the text  */
	/*------------------------------------------------------------------------*/
	if (te->text == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Memory not currently allocated; allocate now.                      */
		/*--------------------------------------------------------------------*/
		te->text = (char*)malloc((sLen+10)*sizeof(char));
		if (te->text == NULL)
		{
			eNum = 2;
			goto out;
		}
		te->textAllocSize = sLen+10;
	}
	else if (sLen > te->textAllocSize)
	{
		/*--------------------------------------------------------------------*/
		/* Reallocate memory so that the string can be fit.                   */
		/*--------------------------------------------------------------------*/
		te->text = (char*)realloc(te->text, (sLen+10)*sizeof(char));
		if (te->text == NULL)
		{
			eNum = 3;
			goto out;
		}
		te->textAllocSize = sLen+10;
	}

	if (text != NULL)
	{
		if (neuik_TextBlock_SetText(te->textBlk, text))
		{
			eNum = 4;
			goto out;
		}

		strcpy(te->text, text);
	}
	else
	{
		te->text[0] = '\0';
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
 *  Name:          neuik_Object_Free__TextEdit
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__TextEdit(
	void * tePtr)  /* [out] the button to free */
{
	int              eNum       = 0; /* which error to report (if any) */
	NEUIK_TextEdit * te         = NULL;
	static char      funcName[] = "neuik_Object_Free__TextEdit";
	static char    * errMsgs[]  = {"",              // [0] no error
		"Argument `tePtr` is not of Button class.", // [1]
		"Failure in function `neuik_Object_Free`.", // [2]
		"Argument `tePtr` is NULL.",                // [3]
	};

	if (tePtr == NULL)
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEdit*)tePtr;

	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(te->objBase.superClassObj))
	{
		eNum = 2;
		goto out;
	}
	if (te->text     != NULL) free(te->text);
	if (te->textSurf != NULL) SDL_FreeSurface(te->textSurf);
	if (te->textTex  != NULL) SDL_DestroyTexture(te->textTex);
	if (te->textRend != NULL) SDL_DestroyRenderer(te->textRend);
	#pragma message("TODO: Free TextBlock Data")

	if(neuik_Object_Free(te->cfg))
	{
		eNum = 2;
		goto out;
	}

	free(te);
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
 *  Name:          neuik_Element_GetMinSize__TextEdit
 *
 *  Description:   Returns the rendered size of a given button.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__TextEdit(
	NEUIK_Element    elem,
	RenderSize     * rSize)
{
	int                    tW;
	int                    tH;
	int                    eNum = 0;    /* which error to report (if any) */
	TTF_Font             * font = NULL;
	NEUIK_TextEdit       * te   = NULL;
	NEUIK_TextEditConfig * aCfg = NULL; /* the active button config */
	static char            funcName[] = "neuik_Element_GetMinSize__TextEdit";
	static char          * errMsgs[]  = {"",         // [0] no error
		"Argument `elem` is not of TextEdit class.", // [1]
		"TextEditConfig* is NULL.",                  // [2]
		"TextEditConfig->FontSet is NULL.",          // [3]
		"FontSet_GetFont returned NULL.",            // [4]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	te = (NEUIK_TextEdit *)elem;
	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}
	
	/* select the correct button config to use (pointer or internal) */
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

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

	/* this textEntry does not contain text */
	TTF_SizeText(font, " ", &tW, &tH);

	rSize->w = tW + aCfg->fontEmWidth;
	rSize->h = 2 + (int)(1.5 * (float)TTF_FontHeight(font));
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
 *  Name:          NEUIK_TextEdit_SetText
 *
 *  Description:   Update the text in a NEUIK_TextEdit.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEdit_SetText(
		NEUIK_TextEdit * te,
		const char     * text)
{
	size_t        sLen    = 1;
	size_t        textLen = 0;
	int           eNum    = 0; /* which error to report (if any) */
	RenderSize    rSize;
	RenderLoc     rLoc;
	static char   funcName[] = "NEUIK_TextEdit_SetText";
	static char * errMsgs[] = {"",                          // [0] no error
		"Argument `te` is not of TextEdit class.",          // [1]
		"Failure to allocate memory.",                      // [2]
		"Failure in function `neuik_TextBlock_SetText`.",   // [3]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [4]
	};

	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Conditionally free button text before setting the new contents         */
	/*------------------------------------------------------------------------*/
	if (te->text != NULL) {
		free(te->text);
	}

	/*------------------------------------------------------------------------*/
	/* Set the new TextEdit text contents                                    */
	/*------------------------------------------------------------------------*/
	if (text == NULL){
		/* button will contain no text */
		te->text = NULL;
	}
	else if (text[0] == '\0')
	{
		/* button will contain no text */
		te->text = NULL;
	}
	else
	{
		if (neuik_TextBlock_SetText(te->textBlk, text))
		{
			eNum = 3;
			goto out;
		}

		textLen = strlen(text);
		sLen += textLen;
		te->text = (char*)malloc(sLen*sizeof(char));
		if (te->text == NULL) {
			eNum = 2;
			goto out;
		}
		/* Allocation successful */
		strcpy(te->text, text);
	}

	te->textLen            = textLen;
	te->highlightIsSet     = 0;
	te->highlightBeginPos  = 0;
	te->highlightBeginLine = 0;
	te->highlightStartPos  = 0;
	te->highlightStartLine = 0;
	te->highlightEndPos    = 0;
	te->highlightEndLine   = 0;
	te->clickOrigin        = 0;
	te->clickHeld          = 0;

	if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
	{
		eNum = 4;
		goto out;
	}
	neuik_Element_RequestRedraw(te, rLoc, rSize);
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
 *  Name:          NEUIK_TextEdit_GetText
 *
 *  Description:   Get a pointer to the text in a NEUIK_TextEdit.
 *
 *  Returns:       NULL if there is a problem; otherwise a valid string
 *
 ******************************************************************************/
const char * NEUIK_TextEdit_GetText(
		NEUIK_TextEdit * te)
{
	int            eNum = 0; /* which error to report (if any) */
	const char   * rvPtr      = NULL;
	static char    emptyStr[] = "";
	static char    funcName[] = "NEUIK_TextEdit_GetText";
	static char  * errMsgs[]  = {"",               // [0] no error
		"Argument `te` is not of TextEdit class.", // [1]
	};

	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the new TextEdit text contents                                    */
	/*------------------------------------------------------------------------*/
	if (te->text == NULL){
		/* button will contain no text */
		rvPtr = emptyStr;
	}
	else
	{
		rvPtr = te->text;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		rvPtr = NULL;
	}

	return rvPtr;
}

void neuik_TextEdit_Configure_capture_segv(
	int sig_num)
{
	static char funcName[] = "NEUIK_TextEdit_Configure";
	static char errMsg[] = 
		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

	NEUIK_RaiseError(funcName, errMsg);
	NEUIK_BacktraceErrors();
	exit(1);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_TextEdit_Configure
 *
 *  Description:   Configure a number of properties specific to NEUIK_TextEdit.
 *
 *                 This list of named sets must be terminated by a NULL pointer.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_TextEdit_Configure(
	NEUIK_TextEdit  * te,
	const char       * set0,
	...)
{
	int                    ctr;
	int                    eNum       = 0; /* which error to report (if any) */
	int                    vaStarted  = 0;
	va_list                args;
	char                   buf[4096];
	char                 * strPtr     = NULL;
	char                 * name       = NULL;
	char                 * value      = NULL;
	const char           * set        = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active button config */
	static char            funcName[] = "NEUIK_TextEdit_Configure";
	static char          * errMsgs[]  = {"",       // [0] no error
		"Argument `te` is not of TextEdit class.", // [1]
		"NamedSet.name is NULL, skipping..",        // [2]
		"NamedSet.name is blank, skipping..",       // [3]
		"NamedSet.name type unknown, skipping.",    // [4]
		"`name=value` string is too long.",         // [5]
		"Invalid `name=value` string.",             // [6]
		"HJustify value is invalid.",               // [7]
		"VJustify value is invalid.",               // [7]
	};

	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}
	set = set0;

	/*------------------------------------------------------------------------*/
	/* select the correct entry config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

	va_start(args, set0);
	vaStarted = 1;

	for (ctr = 0;; ctr++)
	{
		name  = NULL;
		value = NULL;

		if (set == NULL) break;

		#ifndef NO_NEUIK_SIGNAL_TRAPPING
			signal(SIGSEGV, neuik_TextEdit_Configure_capture_segv);
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
				/* `name=value` string is missing the '=' char */
				NEUIK_RaiseError(funcName, errMsgs[6]);
				set = va_arg(args, const char *);
				continue;
			}
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

		if (name == NULL)
		{
			NEUIK_RaiseError(funcName, errMsgs[2]);
		}
		else if (name[0] == 0)
		{
			NEUIK_RaiseError(funcName, errMsgs[3]);
		}
		else if (!strcmp("HJustify", name))
		{
			if (!strcmp("left", value))
			{
				aCfg->textHJustify = NEUIK_HJUSTIFY_LEFT;
			}
			else if (!strcmp("center", value))
			{
				aCfg->textHJustify = NEUIK_HJUSTIFY_CENTER;
			}
			else if (!strcmp("right", value))
			{
				aCfg->textHJustify = NEUIK_HJUSTIFY_RIGHT;
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
				aCfg->textVJustify = NEUIK_VJUSTIFY_TOP;
			}
			else if (!strcmp("center", value))
			{
				aCfg->textVJustify = NEUIK_VJUSTIFY_CENTER;
			}
			else if (!strcmp("bottom", value))
			{
				aCfg->textVJustify = NEUIK_VJUSTIFY_BOTTOM;
			}
			else 
			{
				NEUIK_RaiseError(funcName, errMsgs[7]);
			}
		}
		// else if (!strcmp("PadLeft", name))
		// {
		// 	aCfg->PadLeft = atoi(value);
		// }
		// else if (!strcmp("PadRight", name))
		// {
		// 	aCfg->PadRight = atoi(value);
		// }
		// else if (!strcmp("PadTop", name))
		// {
		// 	aCfg->PadTop = atoi(value);
		// }
		// else if (!strcmp("PadBottom", name))
		// {
		// 	aCfg->PadBottom = atoi(value);
		// }
		// else if (!strcmp("PadAll", name))
		// {
		// 	aCfg->PadLeft   = atoi(value);
		// 	aCfg->PadRight  = aCfg->PadLeft;
		// 	aCfg->PadTop    = aCfg->PadLeft;
		// 	aCfg->PadBottom = aCfg->PadLeft;
		// }
		else
		{
			NEUIK_RaiseError(funcName, errMsgs[4]);
		}

		/* before starting */
		set = va_arg(args, const char *);
	}
out:
	if (vaStarted) va_end(args);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__TextEdit
 *
 *  Description:   Renders a single TextEdit as an SDL_Texture*.
 *
 *                 If `*rSize = (0, 0)`; use the native GetSize function to 
 *                 determine the rendered object size. Otherwise use the 
 *                 specified rSize.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__TextEdit(
	NEUIK_Element   elem,
	RenderSize    * rSize, /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod, /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend, /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf, /* the external surface (used for transp. bg) */
	int             mock)  /* If true; calculate sizes/locations but don't draw */
{
	char                   tempChar;          /* a temporary character */
	int                    yPos       = 0;
	int                    textW      = 0;
	int                    textH      = 0;
	int                    textWFull  = 0;
	int                    textHFull  = 0;
	int                    hlWidth    = 0;    /* highlight bg Width */
	int                    eNum       = 0;    /* which error to report (if any) */
	int                    hasText    = 1;
	size_t                 lineLen;
	size_t                 lineCtr;
	size_t                 nLines;
	char                 * lineBytes  = NULL;
	RenderLoc              rl;
	SDL_Rect               rect;
	const NEUIK_Color    * fgClr      = NULL;
	const NEUIK_Color    * bgClr      = NULL;
	const NEUIK_Color    * bClr       = NULL; /* border color */
	SDL_Renderer         * rend       = NULL;
	SDL_Texture          * tTex       = NULL; /* text texture */
	TTF_Font             * font       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	neuik_MaskMap        * maskMap    = NULL;
	NEUIK_TextEdit       * te         = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active textEntry config */
	static char            funcName[] = "neuik_Element_Render__TextEdit";
	static char          * errMsgs[]  = {"",                             // [0] no error
		"Argument `elem` is not of TextEdit class.",                     // [1]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"", // [3]
		"Invalid specified `rSize` (negative values).",                  // [4]
		"Failure in `neuik_MakeMaskMap()`",                              // [5]
		"FontSet_GetFont returned NULL.",                                // [6]
		"", // [7]
		"Failure in function `neuik_TextBlock_GetLineCount`.",           // [8]
		"Failure in function `neuik_TextBlock_GetLine`.",                // [9]
		"Failure in function `neuik_TextBlock_GetLineLength`.",          // [10]
		"Failure in neuik_Element_RedrawBackground().",                  // [11]
	};

	te = (NEUIK_TextEdit *)elem;
	if (!neuik_Object_IsClass(te, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
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
	/* Select the correct entry config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfg;
	if (te->cfgPtr != NULL)
	{
		aCfg = te->cfgPtr;
	}

	/* extract the current fg/bg colors */
	bgClr = &(aCfg->bgColor);
	fgClr = &(aCfg->fgColor);

	/*------------------------------------------------------------------------*/
	/* Get the pointer to the currently active font (if text is present)      */
	/*------------------------------------------------------------------------*/
	/* Determine the full size of the rendered text content */
	font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
		aCfg->fontBold, aCfg->fontItalic);
	if (font == NULL) 
	{
		eNum = 6;
		goto out;
	}

	if (te->textSurf != NULL)
	{
		SDL_FreeSurface(te->textSurf);
		te->textSurf = NULL;
	}
	if (te->textRend != NULL)
	{
		SDL_DestroyRenderer(te->textRend);
		te->textRend = NULL;
	}
	if (te->textTex != NULL)
	{
		SDL_DestroyTexture(te->textTex);
		te->textTex = NULL;
	}

	/*------------------------------------------------------------------------*/
	/* Create a MaskMap an mark off the transparent pixels.                   */
	/*------------------------------------------------------------------------*/
	if (neuik_MakeMaskMap(&maskMap, rSize->w, rSize->h))
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Mark off the rounded sections of the button within the MaskMap.        */
	/*------------------------------------------------------------------------*/
	/* upper border line */
	neuik_MaskMap_MaskLine(maskMap, 
		0,            0, 
		rSize->w - 1, 0); 
	/* left border line */
	neuik_MaskMap_MaskLine(maskMap, 
		0, 0, 
		0, rSize->h - 1); 
	/* right border line */
	neuik_MaskMap_MaskLine(maskMap, 
		rSize->w - 1, 0, 
		rSize->w - 1, rSize->h - 1); 
	/* lower border line */
	neuik_MaskMap_MaskLine(maskMap, 
		0,            rSize->h - 1, 
		rSize->w - 1, rSize->h - 1);

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (neuik_Element_RedrawBackground(te, xSurf, rlMod, maskMap))
	{
		eNum = 11;
		goto out;
	}
	bgClr = &(aCfg->bgColor);

	rl = eBase->eSt.rLoc;

	/*------------------------------------------------------------------------*/
	/* Redraw the contained text and highlighting (if present)                */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
	{
		eNum = 8;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* If there is only one line of text, check to see if there is any text   */
	/* data at all before going through the trouble of drawing text.          */
	/*------------------------------------------------------------------------*/
	if (nLines == 1)
	{
		if (neuik_TextBlock_GetLineLength(te->textBlk, 0, &lineLen))
		{
			eNum = 10;
			goto out;
		}
		if (lineLen == 0)
		{
			hasText = 0;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Only redraw the text section if there is valid text in this TextBlock. */
	/*------------------------------------------------------------------------*/
	if (!hasText) goto draw_border;

	/*------------------------------------------------------------------------*/
	/* There appears to be one or more lines of valid text in the Block.      */
	/* Place the lines one-at-a-time where they should go.                    */
	/*------------------------------------------------------------------------*/
	yPos = 6;
	for (lineCtr = 0; lineCtr < nLines; lineCtr++)
	{
		if (neuik_TextBlock_GetLine(te->textBlk, lineCtr, &lineBytes))
		{
			eNum = 9;
			goto out;
		}
		// printf("[TB.GL] `%s`\n", lineBytes);

		if (lineBytes[0] != '\0')
		{
			/* Determine the full size of the rendered text content */
			TTF_SizeText(font, lineBytes, &textW, &textH);
			textWFull = textW;

			/*----------------------------------------------------------------*/
			/* Create an SDL_Surface for the text within the element          */
			/*----------------------------------------------------------------*/
			textHFull = (int)(1.1*textH);
			te->textSurf = SDL_CreateRGBSurface(
				0, textW+1, textHFull, 32, 0, 0, 0, 0);
			if (te->textSurf == NULL)
			{
				eNum = 8;
				goto out;
			}

			te->textRend = SDL_CreateSoftwareRenderer(te->textSurf);
			if (te->textRend == NULL)
			{
				eNum = 9;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Fill the background with it's color                            */
			/*----------------------------------------------------------------*/
			SDL_SetRenderDrawColor(
				te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
			SDL_RenderClear(te->textRend);

			/*----------------------------------------------------------------*/
			/* Render the Text now, it will be copied on after highlighting.  */
			/*----------------------------------------------------------------*/
			tTex = NEUIK_RenderText(
				lineBytes, font, *fgClr, te->textRend, &textW, &textH);
			if (tTex == NULL)
			{
				eNum = 6;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Check for and fill in highlight text selection background.     */
			/*----------------------------------------------------------------*/
			if (eBase->eSt.hasFocus && te->highlightIsSet)
			{
				if (lineCtr >= te->highlightStartLine &&
					lineCtr <= te->highlightEndLine)
				{
					rect.x = 0;
					rect.y = 0;
					rect.w = textW + 1;
					rect.h = textHFull;

					textW = 0;
					textH = 0;
					/* determine the point of the start of the bgkd highlight */
					if (lineCtr > te->highlightStartLine)
					{
						/*----------------------------------------------------*/
						/* The start of the line will be highlighted.         */
						/*----------------------------------------------------*/
						if (lineCtr < te->highlightEndLine)
						{
							/*------------------------------------------------*/
							/* highlight the entire line.                     */
							/*------------------------------------------------*/
							TTF_SizeText(font, lineBytes, &textW, &textH);
						}
						else if (te->highlightEndPos != 0)
						{
							tempChar = lineBytes[te->highlightEndPos];
							lineBytes[te->highlightEndPos] = '\0';
							TTF_SizeText(font, lineBytes, &textW, &textH);
							lineBytes[te->highlightEndPos] = tempChar;
						}
						else
						{
							/*------------------------------------------------*/
							/* The highlight ends at the start of line.       */
							/*------------------------------------------------*/
							TTF_SizeText(font, " ", &textW, &textH);
						}
					}
					else if (lineCtr == te->highlightStartLine)
					{
						/*----------------------------------------------------*/
						/* The highlighted block starts on this line.         */
						/*----------------------------------------------------*/
						if (te->highlightStartPos != 0)
						{
							tempChar = lineBytes[te->highlightStartPos];
							lineBytes[te->highlightStartPos] = '\0';
							TTF_SizeText(font, lineBytes, &textW, &textH);
							lineBytes[te->highlightStartPos] = tempChar;
						}
						rect.x += textW;

						/*----------------------------------------------------*/
						/* determine the point of the start of the bgkd       */
						/* highlight                                          */
						/*----------------------------------------------------*/
						lineLen = strlen(lineBytes);

						if (te->highlightEndLine > lineCtr)
						{
							TTF_SizeText(font, 
								lineBytes + te->highlightStartPos, 
								&textW, &textH);
						}
						else
						{
							tempChar = lineBytes[te->highlightEndPos];
							lineBytes[te->highlightEndPos] = '\0';
							TTF_SizeText(font, 
								lineBytes + te->highlightStartPos, 
								&textW, &textH);
							lineBytes[te->highlightEndPos] = tempChar;
						}
					}
					hlWidth = textW;
					rect.w = hlWidth;

					bgClr = &(aCfg->bgColorHl);
					SDL_SetRenderDrawColor(
						te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
					SDL_RenderFillRect(te->textRend, &rect);
					bgClr = &(aCfg->bgColor);
				}
			}

			/*----------------------------------------------------------------*/
			/* Copy over the previously rendered text.                        */
			/*----------------------------------------------------------------*/
			rect.x = 0;
			rect.y = 0;
			rect.w = textWFull + 1;
			rect.h = textHFull;

			SDL_RenderCopy(te->textRend, tTex, NULL, &rect);

			/*----------------------------------------------------------------*/
			/* Draw the cursor (if TextEdit is focused)                       */
			/*----------------------------------------------------------------*/
			if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
			{
				/*------------------------------------------------------------*/
				/* Draw the cursor line into the TextEdit element             */
				/*------------------------------------------------------------*/
				SDL_SetRenderDrawColor(
					te->textRend, fgClr->r, fgClr->g, fgClr->b, 255);

				tempChar = lineBytes[te->cursorPos];
				if (tempChar == '\0')
				{
					rect.x = textWFull - 1;
				}
				else
				{
					lineBytes[te->cursorPos] = '\0';
					TTF_SizeText(font, lineBytes, &textW, &textH);
					lineBytes[te->cursorPos] = tempChar;

					/* this will be the position of the cursor */
					rect.x = textW;
				}
				te->cursorX = rect.x;
				SDL_RenderDrawLine(te->textRend, 
					rect.x, rect.y, 
					rect.x, rect.y + rect.h); 
			}

			SDL_RenderPresent(te->textRend);
			te->textTex = SDL_CreateTextureFromSurface(rend, te->textSurf);
			if (te->textTex == NULL)
			{
				eNum = 7;
				goto out;
			}

			rect.x = rl.x + 6;
			rect.y = rl.y + yPos;
			rect.w = textWFull + 1;
			rect.h = textHFull;

			SDL_RenderCopy(rend, te->textTex, NULL, &rect);
			ConditionallyDestroyTexture(&tTex);
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* This is a blank line but the cursor may be present.            */
			/*----------------------------------------------------------------*/
			TTF_SizeText(font, " ", &textW, &textH);
			textHFull = (int)(1.1*textH);

			/*----------------------------------------------------------------*/
			/* Conditionally draw the cursor line into the TextEdit.          */
			/*----------------------------------------------------------------*/
			if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
			{
				/*------------------------------------------------------------*/
				/* Position the cursor at the start of the line.              */
				/*------------------------------------------------------------*/
				rect.x = rl.x + 6;
				rect.y = rl.y + yPos;
				rect.h = textHFull;

				SDL_SetRenderDrawColor(rend, fgClr->r, fgClr->g, fgClr->b, 255);
				SDL_RenderDrawLine(rend, 
					rect.x, rect.y, 
					rect.x, rect.y + rect.h); 
			}
		}

		yPos += textHFull;

		if (lineBytes != NULL)
		{
			free(lineBytes);
			lineBytes = NULL;
		}
		if (te->textTex != NULL)
		{
			SDL_DestroyTexture(te->textTex);
			te->textTex = NULL;
		}
		if (te->textSurf != NULL)
		{
			SDL_FreeSurface(te->textSurf);
			te->textSurf = NULL;
		}
		if (te->textRend != NULL)
		{
			SDL_DestroyRenderer(te->textRend);
			te->textRend = NULL;
		}
	}

draw_border:
	/*------------------------------------------------------------------------*/
	/* Draw the border around the TextEdit.                                   */
	/*------------------------------------------------------------------------*/
	bClr = &(aCfg->borderColor);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

	/* upper border line */
	SDL_RenderDrawLine(rend, 
		rl.x + 1,              rl.y + 1, 
		rl.x + (rSize->w - 2), rl.y + 1);
	/* left border line */
	SDL_RenderDrawLine(rend, 
		rl.x + 1, rl.y + 1, 
		rl.x + 1, rl.y + (rSize->h - 2));
	/* right border line */
	SDL_RenderDrawLine(rend, 
		rl.x + (rSize->w - 2), rl.y + (1), 
		rl.x + (rSize->w - 2), rl.y + (rSize->h - 2)); 

	/* lower border line */
	bClr = &(aCfg->borderColorDark);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
	SDL_RenderDrawLine(rend, 
		rl.x + 2,              rl.y + (rSize->h - 2),
		rl.x + (rSize->w - 3), rl.y + (rSize->h - 2));
out:
	if (!mock) eBase->eSt.doRedraw = 0;

	ConditionallyDestroyTexture(&tTex);
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
 *  Name:          neuik_Element_Defocus__TextEdit
 *
 *  Description:   Defocus the TextEdit element.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Element_Defocus__TextEdit(
	NEUIK_Element el)
{
	NEUIK_TextEdit * te;
	RenderSize       rSize = {0, 0};
	RenderLoc        rLoc  = {0, 0};

	SDL_StopTextInput();
	te = (NEUIK_TextEdit*) el;

	if (neuik_Element_GetSizeAndLocation(te, &rSize, &rLoc))
	{
		return;
	}
	neuik_Element_RequestRedraw(te, rLoc, rSize);
	te->highlightIsSet     = 0;
	te->highlightBeginLine = 0;
	te->highlightBeginPos  = 0;
	te->highlightStartLine = 0;
	te->highlightStartPos  = 0;
	te->highlightEndLine   = 0;
	te->highlightEndPos    = 0;
	te->clickOrigin        = 0;
	te->clickHeld          = 0;
}


