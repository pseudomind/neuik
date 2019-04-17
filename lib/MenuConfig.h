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
#ifndef MENUCONFIG_H
#define MENUCONFIG_H

#include "NEUIK_structs_basic.h"
#include "NEUIK_FontSet.h"


typedef struct {
		NEUIK_FontSet * fontSet;       /* NEUIK_FontSet */
		int             fontSize;      /* point size to use for the TTF_Font */
		char          * fontName;      /* font name for the TTF_Font */
		NEUIK_Color     bgColor;       /* background color used when not selected */
		NEUIK_Color     fgColor;       /* foreground color used when not selected */
		NEUIK_Color     bgColorSelect; /* background color used when selected */
		NEUIK_Color     fgColorSelect; /* foreground color used when selected */
		NEUIK_Color     sepColor;      /* color of menu separator */
		NEUIK_Color     sepColorDark;  /* darker color of menu separator */
		int             height;        /* desired height of the menu bar */
		int             fontEmWidth;   /* nuber of pixels required for an `M` of this font */
} NEUIK_MenuConfig;

NEUIK_MenuConfig *NEUIK_GetDefaultMenuConfig();


#endif /* MENUCONFIG_H */
