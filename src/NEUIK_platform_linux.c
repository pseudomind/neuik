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
#include <SDL.h>
#include <stdio.h>

#include "NEUIK_platform.h"

/* Ctrl + X */
int neuik_KeyShortcut_Cut(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_x) return 1;

	return 0;
}

/* Ctrl + C */
int neuik_KeyShortcut_Copy(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_c) return 1;

	return 0;
}

/* Ctrl + V */
int neuik_KeyShortcut_Paste(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_v) return 1;

	return 0;
}

/* Ctrl + A */
int neuik_KeyShortcut_SelectAll(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_a) return 1;

	return 0;
}

/* Ctrl + Z */
int neuik_KeyShortcut_Undo(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_z) return 1;

	return 0;
}

/* Ctrl + S */
int neuik_KeyShortcut_Save(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_s) return 1;

	return 0;
}

/* Ctrl + N */
int neuik_KeyShortcut_New(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyEv->keysym.sym == SDLK_n) return 1;

	return 0;
}

/* Ctrl + Shift + N */
int neuik_KeyShortcut_NewWindow(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL && keyMod & KMOD_SHIFT &&keyEv->keysym.sym == SDLK_n) return 1;

	return 0;
}

/* Ctrl + F */
int neuik_KeyShortcut_Find(
		SDL_KeyboardEvent  * keyEv, 
		SDL_Keymod           keyMod)
{
	if (keyMod & KMOD_CTRL &&keyEv->keysym.sym == SDLK_f) return 1;

	return 0;
}

