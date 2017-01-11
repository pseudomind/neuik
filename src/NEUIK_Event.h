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
#ifndef NEUIK_EVENT_H
#define NEUIK_EVENT_H

#define NEUIK_EVENTHANDLER_BEFORE    0
#define NEUIK_EVENTHANDLER_AFTER     1
#define NEUIK_EVENTHANDLER_OVERRIDE  2

/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef void * ptrTo_SDL_Event;

typedef struct {
	int   (* eHFn)(void * container, ptrTo_SDL_Event ev, int * captured, void * arg1, void * arg2);
	/*------------------------------------------------------------------------*/
	/* These functions should have the following arguments:                   */
	/* void* container, int* captured,                                        */
	/*------------------------------------------------------------------------*/
	void   * eHArg1;
	void   * eHArg2;
} NEUIK_EventHandler;

typedef struct {
	NEUIK_EventHandler  * Before;   /* 0 */
	NEUIK_EventHandler  * After;    /* 1 */
	NEUIK_EventHandler  * Override; /* 2 */
} NEUIK_EventHandlerTable;


void NEUIK_EventLoop(int killOnError);
void NEUIK_EventLoopNoErrHandling();

NEUIK_EventHandler * 
	NEUIK_NewEventHandler(
			void * evFunc, 
			void * evArg1, 
			void * evArg2);

int 
	NEUIK_EventHandler_Capture(
			NEUIK_EventHandler * eH, 
			void               * container, 
			int                * captured, 
			ptrTo_SDL_Event      ev);

NEUIK_EventHandlerTable NEUIK_NewEventHandlerTable();

#endif /* NEUIK_EVENT_H */
