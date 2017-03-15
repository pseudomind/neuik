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
#ifndef NEUIK_ELEMENT_INTERNAL_H
#define NEUIK_ELEMENT_INTERNAL_H


#include "NEUIK_Element.h"

/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct {
	float                VScale;    /* Scale Factor : 0 = Doesn't stretch; other value does */
	float                HScale;    /* Scale Factor : 0 = Doesn't stretch; other value does */
	int                  VFill;     /* Element fills Vertically   : 1 = true; 0 = false */
	int                  HFill;     /* Element fills Horizontally : 1 = true; 0 = false */
	enum neuik_VJustify  VJustify;  /* Vertical   justification */
	enum neuik_HJustify  HJustify;  /* Horizontal justification */
	int                  PadTop;    /* Pad the top of the element with transparent space */
	int                  PadBottom; /* Pad the bottom of the element with transparent space */
	int                  PadLeft;   /* Pad the left of the element with transparent space */
	int                  PadRight;  /* Pad the right of the element with transparent space */
	int                  minW;      /* Minimum Width */
	int                  maxW;      /* Maximum Width */
	int                  minH;      /* Minimum Height */
	int                  maxH;      /* Maximum Height */
	int                  Show;      /* Whether or not the element is shown */
} NEUIK_ElementConfig;


enum neuik_bgstyle {
	NEUIK_BGSTYLE_SOLID,
	NEUIK_BGSTYLE_GRADIENT,
	NEUIK_BGSTYLE_TRANSPARENT,
};

enum neuik_focusstate {
	NEUIK_FOCUSSTATE_NORMAL,
	NEUIK_FOCUSSTATE_SELECTED,
	NEUIK_FOCUSSTATE_HOVERED,
};


typedef struct {
	enum neuik_bgstyle    bgstyle_normal;    /* style to use when element is unselected */
	enum neuik_bgstyle    bgstyle_selected;  /* style to use when element is selected */
	enum neuik_bgstyle    bgstyle_hover;     /* style to use when element is hovered */
	NEUIK_Color           solid_normal;      /* solid color to use under normal condtions */
	NEUIK_Color           solid_selected;    /* solid color to use when selected */
	NEUIK_Color           solid_hover;       /* solid color to use being hovered over */
	char                  gradient_dirn;     /* direction to use for the gradient (`v` or `h`) */
	NEUIK_ColorStop    ** gradient_normal;   /* color gradient to use under normal condtions */
	NEUIK_ColorStop    ** gradient_selected; /* color gradient to use when selected */
	NEUIK_ColorStop    ** gradient_hover;    /* color gradient to use being hovered over */
} NEUIK_ElementBackground;

typedef struct {
	int                     doRedraw;   /* if this element needs to be redrawn */
	int                     hasFocus;   /* if this element has focus in the window */
	int                     doesBlend;  /* if alpha blending should be used */
	int                     isActive;   /* if the element is currently active */
	enum neuik_focusstate   focusstate; /* identifies how the element should be redrawn */
	void                  * window;     /* 'NEUIK_Window *' Containing Window */
	NEUIK_Element         * parent;     /* Parent Element */
	NEUIK_Element         * popup;      /* If this contains a popup, this points to it */
	SDL_Texture           * texture;    /* The rendered texture */
	SDL_Surface           * surf;       /* The surface for this element */
	SDL_Renderer          * rend;       /* The renderer for this surface */
	SDL_Renderer          * xRend;      /* The previously used rendered texture */
	RenderSize              rSize;      /* Size of the rendered texture */
	RenderSize              rSizeOld;   /* Old size of the rendered texture */
	RenderLoc               rLoc;       /* Location of the rendered texture */
} NEUIK_ElementState;


typedef struct {
	/* GetMinSize(): Get the minimum required size for the element  */
	int          (*GetMinSize)    (NEUIK_Element, RenderSize *);

	/* Render(): Redraw the element  element  */
	SDL_Texture* (*Render)        (NEUIK_Element, RenderSize *, SDL_Renderer *);

	/* CaptureEvent(): Determine if this element caputures a given event */
	int          (*CaptureEvent)  (NEUIK_Element, SDL_Event *);
	
	/* Defocus(): This function will be called when an element looses focus */
	void         (*Defocus)       (NEUIK_Element);

} NEUIK_Element_FuncTable;


typedef struct {
	neuik_Object              objBase; /* this structure is requied to be an neuik object */
	NEUIK_Element_FuncTable * eFT;
	NEUIK_ElementConfig       eCfg;
	NEUIK_ElementState        eSt;
	NEUIK_ElementBackground   eBg;
	NEUIK_CallbackTable       eCT;
} NEUIK_ElementBase;



NEUIK_ElementConfig 
	neuik_GetDefaultElementConfig();

NEUIK_ElementState  
	neuik_GetDefaultElementState();

void 
	neuik_SetDefaultElementConfig(
			NEUIK_ElementConfig eCfg);

int 
	neuik_Element_CaptureEvent(
			NEUIK_Element   elem, 
			SDL_Event     * ev);

void 
	neuik_Element_Defocus(
			NEUIK_Element elem);

int 
	neuik_Element_ForceRedraw(
			NEUIK_Element elem);

NEUIK_ElementConfig * 
	neuik_Element_GetConfig(
			NEUIK_Element elem);

int 
	neuik_Element_GetMinSize(
			NEUIK_Element    elem, 
			RenderSize     * rSize);

int 
	neuik_Element_GetLocation(
			NEUIK_Element   elem, 
			RenderLoc     * rLoc);

int 
	neuik_Element_NeedsRedraw(
			NEUIK_Element elem);

SDL_Texture *
	neuik_Element_Render(
	 		NEUIK_Element    elem, 
	 		RenderSize     * rSize, 
	 		SDL_Renderer   * xRend);

int 
	neuik_Element_RequestRedraw(
			NEUIK_Element elem);

int 
	neuik_Element_Resize(
			NEUIK_Element elem, 
			RenderSize    nSize);

int 
	neuik_Element_ResizeTransparent(
			NEUIK_Element elem, 
			RenderSize    nSize);

void 
	neuik_Element_SetParentPointer(
			NEUIK_Element   elem, 
			void          * parent);

int
	neuik_Element_SetChildPopup(
			NEUIK_Element  parent, 
			NEUIK_Element  pu);

int 
	neuik_Element_SetWindowPointer(
			NEUIK_Element   elem, 
			void          * win);

void 
	neuik_Element_StoreSizeAndLocation(
			NEUIK_Element elem, 
			RenderSize    rSize, 
			RenderLoc     rLoc);

int 
	neuik_Element_TriggerCallback(
			NEUIK_Element      elem, 
			neuik_CallbackEnum cbType);

int 
	neuik_Element_SetFuncTable(
			NEUIK_Element             elem, 
			NEUIK_Element_FuncTable * eFT);

int 
	neuik_NewElement(
			NEUIK_Element ** elemPtr);

void 
	neuik_Element_SetActive(
			NEUIK_Element elem,
			int           isActive);

int 
	neuik_Element_IsActive(
			NEUIK_Element elem);

#endif /* NEUIK_ELEMENT_INTERNAL_H */
