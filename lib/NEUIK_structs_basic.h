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
#ifndef NEUIK_STRUCTS_BASIC_H
#define NEUIK_STRUCTS_BASIC_H

#ifndef   uint8
	typedef int uint8;
#endif /* uint8 */


typedef void * neuik_ptrTo_TTF_Font;
typedef void * neuik_ptrTo_SDL_Surface;
typedef void * neuik_ptrTo_SDL_Texture;
typedef void * neuik_ptrTo_SDL_Renderer;
typedef void * neuik_ptrTo_NEUIK_Window;


typedef struct {
		int  w; /* best width  for resutling texture (px) */
		int  h; /* best height for resutling texture (px) */
} RenderSize;


typedef struct {
		int  x; /* x-position for item (px) */
		int  y; /* y-position for item (px) */
} RenderLoc;


typedef struct {
		uint8 r;  /* red   0-255 */
		uint8 g;  /* green 0-255 */
		uint8 b;  /* blue  0-255 */
		uint8 a;  /* alpha 0-255 */
} NEUIK_Color;

/* these margins are in px */
typedef struct {
		int left;
		int right;
		int top;
		int bottom;
} NEUIK_Margins;

/* NEUIK_ColorStop */
/* Two or more ColorStops may be used together to  describe a color gradient */
typedef struct {
	NEUIK_Color  color; /* color of this stop */
	float        frac;  /* fraction of gradient at which to use this color */
} NEUIK_ColorStop;

/* the following is used for creating color gradients */
typedef struct {
	float r;
	float g;
	float b;
	float a;
} colorDeltas;

#endif /* NEUIK_STRUCTS_BASIC_H */
