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
#ifndef NEUIK_LABEL_H
#define NEUIK_LABEL_H

#include "NEUIK_neuik.h"
#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_LabelConfig.h"


typedef struct {
		neuik_Object        objBase; /* this structure is requied to be an neuik object */
		NEUIK_LabelConfig * cfg;
		NEUIK_LabelConfig * cfgPtr; /* if NULL, the non-Pointer version is used */
		char              * text;
		int                 needsRedraw;
} NEUIK_Label;


int
	NEUIK_NewLabel(
			NEUIK_Label ** lblPtr);

int
	NEUIK_MakeLabel(
			NEUIK_Label ** lblPtr,
			const char   * text);

const char *
	NEUIK_Label_GetText(
			NEUIK_Label  * label);

int 
	NEUIK_Label_SetText(
			NEUIK_Label  * label,
			const char   * text);

int 
	NEUIK_Label_Configure(
			NEUIK_Label * lbl,
			const char  * set0,
			...);


#endif /* NEUIK_LABEL_H */
