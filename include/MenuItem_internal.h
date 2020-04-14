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
#ifndef MENUITEM_RENDER_H
#define MENUITEM_RENDER_H

#include <SDL.h>

#include "NEUIK_structs_basic.h"
#include "MenuItem.h"


SDL_Texture* 
	NEUIK_RenderMenuItem(
			const NEUIK_MenuItem  *mi,
			int                    menuW,
			RenderSize            *rSize,
			SDL_Renderer          *extRend);

SDL_Texture* 
	NEUIK_RenderSubMenu(
			const NEUIK_MenuItem  **miList,
			RenderSize             *rSize,
			SDL_Renderer           *extRend);

int 
	NEUIK_MenuItem_CaptureEvent(
			NEUIK_MenuItem * mi,
			SDL_Event      * ev);

void
	NEUIK_MenuItem_StoreSizeAndLocation(
			NEUIK_MenuItem  *mi,
			RenderSize       size,
			RenderLoc        loc);

void
	neuik_MenuItem_SetWindowPointer(
			NEUIK_MenuItem  *mi,
			void            *win);


#endif /* MENUITEM_RENDER_H */
