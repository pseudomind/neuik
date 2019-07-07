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
#ifndef NEUIK_CALLBACK_H
#define NEUIK_CALLBACK_H

typedef enum {
	NEUIK_CALLBACK_CUSTOM,
	NEUIK_CALLBACK_ON_CLICK,
	NEUIK_CALLBACK_ON_CLICKED,
	NEUIK_CALLBACK_ON_CREATED,
	NEUIK_CALLBACK_ON_HOVER,
	NEUIK_CALLBACK_ON_MOUSE_ENTER,
	NEUIK_CALLBACK_ON_MOUSE_LEAVE,
	NEUIK_CALLBACK_ON_MOUSE_OVER,
	NEUIK_CALLBACK_ON_SELECTED,
	NEUIK_CALLBACK_ON_DESELECTED,
	NEUIK_CALLBACK_ON_ACTIVATED,
	NEUIK_CALLBACK_ON_DEACTIVATED,
	NEUIK_CALLBACK_ON_TEXT_CHANGED,
	NEUIK_CALLBACK_ON_EXPANDED,
	NEUIK_CALLBACK_ON_COLLAPSED,
} neuik_CallbackEnum;


/*----------------------------------------------------------------------------*/
/* The callback function will receive a pointer to the NEUIK_Window and both  */
/* of the supplied callback arguments.                                        */
/*----------------------------------------------------------------------------*/
typedef struct {
	void         (* cbFn)(void*, void*, void*);
	void          * cbArg1;
	void          * cbArg2;
	int             isBindingCallback;
	unsigned int    bindID;
} NEUIK_Callback;


typedef struct {
	/*------------------------------------------------------------------------*/
	/* The user can create any number of custom events which will be stored   */
	/* within the following array.                                            */
	/*------------------------------------------------------------------------*/
	void  ** CustomEvents;  /*  0 */
	/*------------------------------------------------------------------------*/
	/* Common event callbacks are explicitly listed for sake of performance.  */
	/*------------------------------------------------------------------------*/
	void  *  OnClick;       /*  1 */
	void  *  OnClicked;     /*  2 */
	void  *  OnCreated;     /*  3 */
	void  *  OnHover;       /*  4 */
	void  *  OnMouseEnter;  /*  5 */
	void  *  OnMouseLeave;  /*  6 */
	void  *  OnMouseOver;   /*  7 */
	void  *  OnSelected;    /*  8 */
	void  *  OnDeselected;  /*  9 */
	void  *  OnActivated;   /* 10 */
	void  *  OnDeactivated; /* 11 */
	void  *  OnTextChanged; /* 12 */
	void  *  OnExpanded;    /* 13 */
	void  *  OnCollapsed;   /* 14 */
} NEUIK_CallbackTable;


NEUIK_CallbackTable NEUIK_NewCallbackTable();

NEUIK_Callback * NEUIK_NewCallback(void * cbFunc, void * cbArg1, void * cbArg2);

NEUIK_Callback * NEUIK_NewBindingCallback(unsigned int bindID);

int NEUIK_PopBindingCallbackFromStack(unsigned int * bindID);

unsigned int NEUIK_WaitForBindingCallback(unsigned int msSleep);

void NEUIK_Callback_Trigger(NEUIK_Callback  * cb, void  * win);


#endif /* NEUIK_CALLBACK_H */
