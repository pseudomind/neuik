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
#ifndef NEUIK_LISTROW_H
#define NEUIK_LISTROW_H

#include "NEUIK_Element.h"


typedef struct {
		neuik_Object objBase;       /* this structure is requied to be an neuik object */
		int          isOddRow;      /* 1 if an odd-numbered row; 0 otherwise */
		int          selectable;    /* If this row can be selected */
		int          HSpacing;
		int          selected;
		int          wasSelected;
		int          isActive;
		int          clickOrigin;
		unsigned int timeLastClick; /* used for identifying doubleclick events */
		NEUIK_Color  colorBGSelect; /* color to use for the selected row(s) */
		NEUIK_Color  colorBGOdd;    /* color to use for unselected odd rows */
		NEUIK_Color  colorBGEven;   /* color to use for unselected even rows */
} NEUIK_ListRow;

int 
	NEUIK_NewListRow(
			NEUIK_ListRow ** row);

int 
	NEUIK_ListRow_SetHSpacing(
			NEUIK_ListRow * row,
			int             spacing);

int 
	NEUIK_ListRow_IsSelected(
			NEUIK_ListRow * row);

int 
	NEUIK_ListRow_SetSelected(
			NEUIK_ListRow  * row,
			int              isSelected);

#endif /* NEUIK_LISTROW_H */
