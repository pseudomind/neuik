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
#ifndef NEUIK_PROGRESSBAR_CONFIG_H
#define NEUIK_PROGRESSBAR_CONFIG_H

#include "NEUIK_structs_basic.h"
#include "NEUIK_FontSet.h"
#include "neuik_internal.h"

typedef struct {
		neuik_Object       objBase;         /* this structure is requied to be an neuik object */
		NEUIK_FontSet    * fontSet;         /* NEUIK_FontSet */
		int                fontSize;        /* point size to use for the TTF_Font */
		int                fontBold;        /* (bool) use bold style */
		int                fontItalic;      /* (bool) use italic style */
		char             * fontName;        /* font name for the TTF_Font */
		NEUIK_ColorStop ** gradCS;
		NEUIK_Color        bgColor;         /* standard background color */
		NEUIK_Color        fgColor;         /* standard text color */
		NEUIK_Color        progColorLight;  /* lighter color for progress bar */
		NEUIK_Color        progColorDark;   /* darker color for progress bar */
		NEUIK_Color        borderColor;     /* color used for the element border */
		NEUIK_Color        borderColorDark; /* color used for the element border */
		int                fontEmWidth;     /* nuber of pixels required for an `M` of this font */
		unsigned int       decimalPlaces;   /* number of decimal places for the % value */
} NEUIK_ProgressBarConfig;

NEUIK_ProgressBarConfig * 
	NEUIK_GetDefaultProgressBarConfig();

int 
	NEUIK_NewProgressBarConfig(
			NEUIK_ProgressBarConfig ** cfgPtr);

int 
	NEUIK_ProgressBarConfig_Copy(
			NEUIK_ProgressBarConfig       * dst,
			const NEUIK_ProgressBarConfig * src);

#endif /* NEUIK_PROGRESSBAR_CONFIG_H */
