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
#ifndef NEUIK_ELEMENT_H
#define NEUIK_ELEMENT_H

#include <stdarg.h>

#include "neuik_internal.h"
#include "NEUIK_neuik.h"
#include "NEUIK_defs.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Event.h"
#include "NEUIK_Callback.h"

typedef void * NEUIK_Element;


int 
	NEUIK_Element_Configure(
			NEUIK_Element    elem, 
			const char     * set0, 
			...);

int 
	NEUIK_Element_SetBackgroundColorGradient(
			NEUIK_Element   elem,
			const char    * styleName,
			char            direction,
			const char    * colorStop0,
			...);

int
	NEUIK_Element_SetBackgroundColorSolid(
			NEUIK_Element   elem,
			const char    * styleName,
			unsigned char   r,
			unsigned char   g,
			unsigned char   b,
			unsigned char   a);

int
	NEUIK_Element_SetBackgroundColorTransparent(
			NEUIK_Element   elem,
			const char    * styleName);

int
	NEUIK_Element_SetCallback(
			NEUIK_Element    elem, 
			const char     * cbName, 
			void           * cbFunc, 
			void           * cbArg1, 
			void           * cbArg2);

int 
	NEUIK_Element_SetBindingCallback(
			NEUIK_Element   elem,
			const char    * cbName,
			unsigned int    bindID);

int 
	NEUIK_Element_IsShown(
			NEUIK_Element elem);

int 
	NEUIK_Element_IsShown_virtual(
			NEUIK_Element elem);


#endif /* NEUIK_ELEMENT_H */
