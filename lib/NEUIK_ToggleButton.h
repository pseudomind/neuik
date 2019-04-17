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
#ifndef NEUIK_TOGGLEBUTTON_H
#define NEUIK_TOGGLEBUTTON_H

#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_ToggleButtonConfig.h"


typedef struct {
		neuik_Object               objBase;  /* this structure is requied to be an neuik object */
		NEUIK_ToggleButtonConfig * cfg;
		NEUIK_ToggleButtonConfig * cfgPtr;  /* if NULL, the non-Pointer version is used */
		char                     * text;
		int                        selected;
		int                        wasSelected;
		int                        activated;
		int                        isActive;
		int                        clickOrigin;
		int                        needsRedraw;
} NEUIK_ToggleButton;

int 
	NEUIK_NewToggleButton(
			NEUIK_ToggleButton ** btnPtr);

int 
	NEUIK_MakeToggleButton(
			NEUIK_ToggleButton ** btnPtr,
			const char          * text);

int 
	NEUIK_ToggleButton_Configure(
			NEUIK_ToggleButton  * btn,
			const char    * set0,
			...);

const char *
	NEUIK_ToggleButton_GetText(
			NEUIK_ToggleButton  * btn);

int 
	NEUIK_ToggleButton_SetText(
			NEUIK_ToggleButton  * btn,
			const char          * text);

int 
	NEUIK_ToggleButton_Activate(
			NEUIK_ToggleButton  * btn);

int 
	NEUIK_ToggleButton_Deactivate(
			NEUIK_ToggleButton  * btn);

int 
	NEUIK_ToggleButton_IsActivated(
			NEUIK_ToggleButton  * btn);



#endif /* NEUIK_TOGGLEBUTTON_H */
