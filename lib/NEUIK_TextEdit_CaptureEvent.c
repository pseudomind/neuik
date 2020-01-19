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
#include <stdio.h>
#include <string.h>

#include "NEUIK_error.h"
#include "NEUIK_TextEdit.h"
#include "NEUIK_TextEdit_internal.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_platform.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

typedef enum {
	E_CHARTYPE_ALPHA_NUMERIC, // char in the following: [a-z], [A-Z], [0-9], '_'
	E_CHARTYPE_WHITESPACE,    // char in the following: ' ', '\t', '\n', '\r'
	E_CHARTYPE_OTHER,         // any character not captured in the two 
	                          // aforementioned groups.
	E_CHARTYPE_OTHER_PLUS,    // Identifies `OTHER` chars and `WHITESPACE` chars
	E_CHARTYPE_UNDEFINED,     // Indicates a failure to initialize a value
} neuik_E_CharType;

static char          * errMsgs[]  = {"", // [ 0] no error
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
	"Failure in function `neuik_TextBlock_GetLine`.",                // [11]
	"Failure in function `neuik_TextBlock_GetSection`.",             // [12]
	"Failure in function `neuik_TextBlock_InsertText`.",             // [13]
	"Failure in function `neuik_getTextSelectionAtPos`.",            // [14]
};


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
	int                    lineLen    = 0;
	size_t                 textLen    = 0;
	char                   tempChar;
	char                 * lineBytes  = NULL;
	TTF_Font             * font       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active textEntry config */
	static char            funcName[] = "neuik_TextEdit_UpdatePanCursor";
	static char          * errMsgs2[] = {"", // [0] no error
		"Argument `te` is not of TextEdit class.",                     // [1]
		"Argument `te` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Failure in function `neuik_TextBlock_GetLength`.",            // [3]
		"FontSet_GetFont returned NULL.",                              // [4]
		"Failure in function `neuik_TextBlock_GetLine`.",              // [5]
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
	if (neuik_TextBlock_GetLength(te->textBlk, &textLen))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check for blank or empty TextBlock; panCursor will always be zero.     */
	/*------------------------------------------------------------------------*/
	if (textLen == 0)
	{
		te->panCursor = 0;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Select the correct entry config to use (pointer or internal)           */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fall back to internal config */

	/*------------------------------------------------------------------------*/
	/* Get the pointer to the currently active font                           */
	/*------------------------------------------------------------------------*/
	/* Determine the full size of the rendered text content */
	font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
		aCfg->fontBold, aCfg->fontItalic);
	if (font == NULL) 
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Before proceeding, check to see where the cursor is located within the */
	/* currently visible TextEdit field.                                     */
	/*------------------------------------------------------------------------*/
	if (neuik_TextBlock_GetLine(te->textBlk, te->cursorLine, &lineBytes))
	{
		eNum = 5;
		goto out;
	}
	lineLen = strlen(lineBytes);

	TTF_SizeText(font, lineBytes, &textW, &textH);
	textW++;
	normWidth = (eBase->eSt.rSize).w - 12; 
	if (textW < normWidth) 
	{
		/*--------------------------------------------------------------------*/
		/* The text doesn't completely fill the available space; don't pan.   */
		/*--------------------------------------------------------------------*/
		te->panCursor = 0;
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* The text more than fills the available space; possible cursor pan. */
		/*--------------------------------------------------------------------*/
		if (te->cursorPos == lineLen)
		{
			/* the cursor is at the end of the line of text, pan necessary */
			te->panCursor = textW - normWidth;
		}


		/*--------------------------------------------------------------------*/
		/* Update the cursorX position                                        */
		/*--------------------------------------------------------------------*/
		tempChar = lineBytes[te->cursorPos];
		if (tempChar != '\0')
		{
			lineBytes[te->cursorPos] = '\0';
		}
		TTF_SizeText(font, lineBytes, &(te->cursorX), &textH);
		lineBytes[te->cursorPos] = tempChar;


		switch (cursorChange)
		{
			case CURSORPAN_MOVE_BACK:
				if (te->cursorX < te->panCursor)
				{
					te->panCursor = te->cursorX;
				}
				break;
			case CURSORPAN_MOVE_FORWARD:
				if (te->cursorX > te->panCursor + normWidth)
				{
					te->panCursor = (1 + te->cursorX) - normWidth;
				}
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
				break;
		}
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs2[eNum]);
	}
	if (lineBytes != NULL) free(lineBytes);

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_charIsAlphaNumeric
 *
 *  Description:   Report if the supplied character is a character or a number.
 *
 *  Returns:       Returns TRUE if supplied char is within [a-z], [A-Z],  
 *                 [0-9], or '_' otherwise returns FALSE.
 *
 ******************************************************************************/
int neuik_charIsAlphaNumeric(
	char c)
{
	switch (c)
	{
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return TRUE;
	}
	return FALSE;
}


/*******************************************************************************
 *
 *  Name:          neuik_charIsWhitespace
 *
 *  Description:   Report if the supplied character is a whitespace character.
 *
 *  Returns:       Returns TRUE if supplied char is a whitespace character,
 *                 otherwise returns FALSE.
 *
 ******************************************************************************/
int neuik_charIsWhitespace(
	char c)
{
	switch (c)
	{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
 *
 *  Name:          neuiK_getCharType
 *
 *  Description:   Identifies the character type of the specified character.
 *
 *  Returns:       Returns the character type associated with the specified 
 *                 character.
 *
 ******************************************************************************/
neuik_E_CharType neuiK_getCharType(
	char c)
{
	if (neuik_charIsWhitespace(c))
	{
		return E_CHARTYPE_WHITESPACE;
	}
	else if (neuik_charIsAlphaNumeric(c))
	{
		return E_CHARTYPE_ALPHA_NUMERIC;
	}
	return E_CHARTYPE_OTHER;
}


/*******************************************************************************
 *
 *  Name:          neuik_getTextSelectionAtPos
 *
 *  Description:   Get the text selection resulting from a double-click at a 
 *                 specified position.
 *
 *  Returns:       A non-zero integer will be returned if the operation fails.
 *
 ******************************************************************************/
int neuik_getTextSelectionAtPos(
	const char *ln,     /* The line of text containing the selection. */
	int         curPos, /* The cursor position within the line of text. */
	int        *sel0,   /* This stores the starting point of the selection group */
	int        *selF)   /* This stores the final point of the selection group */
{
	int              ckLeft  = FALSE; /* check for expansion to the left */
	int              ckRight = FALSE; /* check for expansion to the right */
	int              lnLen = 0;
	int              ckPos = 0;
	char             aChar;
	neuik_E_CharType aType   = E_CHARTYPE_UNDEFINED; /* char type of the current active character */
	neuik_E_CharType lType   = E_CHARTYPE_UNDEFINED; /* char type of the rightmost character */
	neuik_E_CharType rType   = E_CHARTYPE_UNDEFINED; /* char type of the leftmost character */
	neuik_E_CharType selType = E_CHARTYPE_UNDEFINED; /* char type of the selection */

	if (ln == NULL)
	{
		return 1;
	}
	if (sel0 == NULL)
	{
		return 2;
	}
	if (selF == NULL)
	{
		return 3;
	}

	lnLen = strlen(ln);
	if (lnLen == 0)
	{
		return 4;
	}

	/*------------------------------------------------------------------------*/
	/* Determine the character type for the selection; that is, the selType.  */
	/*------------------------------------------------------------------------*/
	if (curPos == 0)
	{
		/*--------------------------------------------------------------------*/
		/* The cursor is located at the start of the line; check only the     */
		/* type of the first character.                                       */
		/*--------------------------------------------------------------------*/
		aChar = ln[curPos];
		selType = neuiK_getCharType(aChar);
		ckRight = TRUE;
		*sel0 = 0;
	}
	else if (curPos == lnLen)
	{
		/*--------------------------------------------------------------------*/
		/* The cursor is located at the end of the line; check only the type  */
		/* of the final character.                                            */
		/*--------------------------------------------------------------------*/
		aChar = ln[curPos-1];
		selType = neuiK_getCharType(aChar);
		ckLeft = TRUE;
		*selF = curPos-1;
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* The cursor is located somewhere in the middle of the line; check   */
		/* the type of the characters on either side of the cursor position   */
		/* to identify what type should be selected an which direction(s) the */
		/* selection should expand.                                           */
		/*--------------------------------------------------------------------*/
		aChar = ln[curPos-1];
		lType = neuiK_getCharType(aChar);
		aChar = ln[curPos];
		rType = neuiK_getCharType(aChar);

		if (lType == E_CHARTYPE_ALPHA_NUMERIC || 
			rType == E_CHARTYPE_ALPHA_NUMERIC)
		{
			/*----------------------------------------------------------------*/
			/* One or both sides of the cursor is `ALPHA_NUMERIC`. Selection  */
			/* of this type of item is prioritized.                           */
			/*----------------------------------------------------------------*/
			selType = E_CHARTYPE_ALPHA_NUMERIC;
			if (lType == E_CHARTYPE_ALPHA_NUMERIC)
			{
				ckLeft = TRUE;
			}
			if (rType == E_CHARTYPE_ALPHA_NUMERIC)
			{
				ckRight = TRUE;
			}
		}
		else if (lType == E_CHARTYPE_OTHER && rType == E_CHARTYPE_OTHER)
		{
			/*----------------------------------------------------------------*/
			/* Both sides of the cursor are `OTHER` type characters. Expand   */
			/* until `WHITESPACE` or `ALPHA_NUMERIC` are encountered.         */
			/*----------------------------------------------------------------*/
			selType = E_CHARTYPE_OTHER;
			ckLeft = TRUE;
			ckRight = TRUE;
		}
		else if (lType == E_CHARTYPE_WHITESPACE && 
				 rType == E_CHARTYPE_WHITESPACE)
		{
			/*----------------------------------------------------------------*/
			/* Both sides of the cursor are `WHITESPACE` type characters.     */
			/* Expand until `OTHER` or `ALPHA_NUMERIC` are encountered.       */
			/*----------------------------------------------------------------*/
			selType = E_CHARTYPE_WHITESPACE;
			ckLeft = TRUE;
			ckRight = TRUE;
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* This is a junction between a `WHITESPACE` character and an     */
			/* `OTHER` char.                                                  */
			/*----------------------------------------------------------------*/
			selType = E_CHARTYPE_OTHER_PLUS;
			ckLeft = TRUE;
			ckRight = TRUE;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Expand the selection to the left.                                      */
	/*------------------------------------------------------------------------*/
	*sel0 = curPos; /* this is used if it doesn't expand to the left */
	if (ckLeft)
	{
		if (selType != E_CHARTYPE_OTHER_PLUS)
		{
			/*----------------------------------------------------------------*/
			/* For cases other than `OTHER_PLUS`, we don't need to check for  */
			/* multiple types, so the code can be reused.                     */
			/*----------------------------------------------------------------*/
			for (ckPos = curPos-1; ckPos > 0; ckPos--)
			{
				aChar = ln[ckPos];
				aType = neuiK_getCharType(aChar);
				if (aType != selType)
				{
					ckPos++;
					break;
				}
			}
			*sel0 = ckPos;
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* Selection is `OTHER_PLUS` check for chars of either `OTHER` or */
			/* whitespace and include them in the selection.                  */
			/*----------------------------------------------------------------*/
			for (ckPos = curPos-1; ckPos > 0; ckPos--)
			{
				aChar = ln[ckPos];
				aType = neuiK_getCharType(aChar);
				if (aType != lType)
				{
					ckPos++;
					break;
				}
			}
			*sel0 = ckPos;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Expand the selection to the right.                                     */
	/*------------------------------------------------------------------------*/
	*selF = curPos; /* this is used if it doesn't expand to the right */
	if (ckRight)
	{
		if (selType != E_CHARTYPE_OTHER_PLUS)
		{
			/*----------------------------------------------------------------*/
			/* For cases other than `OTHER_PLUS`, we don't need to check for  */
			/* multiple types, so the code can be reused.                     */
			/*----------------------------------------------------------------*/
			for (ckPos = curPos; ckPos < lnLen; ckPos++)
			{
				aChar = ln[ckPos];
				aType = neuiK_getCharType(aChar);
				if (aType != selType)
				{
					break;
				}
			}
			*selF = ckPos;
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* Selection is `OTHER_PLUS` check for chars of either `OTHER` or */
			/* whitespace and include them in the selection.                  */
			/*----------------------------------------------------------------*/
			for (ckPos = curPos; ckPos < lnLen; ckPos++)
			{
				aChar = ln[ckPos];
				aType = neuiK_getCharType(aChar);
				if (aType != rType)
				{
					break;
				}
			}
			*selF = ckPos;
		}
	}

	/*------------------------------------------------------------------------*/
	/* There were no issues; return 0.                                        */
	/*------------------------------------------------------------------------*/
	return 0;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__TextEdit_MouseEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEdit.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEdit_MouseEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                    doContinue   = FALSE;
	int                    evCaptured   = FALSE;
	int                    textW        = 0;
	int                    textH        = 0;
	float                  textHFull    = 0;
	int                    charW        = 0;
	int                    eNum         = 0; /* which error to report (if any) */
	int                    lastW        = 0; /* position of previous char */
	int                    normWidth    = 0;
	int                    yRel         = 0;
	int                    yPos         = 0;
	int                    shift_held   = FALSE;
	int                    sel0         = 0; /* This stores the starting     */
	                                         /* point of the selection group */
	int                    selF         = 0; /* This stores the final point  */
	                                         /* of the selection group       */
	unsigned int           timeAtClick  = 0;
	size_t                 lineLen      = 0;
	size_t                 nLines       = 0;
	size_t                 clickLine    = 0;
	size_t                 oldCursorPos = 0;
	size_t                 ctr          = 0;
	char                   aChar        = 0;
	char                 * lineBytes    = NULL; /* FREE at exit */
	TTF_Font             * font         = NULL;
	SDL_Rect               rect         = {0, 0, 0 ,0};
	SDL_Keymod             keyMod;
	RenderSize             rSize;
	RenderLoc              rLoc;
	RenderSize           * rSizePtr;
	SDL_MouseMotionEvent * mouseMotEv;
	SDL_MouseButtonEvent * mouseButEv;
	NEUIK_TextEdit       * te         = NULL;
	NEUIK_TextEditConfig * aCfg       = NULL; /* the active button config */
	NEUIK_ElementBase    * eBase      = NULL;
	static char            funcName[] = 
		"neuik_Element_CaptureEvent__TextEdit_MouseEvent";

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
	/* select the correct element config to use (pointer or internal)         */
	/*------------------------------------------------------------------------*/
	aCfg = te->cfgPtr;
	if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

	if (aCfg->fontMono)
	{
		font = NEUIK_FontSet_GetFont(aCfg->fontSetMS, aCfg->fontSize,
			aCfg->fontBold, aCfg->fontItalic);
	}
	else
	{
		font = NEUIK_FontSet_GetFont(aCfg->fontSet, aCfg->fontSize,
			aCfg->fontBold, aCfg->fontItalic);
	}
	if (font == NULL)
	{
		eNum = 1;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check if the shift-key is being held. This will be used later on.      */
	/*------------------------------------------------------------------------*/
	keyMod = SDL_GetModState();
	if (keyMod & KMOD_SHIFT)
	{
		shift_held = TRUE;
	}


	/*------------------------------------------------------------------------*/
	/* Redirect the MouseEvent to the appropriate handling section            */
	/*------------------------------------------------------------------------*/
	switch (ev->type)
	{
	case SDL_MOUSEBUTTONDOWN:
		mouseButEv = (SDL_MouseButtonEvent*)(ev);

		/*--------------------------------------------------------------------*/
		/* Check to see if the click was within the bounds of this element    */
		/*--------------------------------------------------------------------*/
		if (mouseButEv->y >= eBase->eSt.rLoc.y && 
			mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
		{
			if (mouseButEv->x >= eBase->eSt.rLoc.x && 
				mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
			{
				/* This mouse click originated within this textEdit */
				if (!eBase->eSt.hasFocus)
				{
					/*--------------------------------------------------------*/
					/* This TextEdit did not have the window focus            */
					/*--------------------------------------------------------*/
					te->selected    = TRUE;
					te->wasSelected = TRUE;
					neuik_Window_TakeFocus(eBase->eSt.window, (NEUIK_Element)te);
					SDL_StartTextInput();

					rSize = eBase->eSt.rSize;
					rLoc  = eBase->eSt.rLoc;
					neuik_Element_RequestRedraw(te, rLoc, rSize);
				}
				doContinue = TRUE;
				evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			}
		}

		if (!doContinue) goto out;
		doContinue = FALSE;

		rSizePtr = &(eBase->eSt.rSize);

		/*--------------------------------------------------------------------*/
		/* If this is the start of text selection highlighting, then save the */
		/* old position as the beginning of the highlight selection.          */
		/*--------------------------------------------------------------------*/
		if (shift_held && !te->highlightIsSet)
		{
			te->highlightIsSet     = TRUE;
			te->highlightBeginLine = te->cursorLine;
			te->highlightBeginPos  = te->cursorPos;
		}
		else if (!(shift_held && te->highlightIsSet))
		{
			te->highlightIsSet = FALSE;
		}

		/*--------------------------------------------------------------------*/
		/* Determine the line of text in which the click occurred             */
		/*--------------------------------------------------------------------*/
		TTF_SizeText(font, " ", &textW, &textH);
		textHFull = 1.1*(float)(textH);

		yPos = 6; /* <-- this is the offset from the top where text begins */
		if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
		{
			eNum = 7;
			goto out;
		}

		yRel = mouseButEv->y - eBase->eSt.rLoc.y;
		clickLine = (size_t)((float)(yRel - yPos)/textHFull);

		if (clickLine < nLines)
		{
			/*----------------------------------------------------------------*/
			/* Get the overall location of the current text                   */
			/*----------------------------------------------------------------*/
			if (neuik_TextBlock_GetLine(te->textBlk, clickLine, &lineBytes))
			{
				eNum = 11;
				goto out;
			}

			doContinue = TRUE;
			te->cursorLine = clickLine;
			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);

			if (lineBytes != NULL)
			{
				if (*lineBytes != '\0')
				{
					normWidth = (eBase->eSt.rSize).w - 12; 
					TTF_SizeText(font, lineBytes, &textW, &textH);
					rect.w = textW;

					if (textW < normWidth) 
					{
						switch (aCfg->textHJustify)
						{
						case NEUIK_HJUSTIFY_LEFT:
							rect.x = 6;
							break;

						case NEUIK_HJUSTIFY_CENTER:
							rect.x = (int) ((float)(rSizePtr->w - textW)/2.0);
							break;

						case NEUIK_HJUSTIFY_RIGHT:
							rect.x = (int) (rSizePtr->w - textW - 6);
							break;
						}
					}
					else
					{
						rect.x = 6;
					}
				}
			}
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* The click originated below the final line in the TextEdit.     */
			/* Position the cursor at the end of the final line.              */
			/*----------------------------------------------------------------*/
			te->cursorLine = nLines - 1;

			if (neuik_TextBlock_GetLineLength(te->textBlk,
				te->cursorLine, &lineLen))
			{
				/* ERR: problem reported from textBlock */
				eNum = 6;
				goto out;
			}

			te->cursorPos = lineLen;
			te->vertMovePos = te->cursorPos;
			rSize = eBase->eSt.rSize;
			rLoc  = eBase->eSt.rLoc;
			neuik_Element_RequestRedraw(te, rLoc, rSize);
			te->clickHeld = 1;
		}

		if (!doContinue) goto out;

		if (neuik_TextBlock_GetLineLength(te->textBlk,
			te->cursorLine, &lineLen))
		{
			/* ERR: problem reported from textBlock */
			eNum = 6;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* If continuing, this TextEdit contains text and so the cursor       */
		/* cursor placement may have been changed.                            */
		/*--------------------------------------------------------------------*/
		timeAtClick = SDL_GetTicks();
		if (timeAtClick - te->timeClickMinus2 < 2*NEUIK_DOUBLE_CLICK_TIMEOUT)
		{
			/*----------------------------------------------------------------*/
			/* React to a triple-click event; select the whole line           */
			/*----------------------------------------------------------------*/
			if (neuik_TextBlock_GetLineLength(te->textBlk,
				te->cursorLine, &lineLen))
			{
				/* ERR: problem reported from textBlock */
				eNum = 6;
				goto out;
			}

			te->highlightIsSet     = TRUE;
			te->highlightStartLine = te->cursorLine;
			te->highlightStartPos  = 0;
			te->highlightEndLine   = te->cursorLine;
			te->highlightEndPos    = lineLen;
			te->cursorPos          = lineLen;
			te->vertMovePos        = te->cursorPos;

			if (te->cursorLine < te->textBlk->nLines)
			{
				/*------------------------------------------------------------*/
				/* For lines before the final line, a full line selection     */
				/* should place the cursor and the end of highlight section   */ 
				/* at the start of the following line.                        */
				/*------------------------------------------------------------*/
				te->cursorLine++;
				te->cursorPos        = 0;
				te->highlightEndLine = te->cursorLine;
				te->highlightEndPos  = 0;
			}


			rSize = eBase->eSt.rSize;
			rLoc  = eBase->eSt.rLoc;
			neuik_Element_RequestRedraw(te, rLoc, rSize);
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			goto out;
		}
		else if (SDL_GetTicks() - te->timeLastClick < NEUIK_DOUBLE_CLICK_TIMEOUT &&
			!te->highlightIsSet)
		{
			/*----------------------------------------------------------------*/
			/* React to a double-click event                                  */
			/*----------------------------------------------------------------*/
			te->timeClickMinus2 = te->timeLastClick;
			te->timeLastClick   = timeAtClick;

			if (lineLen > 0)
			{
				if (lineBytes != NULL) free(lineBytes);
				if (neuik_TextBlock_GetLine(te->textBlk, clickLine, &lineBytes))
				{
					eNum = 11;
					goto out;
				}

				if (neuik_getTextSelectionAtPos(
					lineBytes, te->cursorPos, &sel0, &selF))
				{
					/* ERR: problem reported from neuik_getTextSelectionAtPos */
					eNum = 14;
					goto out;
				}

				te->highlightIsSet     = TRUE;
				te->highlightStartLine = te->cursorLine;
				te->highlightStartPos  = sel0;
				te->highlightEndLine   = te->cursorLine;
				te->highlightEndPos    = selF;
				te->cursorPos          = selF;
				te->vertMovePos = te->cursorPos;

				rSize = eBase->eSt.rSize;
				rLoc  = eBase->eSt.rLoc;
				neuik_Element_RequestRedraw(te, rLoc, rSize);
				evCaptured = NEUIK_EVENTSTATE_CAPTURED;
				goto out;
			}
		}
		else if (te->panCursor == 0 && 
			mouseButEv->x <= eBase->eSt.rLoc.x + rect.x)
		{
			/*----------------------------------------------------------------*/
			/* Move the cursor position to the start of the line.             */
			/*----------------------------------------------------------------*/
			te->cursorPos = 0;
			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
		}
		else if (mouseButEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
		{
			/*----------------------------------------------------------------*/
			/* Move the cursor position to the end of the line.               */
			/*----------------------------------------------------------------*/
			te->cursorPos = lineLen;
			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
		}
		else if (lineLen < 1)
		{
			/*----------------------------------------------------------------*/
			/* There is no text on this line, move the cursor to the zero     */
			/* position.                                                      */
			/*----------------------------------------------------------------*/
			te->cursorPos = 0;

		}
		else
		{
			/*----------------------------------------------------------------*/
			/* move the cursor somewhere within the line.                     */
			/*----------------------------------------------------------------*/
			if (lineBytes != NULL) free(lineBytes);
			if (neuik_TextBlock_GetLine(te->textBlk, clickLine, &lineBytes))
			{
				eNum = 11;
				goto out;
			}

			oldCursorPos = te->cursorPos;
			for (ctr = 1;;ctr++)
			{
				aChar = lineBytes[ctr];

				lineBytes[ctr] = '\0';
				TTF_SizeText(font, lineBytes, &textW, &textH);
				lineBytes[ctr] = aChar;

				if (mouseButEv->x + te->panCursor <= 
					eBase->eSt.rLoc.x + rect.x + textW)
				{
					/* cursor will be before this char */
					te->cursorPos   = ctr - 1;
					te->vertMovePos = te->cursorPos;
					charW = textW - lastW;
					if (mouseButEv->x + te->panCursor <= 
						eBase->eSt.rLoc.x + rect.x + textW - charW/3)
					{
						/* cursor will be before this char */
						te->cursorPos = ctr - 1;
					}
					else
					{
						/* cursor will be after char */
						te->cursorPos = ctr;
					}

					/*--------------------------------------------------------*/
					/* Update the cursor Panning (if necessary).              */
					/*--------------------------------------------------------*/
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
			lineBytes[ctr] = aChar;
		}
		te->clickOrigin     = te->cursorPos;
		te->timeClickMinus2 = te->timeLastClick;
		te->timeLastClick   = timeAtClick;

		if (te->highlightIsSet) {
			/*----------------------------------------------------------------*/
			/* Update the highlight selections .                              */
			/*----------------------------------------------------------------*/
			if (te->cursorLine < te->highlightBeginLine ||
					(te->cursorLine == te->highlightBeginLine &&
					 te->cursorPos < te->highlightBeginPos))
			{
				/* highlight is expanding/contracting to the above/left */
				te->highlightStartLine = te->cursorLine;
				te->highlightStartPos  = te->cursorPos;
				te->highlightEndLine   = te->highlightBeginLine;
				te->highlightEndPos    = te->highlightBeginPos;
			}
			else
			{
				/* highlight is expanding/contracting to the below/right */
				te->highlightStartLine = te->highlightBeginLine;
				te->highlightStartPos  = te->highlightBeginPos;
				te->highlightEndLine   = te->cursorLine;
				te->highlightEndPos    = te->cursorPos;
			}
		}

		rSize = eBase->eSt.rSize;
		rLoc  = eBase->eSt.rLoc;
		neuik_Element_RequestRedraw(te, rLoc, rSize);
		evCaptured = NEUIK_EVENTSTATE_CAPTURED;

		te->clickHeld = 1;
		break;
	case SDL_MOUSEBUTTONUP:
		if (eBase->eSt.hasFocus)
		{
			/*----------------------------------------------------------------*/
			/* This text entry has the window focus (unset `clickHeld`)       */
			/*----------------------------------------------------------------*/
			te->clickHeld = FALSE;
			evCaptured    = NEUIK_EVENTSTATE_CAPTURED;
		}
		break;
	case SDL_MOUSEMOTION:
		if (eBase->eSt.hasFocus && te->clickHeld)
		{
			/*----------------------------------------------------------------*/
			/* This text entry currently has the window focus and the mouse   */
			/* button is still being held down. **Drag Select**               */
			/*----------------------------------------------------------------*/
			mouseMotEv = (SDL_MouseMotionEvent*)(ev);
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			if (mouseMotEv->y < eBase->eSt.rLoc.y)
			{
				/*------------------------------------------------------------*/
				/* If the mouse motion event is above the top of the TextEdit */
				/* treat it as if it occurred at the top of the widget.       */
				/*------------------------------------------------------------*/
				mouseMotEv->y = eBase->eSt.rLoc.y;
			}
			else if (mouseMotEv->y > eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				/*------------------------------------------------------------*/
				/* If the mouse motion event is below the bottom of the       */
				/* TextEdit treat it as if it occurred at the top of the      */
				/* widget.                                                    */
				/*------------------------------------------------------------*/
				mouseMotEv->y = eBase->eSt.rLoc.y + eBase->eSt.rSize.h;
			}

			rSizePtr = &(eBase->eSt.rSize);

			/*----------------------------------------------------------------*/
			/* Determine the line of text in which the click occurred         */
			/*----------------------------------------------------------------*/
			TTF_SizeText(font, " ", &textW, &textH);
			textHFull = 1.1*(float)(textH);

			yPos = 6; /* <-- this is the offset from the top where text begins */
			if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
			{
				eNum = 7;
				goto out;
			}

			yRel = mouseMotEv->y - eBase->eSt.rLoc.y;
			clickLine = (size_t)((float)(yRel - yPos)/textHFull);

			/*----------------------------------------------------------------*/
			/* If this is the start of text selection highlighting, then save */
			/* the old position as the beginning of the highlight selection.  */
			/*----------------------------------------------------------------*/
			if (!te->highlightIsSet)
			{
				te->highlightIsSet     = TRUE;
				te->highlightBeginLine = te->cursorLine;
				te->highlightBeginPos  = te->cursorPos;
			}

			/*----------------------------------------------------------------*/
			/* Determine the new cursor position.                             */
			/*----------------------------------------------------------------*/
			if (clickLine < nLines)
			{
				/*------------------------------------------------------------*/
				/* Get the overall location of the current text               */
				/*------------------------------------------------------------*/
				if (lineBytes != NULL) free(lineBytes);
				if (neuik_TextBlock_GetLine(te->textBlk, clickLine, &lineBytes))
				{
					eNum = 11;
					goto out;
				}

				te->cursorLine = clickLine;
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);

				if (lineBytes != NULL)
				{
					if (*lineBytes != '\0')
					{
						normWidth = (eBase->eSt.rSize).w - 12; 
						TTF_SizeText(font, lineBytes, &textW, &textH);
						rect.w = textW;

						if (textW < normWidth) 
						{
							switch (aCfg->textHJustify)
							{
								case NEUIK_HJUSTIFY_LEFT:
									rect.x = 6;
									break;

								case NEUIK_HJUSTIFY_CENTER:
									rect.x = (int) ((float)(rSizePtr->w - textW)/2.0);
									break;

								case NEUIK_HJUSTIFY_RIGHT:
									rect.x = (int) (rSizePtr->w - textW - 6);
									break;
							}
						}
						else
						{
							rect.x = 6;
						}
					}
				}
			}
			else
			{
				te->cursorLine = nLines - 1;

				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}
				te->cursorPos       = lineLen;
				te->highlightEndPos = te->cursorPos;
				te->vertMovePos     = te->cursorPos;

				/*------------------------------------------------------------*/
				/* Update the highlight selections .                          */
				/*------------------------------------------------------------*/
				if (te->cursorLine < te->highlightBeginLine ||
						(te->cursorLine == te->highlightBeginLine &&
						 te->cursorPos < te->highlightBeginPos))
				{
					/* highlight is expanding/contracting to the above/left */
					te->highlightStartLine = te->cursorLine;
					te->highlightStartPos  = te->cursorPos;
					te->highlightEndLine   = te->highlightBeginLine;
					te->highlightEndPos    = te->highlightBeginPos;
				}
				else
				{
					/* highlight is expanding/contracting to the below/right */
					te->highlightStartLine = te->highlightBeginLine;
					te->highlightStartPos  = te->highlightBeginPos;
					te->highlightEndLine   = te->cursorLine;
					te->highlightEndPos    = te->cursorPos;
				}

				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
				rSize = eBase->eSt.rSize;
				rLoc  = eBase->eSt.rLoc;
				neuik_Element_RequestRedraw(te, rLoc, rSize);
				evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			}

			/*----------------------------------------------------------------*/
			/* If continuing, this TextEdit contains text and so the cursor   */
			/* placement could have been changed.                             */
			/*----------------------------------------------------------------*/
			if (te->panCursor == 0 && 
				mouseMotEv->x <= eBase->eSt.rLoc.x + rect.x)
			{
				/* move the cursor position all the way to the start */
				te->cursorPos   = 0;
				te->vertMovePos = te->cursorPos;
			}
			else if (mouseMotEv->x >= 
				eBase->eSt.rLoc.x + rect.x + rect.w)
			{
				/* move the cursor position all the way to the end */
				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}
				te->cursorPos   = lineLen;
				te->vertMovePos = te->cursorPos;
			}
			else
			{
				/* move the cursor somewhere within the text */
				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}

				if (lineBytes != NULL) free(lineBytes);
				if (neuik_TextBlock_GetLine(te->textBlk, clickLine, &lineBytes))
				{
					eNum = 11;
					goto out;
				}

				if (lineLen > 1)
				{
					oldCursorPos = te->cursorPos;

					for (ctr = 0; ctr < lineLen; ctr++)
					{
						aChar = lineBytes[ctr];

						lineBytes[ctr] = '\0';
						TTF_SizeText(font, lineBytes, &textW, &textH);
						lineBytes[ctr] = aChar;

						if (mouseMotEv->x + te->panCursor <= 
							eBase->eSt.rLoc.x + rect.x + textW)
						{
							/* cursor will be before this char */
							te->cursorPos   = ctr - 1;
							charW = textW - lastW;
							if (mouseMotEv->x + te->panCursor <= 
								eBase->eSt.rLoc.x + rect.x + textW - charW/3)
							{
								/* cursor will be before this char */
								te->cursorPos = ctr - 1;
							}
							else
							{
								/* cursor will be after char */
								te->cursorPos = ctr;
							}
							te->vertMovePos = te->cursorPos;

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
					lineBytes[ctr] = aChar;
				}
				else
				{
					/*--------------------------------------------------------*/
					/* There is no text on this line, move the cursor to the  */
					/* zero position.                                         */
					/*--------------------------------------------------------*/
					te->cursorPos   = 0;
					te->vertMovePos = te->cursorPos;
				}
			}

			/*----------------------------------------------------------------*/
			/* Update the highlight selections .  */
			/*----------------------------------------------------------------*/
			if (te->cursorLine < te->highlightBeginLine ||
					(te->cursorLine == te->highlightBeginLine &&
					 te->cursorPos < te->highlightBeginPos))
			{
				/* highlight is expanding/contracting to the above/left */
				te->highlightStartLine = te->cursorLine;
				te->highlightStartPos  = te->cursorPos;
				te->highlightEndLine   = te->highlightBeginLine;
				te->highlightEndPos    = te->highlightBeginPos;
			}
			else
			{
				/* highlight is expanding/contracting to the below/right */
				te->highlightStartLine = te->highlightBeginLine;
				te->highlightStartPos  = te->highlightBeginPos;
				te->highlightEndLine   = te->cursorLine;
				te->highlightEndPos    = te->cursorPos;
			}

			rSize = eBase->eSt.rSize;
			rLoc  = eBase->eSt.rLoc;
			neuik_Element_RequestRedraw(te, rLoc, rSize);
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
		}
		break;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (lineBytes != NULL) free(lineBytes);

	return evCaptured;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__TextEdit_TextInputEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEdit.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEdit_TextInputEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                  evCaptured = 0;
	int                  eNum       = 0; /* which error to report (if any) */
	size_t               inpLen     = 0; /* length of text input */
	char               * clipText   = NULL;
	SDL_TextInputEvent * textInpEv  = NULL;
	NEUIK_TextEdit     * te         = NULL;
	NEUIK_ElementBase  * eBase      = NULL;
	RenderSize           rSize;
	RenderLoc            rLoc;
	static char          funcName[] = 
		"neuik_Element_CaptureEvent__TextEdit_TextInputEvent";

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

	if (!eBase->eSt.hasFocus)
	{
		goto out;
	}
	textInpEv = (SDL_TextInputEvent*)(ev);

	/*--------------------------------------------------------------------*/
	/* First delete the currently highlighted section (if it exists)      */
	/*--------------------------------------------------------------------*/
	if (te->highlightIsSet)
	{
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
	}

	/*--------------------------------------------------------------------*/
	/* Now insert the new character(s)                                    */
	/*--------------------------------------------------------------------*/
	inpLen = strlen(textInpEv->text);
	if (strlen(textInpEv->text) == 1)
	{
		if (neuik_TextBlock_InsertChar(te->textBlk, 
			te->cursorLine, te->cursorPos, textInpEv->text[0]))
		{
			printf("InsertChar `%c` at [%u:%u]\n", textInpEv->text[0],
				(unsigned int)(te->cursorLine), (unsigned int)(te->cursorPos));
			eNum = 5;
			goto out;
		}
	}
	else
	{
		fprintf(stderr, "[TODO] neuik_Element_CaptureEvent__TextEdit: add chars for inpLen > 1\n");
	}
	te->cursorPos += inpLen;

	neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_INSERTED);
	rSize = eBase->eSt.rSize;
	rLoc  = eBase->eSt.rLoc;
	neuik_Element_RequestRedraw(te, rLoc, rSize);
	evCaptured = NEUIK_EVENTSTATE_CAPTURED;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (clipText != NULL) free(clipText);

	return evCaptured;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__TextEdit_KeyDownEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEdit.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEdit_KeyDownEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                 evCaptured = 0;
	int                 doRedraw   = 0;
	int                 eNum       = 0; /* which error to report (if any) */
	size_t              lineLen    = 0;
	size_t              nLines     = 0;
	char              * clipText   = NULL;
	SDL_Keymod          keyMod;
	SDL_KeyboardEvent * keyEv;
	NEUIK_TextEdit    * te         = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	RenderSize          rSize;
	RenderLoc           rLoc;
	static char         funcName[] = 
		"neuik_Element_CaptureEvent__TextEdit_KeyDownEvent";

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

	if (!eBase->eSt.hasFocus)
	{
		goto out;
	}

	keyEv  = (SDL_KeyboardEvent*)(ev);
	keyMod = SDL_GetModState();
	switch (keyEv->keysym.sym)
	{
		case SDLK_LEFT:

			te->vertMovePos = UNDEFINED;
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
						te->highlightEndPos    = te->highlightBeginPos;
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

		case SDLK_RIGHT:

			te->vertMovePos = UNDEFINED;
			if (!(keyMod & KMOD_SHIFT))
			{
				/* SHIFT key is not being held down */
				if (te->highlightIsSet)
				{
					/*--------------------------------------------------------*/
					/* Break out of a highlight selection                     */
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

					te->highlightIsSet = 0;
					te->cursorLine     = te->highlightEndLine;
					te->cursorPos      = te->highlightEndPos + 1;
					if (te->cursorPos >= lineLen)
					{
						te->cursorPos--;
					}
					doRedraw = 1;
				}
				else
				{
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
						te->cursorPos++;
						doRedraw = 1;
					}
					else if (te->cursorPos == lineLen && 
							te->cursorLine < te->textBlk->nLines)
					{
						/*----------------------------------------------------*/
						/* For lines before the final line, attempting to     */
						/* right should cause the cursor to move to the start */
						/* of the following line.                             */
						/*----------------------------------------------------*/
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
				/*------------------------------------------------------------*/
				/* Prevent the cursor from moving to a position in excess of  */
				/* the line length.                                           */
				/*------------------------------------------------------------*/
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
						te->highlightEndPos    = te->cursorPos;
					}
					else
					{
						te->highlightStartLine = te->cursorLine;
						te->highlightStartPos  = te->cursorPos;
						te->highlightEndLine   = te->highlightBeginLine;
						te->highlightEndPos    = te->highlightBeginPos;
					}
				}
				else if (te->cursorPos == lineLen && 
						te->cursorLine < te->textBlk->nLines)
				{
					/*--------------------------------------------------------*/
					/* For lines before the final line, attempting to move    */
					/* right should cause the cursor to move to the start of  */
					/* the following line.                                    */
					/*--------------------------------------------------------*/
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
				/*------------------------------------------------------------*/
				/* There is no current text highlighting.                     */
				/*------------------------------------------------------------*/
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
					/*--------------------------------------------------------*/
					/* The cursor is in the first position of a line that is  */
					/* not the first line. A backspace here will combine the  */
					/* current line to the preceding line.                    */
					/*--------------------------------------------------------*/
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
				/*------------------------------------------------------------*/
				/* There is text highlighting within the line.                */
				/*------------------------------------------------------------*/
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
				/*------------------------------------------------------------*/
				/* There is no current text highlighting.                     */
				/*------------------------------------------------------------*/

				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}
				if (lineLen == 0 && (te->textBlk->nLines > te->cursorLine))
				{
					/*--------------------------------------------------------*/
					/* The cursor is in the final position of a line that is  */
					/* not the final line. A delete here will combine the     */
					/* current line to the following line.                    */
					/*--------------------------------------------------------*/
					if (neuik_TextBlock_MergeLines(te->textBlk, te->cursorLine))
					{
						/* ERR: problem reported from textBlock */
						eNum = 9;
						goto out;
					}
					doRedraw = 1;
				}
				else if (te->cursorPos < lineLen - 1)
				{
					/*--------------------------------------------------------*/
					/* Prevent the deletion of the final terminating NULL     */
					/* character.                                             */
					/*--------------------------------------------------------*/
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
					/*--------------------------------------------------------*/
					/* The cursor is in the final position of a line that is  */
					/* not the final line. A delete here will combine the     */
					/* current line to the following line.                    */
					/*--------------------------------------------------------*/
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
				/*------------------------------------------------------------*/
				/* There is text highlighting within the line.                */
				/*------------------------------------------------------------*/
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
			if (te->cursorLine > 0)
			{
				/*------------------------------------------------------------*/
				/* The cursor is on a lower line. Move the cursor upwards     */
				/* preferentially keeping the initial x position.             */
				/*------------------------------------------------------------*/
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */

					te->highlightIsSet = FALSE;
					te->clickOrigin    = UNDEFINED;
					te->cursorLine--;

					if (te->vertMovePos == UNDEFINED)
					{
						/*----------------------------------------------------*/
						/* Initial x position for vertical movement not set.  */
						/* Set it now...                                      */
						/*----------------------------------------------------*/
						te->vertMovePos = te->cursorPos;
					}

					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}

					te->cursorPos = lineLen;
					if (lineLen > te->vertMovePos)
					{
						te->cursorPos = te->vertMovePos;
					}
					doRedraw = TRUE;
				}
				else
				{
					/* SHIFT key IS being held down */
					if (!te->highlightIsSet)
					{
						te->highlightIsSet     = TRUE;
						te->highlightBeginLine = te->cursorLine;
						te->highlightBeginPos  = te->cursorPos;
					}

					te->cursorLine--;
					if (te->vertMovePos == UNDEFINED)
					{
						/*----------------------------------------------------*/
						/* Initial x position for vertical movement not set.  */
						/* Set it now...                                      */
						/*----------------------------------------------------*/
						te->vertMovePos = te->cursorPos;
					}

					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}

					te->cursorPos = lineLen;
					if (lineLen > te->vertMovePos)
					{
						te->cursorPos = te->vertMovePos;
					}
					doRedraw = TRUE;

					if (te->cursorLine < te->highlightBeginLine ||
							(te->cursorLine == te->highlightBeginLine &&
							 te->cursorPos < te->highlightBeginPos))
					{
						/* highlight is expanding to the left */
						te->highlightStartLine = te->cursorLine;
						te->highlightStartPos  = te->cursorPos;
						te->highlightEndLine   = te->highlightBeginLine;
						te->highlightEndPos    = te->highlightBeginPos;
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
				neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
			}
			else
			{
				/*------------------------------------------------------------*/
				/* The cursor is already in the top line. Simply move the     */
				/* cursor over to the start of the first line of text.        */
				/*------------------------------------------------------------*/
				te->vertMovePos = 0;
				if (te->cursorPos > 0)
				{
					if (!(keyMod & KMOD_SHIFT))
					{
						/* SHIFT key is not being held down */
						te->highlightIsSet = 0;
						te->clickOrigin    = UNDEFINED;
						te->cursorPos      = 0;
					}
					else
					{
						/* SHIFT key IS being held down */
						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = TRUE;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}
						te->cursorPos = 0;

						if (te->cursorLine < te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos < te->highlightBeginPos))
						{
							/* highlight is expanding to the left */
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos;
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
					doRedraw = TRUE;
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				}
			}
			break;

		case SDLK_DOWN:
			/* Move the cursor to the end of the line of text */
			if (neuik_TextBlock_GetLineCount(te->textBlk, &lineLen))
			{
				eNum = 7;
				goto out;
			}

			if (te->cursorLine < lineLen-1)
			{
				/*--------------------------------------------------------*/
				/* Prevent the cursor from moving to a line that is in    */
				/* excess of the number of lines.                         */
				/*--------------------------------------------------------*/
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					te->highlightIsSet = FALSE;
					te->clickOrigin    = UNDEFINED;

					te->cursorLine++;

					if (te->vertMovePos == UNDEFINED)
					{
						/*----------------------------------------------------*/
						/* Initial x position for vertical movement not set.  */
						/* Set it now...                                      */
						/*----------------------------------------------------*/
						te->vertMovePos = te->cursorPos;
					}

					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}

					te->cursorPos = lineLen;
					if (lineLen > te->vertMovePos)
					{
						te->cursorPos = te->vertMovePos;
					}
					doRedraw = TRUE;
				}
				else
				{
					if (!te->highlightIsSet)
					{
						te->highlightIsSet     = 1;
						te->highlightBeginLine = te->cursorLine;
						te->highlightBeginPos  = te->cursorPos;
					}

					te->cursorLine++;

					if (te->vertMovePos == UNDEFINED)
					{
						/*----------------------------------------------------*/
						/* Initial x position for vertical movement not set.  */
						/* Set it now...                                      */
						/*----------------------------------------------------*/
						te->vertMovePos = te->cursorPos;
					}

					if (neuik_TextBlock_GetLineLength(te->textBlk,
						te->cursorLine, &lineLen))
					{
						/* ERR: problem reported from textBlock */
						eNum = 6;
						goto out;
					}

					te->cursorPos = lineLen;
					if (lineLen > te->vertMovePos)
					{
						te->cursorPos = te->vertMovePos;
					}

					if (te->cursorLine > te->highlightBeginLine ||
							(te->cursorLine == te->highlightBeginLine &&
							 te->cursorPos > te->highlightBeginPos))
					{
						/* highlight is expanding to the right */
						te->highlightStartLine = te->highlightBeginLine;
						te->highlightStartPos  = te->highlightBeginPos;
						te->highlightEndLine   = te->cursorLine;
						te->highlightEndPos    = te->cursorPos;
					}
					else
					{
						te->highlightStartLine = te->cursorLine;
						te->highlightStartPos  = te->cursorPos;
						te->highlightEndLine   = te->highlightBeginLine;
						te->highlightEndPos    = te->highlightBeginPos;
					}
					doRedraw = TRUE;
				}
			}
			else
			{
				/*------------------------------------------------------------*/
				/* The cursor is already in the bottom line. Simply move the  */
				/* cursor over to the end of the final line of text.          */
				/*------------------------------------------------------------*/
				if (neuik_TextBlock_GetLineLength(te->textBlk,
					te->cursorLine, &lineLen))
				{
					/* ERR: problem reported from textBlock */
					eNum = 6;
					goto out;
				}
				te->vertMovePos = lineLen;

				if (te->cursorPos < lineLen)
				{

					if (!(keyMod & KMOD_SHIFT))
					{
						/* SHIFT key is not being held down */
						te->highlightIsSet = 0;
						te->clickOrigin    = UNDEFINED;
						te->cursorPos      = lineLen;
					}
					else
					{
						/* SHIFT key IS being held down */
						if (!te->highlightIsSet)
						{
							te->highlightIsSet     = TRUE;
							te->highlightBeginLine = te->cursorLine;
							te->highlightBeginPos  = te->cursorPos;
						}
						te->cursorPos = lineLen;

						if (te->cursorLine < te->highlightBeginLine ||
								(te->cursorLine == te->highlightBeginLine &&
								 te->cursorPos < te->highlightBeginPos))
						{
							/* highlight is expanding to the left */
							te->highlightStartLine = te->cursorLine;
							te->highlightStartPos  = te->cursorPos;
							te->highlightEndLine   = te->highlightBeginLine;
							te->highlightEndPos    = te->highlightBeginPos;
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
					neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
				}
				doRedraw = TRUE;
			}
			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
			break;

		case SDLK_RETURN:
			/*----------------------------------------------------------------*/
			/* Insert a line break.                                           */
			/*----------------------------------------------------------------*/
			if (neuik_TextBlock_GetLineLength(te->textBlk,
				te->cursorLine, &lineLen))
			{
				/* ERR: problem reported from textBlock */
				eNum = 6;
				goto out;
			}

			if (te->cursorPos == lineLen)
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
			te->cursorLine += 1;
			te->cursorPos   = 0;

			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_INSERTED);
			rSize = eBase->eSt.rSize;
			rLoc  = eBase->eSt.rLoc;
			neuik_Element_RequestRedraw(te, rLoc, rSize);
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			goto out;

		case SDLK_HOME:

			te->vertMovePos = UNDEFINED;
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

					if (te->cursorLine < te->highlightBeginLine ||
							(te->cursorLine == te->highlightBeginLine &&
							 te->cursorPos < te->highlightBeginPos))
					{
						/* highlight is expanding to the left */
						te->highlightStartLine = te->cursorLine;
						te->highlightStartPos  = te->cursorPos;
						te->highlightEndLine   = te->highlightBeginLine;
						te->highlightEndPos    = te->highlightBeginPos;
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

			te->vertMovePos = UNDEFINED;
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
				if (te->cursorPos < lineLen)
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
						te->highlightEndPos    = te->cursorPos;
					}
					else
					{
						te->highlightStartLine = te->cursorLine;
						te->highlightStartPos  = te->cursorPos;
						te->highlightEndLine   = te->highlightBeginLine;
						te->highlightEndPos    = te->highlightBeginPos;
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
			/*----------------------------------------------------------------*/
			/* There is text highlighting within the line                     */
			/*----------------------------------------------------------------*/
			if (neuik_TextBlock_GetSection(te->textBlk,
				te->highlightStartLine, te->highlightStartPos, 
				te->highlightEndLine, te->highlightEndPos,
				&clipText))
			{
				eNum = 12;
				goto out;
			}
			SDL_SetClipboardText(clipText);
		}
	}
	else if (neuik_KeyShortcut_Cut(keyEv, keyMod))
	{
		if (te->highlightIsSet)
		{
			/*----------------------------------------------------------------*/
			/* Copy the section of highlighted text and store it in the       */
			/* system clipboard.                                              */
			/*----------------------------------------------------------------*/
			if (neuik_TextBlock_GetSection(te->textBlk,
				te->highlightStartLine, te->highlightStartPos, 
				te->highlightEndLine, te->highlightEndPos,
				&clipText))
			{
				eNum = 12;
				goto out;
			}
			SDL_SetClipboardText(clipText);

			/*----------------------------------------------------------------*/
			/* Delete the section of highlighted text                         */
			/*----------------------------------------------------------------*/
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

			neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
		}
	}
	else if (neuik_KeyShortcut_Paste(keyEv, keyMod) && SDL_HasClipboardText())
	{
		if (te->highlightIsSet)
		{
			/*----------------------------------------------------------------*/
			/* Delete the section of highlighted text                         */
			/*----------------------------------------------------------------*/
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

		/*--------------------------------------------------------------------*/
		/* Get a copy of the text within the clipboard buffer                 */
		/*--------------------------------------------------------------------*/
		clipText = SDL_GetClipboardText();
		if (clipText == NULL)
		{
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			eNum = 2;
			goto out;
		}

		if (neuik_TextBlock_InsertText(te->textBlk,
			te->cursorLine, te->cursorPos, clipText,
			&te->cursorLine, &te->cursorPos))
		{
			eNum = 13;
			goto out;
		}

		neuik_TextEdit_UpdatePanCursor(te, CURSORPAN_TEXT_ADD_REMOVE);
		rSize = eBase->eSt.rSize;
		rLoc  = eBase->eSt.rLoc;
		neuik_Element_RequestRedraw(te, rLoc, rSize);
		evCaptured = NEUIK_EVENTSTATE_CAPTURED;
	}
	else if (neuik_KeyShortcut_SelectAll(keyEv, keyMod))
	{
		/*--------------------------------------------------------------------*/
		/* Get the total number of lines and the length of the final line.    */
		/*--------------------------------------------------------------------*/
		if (neuik_TextBlock_GetLineCount(te->textBlk, &nLines))
		{
			eNum = 7;
			goto out;
		}
		if (neuik_TextBlock_GetLineLength(te->textBlk,
			nLines - 1, &lineLen))
		{
			/* ERR: problem reported from textBlock */
			eNum = 6;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Only actually highlight the text if there is some text.            */
		/*--------------------------------------------------------------------*/
		if (!(nLines == 1 && lineLen == 0))
		{
			te->highlightIsSet = 1;
			te->cursorLine     = nLines - 1;
			te->cursorPos      = lineLen;
			te->highlightBeginLine = 0;
			te->highlightBeginPos  = 0;
			te->highlightStartLine = te->highlightBeginLine;
			te->highlightStartPos  = te->highlightBeginPos;
			te->highlightEndLine   = te->cursorLine;
			te->highlightEndPos    = te->cursorPos;
			doRedraw = 1;
		}
	}

	if (doRedraw)
	{
		rSize = eBase->eSt.rSize;
		rLoc  = eBase->eSt.rLoc;
		neuik_Element_RequestRedraw(te, rLoc, rSize);
	}
	evCaptured = NEUIK_EVENTSTATE_CAPTURED;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (clipText != NULL) free(clipText);

	return evCaptured;
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
neuik_EventState neuik_Element_CaptureEvent__TextEdit(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	neuik_EventState    evCaptured = NEUIK_EVENTSTATE_NOT_CAPTURED;
	int                 eNum       = 0; /* which error to report (if any) */
	char              * clipText   = NULL;
	NEUIK_TextEdit    * te         = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "neuik_Element_Render__TextEdit";

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
	/* Check if the event is captured by the menu (mouse-click/mouse-motion). */
	/*------------------------------------------------------------------------*/
	switch (ev->type)
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
		evCaptured = neuik_Element_CaptureEvent__TextEdit_MouseEvent(elem, ev);
		break;

	case SDL_TEXTINPUT:
		evCaptured = neuik_Element_CaptureEvent__TextEdit_TextInputEvent(elem, ev);
		break;

	case SDL_KEYDOWN:
		evCaptured = neuik_Element_CaptureEvent__TextEdit_KeyDownEvent(elem, ev);
		break;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}
	if (clipText != NULL) free(clipText);

	return evCaptured;
}
