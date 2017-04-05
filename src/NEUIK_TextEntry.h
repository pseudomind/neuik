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
#ifndef NEUIK_TEXTENTRY_H
#define NEUIK_TEXTENTRY_H

#include <stdarg.h>

#include "NEUIK_defs.h"
#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_TextEntryConfig.h"


typedef struct {
		neuik_Object            objBase;       /* this structure is requied to be an neuik object */
		NEUIK_TextEntryConfig * cfg;
		NEUIK_TextEntryConfig * cfgPtr;        /* if NULL, the non-Pointer version is used */
		void                  * textSurf;      /*  `SDL_Surface *` */ 
		void                  * textTex;       /*  `SDL_Texture *` */ 
		void                  * textRend;      /*  `SDL_Renderer*` */ 
		char                  * text;
		size_t                  textLen;       /* current length of the text */
		size_t                  textAllocSize; /* current mem alloc for text */
		size_t                  cursorPos;     /* position of cursor in text */
		int                     cursorX;       /* px pos of cursor (not considering pan) */
		int                     selected;
		int                     wasSelected;
		size_t                  highlightBegin;
		size_t                  highlightStart;
		size_t                  highlightEnd;
		int                     panX;          /* px of text pan */
		int                     panCursor;     /* px pos of cursor (may cause view to move) */
		int                     isActive;
		size_t                  clickOrigin;   /* cursorPos @ start of select click */
		int                     clickHeld;     /* click being held following select click */
		int                     needsRedraw;
		unsigned int            timeLastClick;
} NEUIK_TextEntry;


int
	NEUIK_NewTextEntry(
			NEUIK_TextEntry ** tePtr);

int
	NEUIK_MakeTextEntry(
			NEUIK_TextEntry ** tePtr,
			const char       * text);

// void 
// 	NEUIK_TextEntry_SetConfig(
// 			NEUIK_TextEntry        * te,
// 			NEUIK_TextEntryConfig  * cfg);

// int 
// 	NEUIK_TextEntry_CopyConfig(
// 			NEUIK_TextEntry        * te,
// 			NEUIK_TextEntryConfig  * cfg);

const char *
	NEUIK_TextEntry_GetText(
			NEUIK_TextEntry  * te);

int 
	NEUIK_TextEntry_SetText(
			NEUIK_TextEntry  * te,
			const char       * text);

int 
	NEUIK_TextEntry_Configure(
			NEUIK_TextEntry  * te,
			const char       * set0,
			...);


#endif /* NEUIK_TEXTENTRY_H */
