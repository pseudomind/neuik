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
int neuik_Object_New__TextEdit(void ** wPtr);
int neuik_Object_Free__TextEdit(void ** wPtr);

int           neuik_Element_GetMinSize__TextEdit(NEUIK_Element, RenderSize*);
int           neuik_Element_CaptureEvent__TextEdit(NEUIK_Element, SDL_Event*);
SDL_Texture * neuik_Element_Render__TextEdit(NEUIK_Element, RenderSize*, SDL_Renderer*);
void          neuik_Element_Defocus__TextEdit(NEUIK_Element);

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
	static char        funcName[] = "neuik_Object_New__TextEdit";
	static char      * errMsgs[]  = {"",                     // [0] no error
		"Failure to allocate memory.",                       // [1]
		"Failure in NEUIK_NewTextEditConfig.",               // [2]
		"Output Argument `tePtr` is NULL.",                  // [3]
		"Failure in function `neuik_Object_New`.",           // [4]
		"Failure in function `neuik_Element_SetFuncTable`.", // [5]
		"Failure in `neuik_GetObjectBaseOfClass`.",          // [6]
		"Failure in function `neuik_NewTextBlock`.",         // [7]
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
	int              sLen       = 1;
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
	void  ** tePtr)  /* [out] the button to free */
{
	int              eNum       = 0; /* which error to report (if any) */
	NEUIK_TextEdit * te         = NULL;
	static char      funcName[] = "neuik_Object_Free__TextEdit";
	static char    * errMsgs[]  = {"",              // [0] no error
		"Argument `btnPtr` is not of Button class.", // [1]
		"Failure in function `neuik_Object_Free`.",  // [2]
		"Argument `btnPtr` is NULL.",                // [3]
	};

	if (tePtr == NULL)
	{
		eNum = 3;
		goto out;
	}

	if (!neuik_Object_IsClass(*tePtr, neuik__Class_TextEdit))
	{
		eNum = 1;
		goto out;
	}
	te = (NEUIK_TextEdit*)(*tePtr);

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(&(te->objBase.superClassObj)))
	{
		eNum = 2;
		goto out;
	}
	if (te->text     != NULL) free(te->text);
	if (te->textSurf != NULL) SDL_FreeSurface(te->textSurf);
	if (te->textTex  != NULL) SDL_DestroyTexture(te->textTex);
	if (te->textRend != NULL) SDL_DestroyRenderer(te->textRend);

	if(neuik_Object_Free((void**)&(te->cfg)))
	{
		eNum = 2;
		goto out;
	}

	free(te);
	(*tePtr) = NULL;
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
	int             sLen    = 1;
	unsigned long   textLen = 0;
	int             eNum    = 0; /* which error to report (if any) */
	static char     funcName[] = "NEUIK_TextEdit_SetText";
	static char   * errMsgs[] = {"",                      // [0] no error
		"Argument `te` is not of TextEdit class.",        // [1]
		"Failure to allocate memory.",                    // [2]
		"Failure in function `neuik_TextBlock_SetText`.", // [3]
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
	neuik_Element_RequestRedraw((NEUIK_Element)te);
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
 *  Name:          neuik_TextEdit_UpdatePanCursor
 *
 *  Description:   Update the `te->panCursor` and maybe `te->cursorX`.
 *
 *  Returns:       A non-zero integer if there is an error.
 *
 ******************************************************************************/
int neuik_TextEdit_UpdatePanCursor(
	NEUIK_TextEdit  * te,
	int               cursorChange)
{
	int                    eNum       = 0; /* which error to report (if any) */
	int                    textW      = 0;
	int                    textH      = 0;
	int                    normWidth  = 0;
	char                   tempChar;
	TTF_Font             * font       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active textEntry config */
	static char            funcName[] = "neuik_TextEdit_UpdatePanCursor";
	static char          * errMsgs[] = {"",                           // [0] no error
		"Argument `te` is not of TextEdit class.",                    // [1]
		"Argument `te` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"FontSet_GetFont returned NULL.",                              // [3]
	};

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

	/*------------------------------------------------------------------------*/
	/* Check for blank or empty TextEntries; panCursor will always be zero.   */
	/*------------------------------------------------------------------------*/
	if (te->text == NULL)
	{
		te->panCursor = 0;
		printf("case0;\n");
		goto out;
	}
	if (te->text[0] == '\0') 
	{
		te->panCursor = 0;
		printf("case1;\n");
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Select the correct entry config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

	/*------------------------------------------------------------------------*/
	/* Get the pointer to the currently active font (if text is present)      */
	/*------------------------------------------------------------------------*/
	if (te->text != NULL)
	{
		if (te->text[0] != '\0')
		{
			/* Determine the full size of the rendered text content */
			font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
				aCfg->fontBold, aCfg->fontItalic);
			if (font == NULL) 
			{
				eNum = 3;
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Before proceeding, check to see where the cursor is located within the */
	/* currently visible TextEdit field.                                     */
	/*------------------------------------------------------------------------*/
	TTF_SizeText(font, te->text, &textW, &textH);
	textW++;
	normWidth = (eBase->eSt.rSize).w - 12; 
	printf("textW: %d, normWidth %d, `%s`\n", textW, normWidth, te->text);
	if (textW < normWidth) 
	{
		/*--------------------------------------------------------------------*/
		/* The text doesn't completely fill the available space; don't pan.   */
		/*--------------------------------------------------------------------*/
		te->panCursor = 0;
		printf("case2;\n");
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* The text more than fills the available space; possible cursor pan. */
		/*--------------------------------------------------------------------*/
		if (te->cursorPos == te->textLen)
		{
			/* the cursor is at the end of the line of text, pan necessary */
			te->panCursor = textW - normWidth;
		}


		/*--------------------------------------------------------------------*/
		/* Update the cursorX position                                        */
		/*--------------------------------------------------------------------*/
		tempChar = te->text[te->cursorPos];
		if (tempChar != '\0')
		{
			te->text[te->cursorPos] = '\0';
		}
		TTF_SizeText(font, te->text, &(te->cursorX), &textH);
		te->text[te->cursorPos] = tempChar;


		switch (cursorChange)
		{
			case CURSORPAN_MOVE_BACK:
				if (te->cursorX < te->panCursor)
				{
					te->panCursor = te->cursorX;
				}
				printf("case3;\n");
				break;
			case CURSORPAN_MOVE_FORWARD:
				if (te->cursorX > te->panCursor + normWidth)
				{
					te->panCursor = (1 + te->cursorX) - normWidth;
				}
				printf("case4;\n");
				break;
			case CURSORPAN_TEXT_DELTETED:
				if (textW - te->panCursor < normWidth)
				{
					/*--------------------------------------------------------*/
					/* Text deleted; no new text was hidden to the right to   */
					/* show, as a result, reduce panCursor so that TextEdit  */
					/* view is filled with text around cursor.                */
					/*--------------------------------------------------------*/
					te->panCursor = textW - normWidth;
				}
				printf("case5;\n");
				break;
		}
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	printf("UpdatePanCursor: te->panCursor = %d\n", te->panCursor);

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
SDL_Texture * neuik_Element_Render__TextEdit(
	NEUIK_Element    elem,
	RenderSize     * rSize, /* in/out the size the tex occupies when complete */
	SDL_Renderer   * xRend) /* the external renderer to prepare the texture for */
{
	char                   tempChar;          /* a temporary character */
	int                    yPos       = 0;
	int                    textW      = 0;
	int                    textH      = 0;
	int                    textWFull  = 0;
	int                    textHFull  = 0;
	int                    normWidth  = 0;
	int                    hlWidth    = 0;    /* highlight bg Width */
	int                    eNum       = 0;    /* which error to report (if any) */
	int                    hasText    = 1;
	unsigned int           lineLen;
	unsigned int           lineCtr;
	unsigned int           nLines;
	char                 * lineBytes  = NULL;
	SDL_Rect               rect;
	SDL_Rect               srcRect;
	const NEUIK_Color    * fgClr      = NULL;
	const NEUIK_Color    * bgClr      = NULL;
	const NEUIK_Color    * bClr       = NULL; /* border color */
	static SDL_Color       tClr       = COLOR_TRANSP;
	SDL_Surface          * surf       = NULL;
	SDL_Renderer         * rend       = NULL;
	SDL_Texture          * tTex       = NULL; /* text texture */
	TTF_Font             * font       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_TextEdit       * te         = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active textEntry config */
	static char            funcName[] = "neuik_Element_Render__TextEdit";
	static char          * errMsgs[]  = {"",                             // [0] no error
		"Argument `elem` is not of TextEdit class.",                     // [1]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"TextEdit_GetMinSize failed.",                                   // [3]
		"Invalid specified `rSize` (negative values).",                  // [4]
		"Failure in Element_Resize().",                                  // [5]
		"FontSet_GetFont returned NULL.",                                // [6]
		"SDL_CreateTextureFromSurface returned NULL.",                   // [7]
		"Failure in function `neuik_TextBlock_GetLineCount`.",           // [8]
		"Failure in function `neuik_TextBlock_GetLine`.",                // [9]
		"Failure in function `neuik_TextBlock_GetLineLength`.",          // [10]
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

	printf("TextEdit_Render [Line:Pos] : [%u:%u]\n", te->cursorLine, te->cursorPos);
	printf("\tHighlightIsSet : %d\n", te->highlightIsSet);
	printf("\tHighlightBegin : [%u:%u]\n", te->highlightBeginLine, te->highlightBeginPos);
	printf("\tHighlightStart : [%u:%u]\n", te->highlightStartLine, te->highlightStartPos);
	printf("\tHighlightEnd   : [%u:%u]\n", te->highlightEndLine,   te->highlightEndPos);

	/*------------------------------------------------------------------------*/
	/* check to see if the requested draw size of the element has changed     */
	/*------------------------------------------------------------------------*/
	if (eBase->eSt.rSize.w == eBase->eSt.rSizeOld.w  &&
		eBase->eSt.rSize.h == eBase->eSt.rSizeOld.h)
	{
		if (!neuik_Element_NeedsRedraw((NEUIK_Element)te) && eBase->eSt.texture != NULL) 
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
		if (neuik_Element_GetMinSize__TextEdit(te, rSize))
		{
			eNum = 3;
			goto out;
		}
	}
	else if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 4;
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
		if (neuik_Element_Resize((NEUIK_Element)te, *rSize) != 0)
		{
			eNum = 5;
			goto out;
		}
	}
	surf = eBase->eSt.surf;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Select the correct entry config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

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
	/* Fill the background with it's color                                    */
	/*------------------------------------------------------------------------*/
	bgClr = &(aCfg->bgColor);
	SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
	SDL_RenderClear(rend);

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
	if (hasText)
	{
		/*--------------------------------------------------------------------*/
		/* There appears to be one or more lines of valid text in the Block.  */
		/* Place the lines one-at-a-time where they should go.                */
		/*--------------------------------------------------------------------*/
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
				textHFull = 1.1*textH;
				te->textSurf = SDL_CreateRGBSurface(0, textW+1, textHFull, 32, 0, 0, 0, 0);
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
				SDL_SetRenderDrawColor(te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
				SDL_RenderClear(te->textRend);

				/*----------------------------------------------------------------*/
				/* Render the Text now, it will be copied on after highlighting   */
				/*----------------------------------------------------------------*/
				tTex = NEUIK_RenderText(lineBytes, font, *fgClr, te->textRend, &textW, &textH);
				if (tTex == NULL)
				{
					eNum = 6;
					goto out;
				}

				/*----------------------------------------------------------------*/
				/* Check for and fill in highlight text selection background      */
				/*----------------------------------------------------------------*/
				if (eBase->eSt.hasFocus && te->highlightIsSet)
				{
					if (lineCtr >= te->highlightStartLine &&
						lineCtr <= te->highlightEndLine)
					{
						rect.x = 0;
						rect.y = 0;
						rect.w = textW;
						rect.h = textHFull;

						textW = 0;
						textH = 0;
						/* determine the point of the start of the bgkd highlight */
						if (lineCtr > te->highlightStartLine)
						{
							/*------------------------------------------------*/
							/* The start of the line will be highlighted      */
							/*------------------------------------------------*/
							if (lineCtr < te->highlightEndLine)
							{
								/*--------------------------------------------*/
								/* highlight the entire line                  */
								/*--------------------------------------------*/
								TTF_SizeText(font, lineBytes, &textW, &textH);
							}
							else if (te->highlightEndPos != 0)
							{
								tempChar = lineBytes[1 + te->highlightEndPos];
								lineBytes[1 + te->highlightEndPos] = '\0';
								TTF_SizeText(font, lineBytes, &textW, &textH);
								lineBytes[1 + te->highlightEndPos] = tempChar;
							}
							else
							{
								/*--------------------------------------------*/
								/* The highlight ends at the start of line    */
								/*--------------------------------------------*/
								TTF_SizeText(font, " ", &textW, &textH);
							}
						}
						else if (lineCtr == te->highlightStartLine)
						{
							/*------------------------------------------------*/
							/* The highlighted block starts on this line      */
							/*------------------------------------------------*/
							if (te->highlightStartPos != 0)
							{
								tempChar = lineBytes[te->highlightStartPos];
								lineBytes[te->highlightStartPos] = '\0';
								TTF_SizeText(font, lineBytes, &textW, &textH);
								lineBytes[te->highlightStartPos] = tempChar;
							}
							rect.x += textW;

							/*------------------------------------------------*/
							/* determine the point of the start of the bgkd   */
							/* highlight                                      */
							/*------------------------------------------------*/
							lineLen = strlen(lineBytes);

							if (te->highlightEndLine > lineCtr)
							{
								TTF_SizeText(font, 
									lineBytes + te->highlightStartPos, 
									&textW, &textH);
							}
							else
							{
								tempChar = lineBytes[1 + te->highlightEndPos];
								lineBytes[1 + te->highlightEndPos] = '\0';
								TTF_SizeText(font, 
									lineBytes + te->highlightStartPos, 
									&textW, &textH);
								lineBytes[1 + te->highlightEndPos] = tempChar;
							}
						}
						hlWidth = textW;
						rect.w = hlWidth;

						bgClr = &(aCfg->bgColorHl);
						SDL_SetRenderDrawColor(te->textRend, bgClr->r, bgClr->g, bgClr->b, 255);
						SDL_RenderFillRect(te->textRend, &rect);
						bgClr = &(aCfg->bgColor);
					}
				}

				/*------------------------------------------------------------*/
				/* Copy over the previously rendered text.                    */
				/*------------------------------------------------------------*/
				rect.x = 0;
				rect.y = 0;
				rect.w = textWFull;
				rect.h = textHFull;

				SDL_RenderCopy(te->textRend, tTex, NULL, &rect);

				/*------------------------------------------------------------*/
				/* Draw the cursor (if textedit is focused)                   */
				/*------------------------------------------------------------*/
				if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
				{
					/*--------------------------------------------------------*/
					/* Draw the cursor line into the textedit element         */
					/*--------------------------------------------------------*/
					SDL_SetRenderDrawColor(te->textRend, fgClr->r, fgClr->g, fgClr->b, 255);

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

						/* this will be the positin of the cursor */
						rect.x = textW;
					}
					te->cursorX = rect.x;
					SDL_RenderDrawLine(te->textRend, rect.x, rect.y, rect.x, rect.y + rect.h); 
				}

				SDL_RenderPresent(te->textRend);
				te->textTex = SDL_CreateTextureFromSurface(rend, te->textSurf);
				if (te->textTex == NULL)
				{
					eNum = 7;
					goto out;
				}

				rect.x = 6;
				rect.y = yPos;
				rect.w = textWFull;
				rect.h = textHFull;

				// SDL_RenderCopy(rend, te->textTex, &srcRect, &rect);
				SDL_RenderCopy(rend, te->textTex, NULL, &rect);
				ConditionallyDestroyTexture(&tTex);
			}
			else
			{
				/*------------------------------------------------------------*/
				/* This is a blank line but the cursor may be present.        */
				/*------------------------------------------------------------*/
				TTF_SizeText(font, " ", &textW, &textH);
				textHFull = 1.1*textH;

				/*------------------------------------------------------------*/
				/* Condtionally draw the cursor line into the textedit.       */
				/*------------------------------------------------------------*/
				if (eBase->eSt.hasFocus && te->cursorLine == lineCtr)
				{
					/*--------------------------------------------------------*/
					/* Position the cursor at the start of the line.          */
					/*--------------------------------------------------------*/
					rect.x = 6;
					rect.y = yPos;
					rect.h = textHFull;

					SDL_SetRenderDrawColor(rend, fgClr->r, fgClr->g, fgClr->b, 255);
					SDL_RenderDrawLine(rend, rect.x, rect.y, rect.x, rect.y + rect.h); 
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
	}

	/*------------------------------------------------------------------------*/
	/* Draw the border in its unselected way                                  */
	/*------------------------------------------------------------------------*/
	/* Fill in the outermost pixels using a transparent color                 */
	/*------------------------------------------------------------------------*/
	SDL_SetColorKey(surf, SDL_TRUE, 
		SDL_MapRGB(surf->format, tClr.r, tClr.g, tClr.b));
	SDL_SetRenderDrawColor(rend, tClr.r, tClr.g, tClr.b, 255);

	/* upper border line */
	SDL_RenderDrawLine(rend, 0, 0, rSize->w - 1, 0); 
	/* left border line */
	SDL_RenderDrawLine(rend, 0, 0, 0, rSize->h - 1); 
	/* right border line */
	SDL_RenderDrawLine(rend, rSize->w - 1, 0, rSize->w - 1, rSize->h - 1); 
	/* lower border line */
	SDL_RenderDrawLine(rend, 0, rSize->h - 1, rSize->w - 1, rSize->h - 1);

	/*------------------------------------------------------------------------*/
	/* Draw the border around the TextEdit.                                   */
	/*------------------------------------------------------------------------*/
	bClr = &(aCfg->borderColor);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);

	/* upper border line */
	SDL_RenderDrawLine(rend, 1, 1, rSize->w - 2, 1); 
	/* left border line */
	SDL_RenderDrawLine(rend, 1, 1, 1, rSize->h - 2); 
	/* right border line */
	SDL_RenderDrawLine(rend, rSize->w - 2, 1, rSize->w - 2, rSize->h - 2); 

	/* lower border line */
	bClr = &(aCfg->borderColorDark);
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
	SDL_RenderDrawLine(rend, 2, rSize->h - 2, rSize->w - 3, rSize->h - 2);

	/*------------------------------------------------------------------------*/
	/* Copy the text onto the renderer and update it                          */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	eBase->eSt.texture = SDL_CreateTextureFromSurface(xRend, surf);
	if (eBase->eSt.texture == NULL)
	{
		eNum = 7;
		goto out;
	}
	eBase->eSt.doRedraw = 0;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	ConditionallyDestroyTexture(&tTex);

	return eBase->eSt.texture;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__TextEdit
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEdit.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
int neuik_Element_CaptureEvent__TextEdit(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                    evCaputred   = 0;
	int                    doRedraw     = 0;
	int                    textW        = 0;
	int                    textH        = 0;
	int                    charW        = 0;
	int                    doContinue   = 0;
	int                    eNum         = 0; /* which error to report (if any) */
	int                    lastW        = 0; /* position of previous char */
	int                    normWidth    = 0;
	unsigned int           lineLen      = 0;
	unsigned long          inpLen       = 0; /* length of text input */
	unsigned long          newSize      = 0; /* realloated text buf size */
	unsigned long          stopPos;
	unsigned long          hlOffset;         /* highlight offset (for copy) */
	unsigned long          oldCursorPos = 0;
	unsigned long          aPos;
	unsigned long          ctr;
	char                   aChar;
	char                 * clipText     = NULL;
	TTF_Font             * font         = NULL;
	SDL_Rect               rect         = {0, 0, 0 ,0};
	SDL_Keymod             keyMod;
	RenderSize           * rSize;
	SDL_Event            * e;
	SDL_MouseMotionEvent * mouseMotEv;
	SDL_MouseButtonEvent * mouseButEv;
	SDL_TextInputEvent   * textInpEv;
	SDL_KeyboardEvent    * keyEv;
	NEUIK_TextEdit       * te         = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active button config */
	NEUIK_ElementBase    * eBase      = NULL;
	static char            funcName[] = "neuik_Element_Render__TextEdit";
	static char          * errMsgs[]  = {"",                             // [ 0] no error
		"FontSet_GetFont returned NULL.",                                // [ 1]
		"Failed to get text from clipboard.",                            // [ 2]
		"Argument `elem` is not of TextEdit class.",                     // [ 3]
		"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [ 4]
		"Failure in function `neuik_TextBlock_InsertChar`.",             // [ 5]
		"Failure in function `neuik_TextBlock_GetLineLength`.",          // [ 6]
		"Failure in function `neuik_TextBlock_GetLineCount`.",           // [ 7]
		"Failure in function `neuik_TextBlock_DeleteChar`.",             // [ 8]
		"Failure in function `neuik_TextBlock_MergeLines`.",             // [ 9]
		"Failure in function `neuik_TextBlock_DeleteSection`.",          // [10]
	};


	if (!neuik_Object_IsClass(elem, neuik__Class_TextEdit))
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEdit*)elem;
	if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (mouseclick/mousemotion).   */
	/*------------------------------------------------------------------------*/
	e = (SDL_Event*)ev;
	switch (e->type)
	{

	case SDL_MOUSEBUTTONDOWN:
		if (!eBase->eSt.hasFocus)
		{
			/*----------------------------------------------------------------*/
			/* This text entry does not currently have the window focus       */
			/*----------------------------------------------------------------*/
			mouseButEv = (SDL_MouseButtonEvent*)(e);
			if (mouseButEv->y >= eBase->eSt.rLoc.y && mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseButEv->x >= eBase->eSt.rLoc.x && mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* This mouse click originated within this button */
					te->selected    = 1;
					te->wasSelected = 1;
					neuik_Window_TakeFocus(eBase->eSt.window, (NEUIK_Element)te);
					SDL_StartTextInput();
					neuik_Element_RequestRedraw((NEUIK_Element)te);
					evCaputred      = 1;
					goto out;
				}
			}
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* This text entry currently has the window focus                 */
			/*----------------------------------------------------------------*/
			mouseButEv = (SDL_MouseButtonEvent*)(e);
			if (mouseButEv->y >= eBase->eSt.rLoc.y && mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseButEv->x >= eBase->eSt.rLoc.x && mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* This mouse click originated within this textEntry */
					doContinue = 1;
					evCaputred = 1;
				}
			}

			if (!doContinue) goto out;
			doContinue = 0;
			/*----------------------------------------------------------------*/
			/* Otherwise, a subsequent click was within the textEntry element */
			/* For this situation, we want to move the textEdit cursor.       */
			/*----------------------------------------------------------------*/

			/*----------------------------------------------------------------*/
			/* select the correct textEntry config to use (ptr or internal)   */
			/*----------------------------------------------------------------*/
			aCfg = te->cfgPtr;
			if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

			rSize = &(eBase->eSt.rSize);

			/*----------------------------------------------------------------*/
			/* Get the overall location of the current text                   */
			/*----------------------------------------------------------------*/
			if (te->text != NULL)
			{
				if (te->text[0] != '\0')
				{
					doContinue = 1;
					font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
						aCfg->fontBold, aCfg->fontItalic);
					if (font == NULL) 
					{
						eNum = 1;
						goto out;
					}

					normWidth = (eBase->eSt.rSize).w - 12; 
					TTF_SizeText(font, te->text, &textW, &textH);
					rect.w = textW;

					if (textW < normWidth) 
					{
						switch (aCfg->textHJustify)
						{
							case NEUIK_HJUSTIFY_LEFT:
								rect.x = 6;
								break;

							case NEUIK_HJUSTIFY_CENTER:
								rect.x = (int) ((float)(rSize->w - textW)/2.0);
								break;

							case NEUIK_HJUSTIFY_RIGHT:
								rect.x = (int) (rSize->w - textW - 6);
								break;
						}
					}
					else
					{
						rect.x = 6;
					}
				}
			}

			if (!doContinue) goto out;

			keyMod = SDL_GetModState();
			if (!(keyMod & KMOD_SHIFT))
			{
				/* The shift-key is NOT being held down */
				/*------------------------------------------------------------*/
				/* If continuing, this textEntry contains text and so the     */
				/* cursor placement could have been changed.                  */
				/*------------------------------------------------------------*/
				if (SDL_GetTicks() - te->timeLastClick < NEUIK_DOUBLE_CLICK_TIMEOUT)
				{
					if (te->textLen > 0)
					{
						// te->highlightIsSet     = 0;
						// te->highlightBeginPos  = 0;
						// te->highlightBeginLine = 0;

						if (neuik_TextBlock_GetLineLength(te->textBlk,
							te->textBlk->nLines, &lineLen))
						{
							/* ERR: problem reported from textBlock */
							eNum = 6;
							goto out;
						}
						#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` DoubleClick")

						// xxxte->cursorPos      = te->textLen;

						// te->highlightStartPos  = 0;
						// te->highlightStartLine = 0;
						// xxxte->highlightEnd   = te->textLen - 1;
					}
				}
				else if (te->panCursor == 0 && mouseButEv->x <= eBase->eSt.rLoc.x + rect.x)
				{
					/* move the cursor position all the way to the start */
					te->cursorPos      = 0;
					te->highlightIsSet = 0;
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				}
				else if (mouseButEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
				{
					/* move the cursor position all the way to the end */
					te->cursorPos      = te->textLen;
					te->highlightIsSet = 0;
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				}
				else
				{
					/* move the cursor somewhere within the text */
					if (te->textLen > 1)
					{
						oldCursorPos = te->cursorPos;
						for (ctr = 1;;ctr++)
						{
							aChar = te->text[ctr];

							te->text[ctr] = '\0';
							TTF_SizeText(font, te->text, &textW, &textH);
							te->text[ctr] = aChar;

							if (mouseButEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW)
							{
								/* cursor will be before this char */
								te->cursorPos = ctr - 1;
								charW = textW - lastW;
								if (mouseButEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW - charW/3)
								{
									/* cursor will be before this char */
									te->cursorPos = ctr - 1;
								}
								else
								{
									/* cursor will be after char */
									te->cursorPos = ctr;
								}

								/*--------------------------------------------*/
								/* Update the cursor Panning (if necessary)   */
								/*--------------------------------------------*/
								if (oldCursorPos > te->cursorPos)
								{
									neuik_TextEdit_UpdatePanCursor(te, 
										CURSORPAN_MOVE_BACK);
								}
								else
								{
									neuik_TextEdit_UpdatePanCursor(te, 
										CURSORPAN_MOVE_FORWARD);
								}
								break;
							}
							lastW = textW;
							if (aChar == '\0') break;
						}
						te->text[ctr] = aChar;
						te->highlightIsSet = 0;
					}
					else
					{
						TTF_SizeText(font, te->text, &textW, &textH);

						if (mouseButEv->x <= eBase->eSt.rLoc.x + rect.x + textW/2)
						{
							/* cursor will be before this char */
							te->cursorPos = 0;
						}
						else
						{
							/* cursor will be after char */
							te->cursorPos = 1;
						}
						te->highlightIsSet = 0;
					}
				}
				te->clickOrigin   = te->cursorPos;
				te->timeLastClick = SDL_GetTicks();
			}
			else
			{
				/* The shift-key IS being held down */
				/*------------------------------------------------------------*/
				/* If continuing, this textEntry contains text and so the     */
				/* cursor placement could have been changed.                  */
				/*------------------------------------------------------------*/
				if (te->clickOrigin == -1)
				{
					if (te->highlightIsSet)
					{
						te->clickOrigin     = te->cursorPos;
						te->clickOriginLine = te->cursorLine;
					}
					else
					{
						te->clickOrigin     = te->highlightBeginPos;
						te->clickOriginLine = te->highlightBeginLine;
					}
				}
				// te->highlightBegin = te->cursorPos;
				if (te->panCursor == 0 && mouseButEv->x <= eBase->eSt.rLoc.x + rect.x)
				{
					/* move the cursor position all the way to the start */
					te->cursorPos      = 0;
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				}
				else if (mouseButEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
				{
					/* move the cursor position all the way to the end */
					te->cursorPos      = te->textLen;
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				}
				else
				{
					/* move the cursor somewhere within the text */
					if (te->textLen > 1)
					{
						oldCursorPos = te->cursorPos;
						for (ctr = 1;;ctr++)
						{
							aChar = te->text[ctr];

							te->text[ctr] = '\0';
							TTF_SizeText(font, te->text, &textW, &textH);
							te->text[ctr] = aChar;

							if (mouseButEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW)
							{
								/* cursor will be before this char */
								te->cursorPos = ctr - 1;
								charW = textW - lastW;
								if (mouseButEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW - charW/3)
								{
									/* cursor will be before this char */
									te->cursorPos = ctr - 1;
								}
								else
								{
									/* cursor will be after char */
									te->cursorPos = ctr;
								}

								/*--------------------------------------------*/
								/* Update the cursor Panning (if necessary)   */
								/*--------------------------------------------*/
								if (oldCursorPos > te->cursorPos)
								{
									neuik_TextEdit_UpdatePanCursor(te, 
										CURSORPAN_MOVE_BACK);
								}
								else
								{
									neuik_TextEdit_UpdatePanCursor(te, 
										CURSORPAN_MOVE_FORWARD);
								}
								break;
							}
							lastW = textW;
							if (aChar == '\0') break;
						}
						te->text[ctr] = aChar;
					}
					else
					{
						TTF_SizeText(font, te->text, &textW, &textH);

						if (mouseButEv->x <= eBase->eSt.rLoc.x + rect.x + textW/2)
						{
							/* cursor will be before this char */
							te->cursorPos = 0;
						}
						else
						{
							/* cursor will be after char */
							te->cursorPos = 1;
						}
					}
				}

				/* Set text highlight (if applicable) */
				te->highlightBeginPos  = te->clickOrigin;
				te->highlightBeginLine = te->clickOriginLine;

				if (te->cursorLine < te->clickOriginLine)
				{
					te->highlightStartPos  = te->cursorPos;
					te->highlightStartLine = te->cursorLine;
					te->highlightEndPos    = te->clickOrigin - 1;
					te->highlightEndLine   = te->clickOriginLine;
				}
				else if (te->cursorLine > te->clickOriginLine)
				{
					te->highlightStartPos  = te->clickOrigin;
					te->highlightStartLine = te->clickOriginLine;
					te->highlightEndPos    = te->cursorPos - 1;
					te->highlightEndLine   = te->cursorLine;
				}
				else
				{
					if (te->cursorPos < te->clickOrigin)
					{
						te->highlightStartPos  = te->cursorPos;
						te->highlightStartLine = te->cursorLine;
						te->highlightEndPos    = te->clickOrigin - 1;
						te->highlightEndLine   = te->clickOriginLine;
					}
					else
					{
						te->highlightStartPos  = te->clickOrigin;
						te->highlightStartLine = te->clickOriginLine;
						te->highlightEndPos    = te->cursorPos - 1;
						te->highlightEndLine   = te->cursorLine;
					}
				}
			}

			neuik_Element_RequestRedraw((NEUIK_Element)te);
			evCaputred = 1;

			te->clickHeld   = 1;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		if (eBase->eSt.hasFocus)
		{
			/*----------------------------------------------------------------*/
			/* This text entry has the window focus (unset `clickHeld`)       */
			/*----------------------------------------------------------------*/
			te->clickHeld =  0;
			evCaputred    =  1;
		}
		goto out;
		break;

	case SDL_MOUSEMOTION:
		if (eBase->eSt.hasFocus && te->clickHeld)
		{
			/*----------------------------------------------------------------*/
			/* This text entry currently has the window focus and the mouse   */
			/* button is still being held down. **Drag Select**               */
			/*----------------------------------------------------------------*/
			mouseMotEv = (SDL_MouseMotionEvent*)(e);
			if (mouseMotEv->y >= eBase->eSt.rLoc.y && mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseMotEv->x >= eBase->eSt.rLoc.x && mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* This mouse click originated within this button */
					doContinue = 1;
					evCaputred = 1;
				}
			}

			if (!doContinue) goto out;
			doContinue = 0;
			/*----------------------------------------------------------------*/
			/* Otherwise, a subsequent click was within the textEntry element */
			/* For this situation, we want to move the textEdit cursor.       */
			/*----------------------------------------------------------------*/

			/*----------------------------------------------------------------*/
			/* select the correct button config to use (pointer or internal)  */
			/*----------------------------------------------------------------*/
			aCfg = te->cfgPtr;
			if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

			rSize = &(eBase->eSt.rSize);

			/*----------------------------------------------------------------*/
			/* Get the overall location of the current text                   */
			/*----------------------------------------------------------------*/
			if (te->text != NULL)
			{
				if (te->text[0] != '\0')
				{
					doContinue = 1;
					font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
						aCfg->fontBold, aCfg->fontItalic);
					if (font == NULL) 
					{
						eNum = 1;
						goto out;
					}


					normWidth = (eBase->eSt.rSize).w - 12; 
					TTF_SizeText(font, te->text, &textW, &textH);
					rect.w = textW;

					if (textW < normWidth) 
					{
						switch (aCfg->textHJustify)
						{
							case NEUIK_HJUSTIFY_LEFT:
								rect.x = 6;
								break;

							case NEUIK_HJUSTIFY_CENTER:
								rect.x = (int) ((float)(rSize->w - textW)/2.0);
								break;

							case NEUIK_HJUSTIFY_RIGHT:
								rect.x = (int) (rSize->w - textW - 6);
								break;
						}
					}
					else
					{
						rect.x = 6;
					}
				}
			}

			if (!doContinue) goto out;
			/*----------------------------------------------------------------*/
			/* If continuing, this textEntry contains text and so the cursor  */
			/* placement could have been changed.                             */
			/*----------------------------------------------------------------*/
			if (te->panCursor == 0 && mouseMotEv->x <= eBase->eSt.rLoc.x + rect.x)
			{
				/* move the cursor position all the way to the start */
				te->cursorPos      = 0;
				te->highlightIsSet = 0;
			}
			else if (mouseMotEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
			{
				/* move the cursor position all the way to the end */
				te->cursorPos      = te->textLen;
				te->highlightIsSet = 0;
			}
			else
			{
				/* move the cursor somewhere within the text */
				if (te->textLen > 1)
				{
					oldCursorPos = te->cursorPos;

					for (ctr = 1;;ctr++)
					{
						aChar = te->text[ctr];

						te->text[ctr] = '\0';
						TTF_SizeText(font, te->text, &textW, &textH);
						te->text[ctr] = aChar;

						if (mouseMotEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW)
						{
							/* cursor will be before this char */
							te->cursorPos = ctr - 1;
							charW = textW - lastW;
							if (mouseMotEv->x + te->panCursor <= eBase->eSt.rLoc.x + rect.x + textW - charW/3)
							{
								/* cursor will be before this char */
								te->cursorPos = ctr - 1;
							}
							else
							{
								/* cursor will be after char */
								te->cursorPos = ctr;
							}

							/*--------------------------------------------*/
							/* Update the cursor Panning (if necessary)   */
							/*--------------------------------------------*/
							if (oldCursorPos > te->cursorPos)
							{
								neuik_TextEdit_UpdatePanCursor(te, 
									CURSORPAN_MOVE_BACK);
							}
							else
							{
								neuik_TextEdit_UpdatePanCursor(te, 
									CURSORPAN_MOVE_FORWARD);
							}
							break;
						}
						lastW = textW;
						if (aChar == '\0') break;
					}
					te->text[ctr] = aChar;
					te->highlightIsSet = 0;
				}
				else
				{
					TTF_SizeText(font, te->text, &textW, &textH);

					if (mouseMotEv->x <= eBase->eSt.rLoc.x + rect.x + textW/2)
					{
						/* cursor will be before this char */
						te->cursorPos = 0;
					}
					else
					{
						/* cursor will be after char */
						te->cursorPos = 1;
					}
					te->highlightIsSet = 0;
				}
			}

			/* Set text highlight (if applicable) */
			te->highlightBeginPos  = te->clickOrigin;
			te->highlightBeginLine = te->clickOriginLine;

			if (te->cursorLine < te->clickOriginLine)
			{
				te->highlightStartPos  = te->cursorPos;
				te->highlightStartLine = te->cursorLine;
				te->highlightEndPos    = te->clickOrigin - 1;
				te->highlightEndLine   = te->clickOriginLine;
			}
			else if (te->cursorLine > te->clickOriginLine)
			{
				te->highlightStartPos  = te->clickOrigin;
				te->highlightStartLine = te->clickOriginLine;
				te->highlightEndPos    = te->cursorPos - 1;
				te->highlightEndLine   = te->cursorLine;
			}
			else
			{
				if (te->cursorPos < te->clickOrigin)
				{
					te->highlightStartPos  = te->cursorPos;
					te->highlightStartLine = te->cursorLine;
					te->highlightEndPos    = te->clickOrigin - 1;
					te->highlightEndLine   = te->clickOriginLine;
				}
				else
				{
					te->highlightStartPos  = te->clickOrigin;
					te->highlightStartLine = te->clickOriginLine;
					te->highlightEndPos    = te->cursorPos - 1;
					te->highlightEndLine   = te->cursorLine;
				}
			}

			neuik_Element_RequestRedraw((NEUIK_Element)te);
			evCaputred = 1;
		}
		goto out;
		break;

	case SDL_TEXTINPUT:
		if (!eBase->eSt.hasFocus) break;
		textInpEv = (SDL_TextInputEvent*)(e);

		if (te->highlightIsSet)
		{
			/*----------------------------------------------------------------*/
			/* Existing text was highlighted when text input was received.    */
			/* This will result in the highlighted text being replaced.       */
			/*----------------------------------------------------------------*/
			#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` TextInput Overwrite")
			// if (te->highlightStart == 0)
			// {
			// 	/*----------------------------------------------------*/
			// 	/* a block of text will be deleted, (block @ start)   */
			// 	/*----------------------------------------------------*/
			// 	if (te->highlightEnd + 1 != te->textLen)
			// 	{
			// 		/* we are not deleting the entire contents */

			// 		for (ctr = 0;; ctr++)
			// 		{
			// 			aChar = te->text[ctr + te->highlightEnd + 1];
			// 			te->text[ctr] = aChar;

			// 			if (aChar == '\0') break;
			// 		}
			// 		te->textLen = strlen(te->text);
			// 	}
			// 	else
			// 	{
			// 		/* delete entire contents of the string */
			// 		te->textLen = 0;
			// 		te->text[0] = '\0';
			// 	}
			// 	te->cursorPos = 0;
			// }
			// else if (te->highlightEnd + 1 == te->textLen)
			// {
			// 	/*----------------------------------------------------*/
			// 	/* a block of text will be deleted, (block @ end)     */
			// 	/*----------------------------------------------------*/
			// 	te->text[te->highlightStart] = '\0';
			// 	te->textLen   = te->highlightStart;
			// 	te->cursorPos = te->textLen;
			// }
			// else
			// {
			// 	/*----------------------------------------------------*/
			// 	/* a block of text will be deleted, (block in middle) */
			// 	/*----------------------------------------------------*/
			// 	te->cursorPos = te->highlightStart;

			// 	hlOffset = 1 + (te->highlightEnd - te->highlightStart);
			// 	for (ctr = te->highlightStart;; ctr++)
			// 	{
			// 		aChar = te->text[ctr + hlOffset];
			// 		te->text[ctr] = aChar;

			// 		if (aChar == '\0') break;
			// 	}
			// 	te->textLen = strlen(te->text);
			// }

			// te->cursorPos = te->highlightStart;
			// te->highlightBegin = -1;
		}

		inpLen = strlen(textInpEv->text);
		if (strlen(textInpEv->text) == 1)
		{
			if (neuik_TextBlock_InsertChar(te->textBlk, 
				te->cursorLine, te->cursorPos, textInpEv->text[0]))
			{
				eNum = 5;
				goto out;
			}
		}
		else
		{
			fprintf(stderr, "[TODO] neuik_Element_CaptureEvent__TextEdit: add chars for inpLen > 1\n");
		}
		te->textLen   += inpLen;
		te->cursorPos += inpLen;

		neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_INSERTED);
		neuik_Element_RequestRedraw((NEUIK_Element)te);
		evCaputred = 1;
		goto out;

	case SDL_KEYDOWN:
		if (!eBase->eSt.hasFocus) break;
		doRedraw = 0;

		keyEv  = (SDL_KeyboardEvent*)(e);
		keyMod = SDL_GetModState();
		switch (keyEv->keysym.sym)
		{
			case SDLK_LEFT:
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					if (te->highlightIsSet)
					{
						/* breaking out of a highlight selection */
						te->highlightIsSet = 0;
						te->cursorLine     = te->highlightStartLine;
						te->cursorPos      = te->highlightStartPos;
						doRedraw = 1;
					}
					else if (te->cursorPos > 0)
					{
						te->cursorPos--;
						doRedraw = 1;
					}
					else if (te->cursorPos == 0 && te->cursorLine > 0)
					{
						/*----------------------------------------------------*/
						/* For lines beyond the first line, attempting to     */
						/* left should cause the cursor to move to the end of */
						/* the preceding line.                                */
						/*----------------------------------------------------*/
						te->cursorLine--;
						if (neuik_TextBlock_GetLineLength(te->textBlk,
							te->cursorLine, &lineLen))
						{
							/* ERR: problem reported from textBlock */
							eNum = 6;
							goto out;
						}
						te->cursorPos = lineLen;
						doRedraw = 1;
					}
					te->clickOrigin = -1;
				}
				else
				{
					/* SHIFT key is being held down */

					/* Start highlight selection process */
					if (te->cursorPos > 0)
					{
						doRedraw = 1;

						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}
						te->cursorPos--;

						if (te->cursorLine < te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos < te->highlightBeginPos))
						{
							/* highlight is expanding to the left */
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
						else
						{
							/* highlight is contracting to the left */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
					}
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				break;

			case SDLK_RIGHT:
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					if (te->highlightIsSet)
					{
						/* breaking out of a highlight selection */
						te->highlightIsSet = 0;
						te->cursorLine     = te->highlightEndLine;
						te->cursorPos      = te->highlightEndPos + 1;
						doRedraw = 1;
					}
					else
					{
						/*----------------------------------------------------*/
						/* Prevent the cursor from moving to a position in    */
						/* excess of the line length.                         */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_GetLineLength(te->textBlk,
							te->cursorLine, &lineLen))
						{
							/* ERR: problem reported from textBlock */
							eNum = 6;
							goto out;
						}
						if (te->cursorPos < lineLen)
						{
							te->cursorPos++;
							doRedraw = 1;
						}
						else if (te->cursorPos == lineLen && 
								te->cursorLine < te->textBlk->nLines)
						{
							/*------------------------------------------------*/
							/* For lines before the final line, attempting to */
							/* right should cause the cursor to move to the   */
							/* start of the following line.                   */
							/*------------------------------------------------*/
							te->cursorLine++;
							te->cursorPos = 0;
							doRedraw = 1;
						}
					}
					te->clickOrigin = -1;
				}
				else
				{
					/* SHIFT key is being held down */

					/* Start highlight selection process */
					/*--------------------------------------------------------*/
					/* Prevent the cursor from moving to a position in excess */
					/* of the line length.                                    */
					/*--------------------------------------------------------*/
					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}
					if (te->cursorPos < lineLen)
					{
						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}
						te->cursorPos++;
						doRedraw = 1;

						if (te->cursorLine > te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos > te->highlightBeginPos))
						{
							/* highlight is expanding to the right */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
						else
						{
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
					}
					else if (te->cursorPos == lineLen && 
							te->cursorLine < te->textBlk->nLines)
					{
						/*------------------------------------------------*/
						/* For lines before the final line, attempting to */
						/* right should cause the cursor to move to the   */
						/* start of the following line.                   */
						/*------------------------------------------------*/
						if (!te->highlightIsSet)
						{
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}

						te->cursorLine++;
						te->cursorPos = 0;
						doRedraw = 1;

						if (te->cursorLine > te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos > te->highlightBeginPos))
						{
							/* highlight is expanding to the right */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
						else
						{
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
					}
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				break;

			case SDLK_BACKSPACE:
				if (!te->highlightIsSet)
				{
					/*--------------------------------------------------------*/
					/* There is no current text highlighting                  */
					/*--------------------------------------------------------*/
					if (te->cursorPos > 0)
					{
						if (neuik_TextBlock_DeleteChar(te->textBlk, 
							te->cursorLine, (te->cursorPos-1)))
						{
							eNum = 8;
							goto out;
						}
						te->cursorPos -= 1;
						doRedraw = 1;
					}
					else if (te->cursorLine > 0 && te->cursorPos == 0)
					{
						/*----------------------------------------------------*/
						/* The cursor is in the first position of a line that */
						/* is not the first line. A backspace here will       */
						/* combine the current line to the preceding line.    */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_GetLineLength(te->textBlk,
							(te->cursorLine - 1), &lineLen))
						{
							/* ERR: problem reported from textBlock */
							eNum = 6;
							goto out;
						}
						if (neuik_TextBlock_MergeLines(te->textBlk,
							(te->cursorLine - 1)))
						{
							/* ERR: problem reported from textBlock */
							eNum = 9;
							goto out;
						}
						te->cursorLine--;
						te->cursorPos = lineLen;
						doRedraw = 1;
					}
				}
				else
				{
					/*--------------------------------------------------------*/
					/* There is text highlighting within the line             */
					/*--------------------------------------------------------*/
					if (neuik_TextBlock_DeleteSection(te->textBlk,
						te->highlightStartLine, te->highlightStartPos, 
						te->highlightEndLine, te->highlightEndPos))
					{
						eNum = 10;
						goto out;
					}
					te->cursorLine     = te->highlightStartLine;
					te->cursorPos      = te->highlightStartPos;
					te->highlightIsSet = 0;
					doRedraw           = 1;
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
				break;

			case SDLK_DELETE:
				if (!te->highlightIsSet)
				{
					/*--------------------------------------------------------*/
					/* There is no current text highlighting                  */
					/*--------------------------------------------------------*/

					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}
					if (te->cursorPos < lineLen - 1)
					{
						/*----------------------------------------------------*/
						/* Prevent the deletion of the final terminating NULL */
						/* character.                                         */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_DeleteChar(te->textBlk,
							te->cursorLine, te->cursorPos))
						{
							eNum = 8;
							goto out;
						}
						doRedraw = 1;
					}
					else if (
						(te->cursorPos == lineLen) &&
						(te->textBlk->nLines > te->cursorLine))
					{
						/*----------------------------------------------------*/
						/* The cursor is in the final position of a line that */
						/* is not the final line. A delete here will combine  */
						/* the current line to the follwoing line.            */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_MergeLines(te->textBlk, te->cursorLine))
						{
							/* ERR: problem reported from textBlock */
							eNum = 9;
							goto out;
						}
						doRedraw = 1;
					}
				}
				else
				{
					/*--------------------------------------------------------*/
					/* There is text highlighting within the line             */
					/*--------------------------------------------------------*/
					if (neuik_TextBlock_DeleteSection(te->textBlk,
						te->highlightStartLine, te->highlightStartPos, 
						te->highlightEndLine, te->highlightEndPos))
					{
						eNum = 10;
						goto out;
					}
					te->cursorLine     = te->highlightStartLine;
					te->cursorPos      = te->highlightStartPos;
					te->highlightIsSet = 0;
					doRedraw           = 1;
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
				break;

			case SDLK_UP:
				/* Move the cursor to the start of the line of text */
				if (te->cursorPos > 0)
				{
					if (!(keyMod & KMOD_SHIFT))
					{
						/* SHIFT key is not being held down */
						te->highlightIsSet = 0;
						te->clickOrigin    = -1;
						if (te->cursorLine > 0)
						{
							te->cursorLine--;
							doRedraw = 1;
						}
					}
					else
					{
						/* SHIFT key IS being held down */
						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}

						if (te->cursorLine > 0)
						{
							te->cursorLine--;
						}
						else
						{
							te->cursorPos = 0;
						}
						doRedraw = 1;

						if (te->cursorLine < te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos < te->highlightBeginPos))
						{
							/* highlight is expanding to the left */
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
						else
						{
							/* highlight is contracting to the left */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
					}
				}
				// neuik_TextEdit_UpdateCursorX(te);
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				break;

			case SDLK_DOWN:
				/* Move the cursor to the end of the line of text */
				if (te->cursorPos < te->textLen)
				{
					if (!(keyMod & KMOD_SHIFT))
					{
						/* SHIFT key is not being held down */
						te->highlightIsSet = 0;
						te->clickOrigin    = -1;

						/*----------------------------------------------------*/
						/* Prevent the cursor from moving to a line that is   */
						/* in excess of the number of lines.                  */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_GetLineCount(te->textBlk, &lineLen))
						{
							eNum = 7;
							goto out;
						}
						if (te->cursorLine < lineLen)
						{
							te->cursorLine++;
							doRedraw = 1;

							/*------------------------------------------------*/
							/* Prevent the cursor from moving to a position   */
							/* in excess of the line length.                  */
							/*------------------------------------------------*/
							if (neuik_TextBlock_GetLineLength(te->textBlk,
								te->cursorLine, &lineLen))
							{
								eNum = 6;
								goto out;
							}
							if (te->cursorPos > lineLen)
							{
								te->cursorPos = lineLen - 1;
							}
						}
					}
					else
					{
						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}

						/*----------------------------------------------------*/
						/* Prevent the cursor from moving to a line that is   */
						/* in excess of the number of lines.                  */
						/*----------------------------------------------------*/
						if (neuik_TextBlock_GetLineCount(te->textBlk, &lineLen))
						{
							eNum = 7;
							goto out;
						}
						if (te->cursorLine < lineLen)
						{
							te->cursorLine++;
							doRedraw = 1;

							/*------------------------------------------------*/
							/* Prevent the cursor from moving to a position   */
							/* in excess of the line length.                  */
							/*------------------------------------------------*/
							if (neuik_TextBlock_GetLineLength(te->textBlk,
								te->cursorLine, &lineLen))
							{
								eNum = 6;
								goto out;
							}
							if (te->cursorPos > lineLen)
							{
								te->cursorPos = lineLen - 1;
							}
						}

						if (te->cursorLine > te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos > te->highlightBeginPos))
						{
							/* highlight is expanding to the right */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
						else
						{
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}

					}
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				// neuik_TextEdit_UpdateCursorX(te);
				break;

			case SDLK_RETURN:
				/* Insert a line break */
				if (te->cursorPos == te->textLen)
				{
					/* cursor is at the end of the current text */
					if (neuik_TextBlock_InsertChar(te->textBlk, 
						te->cursorLine, te->cursorPos, '\n'))
					{
						eNum = 5;
						goto out;
					}
				}
				else if (te->cursorPos == 0)
				{
					/* cursor is at the start of the current text */
					if (neuik_TextBlock_InsertChar(te->textBlk, 
						te->cursorLine, te->cursorPos, '\n'))
					{
						eNum = 5;
						goto out;
					}
				}
				else
				{
					/* cursor is somewhere in the middle of the text */
					if (neuik_TextBlock_InsertChar(te->textBlk, 
						te->cursorLine, te->cursorPos, '\n'))
					{
						eNum = 5;
						goto out;
					}
				}
				te->textLen    += 1;
				te->cursorLine += 1;
				te->cursorPos   = 0;

				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_INSERTED);
				neuik_Element_RequestRedraw((NEUIK_Element)te);
				evCaputred = 1;
				goto out;

			case SDLK_HOME:
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					if (te->highlightIsSet)
					{
						/* breaking out of a highlight selection */
						te->cursorPos      = 0;
						te->highlightIsSet = 0;
					}
					else if (te->cursorPos > 0)
					{
						te->cursorPos = 0;
						doRedraw      = 1;
					}
					te->clickOrigin = -1;
				}
				else
				{
					/* SHIFT key is being held down */

					/* Start highlight selection process */
					if (te->cursorPos > 0)
					{
						doRedraw = 1;

						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}
						te->cursorPos = 0;

						// if (te->cursorPos < te->highlightBegin)
						// {
						// 	te->highlightStart = te->cursorPos;
						// 	te->highlightEnd   = te->highlightBegin - 1;
						// }
						// else
						// {
						// 	te->highlightStart = te->highlightBegin;
						// 	te->highlightEnd   = te->cursorPos - 1;
						// }
						if (te->cursorLine < te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos < te->highlightBeginPos))
						{
							/* highlight is expanding to the left */
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
						else
						{
							/* highlight is contracting to the left */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos;
						}
					}
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				break;

			case SDLK_END:

				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}

				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					if (te->highlightIsSet)
					{
						/* breaking out of a highlight selection */
						te->cursorPos      = lineLen;
						te->highlightIsSet = 0;
					}
					else if (te->cursorPos != lineLen)
					{
						te->cursorPos = lineLen;
						doRedraw = 1;
					}
					te->clickOrigin = -1;
				}
				else
				{
					/* SHIFT key is being held down */

					/* Start highlight selection process */
					if (te->cursorPos < te->textLen)
					{
						if (!te->highlightIsSet)
						{
							/* highlight was not previously set */
							te->highlightIsSet = 1;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}

						te->cursorPos = lineLen;
						doRedraw      = 1;

						if (te->cursorLine > te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos > te->highlightBeginPos))
						{
							/* highlight is expanding to the right */
							te->highlightStartLine = te->highlightBeginLine;
							te->highlightStartPos  = te->highlightBeginPos;
							te->highlightEndLine   = te->cursorLine;
							te->highlightEndPos    = te->cursorPos - 1;
						}
						else
						{
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos - 1;
						}
					}
				}
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				break;
		}

		if (neuik_KeyShortcut_Copy(keyEv, keyMod))
		{
			if (te->highlightIsSet)
			{
				#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` Copy")
				// aChar = te->text[te->highlightEnd + 1];
				// te->text[te->highlightEnd + 1] = '\0';

				// SDL_SetClipboardText(te->text + te->highlightStart);
				// te->text[te->highlightEnd + 1] = aChar;
			}
		}
		else if (neuik_KeyShortcut_Cut(keyEv, keyMod))
		{
			if (te->highlightIsSet)
			{
				#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` Cut")
				// aChar = te->text[te->highlightEnd + 1];
				// te->text[te->highlightEnd + 1] = '\0';

				// SDL_SetClipboardText(te->text + te->highlightStart);
				// te->text[te->highlightEnd + 1] = aChar;

				// /*--------------------------------------------------------*/
				// /* There is text highlighting within the line             */
				// /*--------------------------------------------------------*/
				// if (te->highlightStart == 0)
				// {
				// 	/*----------------------------------------------------*/
				// 	/* a block of text will be deleted, (block @ start)   */
				// 	/*----------------------------------------------------*/
				// 	if (te->highlightEnd + 1 != te->textLen)
				// 	{
				// 		/* we are not deleting the entire contents */

				// 		for (ctr = 0;; ctr++)
				// 		{
				// 			aChar = te->text[ctr + te->highlightEnd + 1];
				// 			te->text[ctr] = aChar;

				// 			if (aChar == '\0') break;
				// 		}
				// 		te->textLen = strlen(te->text);
				// 	}
				// 	else
				// 	{
				// 		/* delete entire contents of the string */
				// 		te->textLen = 0;
				// 		te->text[0] = '\0';
				// 	}
				// 	te->cursorPos      = 0;
				// }
				// else if (te->highlightEnd + 1 == te->textLen)
				// {
				// 	/*----------------------------------------------------*/
				// 	/* a block of text will be deleted, (block @ end)     */
				// 	/*----------------------------------------------------*/
				// 	te->text[te->highlightStart] = '\0';
				// 	te->textLen   = te->highlightStart;
				// 	te->cursorPos = te->textLen;
				// }
				// else
				// {
				// 	/*----------------------------------------------------*/
				// 	/* a block of text will be deleted, (block in middle) */
				// 	/*----------------------------------------------------*/
				// 	te->cursorPos = te->highlightStart;

				// 	hlOffset = 1 + (te->highlightEnd - te->highlightStart);
				// 	for (ctr = te->highlightStart;; ctr++)
				// 	{
				// 		aChar = te->text[ctr + hlOffset];
				// 		te->text[ctr] = aChar;

				// 		if (aChar == '\0') break;
				// 	}
				// 	te->textLen = strlen(te->text);
				// }

				// te->highlightBegin = -1;
				// doRedraw           = 1;
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
			}
		}
		else if (neuik_KeyShortcut_Paste(keyEv, keyMod) && SDL_HasClipboardText())
		{
			#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` Paste")
			// if (te->highlightIsSet)
			// {
			// 	/*--------------------------------------------------------*/
			// 	/* There is text highlighting within the line             */
			// 	/*--------------------------------------------------------*/
			// 	if (te->highlightStart == 0)
			// 	{
			// 		/*----------------------------------------------------*/
			// 		/* a block of text will be deleted, (block @ start)   */
			// 		/*----------------------------------------------------*/
			// 		if (te->highlightEnd + 1 != te->textLen)
			// 		{
			// 			/* we are not deleting the entire contents */

			// 			for (ctr = 0;; ctr++)
			// 			{
			// 				aChar = te->text[ctr + te->highlightEnd + 1];
			// 				te->text[ctr] = aChar;

			// 				if (aChar == '\0') break;
			// 			}
			// 			te->textLen = strlen(te->text);
			// 		}
			// 		else
			// 		{
			// 			/* delete entire contents of the string */
			// 			te->textLen = 0;
			// 			te->text[0] = '\0';
			// 		}
			// 		te->cursorPos      = 0;
			// 	}
			// 	else if (te->highlightEnd + 1 == te->textLen)
			// 	{
			// 		/*----------------------------------------------------*/
			// 		/* a block of text will be deleted, (block @ end)     */
			// 		/*----------------------------------------------------*/
			// 		te->text[te->highlightStart] = '\0';
			// 		te->textLen   = te->highlightStart;
			// 		te->cursorPos = te->textLen;
			// 	}
			// 	else
			// 	{
			// 		/*----------------------------------------------------*/
			// 		/* a block of text will be deleted, (block in middle) */
			// 		/*----------------------------------------------------*/
			// 		te->cursorPos = te->highlightStart;

			// 		hlOffset = 1 + (te->highlightEnd - te->highlightStart);
			// 		for (ctr = te->highlightStart;; ctr++)
			// 		{
			// 			aChar = te->text[ctr + hlOffset];
			// 			te->text[ctr] = aChar;

			// 			if (aChar == '\0') break;
			// 		}
			// 		te->textLen = strlen(te->text);
			// 	}

			// 	te->highlightBegin = -1;
			// }

			// clipText = SDL_GetClipboardText();
			// if (clipText == NULL)
			// {
			// 	evCaputred = 1;
			// 	eNum = 2;
			// 	goto out;
			// }

			// inpLen = strlen(clipText);
			// if (te->cursorPos == te->textLen)
			// {
			// 	/* cursor is at the end of the current text */
			// 	if (inpLen + te->textLen < te->textAllocSize)
			// 	{
			// 		/* the text buffer will need to be resized to fit this text */
			// 		newSize = 2 * (inpLen + te->textLen);
			// 		te->text = (char*)realloc(te->text, newSize*sizeof(char));
			// 	}
			// 	strcat(te->text, clipText);
			// 	te->textLen   += inpLen;
			// 	te->cursorPos += inpLen;
			// }
			// else if (te->cursorPos == 0)
			// {
			// 	/* cursor is at the start of the current text */
			// 	if (inpLen + te->textLen < te->textAllocSize)
			// 	{
			// 		/* the text buffer will need to be resized to fit this text */
			// 		newSize = 2 * (inpLen + te->textLen);
			// 		te->text = (char*)realloc(te->text, newSize*sizeof(char));
			// 	}

			// 	/* first move over the old text */
			// 	for (ctr = te->textLen + inpLen; ctr >= inpLen; ctr--)
			// 	{
			// 		te->text[ctr] = te->text[ctr - inpLen];
			// 	}

			// 	/* now copy in the new text */
			// 	for (ctr = 0;; ctr++)
			// 	{
			// 		if (clipText[ctr] == 0) break;

			// 		te->text[ctr] = clipText[ctr];
			// 	}
			// 	te->textLen   += inpLen;
			// 	te->cursorPos += inpLen;
			// }
			// else
			// {
			// 	/* cursor is somewhere in the middle of the text */
			// 	if (inpLen + te->textLen < te->textAllocSize)
			// 	{
			// 		/* the text buffer will need to be resized to fit this text */
			// 		newSize = 2 * (inpLen + te->textLen);
			// 		te->text = (char*)realloc(te->text, newSize*sizeof(char));
			// 	}

			// 	/* first move over the old text */
			// 	stopPos = (te->cursorPos - 1) + inpLen; 
			// 	for (ctr = te->textLen + inpLen; ctr >= stopPos; ctr--)
			// 	{
			// 		te->text[ctr] = te->text[ctr - inpLen];
			// 	}

			// 	/* now copy in the new text */
			// 	for (ctr = 0;; ctr++)
			// 	{
			// 		aPos = te->cursorPos + ctr;
			// 		if (clipText[ctr] == 0) break;

			// 		te->text[aPos] = clipText[ctr];
			// 	}
			// 	te->textLen   += inpLen;
			// 	te->cursorPos += inpLen;
			// }

			// // neuik_TextEdit_UpdateCursorX(te);
			// neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_ADD_REMOVE);
			// neuik_Element_RequestRedraw((NEUIK_Element)te);
			// evCaputred = 1;
			// goto out;
		}
		else if (neuik_KeyShortcut_SelectAll(keyEv, keyMod))
		{
			#pragma message("[TODO] `neuik_Element_CaptureEvent__TextEdit` SelectAll")
			// if (te->textLen > 0)
			// {
			// 	te->highlightBegin = 0;
			// 	te->cursorPos      = te->textLen;
			// 	// neuik_TextEdit_UpdateCursorX(te);
			// 	neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);

			// 	te->highlightStart = 0;
			// 	te->highlightEnd   = te->textLen - 1;
			// 	doRedraw = 1;
			// }
		}


		if (doRedraw) neuik_Element_RequestRedraw((NEUIK_Element)te);
		evCaputred = 1;
		goto out;

		break;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (clipText != NULL) free(clipText);

	return evCaputred;
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
		NEUIK_Element    el)
{
	NEUIK_TextEdit * te;

	SDL_StopTextInput();
	neuik_Element_RequestRedraw((NEUIK_Element)el);
	te = (NEUIK_TextEdit*) el;
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


