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
#ifndef NEUIK_TEXTENTRY_INTERNAL_H
#define NEUIK_TEXTENTRY_INTERNAL_H
#include <SDL.h>

#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_Element_internal.h"
// #include "neuik_classes.h"

extern int neuik__isInitialized;

#define CURSORPAN_TEXT_INSERTED   0
#define CURSORPAN_TEXT_DELTETED   1
#define CURSORPAN_TEXT_ADD_REMOVE 2
#define CURSORPAN_MOVE_BACK       3
#define CURSORPAN_MOVE_FORWARD    4

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__TextEntry(void ** tePtr);
int neuik_Object_Free__TextEntry(void * tePtr);

int           neuik_Element_GetMinSize__TextEntry(NEUIK_Element, RenderSize*);
int           neuik_Element_CaptureEvent__TextEntry(NEUIK_Element, SDL_Event*);
SDL_Texture * neuik_Element_Render__TextEntry(NEUIK_Element, RenderSize*, SDL_Renderer*);
void          neuik_Element_Defocus__TextEntry(NEUIK_Element);

#endif /* NEUIK_TEXTENTRY_INTERNAL_H */
