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
#ifndef NEUIK_FONTSET_H
#define NEUIK_FONTSET_H

#include "NEUIK_structs_basic.h"

/*  */
typedef struct {
	int                    Available;
	char                 * FontName;
	unsigned int           MaxSize;  /* Len of TTF_Font array also the largest font size used */
	unsigned int         * NRef;     /* Number of times each FSize is referenced */
	neuik_ptrTo_TTF_Font * Fonts;    /* (TTF_Font **) */
} NEUIK_FontFileSet;

typedef struct {
	char              * BaseFontName;
	NEUIK_FontFileSet   Standard;
	NEUIK_FontFileSet   Bold;
	NEUIK_FontFileSet   Italic;
	NEUIK_FontFileSet   BoldItalic;
} NEUIK_FontSet;


NEUIK_FontSet * 
	NEUIK_NewFontSet(
			const char * fNameBase,
			const char * fNameStd,
			const char * fNameBold,
			const char * fNameItalic,
			const char * fNameBoldItalic);

NEUIK_FontSet * 
	NEUIK_GetFontSet(
			const char * fName);

NEUIK_FontSet * 
	NEUIK_GetDefaultFontSet(
			char ** baseName);

NEUIK_FontSet * 
	NEUIK_GetDefaultMSFontSet(
			char ** baseName);

neuik_ptrTo_TTF_Font 
	NEUIK_FontSet_GetFont(
			NEUIK_FontSet * fs,
			unsigned int    fSize,
			int             useBold,
			int             useItalic);

int 
	NEUIK_GetTTFLocation(
			const char * fName, 
			char ** loc);
int 
	NEUIK_GetBoldTTFLocation(
			const char * fName, 
			char ** loc);
int 
	NEUIK_GetItalicTTFLocation(
			const char * fName, 
			char ** loc);
int 
	NEUIK_GetBoldItalicTTFLocation(
			const char * fName, 
			char ** loc);


#endif /* NEUIK_FONTSET_H */
