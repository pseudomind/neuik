/*******************************************************************************
 * Copyright (c) 2014-2021, Michael Leimon <mike@leimon.net>
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
		neuik_TextBlock      * textBlk;
		double                 scrollPct;     /* percent of total TextEdit lines scrolled from top */
		double                 viewPct;       /* percent of total TextEdit lines currently viewed */
		size_t                 cursorLine;    /* line on which the cursor is */
		size_t                 cursorPos;     /* position of cursor within line */
		size_t                 vertMovePos;   /* desired position of vertical movement (-1=unset) */
		unsigned long long     vertPanLn;     /* Vertical number of lines the view is panned */
		unsigned int           vertPanPx;     /* Additional pixels of vertical view panning */
		int                    cursorX;       /* px pos of cursor (not considering pan) */
		int                    lastMouseX;    /* X-position of associated last mouse event */
		int                    lastMouseY;    /* Y-position of associated last mouse event */
		int                    selected;
		int                    wasSelected;
		int                    highlightIsSet;
		size_t                 highlightBeginPos;
		size_t                 highlightBeginLine;
		size_t                 highlightStartPos;
		size_t                 highlightStartLine;
		size_t                 highlightEndPos;
		size_t                 highlightEndLine;
		int                    panX;          /* px of text pan */
		int                    panCursor;     /* px pos of cursor (may cause view to move) */
		int                    isActive;
		size_t                 clickOrigin;     /* cursorPos @ start of select click */
		size_t                 clickOriginLine; /* cursorLine @ start of select click */
		int                    clickHeld;       /* click being held following select click */
		int                    needsRedraw;
		unsigned int           timeLastClick;
		unsigned int           timeClickMinus2; /* time at which the penultimate */
												/* preceding click was clicked.*/
} NEUIK_TextEdit;


int
	NEUIK_NewTextEdit(
			NEUIK_TextEdit ** tePtr);

int
	NEUIK_MakeTextEdit(
			NEUIK_TextEdit ** tePtr,
			const char      * text);

int 
	NEUIK_TextEdit_GetText(
			NEUIK_TextEdit  * te,
			char           ** textPtr);

int
	NEUIK_TextEdit_GetHighlightInfo(
			NEUIK_TextEdit * te,
			size_t         * nLines,
			size_t         * nChars);

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
