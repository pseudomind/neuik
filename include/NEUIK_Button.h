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
#ifndef NEUIK_BUTTON_H
#define NEUIK_BUTTON_H

#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_ButtonConfig.h"


typedef struct {
		neuik_Object         objBase; /* this structure is requied to be an neuik object */
		NEUIK_ButtonConfig * cfg;
		NEUIK_ButtonConfig * cfgPtr;  /* if NULL, the non-Pointer version is used */
		char               * text;
		int                  selected;
		int                  wasSelected;
		int                  isActive;
		int                  clickOrigin;
		int                  needsRedraw;
} NEUIK_Button;

int 
	NEUIK_NewButton(
			NEUIK_Button ** btnPtr);

int 
	NEUIK_MakeButton(
			NEUIK_Button ** btnPtr,
			const char    * text);

const char *
	NEUIK_Button_GetText(
			NEUIK_Button  * btn);

int 
	NEUIK_Button_SetText(
			NEUIK_Button  * btn,
			const char    * text);

int 
	NEUIK_Button_Configure(
			NEUIK_Button * btn,
			const char   * set0,
			...);


#endif /* NEUIK_BUTTON_H */
