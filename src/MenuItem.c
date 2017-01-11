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
#include <stdlib.h>
#include <string.h>

#include "NEUIK_render.h"

#include "MenuItem.h"


NEUIK_MenuItem* NEUIK_NewMenuItem(
	const char *name)
{
	int sLen = 1;
	NEUIK_MenuItem *rvM = NULL;

	rvM = (NEUIK_MenuItem*) malloc(sizeof(NEUIK_MenuItem));
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
	rvM->selected     = 0;
	rvM->subList      = NULL;
	rvM->callbackFn   = NULL;
	rvM->callbackArg1 = NULL;
	rvM->callbackArg2 = NULL;

out:
	return rvM;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_GetSize
 *
 *  Description:   Returns the rendered size of a given menuItem.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MenuItem_GetSize(
		const NEUIK_MenuItem  *mItem,
		int                    includeSubMenu,
		RenderSize            *rSize)
{
	int               tW;
	int               tH;
	int               ctr   = 0;
	int               rvErr = 0;
	RenderSize        rs;
	NEUIK_MenuItem  * mi    = NULL;
	TTF_Font        * font  = NULL;

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	if (mItem == NULL)
	{
		rSize->w = -1;
		rSize->h = -1;
		rvErr = 1;
		goto out;
	}
	if (mItem->cfg == NULL)
	{
		rSize->w = -2;
		rSize->h = -2;
		rvErr = 1;
		goto out;
	} 
	font = NEUIK_FontSet_GetFont(mItem->cfg->fontSet, mItem->cfg->fontSize, 0, 0);
	if (font == NULL) 
	{
		rSize->w = -3;
		rSize->h = -3;
		rvErr = 1;
		goto out;
	}

	TTF_SizeText(font, mItem->name, &tW, &tH);
	rSize->w = tW + (1.5)*mItem->cfg->fontEmWidth;
	rSize->h = mItem->cfg->height;

	/*------------------------------------------------------------------------*/
	/* If selected, the size of this menuItem button may need to be expanded  */
	/*------------------------------------------------------------------------*/
	if (mItem->selected)
	{
		if (mItem->subList == NULL) {
			/* there are no submenus contained by this mItem */
			goto out;
		}

		if (includeSubMenu)
		{
			for (ctr = 0;; ctr++)
			{
				mi = (NEUIK_MenuItem*)mItem->subList[ctr];
				if (mi == NULL)
				{
					break;
				}
				NEUIK_MenuItem_GetSize(mi, 1, &rs);

				if (mi->selected) 
				{
					/* this item has a submenu which is being displayed */
					rSize->w += rs.w;
				}
				if (rs.h > rSize->h)
				{
					/* this item has a submenu which is being displayed */
					rSize->h = rs.h;
				}
			}
		}

	}

out:
	return rvErr;
}


void NEUIK_MenuItem_SetConfig(
		NEUIK_MenuItem    *mi,
		NEUIK_MenuConfig  *cfg)
{
	int              ctr;
	NEUIK_MenuItem  *si;

	if (mi  == NULL) return;
	if (cfg == NULL) return;
	mi->cfg = cfg;

	/*------------------------------------------------------------------------*/
	/* If this menu contains a subList, set the config on those items as well */
	/*------------------------------------------------------------------------*/
	if (mi->subList == NULL) return;
	for (ctr = 0;; ctr++)
	{
		si = (NEUIK_MenuItem*)(mi->subList[ctr]);
		if (si == NULL)
		{
			break;
		}
		NEUIK_MenuItem_SetConfig(si, cfg);
	}
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_SetCallbackFunc
 *
 *  Description:   Set a pointer for the function to call when activated.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
// void NEUIK_MenuItem_SetCallbackFunc(
// 		NEUIK_MenuItem   *mi,
// 		void             *cbFunc)
// {
// 	if (mi != NULL)
// 	{
// 		mi->callbackFn = cbFunc;
// 	}
// }

/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_SetCallbackFunc
 *
 *  Description:   Set a pointer to a function (and optionally up to two user 
 *                 specifiable arguments) to call when activated.
 *
 *                 Note to end users: If one or both of the optional arguments
 *                 are not needed, it is recommended practice to pass NULL 
 *                 pointers to those unneeded arguments.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_MenuItem_SetCallbackFunc(
		NEUIK_MenuItem  * mi,     /* The menu item to set the cbFunc for */
		void            * cbFunc, /* The function to call when activated */
		void            * cbArg1, /* [optional] User specifiable arg to cbFunc */
		void            * cbArg2) /* [optional] User specifiable arg to cbFunc */
{
	if (mi != NULL)
	{
		mi->callbackFn   = cbFunc;
		mi->callbackArg1 = cbArg1;
		mi->callbackArg2 = cbArg2;
	}
}


/*******************************************************************************
 *
 *  Name:          NEUIK_RenderMenuItem
 *
 *  Description:   Renders a single menu item as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
SDL_Texture*  NEUIK_RenderMenuItem(
		const NEUIK_MenuItem  *mi,
		int                    menuW,   /* the width of the items menu */
		RenderSize            *rSize,   /* the size this texture should occupy when complete */
		SDL_Renderer          *extRend) /* the external renderer to prepare the texture for */
{
	const NEUIK_Color  *fgClr = NULL;
	const NEUIK_Color  *bgClr = NULL;
	const NEUIK_Color  *bClr  = NULL; /* border color */
	SDL_Surface        *surf  = NULL;
	SDL_Renderer       *rend  = NULL;

	int            textW = 0;
	int            textH = 0;
	SDL_Rect       rect;
	RenderSize     rs;     /* the size this texture should occupy when complete */
	TTF_Font     * font  = NULL;
	SDL_Texture  * tTex  = NULL;
	SDL_Texture  * rvTex = NULL;

	/*------------------------------------------------------------------------*/
	/* Calculate the required size of the resultant texture                   */
	/*------------------------------------------------------------------------*/
	NEUIK_MenuItem_GetSize(mi, 1, rSize);

	if (extRend == NULL) 
	{
		/*---------------------------------------------------------*/
		/* Just return the required size for the resultant texture */
		/*---------------------------------------------------------*/
		goto out;
	}

	if (menuW > rSize->w)
	{
		rSize->w = menuW;
	}

	/*------------------------------------------------------------------------*/
	/* Create a surface and a software renderer on which to draw the button   */
	/*------------------------------------------------------------------------*/
	surf = SDL_CreateRGBSurface(0, rSize->w, rSize->h, 32, 0, 0, 0, 0);
	if (surf == NULL) goto out;

	rend = SDL_CreateSoftwareRenderer(surf);
	if (rend == NULL) goto out;

	NEUIK_MenuItem_GetSize(mi, 0, &rs);

	/*------------------------------------------------------------------------*/
	/* Fill the background with it's color                                    */
	/*------------------------------------------------------------------------*/
	if (mi->selected)
	{
		bgClr = &(mi->cfg->bgColorSelect);
		fgClr = &(mi->cfg->fgColorSelect);
	}
	else
	{
		/* use the unselected colors */
		bgClr = &(mi->cfg->bgColor);
		fgClr = &(mi->cfg->fgColor);
	}
	SDL_SetRenderDrawColor(rend, bgClr->r, bgClr->g, bgClr->b, 255);
	SDL_RenderClear(rend);

	/*------------------------------------------------------------------------*/
	/* Draw the menu border on the right side                                 */
	/*------------------------------------------------------------------------*/
	if (!mi->selected)
	{
		bClr = &(mi->cfg->sepColor);
	}
	else
	{
		bClr = &(mi->cfg->sepColorDark);
	}
	SDL_SetRenderDrawColor(rend, bClr->r, bClr->g, bClr->b, 255);
	SDL_RenderDrawLine(rend, rSize->w - 1, 0, rSize->w - 1, rSize->h - 1);

	/*------------------------------------------------------------------------*/
	/* Render the text                                                        */
	/*------------------------------------------------------------------------*/
	font = NEUIK_FontSet_GetFont(mi->cfg->fontSet, mi->cfg->fontSize, 0, 0);
	if (font == NULL) goto out;
	tTex = NEUIK_RenderText(mi->name, font, *fgClr, rend, &textW, &textH);
	if (tTex == NULL) goto out;

	rect.x = (int) ((float)(rs.w - textW)/2.0) + 0.3*mi->cfg->fontEmWidth;
	rect.y = (int) ((float)(rs.h - textH)/2.0);
	rect.w = textW;
	rect.h = 1.1*textH;
	SDL_RenderCopy(rend, tTex, NULL, &rect);

	/*------------------------------------------------------------------------*/
	/* Copy the text onto the renderer and update it                          */
	/*------------------------------------------------------------------------*/
	SDL_RenderPresent(rend);
	rvTex = SDL_CreateTextureFromSurface(extRend, surf);

out:
	if (surf != NULL) SDL_FreeSurface(surf);
	if (rend != NULL) SDL_DestroyRenderer(rend);
	ConditionallyDestroyTexture(&tTex);

	return rvTex;
}


SDL_Texture* 
RenderSubMenu(
		const NEUIK_MenuItem  **miList,
		    RenderSize  *rSize,
		  SDL_Renderer  *extRend);

/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_CaptureEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_MenuItem.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
int NEUIK_MenuItem_CaptureEvent(
		NEUIK_MenuItem * mi,
		SDL_Event      * ev)
{
	int ctr        = 0;
	int evCaputred = 0;
	NEUIK_MenuItem  *smi;

	SDL_Event             *e;
	SDL_KeyboardEvent     *keyEv;
	SDL_MouseMotionEvent  *mouseMotEv;
	SDL_MouseButtonEvent  *mouseButEv;
	
	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (mouseclick/mousemotion).   */
	/*------------------------------------------------------------------------*/
	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_MOUSEBUTTONDOWN:
		mouseButEv = (SDL_MouseButtonEvent*)(e);
		if (mouseButEv->y >= mi->loc.y && mouseButEv->y <= mi->loc.y + mi->size.h)
		{
			if (mouseButEv->x >= mi->loc.x && mouseButEv->x <= mi->loc.x + mi->size.w)
			{
				/* make sure all subitems are deselected */
				if (mi->callbackFn != NULL)
				{
					mi->callbackFn(mi->window, mi->callbackArg1, mi->callbackArg2);
				}

				mi->selected = 1;
				evCaputred = 1;
				goto out;
			}
		}
		break;
	case SDL_MOUSEMOTION:
		mouseMotEv = (SDL_MouseMotionEvent*)(e);
		if (mouseMotEv->y >= mi->loc.y && mouseMotEv->y <= mi->loc.y + mi->size.h)
		{
			if (mouseMotEv->x >= mi->loc.x && mouseMotEv->x <= mi->loc.x + mi->size.w)
			{
				/* make sure all subitems are deselected */
				mi->selected = 1;
				evCaputred = 1;
				goto out;
			}
		}
		break;
	}


	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (keyUp/keyDown).            */
	/*------------------------------------------------------------------------*/
	// TODO

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the one of the menu items.           */
	/*------------------------------------------------------------------------*/
	if (mi->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			smi = (NEUIK_MenuItem*)mi->subList[ctr];
			if (smi == NULL)
			{
				break;
			}

			evCaputred = NEUIK_MenuItem_CaptureEvent(smi, ev);
			if (evCaputred)
			{
				goto out;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Check if the event is captured by the menu (enter/space).             */
	/*------------------------------------------------------------------------*/
	if (mi->selected)
	{
		switch (e->type)
		{
		case SDL_KEYDOWN:
			keyEv = (SDL_KeyboardEvent*)(e);
			switch (keyEv->keysym.sym)
			{
			case SDLK_SPACE:
			case SDLK_RETURN:
				if (mi->callbackFn != NULL)
				{
					mi->callbackFn(mi->window, mi->callbackArg1, mi->callbackArg2);
				}

				mi->selected = 1;
				evCaputred = 1;
				goto out;
				break;
			}
		}		
	}

out:
	return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_StoreSizeAndLocation
 *
 *  Description:   Store the size and location of this item.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_MenuItem_StoreSizeAndLocation(
		NEUIK_MenuItem  *mi,
		RenderSize       size,
		RenderLoc        loc)
{
	mi->size = size;
	mi->loc = loc;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MenuItem_Deselect
 *
 *  Description:   Deselect this menuItem and any selected subitems.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_MenuItem_Deselect(
		NEUIK_MenuItem   *mi)
{
	int ctr = 0;
	NEUIK_MenuItem  *smi;

	mi->selected = 0;

	if (mi->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			smi = (NEUIK_MenuItem*)mi->subList[ctr];
			if (smi == NULL)
			{
				break;
			}

			NEUIK_MenuItem_Deselect(smi);
		}
	}
}


/*******************************************************************************
 *
 *  Name:          neuik_MenuItem_SetWindowPointer
 *
 *  Description:   Set the window pointer for this and all subitems.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_MenuItem_SetWindowPointer(
		NEUIK_MenuItem  *mi,
		void            *win)
{
	int ctr = 0;
	NEUIK_MenuItem  *smi;

	if (mi == NULL) return;
	mi->window = win;

	if (mi->subList != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			smi = (NEUIK_MenuItem*)mi->subList[ctr];
			if (smi == NULL)
			{
				break;
			}

			neuik_MenuItem_SetWindowPointer(smi, win);
		}
	}
}