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
#ifndef NEUIK_RENDER_H
#define NEUIK_RENDER_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "NEUIK_structs_basic.h" /* for RenderSize */

void
	ConditionallyDestroyTexture(
			SDL_Texture **tex);

SDL_Texture * 
	NEUIK_RenderArrowDown(
			NEUIK_Color    color,
			SDL_Renderer * xRend,
			RenderSize     rSize);

SDL_Texture * 
	NEUIK_RenderText(
			const char   * textStr, 
			TTF_Font     * font, 
			NEUIK_Color    color,
			SDL_Renderer * renderer, 
			int          * rvW, 
			int          * rvH);


SDL_Surface * 
	NEUIK_RenderTextAsSurface(
			const char  * textStr, 
			TTF_Font    * font, 
			NEUIK_Color   color,
			int         * rvW, 
			int         * rvH);

SDL_Texture * 
	NEUIK_RenderText_Solid(
			const char   * textStr, 
			TTF_Font     * font, 
			NEUIK_Color    color,
			SDL_Renderer * renderer, 
			int          * rvW, 
			int          * rvH);

SDL_Texture * 
	NEUIK_RenderGradient(
			NEUIK_ColorStop ** cs,
			char               dirn,
			SDL_Renderer     * renderer,
			RenderSize         rSize);

void 
	String_Duplicate(
			char       ** dst, 
			const char  * src);


#endif /* NEUIK_RENDER_H */
