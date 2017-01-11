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
#ifndef NEUIK_WINDOW_CONFIG_H
#define NEUIK_WINDOW_CONFIG_H

#include "neuik_internal.h"
#include "NEUIK_structs_basic.h"

#define NEUIK_WINDOW_RESIZE_ANY              0
#define NEUIK_WINDOW_RESIZE_ONLY_EXPAND      1
#define NEUIK_WINDOW_RESIZE_ONLY_CONTRACT    2


/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct {
	neuik_Object  objBase;      /* this structure is requied to be an neuik object */
	NEUIK_Color   colorBG;
	int           autoResizeW;  /* whether and how the window can auto resize width */ 
	int           autoResizeH;  /* whether and how the window can auto resize height */ 
	int           canResizeW;   /* whether or not a window can be expand its width */
	int           canResizeH;   /* whether or not a window can be expand its width */
	int           isResizable;  /* whether or not a window is resizable by the WM */
	int           isBorderless; /* whether or not a window is borderless */
	int           isFullscreen; /* whether or not a window is fullscreen */
	int           isMaximized;  /* whether or not a window is maximized */
	int           isMinimized;  /* whether or not a window is minimized */
} NEUIK_WindowConfig;


NEUIK_WindowConfig * NEUIK_GetDefaultWindowConfig();

int 
	NEUIK_NewWindowConfig(
			NEUIK_WindowConfig ** cfg);

int 
	NEUIK_WindowConfig_Copy(
			NEUIK_WindowConfig        * dst,
			const NEUIK_WindowConfig  * src);

int 
	NEUIK_WindowConfig_Free(
			NEUIK_WindowConfig ** cfg);

void 
	NEUIK_WindowConfig_SetBGColor(
			NEUIK_WindowConfig  *wc,
			NEUIK_Color          clr);

#endif /* NEUIK_WINDOW_CONFIG_H */
