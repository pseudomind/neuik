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
#include <stdio.h>
#include <string.h>

#include "NEUIK_error.h"
#include "NEUIK_TextEntry.h"
#include "NEUIK_TextEntry_internal.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Window_internal.h"
#include "NEUIK_platform.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

static char * errMsgs[]  = {"",                                      // [0] no error
	"FontSet_GetFont returned NULL."                                 // [1]
	"Failed to get text from clipboard."                             // [2]
	"Argument `elem` is not of TextEntry class.",                    // [3]
	"Argument `elem` caused `neuik_Object_GetClassObject` to fail.", // [4]
};


/*******************************************************************************
 *
 *  Name:          neuik_TextEntry_UpdatePanCursor
 *
 *  Description:   Update the `te->panCursor` and maybe `te->cursorX`.
 *
 *  Returns:       A non-zero integer if there is an error.
 *
 ******************************************************************************/
int neuik_TextEntry_UpdatePanCursor(
	NEUIK_TextEntry  * te,
	int                cursorChange)
{
	int                     eNum       = 0; /* which error to report (if any) */
	int                     textW      = 0;
	int                     textH      = 0;
	int                     normWidth  = 0;
	char                    tempChar;
	TTF_Font              * font       = NULL;
	NEUIK_ElementBase     * eBase      = NULL;
	NEUIK_TextEntryConfig * aCfg       = NULL; /* the active textEntry config */
	static char             funcName[] = "neuik_TextEntry_UpdatePanCursor";
	static char           * errMsgs[] = {"",                           // [0] no error
		"Argument `te` is not of TextEntry class.",                    // [1]
		"Argument `te` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"FontSet_GetFont returned NULL.",                              // [3]
	};

	if (!neuik_Object_IsClass(te, neuik__Class_TextEntry))
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
	/* currently visible TextEntry field.                                     */
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
					/* show, as a result, reduce panCursor so that TextEntry  */
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
 *  Name:          neuik_Element_CaptureEvent__TextEntry_MouseEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEntry.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEntry_MouseEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	neuik_EventState        evCaptured   = NEUIK_EVENTSTATE_NOT_CAPTURED;
	int                     textW        = 0;
	int                     textH        = 0;
	int                     charW        = 0;
	int                     doContinue   = 0;
	int                     eNum         = 0; /* which error to report (if any) */
	int                     lastW        = 0; /* position of previous char */
	int                     normWidth    = 0;
	size_t                  oldCursorPos = 0;
	size_t                  ctr;
	char                    aChar;
	char                  * clipText     = NULL;
	TTF_Font              * font         = NULL;
	SDL_Rect                rect         = {0, 0, 0 ,0};
	SDL_Keymod              keyMod;
	RenderSize            * rSize;
	SDL_MouseMotionEvent  * mouseMotEv;
	SDL_MouseButtonEvent  * mouseButEv;
	NEUIK_TextEntry       * te         = NULL;
	NEUIK_TextEntryConfig * aCfg       = NULL; /* the active button config */
	NEUIK_ElementBase     * eBase      = NULL;
	static char             funcName[] = "neuik_Element_CaptureEvent__TextEntry_MouseEvent";

	if (!neuik_Object_IsClass(elem, neuik__Class_TextEntry))
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEntry*)elem;
	if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Redirect the MouseEvent to the appropriate handling section            */
	/*------------------------------------------------------------------------*/
	if (ev->type == SDL_MOUSEBUTTONDOWN)
	{
		if (!eBase->eSt.hasFocus)
		{
			/*----------------------------------------------------------------*/
			/* This text entry does not currently have the window focus       */
			/*----------------------------------------------------------------*/
			mouseButEv = (SDL_MouseButtonEvent*)(ev);
			if (mouseButEv->y >= eBase->eSt.rLoc.y && 
				mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseButEv->x >= eBase->eSt.rLoc.x && 
					mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* This mouse click originated within this button */
					te->selected    = 1;
					te->wasSelected = 1;
					neuik_Window_TakeFocus(eBase->eSt.window, (NEUIK_Element)te);
					SDL_StartTextInput();
					neuik_Element_RequestRedraw((NEUIK_Element)te);
					evCaptured = NEUIK_EVENTSTATE_CAPTURED;
				}
				else
				{
					goto out;
				}
			}
		}

		/*--------------------------------------------------------------------*/
		/* This text entry currently has the window focus                     */
		/*--------------------------------------------------------------------*/
		mouseButEv = (SDL_MouseButtonEvent*)(ev);
		if (mouseButEv->y >= eBase->eSt.rLoc.y && 
			mouseButEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
		{
			if (mouseButEv->x >= eBase->eSt.rLoc.x && 
				mouseButEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
			{
				/* This mouse click originated within this textEntry */
				doContinue = 1;
				evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			}
		}

		if (!doContinue) goto out;
		doContinue = 0;
		/*--------------------------------------------------------------------*/
		/* Otherwise, a subsequent click was within the textEntry element     */
		/* For this situation, we want to move the textEdit cursor.           */
		/*--------------------------------------------------------------------*/

		/*--------------------------------------------------------------------*/
		/* select the correct textEntry config to use (ptr or internal)       */
		/*--------------------------------------------------------------------*/
		aCfg = te->cfgPtr;
		if (aCfg == NULL)  aCfg = te->cfg;  /* Fallback to internal config */

		rSize = &(eBase->eSt.rSize);

		/*--------------------------------------------------------------------*/
		/* Get the overall location of the current text                       */
		/*--------------------------------------------------------------------*/
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
			/*----------------------------------------------------------------*/
			/* If continuing, this textEntry contains text and so the cursor  */
			/* placement could have been changed.                             */
			/*----------------------------------------------------------------*/
			if (SDL_GetTicks() - te->timeLastClick < NEUIK_DOUBLE_CLICK_TIMEOUT)
			{
				if (te->textLen > 0)
				{
					te->highlightBegin = 0;
					te->cursorPos      = te->textLen;

					te->highlightStart = 0;
					te->highlightEnd   = te->textLen - 1;
				}
			}
			else if (te->panCursor == 0 && mouseButEv->x <= eBase->eSt.rLoc.x + rect.x)
			{
				/* move the cursor position all the way to the start */
				te->cursorPos      = 0;
				te->highlightBegin = -1; /* unhighlight text */
				neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
			}
			else if (mouseButEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
			{
				/* move the cursor position all the way to the end */
				te->cursorPos      = te->textLen;
				te->highlightBegin = -1; /* unhighlight text */
				neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
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

							/*------------------------------------------------*/
							/* Update the cursor Panning (if necessary)       */
							/*------------------------------------------------*/
							if (oldCursorPos > te->cursorPos)
							{
								neuik_TextEntry_UpdatePanCursor(te, 
									CURSORPAN_MOVE_BACK);
							}
							else
							{
								neuik_TextEntry_UpdatePanCursor(te, 
									CURSORPAN_MOVE_FORWARD);
							}
							break;
						}
						lastW = textW;
						if (aChar == '\0') break;
					}
					te->text[ctr] = aChar;
					te->highlightBegin = -1; /* unhighlight text */
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
					te->highlightBegin = -1; /* unhighlight text */
				}
			}
			te->clickOrigin   = te->cursorPos;
			te->timeLastClick = SDL_GetTicks();
		}
		else
		{
			/* The shift-key IS being held down */
			/*----------------------------------------------------------------*/
			/* If continuing, this textEntry contains text and so the cursor  */
			/* placement could have been changed.                             */
			/*----------------------------------------------------------------*/
			if (te->clickOrigin == -1)
			{
				if (te->highlightBegin == -1)
				{
					te->clickOrigin = te->cursorPos;
				}
				else
				{
					te->clickOrigin = te->highlightBegin;
				}
			}
			// te->highlightBegin = te->cursorPos;
			if (te->panCursor == 0 && mouseButEv->x <= eBase->eSt.rLoc.x + rect.x)
			{
				/* move the cursor position all the way to the start */
				te->cursorPos      = 0;
				neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
			}
			else if (mouseButEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
			{
				/* move the cursor position all the way to the end */
				te->cursorPos      = te->textLen;
				neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
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

							/*------------------------------------------------*/
							/* Update the cursor Panning (if necessary)       */
							/*------------------------------------------------*/
							if (oldCursorPos > te->cursorPos)
							{
								neuik_TextEntry_UpdatePanCursor(te, 
									CURSORPAN_MOVE_BACK);
							}
							else
							{
								neuik_TextEntry_UpdatePanCursor(te, 
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
			te->highlightBegin = te->clickOrigin;
			if (te->cursorPos < te->clickOrigin)
			{
				te->highlightStart = te->cursorPos;
				te->highlightEnd   = te->clickOrigin - 1;
			}
			else
			{
				te->highlightStart = te->clickOrigin;
				te->highlightEnd   = te->cursorPos - 1;
			}
		}

		neuik_Element_RequestRedraw((NEUIK_Element)te);
		evCaptured = NEUIK_EVENTSTATE_CAPTURED;

		te->clickHeld   = 1;
	}
	else if (ev->type == SDL_MOUSEBUTTONUP)
	{
		if (eBase->eSt.hasFocus)
		{
			/*----------------------------------------------------------------*/
			/* This text entry has the window focus (unset `clickHeld`)       */
			/*----------------------------------------------------------------*/
			te->clickHeld =  0;
			evCaptured    = NEUIK_EVENTSTATE_CAPTURED;
		}
		goto out;
	}
	else if (ev->type == SDL_MOUSEMOTION)
	{
		if (eBase->eSt.hasFocus && te->clickHeld)
		{
			/*----------------------------------------------------------------*/
			/* This text entry currently has the window focus and the mouse   */
			/* button is still being held down. **Drag Select**               */
			/*----------------------------------------------------------------*/
			mouseMotEv = (SDL_MouseMotionEvent*)(ev);
			if (mouseMotEv->y >= eBase->eSt.rLoc.y &&
				mouseMotEv->y <= eBase->eSt.rLoc.y + eBase->eSt.rSize.h)
			{
				if (mouseMotEv->x >= eBase->eSt.rLoc.x &&
					mouseMotEv->x <= eBase->eSt.rLoc.x + eBase->eSt.rSize.w)
				{
					/* This mouse click originated within this button */
					doContinue = 1;
					evCaptured = NEUIK_EVENTSTATE_CAPTURED;
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
				te->highlightBegin = -1; /* unhighlight text */
			}
			else if (mouseMotEv->x >= eBase->eSt.rLoc.x + rect.x + rect.w)
			{
				/* move the cursor position all the way to the end */
				te->cursorPos      = te->textLen;
				te->highlightBegin = -1; /* unhighlight text */
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
								neuik_TextEntry_UpdatePanCursor(te, 
									CURSORPAN_MOVE_BACK);
							}
							else
							{
								neuik_TextEntry_UpdatePanCursor(te, 
									CURSORPAN_MOVE_FORWARD);
							}
							break;
						}
						lastW = textW;
						if (aChar == '\0') break;
					}
					te->text[ctr] = aChar;
					te->highlightBegin = -1; /* unhighlight text */
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
					te->highlightBegin = -1; /* unhighlight text */
				}
			}

			/* Set text highlight (if applicable) */
			te->highlightBegin = te->clickOrigin;
			if (te->cursorPos < te->clickOrigin)
			{
				te->highlightStart = te->cursorPos;
				te->highlightEnd   = te->clickOrigin - 1;
			}
			else
			{
				te->highlightStart = te->clickOrigin;
				te->highlightEnd   = te->cursorPos - 1;
			}

			neuik_Element_RequestRedraw((NEUIK_Element)te);
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
		}
		goto out;
	}
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
 *  Name:          neuik_Element_CaptureEvent__TextEntry_TextInputEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEntry.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEntry_TextInputEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	neuik_EventState     evCaptured   = NEUIK_EVENTSTATE_NOT_CAPTURED;
	int                  eNum         = 0; /* which error to report (if any) */
	size_t               inpLen       = 0;  /* length of text input */
	size_t               newSize      = 0;  /* realloated text buf size */
	size_t               stopPos;
	size_t               hlOffset;         /* highlight offset (for copy) */
	size_t               aPos;
	size_t               ctr;
	char                 aChar;
	char               * clipText     = NULL;
	SDL_TextInputEvent * textInpEv;
	NEUIK_TextEntry    * te         = NULL;
	NEUIK_ElementBase  * eBase      = NULL;
	static char          funcName[] = "neuik_Element_CaptureEvent__TextEntry_TextInputEvent";

	if (!neuik_Object_IsClass(elem, neuik__Class_TextEntry))
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEntry*)elem;
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

	if (te->highlightBegin != -1)
	{
		/*--------------------------------------------------------------------*/
		/* Existing text was highlighted when text input was received.        */
		/* This will result in the highlighted text being replaced.           */
		/*--------------------------------------------------------------------*/
		if (te->highlightStart == 0)
		{
			/*----------------------------------------------------------------*/
			/* a block of text will be deleted, (block @ start)               */
			/*----------------------------------------------------------------*/
			if (te->highlightEnd + 1 != te->textLen)
			{
				/* we are not deleting the entire contents */

				for (ctr = 0;; ctr++)
				{
					aChar = te->text[ctr + te->highlightEnd + 1];
					te->text[ctr] = aChar;

					if (aChar == '\0') break;
				}
				te->textLen = strlen(te->text);
			}
			else
			{
				/* delete entire contents of the string */
				te->textLen = 0;
				te->text[0] = '\0';
			}
			te->cursorPos = 0;
		}
		else if (te->highlightEnd + 1 == te->textLen)
		{
			/*----------------------------------------------------------------*/
			/* a block of text will be deleted, (block @ end)                 */
			/*----------------------------------------------------------------*/
			te->text[te->highlightStart] = '\0';
			te->textLen   = te->highlightStart;
			te->cursorPos = te->textLen;
		}
		else
		{
			/*----------------------------------------------------------------*/
			/* a block of text will be deleted, (block in middle)             */
			/*----------------------------------------------------------------*/
			te->cursorPos = te->highlightStart;

			hlOffset = 1 + (te->highlightEnd - te->highlightStart);
			for (ctr = te->highlightStart;; ctr++)
			{
				aChar = te->text[ctr + hlOffset];
				te->text[ctr] = aChar;

				if (aChar == '\0') break;
			}
			te->textLen = strlen(te->text);
		}

		te->cursorPos = te->highlightStart;
		te->highlightBegin = -1;
	}

	inpLen = strlen(textInpEv->text);
	if (te->cursorPos == te->textLen)
	{
		/* cursor is at the end of the current text */
		if (inpLen + te->textLen < te->textAllocSize)
		{
			/* the text buffer will need to be resized to fit this text */
			newSize = 2 * (inpLen + te->textLen);
			te->text = (char*)realloc(te->text, newSize*sizeof(char));
		}
		strcat(te->text, textInpEv->text);
		te->textLen   += inpLen;
		te->cursorPos += inpLen;
	}
	else if (te->cursorPos == 0)
	{
		/* cursor is at the start of the current text */
		if (inpLen + te->textLen < te->textAllocSize)
		{
			/* the text buffer will need to be resized to fit this text */
			newSize = 2 * (inpLen + te->textLen);
			te->text = (char*)realloc(te->text, newSize*sizeof(char));
		}

		/* first move over the old text */
		for (ctr = te->textLen + inpLen; ctr >= inpLen; ctr--)
		{
			te->text[ctr] = te->text[ctr - inpLen];
		}

		/* now copy in the new text */
		for (ctr = 0;; ctr++)
		{
			if (textInpEv->text[ctr] == 0) break;

			te->text[ctr] = textInpEv->text[ctr];
		}
		te->textLen   += inpLen;
		te->cursorPos += inpLen;
	}
	else
	{
		/* cursor is somewhere in the middle of the text */
		if (inpLen + te->textLen < te->textAllocSize)
		{
			/* the text buffer will need to be resized to fit this text */
			newSize = 2 * (inpLen + te->textLen);
			te->text = (char*)realloc(te->text, newSize*sizeof(char));
		}

		/* first move over the old text */
		stopPos = (te->cursorPos - 1) + inpLen; 
		for (ctr = te->textLen + inpLen; ctr >= stopPos; ctr--)
		{
			te->text[ctr] = te->text[ctr - inpLen];
		}

		/* now copy in the new text */
		for (ctr = 0;; ctr++)
		{
			aPos = te->cursorPos + ctr;
			if (textInpEv->text[ctr] == 0) break;

			te->text[aPos] = textInpEv->text[ctr];
		}
		te->textLen   += inpLen;
		te->cursorPos += inpLen;
	}

	neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_TEXT_INSERTED);
	neuik_Element_RequestRedraw((NEUIK_Element)te);
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
 *  Name:          neuik_Element_CaptureEvent__TextEntry_KeyDownEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEntry.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEntry_KeyDownEvent(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	int                 evCaptured   = 0;
	int                 doRedraw     = 0;
	int                 eNum         = 0; /* which error to report (if any) */
	size_t              inpLen       = 0;  /* length of text input */
	size_t              newSize      = 0;  /* realloated text buf size */
	size_t              stopPos;
	size_t              hlOffset;         /* highlight offset (for copy) */
	size_t              aPos;
	size_t              ctr;
	char                aChar;
	char              * clipText     = NULL;
	SDL_Keymod          keyMod;
	SDL_KeyboardEvent * keyEv;
	NEUIK_TextEntry   * te         = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "neuik_Element_CaptureEvent__TextEntry_KeyDownEvent";

	if (!neuik_Object_IsClass(elem, neuik__Class_TextEntry))
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEntry*)elem;
	if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	if (!eBase->eSt.hasFocus)
	{
		goto out;
	}
	doRedraw = 0;

	keyEv  = (SDL_KeyboardEvent*)(ev);
	keyMod = SDL_GetModState();
	switch (keyEv->keysym.sym)
	{
		case SDLK_LEFT:
			if (!(keyMod & KMOD_SHIFT))
			{
				/* SHIFT key is not being held down */
				if (te->highlightBegin != -1)
				{
					/* breaking out of a highlight selection */
					if (te->cursorPos > te->highlightBegin)
					{
						/* break out at leftmost side of highlight */
						te->cursorPos = te->highlightBegin;
					}
					te->highlightBegin = -1;
				}
				else if (te->cursorPos > 0)
				{
					te->cursorPos--;
				}
				doRedraw        = 1;
				evCaptured      = NEUIK_EVENTSTATE_CAPTURED;
				te->clickOrigin = -1;
			}
			else
			{
				/* SHIFT key is being held down */

				/* Start highlight selection process */
				if (te->cursorPos > 0)
				{
					doRedraw = 1;

					if (te->highlightBegin == -1)
					{
						te->highlightBegin = te->cursorPos;
					}
					te->cursorPos--;

					if (te->cursorPos < te->highlightBegin)
					{
						te->highlightStart = te->cursorPos;
						te->highlightEnd   = te->highlightBegin - 1;
					}
					else
					{
						te->highlightStart = te->highlightBegin;
						te->highlightEnd   = te->cursorPos - 1;
					}
				}
			}
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
			break;

		case SDLK_RIGHT:
			if (!(keyMod & KMOD_SHIFT))
			{
				/* SHIFT key is not being held down */

				if (te->highlightBegin != -1)
				{
					/* breaking out of a highlight selection */
					if (te->cursorPos < te->highlightBegin)
					{
						/* break out at rightmost side of highlight */
						te->cursorPos = te->highlightBegin;
					}
					te->highlightBegin = -1;
				}
				else if (te->cursorPos < te->textLen)
				{
					te->cursorPos++;
				}
				doRedraw        = 1;
				evCaptured      = NEUIK_EVENTSTATE_CAPTURED;
				te->clickOrigin = -1;
			}
			else
			{
				/* SHIFT key is being held down */

				/* Start highlight selection process */
				if (te->cursorPos < te->textLen)
				{
					if (te->highlightBegin == -1)
					{
						te->highlightBegin = te->cursorPos;
					}

					te->cursorPos++;
					doRedraw = 1;

					if (te->cursorPos > te->highlightBegin)
					{
						te->highlightStart = te->highlightBegin;
						te->highlightEnd   = te->cursorPos - 1;
					}
					else
					{
						te->highlightStart = te->cursorPos;
						te->highlightEnd   = te->highlightBegin - 1;
					}
				}
			}
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
			break;

		case SDLK_BACKSPACE:
			if (te->highlightBegin == -1)
			{
				/*--------------------------------------------------------*/
				/* There is no current text highlighting                  */
				/*--------------------------------------------------------*/
				if (te->cursorPos > 0)
				{
					strcpy(te->text + (te->cursorPos - 1), te->text + (te->cursorPos));
					te->textLen   -= 1;
					te->cursorPos -= 1;
					doRedraw = 1;
				}
			}
			else
			{
				/*--------------------------------------------------------*/
				/* There is text highlighting within the line             */
				/*--------------------------------------------------------*/
				if (te->highlightStart == 0)
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block @ start)   */
					/*----------------------------------------------------*/
					if (te->highlightEnd + 1 != te->textLen)
					{
						/* we are not deleting the entire contents */

						for (ctr = 0;; ctr++)
						{
							aChar = te->text[ctr + te->highlightEnd + 1];
							te->text[ctr] = aChar;

							if (aChar == '\0') break;
						}
						te->textLen = strlen(te->text);
					}
					else
					{
						/* delete entire contents of the string */
						te->textLen = 0;
						te->text[0] = '\0';
					}
					te->cursorPos      = 0;
				}
				else if (te->highlightEnd + 1 == te->textLen)
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block @ end)     */
					/*----------------------------------------------------*/
					te->text[te->highlightStart] = '\0';
					te->textLen   = te->highlightStart;
					te->cursorPos = te->textLen;
				}
				else
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block in middle) */
					/*----------------------------------------------------*/
					te->cursorPos = te->highlightStart;

					hlOffset = 1 + (te->highlightEnd - te->highlightStart);
					for (ctr = te->highlightStart;; ctr++)
					{
						aChar = te->text[ctr + hlOffset];
						te->text[ctr] = aChar;

						if (aChar == '\0') break;
					}
					te->textLen = strlen(te->text);
				}

				te->highlightBegin = -1;
				doRedraw           = 1;
			}
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
			break;

		case SDLK_DELETE:
			if (te->highlightBegin == -1)
			{
				/*--------------------------------------------------------*/
				/* There is no current text highlighting                  */
				/*--------------------------------------------------------*/
				if (te->cursorPos < te->textLen)
				{
					strcpy(te->text + te->cursorPos, te->text + (te->cursorPos + 1));
					te->textLen -= 1;
					doRedraw     = 1;
				}
			}
			else
			{
				/*--------------------------------------------------------*/
				/* There is text highlighting within the line             */
				/*--------------------------------------------------------*/
				if (te->highlightStart == 0)
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block @ start)   */
					/*----------------------------------------------------*/
					if (te->highlightEnd + 1 != te->textLen)
					{
						/* we are not deleting the entire contents */

						for (ctr = 0;; ctr++)
						{
							aChar = te->text[ctr + te->highlightEnd + 1];
							te->text[ctr] = aChar;

							if (aChar == '\0') break;
						}
						te->textLen = strlen(te->text);
					}
					else
					{
						/* delete entire contents of the string */
						te->textLen = 0;
						te->text[0] = '\0';
					}
					te->cursorPos      = 0;
				}
				else if (te->highlightEnd + 1 == te->textLen)
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block @ end)     */
					/*----------------------------------------------------*/
					te->text[te->highlightStart] = '\0';
					te->textLen   = te->highlightStart;
					te->cursorPos = te->textLen;
				}
				else
				{
					/*----------------------------------------------------*/
					/* a block of text will be deleted, (block in middle) */
					/*----------------------------------------------------*/
					te->cursorPos = te->highlightStart;

					hlOffset = 1 + (te->highlightEnd - te->highlightStart);
					for (ctr = te->highlightStart;; ctr++)
					{
						aChar = te->text[ctr + hlOffset];
						te->text[ctr] = aChar;

						if (aChar == '\0') break;
					}
					te->textLen = strlen(te->text);
				}

				te->highlightBegin = -1;
				doRedraw           = 1;
			}
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
			break;

		case SDLK_UP:
		case SDLK_HOME:
			/* Move the cursor to the start of the line of text */
			if (te->cursorPos > 0)
			{
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					te->highlightBegin = -1;
					te->clickOrigin    = -1;
					te->cursorPos      = 0;
					doRedraw           = 1;
				}
				else
				{
					/* SHIFT key IS being held down */
					if (te->highlightBegin == -1)
					{
						te->highlightBegin = te->cursorPos;
					}
					te->cursorPos = 0;
					doRedraw = 1;

					if (te->cursorPos < te->highlightBegin)
					{
						te->highlightStart = te->cursorPos;
						te->highlightEnd   = te->highlightBegin - 1;
					}
					else
					{
						te->highlightStart = te->highlightBegin;
						te->highlightEnd   = te->cursorPos - 1;
					}
				}
			}
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_BACK);
			break;

		case SDLK_DOWN:
		case SDLK_END:
			/* Move the cursor to the end of the line of text */
			if (te->cursorPos < te->textLen)
			{
				if (!(keyMod & KMOD_SHIFT))
				{
					/* SHIFT key is not being held down */
					te->highlightBegin = -1;
					te->clickOrigin    = -1;
					te->cursorPos      = te->textLen;
					doRedraw           = 1;
				}
				else
				{
					if (te->highlightBegin == -1)
					{
						te->highlightBegin = te->cursorPos;
					}
					te->cursorPos = te->textLen;
					doRedraw = 1;

					if (te->cursorPos > te->highlightBegin)
					{
						te->highlightStart = te->highlightBegin;
						te->highlightEnd   = te->cursorPos - 1;
					}
					else
					{
						te->highlightStart = te->cursorPos;
						te->highlightEnd   = te->highlightBegin - 1;
					}
				}
			}
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);
			// neuik_TextEntry_UpdateCursorX(te);
			break;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			/* send the activate trigger */
			neuik_Element_TriggerCallback(te, NEUIK_CALLBACK_ON_ACTIVATED);
			break;
	}

	if (neuik_KeyShortcut_Copy(keyEv, keyMod))
	{
		if (te->highlightBegin != -1)
		{
			aChar = te->text[te->highlightEnd + 1];
			te->text[te->highlightEnd + 1] = '\0';

			SDL_SetClipboardText(te->text + te->highlightStart);
			te->text[te->highlightEnd + 1] = aChar;
		}
	}
	else if (neuik_KeyShortcut_Cut(keyEv, keyMod))
	{
		if (te->highlightBegin != -1)
		{
			aChar = te->text[te->highlightEnd + 1];
			te->text[te->highlightEnd + 1] = '\0';

			SDL_SetClipboardText(te->text + te->highlightStart);
			te->text[te->highlightEnd + 1] = aChar;

			/*--------------------------------------------------------*/
			/* There is text highlighting within the line             */
			/*--------------------------------------------------------*/
			if (te->highlightStart == 0)
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block @ start)   */
				/*----------------------------------------------------*/
				if (te->highlightEnd + 1 != te->textLen)
				{
					/* we are not deleting the entire contents */

					for (ctr = 0;; ctr++)
					{
						aChar = te->text[ctr + te->highlightEnd + 1];
						te->text[ctr] = aChar;

						if (aChar == '\0') break;
					}
					te->textLen = strlen(te->text);
				}
				else
				{
					/* delete entire contents of the string */
					te->textLen = 0;
					te->text[0] = '\0';
				}
				te->cursorPos      = 0;
			}
			else if (te->highlightEnd + 1 == te->textLen)
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block @ end)     */
				/*----------------------------------------------------*/
				te->text[te->highlightStart] = '\0';
				te->textLen   = te->highlightStart;
				te->cursorPos = te->textLen;
			}
			else
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block in middle) */
				/*----------------------------------------------------*/
				te->cursorPos = te->highlightStart;

				hlOffset = 1 + (te->highlightEnd - te->highlightStart);
				for (ctr = te->highlightStart;; ctr++)
				{
					aChar = te->text[ctr + hlOffset];
					te->text[ctr] = aChar;

					if (aChar == '\0') break;
				}
				te->textLen = strlen(te->text);
			}

			te->highlightBegin = -1;
			doRedraw           = 1;
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_TEXT_DELTETED);
		}
	}
	else if (neuik_KeyShortcut_Paste(keyEv, keyMod) && SDL_HasClipboardText())
	{
		if (te->highlightBegin != -1)
		{
			/*--------------------------------------------------------*/
			/* There is text highlighting within the line             */
			/*--------------------------------------------------------*/
			if (te->highlightStart == 0)
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block @ start)   */
				/*----------------------------------------------------*/
				if (te->highlightEnd + 1 != te->textLen)
				{
					/* we are not deleting the entire contents */

					for (ctr = 0;; ctr++)
					{
						aChar = te->text[ctr + te->highlightEnd + 1];
						te->text[ctr] = aChar;

						if (aChar == '\0') break;
					}
					te->textLen = strlen(te->text);
				}
				else
				{
					/* delete entire contents of the string */
					te->textLen = 0;
					te->text[0] = '\0';
				}
				te->cursorPos      = 0;
			}
			else if (te->highlightEnd + 1 == te->textLen)
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block @ end)     */
				/*----------------------------------------------------*/
				te->text[te->highlightStart] = '\0';
				te->textLen   = te->highlightStart;
				te->cursorPos = te->textLen;
			}
			else
			{
				/*----------------------------------------------------*/
				/* a block of text will be deleted, (block in middle) */
				/*----------------------------------------------------*/
				te->cursorPos = te->highlightStart;

				hlOffset = 1 + (te->highlightEnd - te->highlightStart);
				for (ctr = te->highlightStart;; ctr++)
				{
					aChar = te->text[ctr + hlOffset];
					te->text[ctr] = aChar;

					if (aChar == '\0') break;
				}
				te->textLen = strlen(te->text);
			}

			te->highlightBegin = -1;
		}

		clipText = SDL_GetClipboardText();
		if (clipText == NULL)
		{
			evCaptured = NEUIK_EVENTSTATE_CAPTURED;
			eNum = 2;
			goto out;
		}

		inpLen = strlen(clipText);
		if (te->cursorPos == te->textLen)
		{
			/* cursor is at the end of the current text */
			if (inpLen + te->textLen < te->textAllocSize)
			{
				/* the text buffer will need to be resized to fit this text */
				newSize = 2 * (inpLen + te->textLen);
				te->text = (char*)realloc(te->text, newSize*sizeof(char));
			}
			strcat(te->text, clipText);
			te->textLen   += inpLen;
			te->cursorPos += inpLen;
		}
		else if (te->cursorPos == 0)
		{
			/* cursor is at the start of the current text */
			if (inpLen + te->textLen < te->textAllocSize)
			{
				/* the text buffer will need to be resized to fit this text */
				newSize = 2 * (inpLen + te->textLen);
				te->text = (char*)realloc(te->text, newSize*sizeof(char));
			}

			/* first move over the old text */
			for (ctr = te->textLen + inpLen; ctr >= inpLen; ctr--)
			{
				te->text[ctr] = te->text[ctr - inpLen];
			}

			/* now copy in the new text */
			for (ctr = 0;; ctr++)
			{
				if (clipText[ctr] == 0) break;

				te->text[ctr] = clipText[ctr];
			}
			te->textLen   += inpLen;
			te->cursorPos += inpLen;
		}
		else
		{
			/* cursor is somewhere in the middle of the text */
			if (inpLen + te->textLen < te->textAllocSize)
			{
				/* the text buffer will need to be resized to fit this text */
				newSize = 2 * (inpLen + te->textLen);
				te->text = (char*)realloc(te->text, newSize*sizeof(char));
			}

			/* first move over the old text */
			stopPos = (te->cursorPos - 1) + inpLen; 
			for (ctr = te->textLen + inpLen; ctr >= stopPos; ctr--)
			{
				te->text[ctr] = te->text[ctr - inpLen];
			}

			/* now copy in the new text */
			for (ctr = 0;; ctr++)
			{
				aPos = te->cursorPos + ctr;
				if (clipText[ctr] == 0) break;

				te->text[aPos] = clipText[ctr];
			}
			te->textLen   += inpLen;
			te->cursorPos += inpLen;
		}

		// neuik_TextEntry_UpdateCursorX(te);
		neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_TEXT_ADD_REMOVE);
		neuik_Element_RequestRedraw((NEUIK_Element)te);
		evCaptured = NEUIK_EVENTSTATE_CAPTURED;
		goto out;

	}
	else if (neuik_KeyShortcut_SelectAll(keyEv, keyMod))
	{
		if (te->textLen > 0)
		{
			te->highlightBegin = 0;
			te->cursorPos      = te->textLen;
			// neuik_TextEntry_UpdateCursorX(te);
			neuik_TextEntry_UpdatePanCursor(te, CURSORPAN_MOVE_FORWARD);

			te->highlightStart = 0;
			te->highlightEnd   = te->textLen - 1;
			doRedraw = 1;
		}
	}

	if (doRedraw) neuik_Element_RequestRedraw((NEUIK_Element)te);
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
 *  Name:          neuik_Element_CaptureEvent__TextEntry
 *
 *  Description:   Check to see if this event is captured by a NEUIK_TextEntry.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__TextEntry(
	NEUIK_Element   elem,
	SDL_Event     * ev)
{
	neuik_EventState    evCaptured = NEUIK_EVENTSTATE_NOT_CAPTURED;
	int                 eNum       = 0; /* which error to report (if any) */
	char              * clipText   = NULL;
	SDL_Event         * e;
	NEUIK_TextEntry   * te         = NULL;
	NEUIK_ElementBase * eBase      = NULL;
	static char         funcName[] = "neuik_Element_CaptureEvent__TextEntry";

	if (!neuik_Object_IsClass(elem, neuik__Class_TextEntry))
	{
		eNum = 3;
		goto out;
	}
	te = (NEUIK_TextEntry*)elem;
	if (neuik_Object_GetClassObject(te, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
		evCaptured = neuik_Element_CaptureEvent__TextEntry_MouseEvent(elem, ev);
		break;

	case SDL_TEXTINPUT:
		evCaptured = neuik_Element_CaptureEvent__TextEntry_TextInputEvent(elem, ev);
		break;

	case SDL_KEYDOWN:
		evCaptured = neuik_Element_CaptureEvent__TextEntry_KeyDownEvent(elem, ev);
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

