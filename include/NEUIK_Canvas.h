/*******************************************************************************
 * Copyright (c) 2014-2021, Michael Leimon <mike@leimon.net>
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
#ifndef NEUIK_CANVAS_H
#define NEUIK_CANVAS_H

#include "NEUIK_Element.h"
#include "NEUIK_FontSet.h"

enum e_neuik_canvas_op
{
	NEUIK_CANVAS_OP_MOVETO,
	NEUIK_CANVAS_OP_SETDRAWCOLOR,
	NEUIK_CANVAS_OP_DRAWPOINT,
	NEUIK_CANVAS_OP_DRAWLINE,
	NEUIK_CANVAS_OP_DRAWTEXT,
	NEUIK_CANVAS_OP_SETTEXTSIZE,
	NEUIK_CANVAS_OP_FILL,
};

typedef struct {
	unsigned int x;
	unsigned int y;
} neuik_canvas_op_moveto;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} neuik_canvas_op_setdrawcolor;

typedef struct {
	unsigned int x;
	unsigned int y;
} neuik_canvas_op_drawline;

typedef struct {
	unsigned int size;
} neuik_canvas_op_settextsize;

typedef struct {
	char text[21];
} neuik_canvas_op_drawtext;

typedef struct {
	enum e_neuik_canvas_op op;

	union {
		neuik_canvas_op_moveto       op_moveto;
		neuik_canvas_op_setdrawcolor op_setdrawcolor;
		neuik_canvas_op_settextsize  op_settextsize;
		neuik_canvas_op_drawline     op_drawline;
		neuik_canvas_op_drawtext     op_drawtext;
	};
} neuik_canvas_op;

typedef struct {
		neuik_Object      objBase;    /* this structure is requied to be an neuik object */
		NEUIK_FontSet   * fontSet;    /* NEUIK_FontSet */
		char            * fontName;   /* font name for the TTF_Font */
		int               fontSize;   /* point size to use for the TTF_Font */
		int               fontBold;   /* (bool) use bold style */
		int               fontItalic; /* (bool) use italic style */
		unsigned int      draw_x;     /* current draw position - x */
		unsigned int      draw_y;     /* current draw position - y */
		unsigned char     draw_clr_r; /* current draw color - Red */
		unsigned char     draw_clr_g; /* current draw color - Green */
		unsigned char     draw_clr_b; /* current draw color - Blue */
		unsigned char     draw_clr_a; /* current draw color - Alpha */
		unsigned int      text_size;  /* text font size */
		neuik_canvas_op * ops;
		unsigned int      ops_allocated;
		unsigned int      ops_used;
} NEUIK_Canvas;


int 
	NEUIK_NewCanvas(
			NEUIK_Canvas ** cnvsPtr);

int NEUIK_Canvas_Clear(
			NEUIK_Canvas * cnvs);

int NEUIK_Canvas_MoveTo(
			NEUIK_Canvas * cnvs,
			unsigned int   x,
			unsigned int   y);

int NEUIK_Canvas_SetDrawColor(
			NEUIK_Canvas * cnvs,
			unsigned char  r,
			unsigned char  g,
			unsigned char  b,
			unsigned char  a);

int NEUIK_Canvas_Fill(
			NEUIK_Canvas * cnvs);

int NEUIK_Canvas_DrawPoint(
			NEUIK_Canvas * cnvs);

int NEUIK_Canvas_DrawLine(
			NEUIK_Canvas * cnvs,
			unsigned int   x2,
			unsigned int   y2);

int NEUIK_Canvas_SetTextSize(
			NEUIK_Canvas * cnvs,
			unsigned int   size);

int NEUIK_Canvas_DrawText(
			NEUIK_Canvas * cnvs,
			const char   * text);

#endif /* NEUIK_CANVAS_H */
