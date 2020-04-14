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
#ifndef NEUIK_CANVAS_INTERNAL_H
#define NEUIK_CANVAS_INTERNAL_H

enum neuik_draw_op
{
	NEUIK_DRAW_OP_FILL,
	NEUIK_DRAW_OP_POINT,
	NEUIK_DRAW_OP_LINE,
};

typedef struct {

} neuik_canvas_draw

#include "NEUIK_Element.h"


typedef struct {
		neuik_Object objBase;     /* this structure is requied to be an neuik object */
} NEUIK_Canvas;


int 
	NEUIK_NewCanvas(
			NEUIK_Canvas ** cnvsPtr);

int NEUIK_Canvas_Clear(
			NEUIK_Canvas * cnvs);

int NEUIK_Canvas_Fill(
			NEUIK_Canvas * cnvs,
			unsigned char  r,
			unsigned char  g,
			unsigned char  b,
			unsigned char  a);

int NEUIK_Canvas_DrawPoint(
			NEUIK_Canvas * cnvs,
			unsigned int   x,
			unsigned int   y,
			unsigned char  r,
			unsigned char  g,
			unsigned char  b,
			unsigned char  a);

int NEUIK_Canvas_DrawLine(
			NEUIK_Canvas * cnvs,
			unsigned int   x1,
			unsigned int   y1,
			unsigned int   x2,
			unsigned int   y2,
			unsigned char  r,
			unsigned char  g,
			unsigned char  b,
			unsigned char  a);

#endif /* NEUIK_CANVAS_INTERNAL_H */
