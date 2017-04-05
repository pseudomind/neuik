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
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"

#include "Menu.h"
#include "Menu_internal.h"
#include "MenuItem.h"
#include "MenuItem_internal.h"


/*******************************************************************************
 *
 *  Name:          NEUIK_NewMenu
 *
 *  Description:   Create and return a pointer to a new NEUIK_Menu.
 *
 *  Returns:       NULL if ther is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
NEUIK_Menu * NEUIK_NewMenu(
		const char *name)
{
	size_t sLen = 1;
	NEUIK_Menu *rvM = NULL;

	rvM = (NEUIK_Menu*) malloc(sizeof(NEUIK_Menu));
	if (rvM == NULL) goto out;

	/* successful allocation of Menu Memory */

	if (name == NULL) {
		rvM->name = (char*)malloc(sizeof(char));
		if (rvM->name == NULL) {
			free(rvM);
			rvM = NULL;
			goto out;
		}
		(rvM->name)[0] = 0;
		goto out;
	}

	sLen += strlen(name);
	rvM->name = (char*)malloc(sLen*sizeof(char));
	if (rvM->name == NULL) {
		free(rvM);
		rvM = NULL;
		goto out;
	}

	/* Allocation successful */
	strcpy(rvM->name, name);
	rvM->selected = 0;
	rvM->isActive = 0;
	rvM->subList  = NULL;

out:
	return rvM;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Menu_GetSize
 *
 *  Description:   Returns the rendered size of a given menu.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_Menu_GetSize(
		const NEUIK_Menu * menu,
		int                includeSubMenu,
		RenderSize       * rSize)
{
	int               eNum = 0; /* which error to report (if any) */
	int               tW;
	int               tH;
	int               ctr  = 0;
	RenderSize        rs;
	TTF_Font        * font = NULL;
	NEUIK_MenuItem  * mi   = NULL;
	static char       funcName[] = "NEUIK_Menu_GetSize";
	static char     * errMsgs[] = {"", // [0] no error
		"Pointer to menu is NULL.",    // [1]
		"MenuConfig* is NULL.",        // [2]
		"MenuConfig Font is NULL.",    // [3]
	};

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (menu == NULL)
	{
		rSize->w = -1;
		rSize->h = -1;

		eNum = 1;
		goto out;
	}
	if (menu->cfg == NULL)
	{
		rSize->w = -2;
		rSize->h = -2;

		eNum = 2;
		goto out;
	}
	font = NEUIK_FontSet_GetFont(menu->cfg->fontSet, menu->cfg->fontSize, 0 ,0);
	if (font == NULL) 
	{
		rSize->w = -3;
		rSize->h = -3;

		eNum = 3;
		goto out;
	}

	TTF_SizeText(font, menu->name, &tW, &tH);
	rSize->w = tW + menu->cfg->fontEmWidth;
	rSize->h = menu->cfg->height;

	/*------------------------------------------------------------------------*/
	/* If selected, the size of this menu button may need to be expanded      */
	/*------------------------------------------------------------------------*/
	if (menu->selected)
	{
		if (!includeSubMenu || menu->subList == NULL) {
			/* there are no submenus contained by this menu */
			goto out;
		}

		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)menu->subList[ctr];
			if (mi == NULL)
			{
				break;
			}
			NEUIK_MenuItem_GetSize(mi, 1, &rs);
			rSize->h += rs.h;

			if (rs.w > rSize->w)
			{
				rSize->w = rs.w;
			}
		}

		/* add in the addtional space for leading and trailing mI list seps */
		rSize->h += 8;
	}
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


void NEUIK_Menu_SetConfig(
		NEUIK_Menu        *m,
		NEUIK_MenuConfig  *cfg)
{
	int        ctr;
	NEUIK_MenuItem  *mi;

	if (m  == NULL) return;
	if (cfg == NULL) return;
	m->cfg = cfg;

	/*------------------------------------------------------------------------*/
	/* If this menu contains a subList, set the config on those items as well */
	/*------------------------------------------------------------------------*/
	if (m->subList == NULL) return;
	for (ctr = 0;; ctr++)
	{
		mi = m->subList[ctr];
		if (mi == NULL)
		{
			break;
		}
		NEUIK_MenuItem_SetConfig(mi, cfg);
	}
}

/**
   @return 0 Success
   @return 1 Error: Param
*/
int NEUIK_Menu_AddMenuItem(
		NEUIK_Menu      *m,   /* the main menu to add the menu to */
		NEUIK_MenuItem  *mi)  /* the menu to add */
{
	int rv  = 0;
	int len = 0;
	int ctr = 0;

	if (m == NULL) goto out;
	if (mi == NULL) goto out;
	if (m->subList == NULL)
	{
		/*-------------------------------------------------------------*/
		/* This is the first menu to be added, allocate initial memory */
		/* This pointer array will be null terminated.                 */
		/*-------------------------------------------------------------*/
		m->subList = (NEUIK_MenuItem**)malloc(2*sizeof(NEUIK_MenuItem*));
		if (m->subList == NULL)
		{
			rv = 1;
			goto out;
		}
		mi->window = m->window;
		m->subList[0] = mi;
		m->subList[1] = NULL;
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* This is subsequent menu item, reallocate memory                    */
		/* This pointer array will be null terminated.                        */
		/*--------------------------------------------------------------------*/
		
		/* determine the current length */
		for (ctr = 0;;ctr++)
		{
			if (m->subList[ctr] == NULL)
			{
				len = 2 + ctr;
				break;
			}
		}

		m->subList = (NEUIK_MenuItem**)realloc(m->subList, len*sizeof(NEUIK_MenuItem*));
		if (m->subList == NULL)
		{
			rv = 1;
			goto out;
		}
		mi->window = m->window;
		m->subList[ctr] = mi;
		m->subList[ctr+1] = NULL;
	}

out:
	return rv;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_RenderMenu
 *
 *  Description:   Renders a single menu as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture * NEUIK_RenderMenu(
		const NEUIK_Menu * menu,
		RenderSize       * rSize,   /* the size this texture should occupy when complete */
		SDL_Renderer     * extRend) /* the external renderer to prepare the texture for */
{
	int                 eNum    = 0; /* which error to report (if any) */
	int                 ctr     = 0;
	int                 textW   = 0;
	int                 textH   = 0;
	int                 yPos    = 0;
	int                 miWidth = 0;
	SDL_Texture       * tex     = NULL; /* texture */
	SDL_Texture       * tTex    = NULL; /* text texture */
	SDL_Rect            rect;
	SDL_Rect            bgRect = {0, 0, 0, 0};
	RenderSize          mainSize;
	RenderSize          rs;
	RenderLoc           rl;
	TTF_Font          * font  = NULL;
	NEUIK_MenuItem    * mi    = NULL;
	const NEUIK_Color * fgClr = NULL;
	const NEUIK_Color * bgClr = NULL;
	const NEUIK_Color * sClr  = NULL; /* separator color */
	SDL_Color           tClr  = COLOR_TRANSP;
	SDL_Surface       * surf  = NULL;
	SDL_Renderer      * rend  = NULL;
	static char         funcName[] = "NEUIK_RenderMenu";
	static char       * errMsgs[] = {"",       // [0] no error
		"Failed to create RGB surface.",       // [1]
		"Failed to create software renderer.", // [2]
		"RenderText returned NULL.",           // [3]
		"RenderMenuItem returned NULL.",       // [4]
		"Menu_GetSize failed.",                // [5]
		"Failure in FontSet_GetFont.",         // [6]
	};

	SDL_Texture  *rvTex  = NULL;

	/*------------------------------------------------------*/
	/* Calculate the required size of the resultant texture */
	/*------------------------------------------------------*/
	if (NEUIK_Menu_GetSize(menu, 1, rSize))
	{
		eNum = 5;
		goto out;
	}

	if (extRend == NULL)
	{
		/*---------------------------------------------------------*/
		/* Just return the required size for the resultant texture */
		/*---------------------------------------------------------*/
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create a surface and a software renderer on which to draw the button   */
	/*------------------------------------------------------------------------*/
	surf = SDL_CreateRGBSurface(0, rSize->w, rSize->h, 32, 0, 0, 0, 0);
	if (surf == NULL)
	{
		eNum = 1;
		goto out;
	}

	rend = SDL_CreateSoftwareRenderer(surf);
	if (rend == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Fill the entire surface background with a transparent color            */
	/*------------------------------------------------------------------------*/
	SDL_SetColorKey(surf, SDL_TRUE, 
		SDL_MapRGB(surf->format, tClr.r, tClr.g, tClr.b));
	SDL_SetRenderDrawColor(rend, tClr.r, tClr.g, tClr.b, 255);
	SDL_RenderClear(rend);

	/*------------------------------------------------------------------------*/
	/* Fill the background with it's color                                    */
	/*------------------------------------------------------------------------*/
	NEUIK_Menu_GetSize(menu, 0, &mainSize);
	if (menu->selected)
	{
		bgClr = &(menu->cfg->bgColorSelect);
		fgClr = &(menu->cfg->fgColorSelect);
	}
	else
	{
		/* use the unselected colors */
		bgClr = &(menu->cfg->bgColor);
		fgClr = &(menu->cfg->fgColor);
	}
	SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);

	/*------------------------------------------------------------------------*/
	/* Fill the background of the menu (title) area                           */
	/*------------------------------------------------------------------------*/
	bgRect.w = mainSize.w;
	bgRect.h = menu->cfg->height;
	SDL_RenderFillRect(rend, &bgRect);

	/*------------------------------------------------------------------------*/
	/* Draw the menu separator line below the main menu element               */
	/*------------------------------------------------------------------------*/
	if (!menu->selected)
	{
		sClr = &(menu->cfg->sepColor);
	}
	else
	{
		sClr = &(menu->cfg->sepColorDark);
	}
	SDL_SetRenderDrawColor(rend, sClr->r, sClr->g, sClr->b, 255);
	SDL_RenderDrawLine(rend, 0, bgRect.h - 1, bgRect.w - 1, bgRect.h - 1);

	/*------------------------------------------------------------------------*/
	/* Render the menu text                                                   */
	/*------------------------------------------------------------------------*/
	font = NEUIK_FontSet_GetFont(menu->cfg->fontSet, menu->cfg->fontSize, 0, 0);
	if (font == NULL) 
	{
		eNum = 6;
		goto out;
	}

	tTex = NEUIK_RenderText(menu->name, font, *fgClr, rend, &textW, &textH);
	if (tTex == NULL)
	{
		eNum = 3;
		goto out;
	}

	rect.x = (int) ((float)(mainSize.w - textW)/2.0);
	rect.y = (int) ((float)(bgRect.h - textH)/2.0);
	rect.w = textW;
	rect.h = (int)(1.1*textH);
	SDL_RenderCopy(rend, tTex, NULL, &rect);

	/*------------------------------------------------------------------------*/
	/* Draw the menuItems onto the main menu                                  */
	/*------------------------------------------------------------------------*/
	if (menu->selected && menu->subList != NULL)
	{
		/*------------------------------------------*/
		/* Determine the width of the menuItem list */
		/*------------------------------------------*/
		miWidth = bgRect.w;
		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)menu->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			NEUIK_MenuItem_GetSize(mi, 0, &rs);

			if (rs.w > miWidth)
			{
				miWidth = rs.w;
			}
		}

		rect.x = 0;
		/* start at the bottom of the menu +1, to allow for the sep line */
		yPos = bgRect.h + 4;

		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)menu->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			ConditionallyDestroyTexture(&tex);
			tex = NEUIK_RenderMenuItem(mi, miWidth, &rs, rend);
			if (tex == NULL)
			{
				eNum = 4;
				goto out;
			}

			rect.x = 0;
			rect.y = yPos;
			rect.w = rs.w;
			rect.h = rs.h;
			SDL_RenderCopy(rend, tex, NULL, &rect);
			rl.x = (menu->loc).x + rect.x;
			rl.y = (menu->loc).y + rect.y;
			NEUIK_MenuItem_StoreSizeAndLocation(mi, rs, rl);

			if (ctr == 0)
			{
				/*------------------------------------------------------------*/
				/* Draw in some separation between the menu and item 1        */
				/* this separation will always be the unselected color        */
				/*------------------------------------------------------------*/
				bgClr = &(menu->cfg->bgColor);
				SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
				SDL_RenderDrawLine(rend, 0, yPos-4, rect.w, yPos-4);
				SDL_RenderDrawLine(rend, 0, yPos-3, rect.w, yPos-3);
				SDL_RenderDrawLine(rend, 0, yPos-2, rect.w, yPos-2);
				SDL_RenderDrawLine(rend, 0, yPos-1, rect.w, yPos-1);

				/* draw the shaded right border */
				sClr = &(menu->cfg->sepColor);
				SDL_SetRenderDrawColor(rend, sClr->r, sClr->g, sClr->b, 255);
				SDL_RenderDrawLine(rend, rect.w - 1, yPos - 1,   rect.w - 1, yPos - 4);
			}
			yPos += rs.h;
		}

		/*--------------------------------------------------------------------*/
		/* Draw a little bit of separation at the end of the item list        */
		/* this separation will always be the unselected color                */
		/*--------------------------------------------------------------------*/
		bgClr = &(menu->cfg->bgColor);
		SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
		SDL_RenderDrawLine(rend, 0, yPos,   rect.w,   yPos);
		SDL_RenderDrawLine(rend, 1, yPos+1, rect.w-1, yPos+1);
		SDL_RenderDrawLine(rend, 2, yPos+2, rect.w-2, yPos+2);

		sClr = &(menu->cfg->sepColor);
		SDL_SetRenderDrawColor(rend, sClr->r, sClr->g, sClr->b, 255);
		SDL_RenderDrawLine(rend, rect.w - 1, yPos - 1,   rect.w - 1, yPos - 4);
		SDL_RenderDrawPoint(rend, rect.w-1,   yPos);
		SDL_RenderDrawLine(rend, rect.w-2, yPos+1, rect.w-1, yPos+1);
		SDL_RenderDrawLine(rend, rect.w-4, yPos+2, rect.w-2, yPos+2);
		SDL_RenderDrawLine(rend, 4, yPos+3, rect.w-4, yPos+3);
		SDL_RenderDrawPoint(rend, rect.w-2, yPos+2);
	}

	/*------------------------------------------------------------------------*/
	/* Present all changes and create a texture from this surface             */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	rvTex = SDL_CreateTextureFromSurface(extRend, surf);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	if (surf != NULL) SDL_FreeSurface(surf);
	if (rend != NULL) SDL_DestroyRenderer(rend);
	ConditionallyDestroyTexture(&tex);
	ConditionallyDestroyTexture(&tTex);

	return rvTex;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Menu_CaptureEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_Menu.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
int NEUIK_Menu_CaptureEvent(
		NEUIK_Menu * m,
		SDL_Event  * ev)
{
	int ctr        = 0;
	int evCaputred = 0;
	NEUIK_MenuItem  *mi;
	
	int selectIndex = -1;
	int finalIndex  = -1;

	SDL_Event             *e;
	SDL_KeyboardEvent     *keyEv;
	SDL_MouseMotionEvent  *mouseMotEv;
	SDL_MouseButtonEvent  *mouseButEv;

	/*------------------------------------------------------------------------*/
	/* See if any of the menus are selected, if so set them all to isActive.  */
	/*------------------------------------------------------------------------*/
	if (m->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			mi = m->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			if (mi->selected)
			{
				selectIndex = ctr;
				break;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (mouseclick/mousemotion).   */
	/*------------------------------------------------------------------------*/
	// TODO
	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_MOUSEBUTTONDOWN:
		mouseButEv = (SDL_MouseButtonEvent*)(e);
		if (mouseButEv->y >= m->loc.y && mouseButEv->y <= m->loc.y + m->size.h)
		{
			if (mouseButEv->x >= m->loc.x && mouseButEv->x <= m->loc.x + m->size.w)
			{
				/* This mouse click originated within this button */
				NEUIK_Menu_Deselect(m);
				m->selected = 1;
				m->isActive = 1;
				evCaputred  = 1;
				goto out;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		if (!m->isActive)
		{
			goto out;
		}

		mouseMotEv = (SDL_MouseMotionEvent*)(e);
		if (mouseMotEv->y >= m->loc.y && mouseMotEv->y <= m->loc.y + m->size.h)
		{
			if (mouseMotEv->x >= m->loc.x && mouseMotEv->x <= m->loc.x + m->size.w)
			{
				/* make sure all subitems are deselected */
				NEUIK_Menu_Deselect(m);
				m->selected = 1;
				evCaputred = 1;
				goto out;
			}
		}
		break;
	}

	if (!m->selected)
	{
		/* Check first to see if this menu is even visible */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the one of the menu items.           */
	/*------------------------------------------------------------------------*/
	if (m->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)m->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			evCaputred = NEUIK_MenuItem_CaptureEvent(mi, ev);
			if (evCaputred)
			{
				if (selectIndex != -1 && selectIndex != ctr)
				{
					/*--------------------------------------------------------*/
					/* a different menu item has captured this event.         */
					/* Deselect the old menu item.                            */
					/*--------------------------------------------------------*/
					NEUIK_MenuItem_Deselect(m->subList[selectIndex]);
				}
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (keyRight/keyLeft).         */
	/*------------------------------------------------------------------------*/
	selectIndex = -1;
	
	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_KEYDOWN:
		keyEv = (SDL_KeyboardEvent*)(e);
		switch (keyEv->keysym.sym)
		{
		case SDLK_UP:
		case SDLK_DOWN:
			/* Identify the position of the currently selected menu */
			if (m->subList == NULL)
			{
				/* event captured; no effect (no menu items) */
				evCaputred = 1;
				goto out;
			}
			for (ctr = 0;; ctr++)
			{
				mi = m->subList[ctr];
				if (mi == NULL)
				{
					finalIndex = ctr - 1;
					break;
				}
				if (mi->selected)
				{
					selectIndex = ctr;
				}
			}

			if (finalIndex == -1)
			{
				/* event captured; no effect (no menu items) */
				evCaputred = 1;
				goto out;
			}

			/*----------------------------------------------------------------*/
			/* Move the index of the selected menuItem                        */
			/*----------------------------------------------------------------*/
			if (selectIndex == -1)
			{
				/* No menuItem currently selected */
				if (keyEv->keysym.sym == SDLK_DOWN)
				{
					selectIndex = 0;
				}
				else /* SDLK_UP */
				{
					selectIndex = finalIndex;
				}
			}
			else
			{
				NEUIK_MenuItem_Deselect(m->subList[selectIndex]);
				if (keyEv->keysym.sym == SDLK_DOWN)
				{
					if (selectIndex < finalIndex) 
					{
						selectIndex++;
					}
				}
				else /* SDLK_UP */
				{
					if (selectIndex > 0)
					{
						selectIndex--;
					}
				}
			}
			m->subList[selectIndex]->selected = 1;

			evCaputred = 1;
			break;
		}
	}
out:
	return evCaputred;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_Menu_StoreSizeAndLocation
 *
 *  Description:   Store the size and location of this item.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_Menu_StoreSizeAndLocation(
		NEUIK_Menu  *m,
		RenderSize   size,
		RenderLoc    loc)
{
	m->size = size;
	m->loc = loc;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_Menu_Deselect
 *
 *  Description:   Deselect this menu and any selected subitems.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_Menu_Deselect(
		NEUIK_Menu   *m)
{
	int ctr = 0;
	NEUIK_MenuItem  *mi;

	m->selected = 0;

	if (m->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)m->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			NEUIK_MenuItem_Deselect(mi);
		}
	}
}

/*******************************************************************************
 *
 *  Name:          neuik_Menu_SetWindowPointer
 *
 *  Description:   Set the window pointer for this and all subitems.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_Menu_SetWindowPointer(
		NEUIK_Menu  *m,
		void        *win)
{
	int               ctr = 0;
	NEUIK_MenuItem *  mi;

	if (m == NULL) return;

	m->window = win;
	if (m->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			mi = (NEUIK_MenuItem*)m->subList[ctr];
			if (mi == NULL)
			{
				break;
			}

			neuik_MenuItem_SetWindowPointer(mi, win);
		}
	}
}
