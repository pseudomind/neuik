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
#ifndef NEUIK_COMBOBOX_CONFIG_H
#define NEUIK_COMBOBOX_CONFIG_H

#include "NEUIK_structs_basic.h"
#include "NEUIK_FontSet.h"
#include "neuik_internal.h"

typedef struct {
		neuik_Object        objBase;         /* this structure is requied to be an neuik object */
		NEUIK_FontSet     * fontSet;         /* NEUIK_FontSet */
		int                 fontSize;        /* point size to use for the TTF_Font */
		int                 fontBold;        /* (bool) use bold style */
		int                 fontItalic;      /* (bool) use italic style */
		char              * fontName;        /* font name for the TTF_Font */
		NEUIK_ColorStop  ** gradCS;          /* background color used when not selected */
		NEUIK_ColorStop  ** gradCSSelect;    /* background color used when selected */
		NEUIK_Color         fgColor;         /* foreground color used when not selected */
		NEUIK_Color         fgColorSelect;   /* foreground color used when selected */
		NEUIK_Color         borderColor;     /* color used for the button border */
		NEUIK_Color         borderColorDark; /* color used for the button border */
		int                 fontEmWidth;     /* nuber of pixels required for an `M` of this font */
} NEUIK_ComboBoxConfig;

NEUIK_ComboBoxConfig *NEUIK_GetDefaultComboBoxConfig();

int 
	NEUIK_NewComboBoxConfig(
			NEUIK_ComboBoxConfig ** cfgPtr);

int 
	NEUIK_ComboBoxConfig_Copy(
			NEUIK_ComboBoxConfig       * dst,
			const NEUIK_ComboBoxConfig * src);

int 
	NEUIK_ComboBoxConfig_Free(
			NEUIK_ComboBoxConfig ** cfgPtr);


#endif /* NEUIK_COMBOBOX_CONFIG_H */
