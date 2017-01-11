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
#ifndef NEUIK_COMBOBOX_H
#define NEUIK_COMBOBOX_H

#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_ComboBoxConfig.h"


typedef struct {
		neuik_Object            objBase; /* this structure is requied to be an neuik object */
		NEUIK_ComboBoxConfig  * cfg;
		NEUIK_ComboBoxConfig  * cfgPtr; /* if NULL, the non-Pointer version is used */
		char                  * aEntry;
		char                 ** entries;
		int                     selected;
		int                     wasSelected;
		int                     isActive;
		int                     expanded;
		int                     clickOrigin;
		int                     needsRedraw;
} NEUIK_ComboBox;


int 
	NEUIK_NewComboBox(
			NEUIK_ComboBox ** cbPtr);

int 
	NEUIK_MakeComboBox(
			NEUIK_ComboBox ** cbPtr,
			const char      * text);

const char *
	NEUIK_ComboBox_GetText(
			NEUIK_ComboBox * cb);

int 
	NEUIK_ComboBox_SetText(
			NEUIK_ComboBox * cb,
			const char     * text);


#endif /* NEUIK_COMBOBOX_H */
