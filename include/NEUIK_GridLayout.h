/*******************************************************************************
 * Copyright (c) 2014-2019, Michael Leimon <leimon@gmail.com>
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
#ifndef NEUIK_GRIDLAYOUT_H
#define NEUIK_GRIDLAYOUT_H

#include "NEUIK_Event.h"
#include "NEUIK_Element.h"


typedef struct {
		neuik_Object objBase;  /* this structure is requied to be an neuik object */
		int          HSpacing;
		int          VSpacing;
		unsigned int xDim;     /* The max number of grid items along the x-axis */
		unsigned int yDim;     /* The max number of grid items along the y-axis */
		int          squareElems; /* If elems should all be square in shape */
		int          selected;
		int          isActive;
} NEUIK_GridLayout;

int 
	NEUIK_NewGridLayout(
			NEUIK_GridLayout ** grid);

int 
	NEUIK_MakeGridLayout(
			NEUIK_GridLayout ** gridPtr,
			unsigned int        xDim,
			unsigned int        yDim);

int 
	NEUIK_GridLayout_SetDimensions(
			NEUIK_GridLayout * grid,
			unsigned int       xDim,
			unsigned int       yDim);

int
	NEUIK_GridLayout_GetElementAt(
			NEUIK_GridLayout * grid,
			unsigned int       xLoc,
			unsigned int       yLoc,
			NEUIK_Element    * elem);

int
	NEUIK_GridLayout_GetElementPos(
			NEUIK_GridLayout * grid,
			NEUIK_Element      elem,
			int              * hasElem,
			int              * xLoc,
			int              * yLoc);

int
	NEUIK_GridLayout_SetElementAt(
			NEUIK_GridLayout * grid,
			unsigned int       xLoc,
			unsigned int       yLoc,
			NEUIK_Element      elem);

int 
	NEUIK_GridLayout_SetHSpacing(
			NEUIK_GridLayout * grid,
			int                spacing);

int 
	NEUIK_GridLayout_SetVSpacing(
			NEUIK_GridLayout * grid,
			int                spacing);

int 
	NEUIK_GridLayout_SetSpacing(
			NEUIK_GridLayout * grid,
			int                spacing);

void 
	NEUIK_GridLayout_Deselect(
			NEUIK_GridLayout * grid);

int 
	NEUIK_GridLayout_Configure(
			NEUIK_GridLayout * grid,
			const char       * set0,
			...);


#endif /* NEUIK_GRIDLAYOUT_H */
