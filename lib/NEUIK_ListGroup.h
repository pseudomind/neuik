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
#ifndef NEUIK_LISTGROUP_H
#define NEUIK_LISTGROUP_H

#include "NEUIK_Element.h"
#include "NEUIK_ListRow.h"

typedef struct {
		neuik_Object objBase;        /* this structure is requied to be an neuik object */
		int          selected;
		int          isActive;
		int          VSpacing;
		int          WidthSep;       /* width of separators */
		int          WidthBorder;    /* width of border */
		NEUIK_Color  colorBorder;    /* color to use for the border */
		NEUIK_Color  colorSeparator; /* color to use for the separator */
		NEUIK_Color  colorBGSelect;  /* color to use for the selected row(s) */
		NEUIK_Color  colorBGOdd;     /* color to use for unselected odd rows */
		NEUIK_Color  colorBGEven;    /* color to use for unselected even rows */
} NEUIK_ListGroup;

int 
	NEUIK_NewListGroup(
			NEUIK_ListGroup ** lgPtr);

void 
	NEUIK_ListGroup_Deselect(
			NEUIK_ListGroup * lg);

int 
	NEUIK_ListGroup_AddRow(
			NEUIK_ListGroup * lg, 
			NEUIK_ListRow   * row);

int 
	NEUIK_ListGroup_AddRows(
			NEUIK_ListGroup * lg, 
			NEUIK_ListRow   * row0, 
			...);

#endif /* NEUIK_LISTGROUP_H */
