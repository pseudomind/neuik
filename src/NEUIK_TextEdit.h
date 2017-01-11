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
#ifndef NEUIK_TEXTEDIT_H
#define NEUIK_TEXTEDIT_H

#include <stdarg.h>

#include "NEUIK_defs.h"
#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "neuik_TextBlock.h"
#include "NEUIK_TextEditConfig.h"


typedef struct {
		neuik_Object           objBase;       /* this structure is requied to be an neuik object */
		NEUIK_TextEditConfig * cfg;
		NEUIK_TextEditConfig * cfgPtr;        /* if NULL, the non-Pointer version is used */
		void                 * textSurf;      /*  `SDL_Surface *` */ 
		void                 * textTex;       /*  `SDL_Texture *` */ 
		void                 * textRend;      /*  `SDL_Renderer*` */ 
		char                 * text;
		neuik_TextBlock      * textBlk;
		unsigned long          textLen;       /* current length of the text */
		int                    textAllocSize; /* current mem alloc for text */
		unsigned int           cursorLine;    /* line on which the cursor is */
		unsigned int           cursorPos;     /* position of cursor within line */
		int                    cursorX;       /* px pos of cursor (not considering pan) */
		int                    selected;
		int                    wasSelected;
		unsigned long          highlightBegin;
		unsigned long          highlightStart;
		unsigned long          highlightEnd;
		int                    panX;          /* px of text pan */
		int                    panCursor;     /* px pos of cursor (may cause view to move) */
		int                    isActive;
		unsigned long          clickOrigin;   /* cursorPos @ start of select click */
		int                    clickHeld;     /* click being held following select click */
		int                    needsRedraw;
		unsigned int           timeLastClick;
} NEUIK_TextEdit;


int
	NEUIK_NewTextEdit(
			NEUIK_TextEdit ** tePtr);

int
	NEUIK_MakeTextEdit(
			NEUIK_TextEdit ** tePtr,
			const char      * text);

const char *
	NEUIK_TextEdit_GetText(
			NEUIK_TextEdit * te);

int 
	NEUIK_TextEdit_SetText(
			NEUIK_TextEdit * te,
			const char     * text);

int 
	NEUIK_TextEdit_Configure(
			NEUIK_TextEdit * te,
			const char     * set0,
			...);


#endif /* NEUIK_TEXTEDIT_H */
