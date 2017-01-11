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
#ifndef MAINMENU_RENDER_H
#define MAINMENU_RENDER_H

#include <SDL.h>

#include "NEUIK_structs_basic.h"
#include "MainMenu.h"


SDL_Texture* 
	NEUIK_RenderMainMenu(
			NEUIK_MainMenu  *mmenu,
			RenderSize      *rSize,
			SDL_Renderer    *extRend);

void
	NEUIK_MainMenu_StoreSizeAndLocation(
			NEUIK_MainMenu  *mm,
			RenderSize       size,
			RenderLoc        loc);

void
	neuik_MainMenu_SetWindowPointer(
			NEUIK_MainMenu  *mm,
			void            *win);

int 
	NEUIK_MainMenu_CaptureEvent(
			NEUIK_MainMenu * mm,
			SDL_Event      * ev);


#endif /* MAINMENU_RENDER_H */
