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
#ifndef NEUIK_WINDOW_H
#define NEUIK_WINDOW_H

#include <stdarg.h>
#include "MainMenu.h"
#include "NEUIK_neuik.h"
#include "NEUIK_Element.h"
#include "NEUIK_Image.h"
#include "NEUIK_Event.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_WindowConfig.h"
#include "NEUIK_Callback.h"

/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct {
	neuik_Object              objBase;   /* this structure is requied to be an neuik object */
	void                    * win;       /* SDL_Window           */
	void                    * rend;      /* SDL_Renderer         */
	char                    * title;     /* title of the window  */
	NEUIK_MainMenu          * mmenu;     /* MainMenu             */
	NEUIK_WindowConfig      * cfg;       /* WindowConfig         */
	NEUIK_WindowConfig      * cfgPtr;    /* if NULL, the non-Pointer version is used */
	NEUIK_Element             elem;
	NEUIK_Element             focused;
	NEUIK_Element           * popups;
	NEUIK_Image             * icon;
	NEUIK_CallbackTable       eCT;
	NEUIK_EventHandlerTable   eHT;       /* optional event handler */

	int                       posX;      /* X position of window */     
	int                       posY;      /* Y position of window */
	int                       sizeW;     /* width of window  (px) */
	int                       sizeH;     /* height of window (px) */
	int                       shown;     /* whether or not the window is shown */
	int                       winID;     /* the Window index when registered for event handling */

	int                       updateIcon;
	int                       updateTitle;
	int                       doRedraw;

} NEUIK_Window;

/*----------------------------------------------------------------------------*/
/* Functions Prototype(s)                                                     */
/*----------------------------------------------------------------------------*/
int 
	NEUIK_NewWindow(
			NEUIK_Window ** wPtr);

int 
	NEUIK_Window_Free(
			NEUIK_Window * wPtr);
int 
	NEUIK_Window_Create(
			NEUIK_Window * w);

int 
	NEUIK_Window_Recreate(
			NEUIK_Window * w);

int 
	NEUIK_Window_CopyConfig(
			NEUIK_Window        * w,
			NEUIK_WindowConfig  * cfg);

int 
	NEUIK_Window_Configure(
			NEUIK_Window   * w,
			const char     * set0,
			...);

//void 
//	NEUIK_Window_Calculate(
//			NEUIK_Window *w);

int 
	NEUIK_Window_Redraw(
			NEUIK_Window *w);
int 
	NEUIK_Window_RedrawNeeded(
			NEUIK_Window *w);
void 
	NEUIK_Window_Add(
			NEUIK_Window *w,
			void         *thing);

int
	NEUIK_Window_SetCallback(
			NEUIK_Window   * w,
			const char     * cbName,
			void           * cbFunc,
			void           * cbArg1,
			void           * cbArg2);

int 
	NEUIK_Window_SetBindingCallback(
			NEUIK_Window * w,
			const char   * cbName,
			unsigned int   bindID);

int
	NEUIK_Window_SetEventHandler(
			NEUIK_Window   * w,
			const char     * evName,
			void           * evFunc,
			void           * evArg1,
			void           * evArg2);

int 
	NEUIK_Window_TriggerCallback(
			NEUIK_Window  * w,
			int             cbType);

void 
	NEUIK_Window_SetShown(
			NEUIK_Window *w, 
			int show);

int 
	NEUIK_Window_SetSize(
			NEUIK_Window  * w, 
			int             width,
			int             height);

int 
	NEUIK_Window_SetTitle(
			NEUIK_Window * w,
			const char   * title);

int
	NEUIK_Window_SetIcon(
			NEUIK_Window * w,
			NEUIK_Image  * img);

int  
	NEUIK_Window_SetEventHandler(
			NEUIK_Window  * w,
			const char    * eHName,
			void          * eHFunc,
			void          * eHArg1,
			void          * eHArg2);

int 
	NEUIK_Window_EventHandler_CaptureEvent(
			NEUIK_Window    * w,
			int               eHType,
			int             * captured,
			ptrTo_SDL_Event   ev);

int 
	NEUIK_Window_SetMainMenu(
			NEUIK_Window *w, 
			void         *mmenu);

int 
	NEUIK_Window_SetElement(
			NEUIK_Window  * w,
			NEUIK_Element   elem);


#endif /* NEUIK_WINDOW_H */
