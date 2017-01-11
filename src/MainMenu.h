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
#ifndef MAINMENU_H
#define MAINMENU_H

#include "NEUIK_Event.h"
#include "NEUIK_structs_basic.h"

#include "Menu.h"


typedef struct {

	NEUIK_Menu        ** menus;
	NEUIK_MenuConfig   * cfg;
	RenderSize           size;
	RenderLoc            loc;
	int                  doStretch;
	int                  isActive;
	void               * window;
	int                  hadEvent;

} NEUIK_MainMenu;


NEUIK_MainMenu*
	NEUIK_NewMainMenu(
			void              * win,
			NEUIK_MenuConfig  * mCfg,
			int                 doStretch);

int
	NEUIK_MainMenu_GetSize(
			NEUIK_MainMenu  * mmenu, /* the main menu to add the menu to */
			RenderSize      * rSize);

int
	NEUIK_MainMenu_AddMenu(
			NEUIK_MainMenu  * mm,
			NEUIK_Menu      * m);

int
	NEUIK_MainMenu_SetConfig(
			NEUIK_MainMenu    * mm,
			NEUIK_MenuConfig  * mCfg);

void 
	NEUIK_MainMenu_Deselect(
			NEUIK_MainMenu   * mm);


#endif /* MAINMENU_H */
