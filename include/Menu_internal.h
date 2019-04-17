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
#ifndef MENU_RENDER_H
#define MENU_RENDER_H

#include <SDL.h>
#include <SDL_ttf.h>


SDL_Texture* 
	NEUIK_RenderMenuButton(
			const char              *text,
			const NEUIK_MenuConfig  *mCfg,
			RenderSize              *rSize,
			SDL_Renderer            *extRend);

SDL_Texture* 
	NEUIK_RenderMenu(
			const NEUIK_Menu  *menu,
			RenderSize        *rSize,
			SDL_Renderer      *extRend);

int 
	NEUIK_Menu_CaptureEvent(
			NEUIK_Menu * m,
			SDL_Event  * ev);

void
	NEUIK_Menu_StoreSizeAndLocation(
			NEUIK_Menu  *m,
			RenderSize   size,
			RenderLoc    loc);

void
	neuik_Menu_SetWindowPointer(
			NEUIK_Menu  *m,
			void        *win);


#endif /* MENU_H */
