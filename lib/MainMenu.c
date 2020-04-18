/*******************************************************************************
 * Copyright (c) 2014-2020, Michael Leimon <leimon@gmail.com>
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

#include "NEUIK_render.h"
#include "NEUIK_colors.h"
#include "NEUIK_error.h"
#include "NEUIK_Window.h"

#include "MainMenu.h"
#include "MenuConfig.h"
#include "Menu_internal.h"


NEUIK_MainMenu* NEUIK_NewMainMenu(
        void              *win,
        NEUIK_MenuConfig  *mCfg, /* the new menu config to use */
        int                doStretch)
{
    int              eNum = 0; /* which error to report (if any) */
    NEUIK_MainMenu * rvMM = NULL;
    static char      funcName[] = "NEUIK_NewMainMenu";
    static char    * errMsgs[] = {"",          // [0] no error
        "GetDefaultMenuConfig returned NULL.", // [1]
    };

    rvMM = (NEUIK_MainMenu*) malloc(sizeof(NEUIK_MainMenu));
    if (rvMM == NULL) goto out;

    rvMM->size.w = 0;
    rvMM->size.h = 0;
    rvMM->menus = NULL;
    rvMM->cfg = mCfg;

    if (mCfg != NULL)
    {
        (rvMM->size).h = mCfg->height;
    }
    else
    {
        rvMM->cfg = NEUIK_GetDefaultMenuConfig();
        if (rvMM->cfg == NULL)
        {
            eNum = 1;
            goto out;
        }
    }

    rvMM->doStretch = doStretch;
    rvMM->window    = win;

out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return rvMM;
}


SDL_Texture* NEUIK_RenderMainMenu(
        NEUIK_MainMenu  *mmenu,
        RenderSize      *rSize,   /* output if extRend=NULL, input otherwise */
        SDL_Renderer    *extRend)
{
    int                 ctr    = 0;
    int                 xPos   = 0;
    int                 eNum   = 0; /* which error to report (if any) */
    SDL_Surface       * surf   = NULL;
    SDL_Renderer      * rend   = NULL;
    SDL_Texture       * tex    = NULL;
    const NEUIK_Color * sClr   = NULL; /* separator color */
    SDL_Rect            rect;
    SDL_Rect            bgRect = {0, 0, 0, 0};
    SDL_Color           tClr   = COLOR_TRANSP;
    RenderSize          rs;
    RenderLoc           rl;
    NEUIK_Menu        * m;
    static char         funcName[] = "NEUIK_RenderMainMenu";
    static char       * errMsgs[] = {"",        // [0] no error
        "Failed to create RGB surface.",        // [1]
        "Failed to create software renderer.",  // [2]
        "RenderMenu returned NULL.",            // [3]
    };

    const NEUIK_Color *clr  = NULL;
    SDL_Texture  *rvTex  = NULL;

    NEUIK_MainMenu_GetSize(mmenu, rSize);

    /*------------------------------------------------------------------------*/
    /* Create a surface and a software renderer on which to draw the menu     */
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
    clr = &(mmenu->cfg->bgColor);
    SDL_SetRenderDrawColor(rend, clr->r, clr->g, clr->b, 255);
    bgRect.w = rSize->w;
    bgRect.h = mmenu->cfg->height;
    SDL_RenderFillRect(rend, &bgRect);

    /*------------------------------------------------------------------------*/
    /* Draw the menu separator line below the main menu element               */
    /*------------------------------------------------------------------------*/
    sClr = &(mmenu->cfg->sepColor);
    SDL_SetRenderDrawColor(rend, sClr->r, sClr->g, sClr->b, 255);
    SDL_RenderDrawLine(rend, 0, bgRect.h - 1, bgRect.w - 1, bgRect.h - 1);

    /*------------------------------------------------------------------------*/
    /* Draw the menus onto the main menu                                      */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        m = mmenu->menus[ctr];
        if (m == NULL)
        {
            break;
        }

        ConditionallyDestroyTexture(&tex);
        tex = NEUIK_RenderMenu(m, &rs, rend);
        if (tex == NULL) 
        {
            eNum = 3;
            goto out;
        }

        rect.x = xPos;
        rect.y = 0;
        rect.w = rs.w;
        rect.h = rs.h;
        SDL_RenderCopy(rend, tex, NULL, &rect);
        NEUIK_Menu_GetSize(m, 0, &rs); /* get the size w/o submenu(s) */
        xPos += rs.w;

        rl.x = (mmenu->loc).x + rect.x;
        rl.y = (mmenu->loc).y;
        NEUIK_Menu_StoreSizeAndLocation(m, rs, rl);
    }

    /*------------------------------------------------------------------------*/
    /* Copy the text onto the renderer and update it                          */
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

    return rvTex;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_GetSize
 *
 *  Description:   Get the current size of the main menu.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_MainMenu_GetSize(
        NEUIK_MainMenu  *mmenu, /* the main menu to add the menu to */
        RenderSize      *rSize)
{
    int winW, winH;
    int rvErr = 0;
    int ctr   = 0;
    int xPos  = 0;
    RenderSize   rs;
    NEUIK_Menu        *m;

    /*-----------------------------------------------------------------*/
    /* Just return the minimum required size for the resultant texture */
    /*-----------------------------------------------------------------*/
    if (mmenu->menus == NULL) {
        /* there are no menus contained by this main menu, return [0,0] */
        rSize->w = 0;
        rSize->h = 0;
        goto out;
    }

    // rSize->h = (mmenu->rSize).h;
    rSize->h = (mmenu->cfg->height);

    for (ctr = 0;; ctr++)
    {
        m = mmenu->menus[ctr];
        if (m == NULL)
        {
            break;
        }
        NEUIK_Menu_GetSize(m, 1, &rs);
        xPos += rs.w;
        if (rs.h > rSize->h)
        {
            rSize->h = rs.h;
        }
    }

    rSize->w = xPos;

    if (mmenu->doStretch && ((NEUIK_Window*)mmenu->window)->win != NULL)
    {
        /* set width as the current window width */
        SDL_GetWindowSize(((NEUIK_Window*)mmenu->window)->win, &winW, &winH);
        rSize->w = winW;
    }

out:
    return rvErr;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_AddMenu
 *
 *  Description:   Add a menu to this NEUIK_MainMenu
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_MainMenu_AddMenu(
        NEUIK_MainMenu  *mm, /* the main menu to add the menu to */
        NEUIK_Menu      *m)  /* the menu to add */
{
    int           len  = 0;
    int           ctr  = 0;
    int           eNum = 0; /* which error to report (if any) */
    static char   funcName[] = "NEUIK_MainMenu_AddMenu";
    static char * errMsgs[] = {"",    // [0] no error
        "Unable to allocate memory.", // [1]
    };

    if (m == NULL) goto out;
    if (mm == NULL) goto out;
    if (mm->menus == NULL)
    {
        /*-------------------------------------------------------------*/
        /* This is the first menu to be added, allocate initial memory */
        /* This pointer array will be null terminated.                 */
        /*-------------------------------------------------------------*/
        mm->menus = (NEUIK_Menu**)malloc(2*sizeof(NEUIK_Menu*));
        if (mm->menus == NULL)
        {
            eNum = 1;
            goto out;
        }
        NEUIK_Menu_SetConfig(m, mm->cfg);
        m->window = mm->window;
        mm->menus[0] = m;
        mm->menus[1] = NULL;
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
            if (mm->menus[ctr] == NULL)
            {
                len = 2 + ctr;
                break;
            }
        }

        mm->menus = (NEUIK_Menu**)realloc(mm->menus, len*sizeof(NEUIK_Menu*));
        if (mm->menus == NULL)
        {
            eNum = 1;
            goto out;
        }
        NEUIK_Menu_SetConfig(m, mm->cfg);
        m->window = mm->window;
        mm->menus[ctr]   = m;
        mm->menus[ctr+1] = NULL;

    }

out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }
    return eNum;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_SetConfig
 *
 *  Description:   Set a MenuConfig for this NEUIK_MainMenu.
 *
 *  Returns:       1 if there is an error, 0 otherwise
 *
 ******************************************************************************/
int NEUIK_MainMenu_SetConfig(
        NEUIK_MainMenu    *mm,   /* the main menu to add the menu to */
        NEUIK_MenuConfig  *mCfg) /* the new menu config to use */
{
    int rv = 0;
    if (mm == NULL)
    {
        rv = 1;
        goto out;
    }

    mm->cfg = mCfg;

out:
    return rv;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_CaptureEvent
 *
 *  Description:   Check to see if this event is captured by a NEUIK_MainMenu.
 *
 *  Returns:       1 if event is captured; 0 otherwise
 *
 ******************************************************************************/
int NEUIK_MainMenu_CaptureEvent(
    NEUIK_MainMenu * mm,
    SDL_Event      * ev)
{
    int ctr         = 0;
    int evCaputred  = 0;
    int selectIndex = -1;
    int finalIndex  = -1;
    int setIsActive = 0;

    SDL_KeyboardEvent  *keyEv;
    SDL_Event          *e;
    NEUIK_Menu         *m;

    /*------------------------------------------------------------------------*/
    /* See if any of the menus are selected, if so set them all to isActive.  */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        m = mm->menus[ctr];
        if (m == NULL)
        {
            break;
        }

        if (m->selected)
        {
            selectIndex = ctr;
            setIsActive = 1;
            break;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Set the state of `isActive` for all menus. This will either allow them */
    /* to capture mouse motion events or not.                                 */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        m = mm->menus[ctr];
        if (m == NULL)
        {
            break;
        }

        m->isActive = setIsActive;
    }

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the main menu.                       */
    /*------------------------------------------------------------------------*/
    // TODO

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the one of the sub menus.            */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;; ctr++)
    {
        m = mm->menus[ctr];
        if (m == NULL)
        {
            break;
        }

        evCaputred = NEUIK_Menu_CaptureEvent(m, ev);
        if (evCaputred && m->selected)
        {
            mm->isActive = 1;
            if (selectIndex != ctr && selectIndex != -1)
            {
                /*------------------------------------------------------------*/
                /* A different menu is now selected from what was selected.   */
                /* Deselect the former menu.                                  */
                /*------------------------------------------------------------*/
                NEUIK_Menu_Deselect(mm->menus[selectIndex]);
            }
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Check if the event is captured by the main menu.                       */
    /*------------------------------------------------------------------------*/
    selectIndex = -1;

    if (mm->isActive)
    {
        e = (SDL_Event*)ev;
        switch (e->type)
        {
        case SDL_KEYDOWN:
            keyEv = (SDL_KeyboardEvent*)(e);
            switch (keyEv->keysym.sym)
            {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                /* Identify the position of the currently selected menu */
                for (ctr = 0;; ctr++)
                {
                    m = mm->menus[ctr];
                    if (m == NULL)
                    {
                        finalIndex = ctr - 1;
                        break;
                    }
                    if (m->selected)
                    {
                        selectIndex = ctr;
                    }
                }

                if (selectIndex != -1 && finalIndex > 0)
                {
                    NEUIK_Menu_Deselect(mm->menus[selectIndex]);
                    if (keyEv->keysym.sym == SDLK_LEFT)
                    {
                        selectIndex--;
                        if (selectIndex < 0) 
                        {
                            selectIndex = finalIndex;
                        }
                    }
                    else
                    {
                        selectIndex++;
                        if (selectIndex > finalIndex) 
                        {
                            selectIndex = 0;
                        }
                    }
                    mm->menus[selectIndex]->selected = 1;
                }
                evCaputred = 1;
                break;
            }
        }
    }

out:
    return evCaputred;
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_StoreSizeAndLocation
 *
 *  Description:   Store the size and location of this item.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_MainMenu_StoreSizeAndLocation(
        NEUIK_MainMenu  *mm,
        RenderSize       size,
        RenderLoc        loc)
{
    mm->size = size;
    mm->loc = loc;
}


/*******************************************************************************
 *
 *  Name:          neuik_MainMenu_SetWindowPointer
 *
 *  Description:   Set the window pointer for this and all subitems.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void neuik_MainMenu_SetWindowPointer(
        NEUIK_MainMenu    *mm,
        void              *win)
{
    int ctr = 0;
    NEUIK_Menu  *m;

    if (mm == NULL) return;

    mm->window = win;
    if (mm->menus != NULL)
    {
        /*--------------------------------------------------------------------*/
        /* Propagate this information to all menus and subitems               */
        /*--------------------------------------------------------------------*/
        for (ctr = 0;; ctr++)
        {
            m = mm->menus[ctr];
            if (m == NULL)
            {
                break;
            }

            neuik_Menu_SetWindowPointer(m, win);
        }
    }
}

/*******************************************************************************
 *
 *  Name:          NEUIK_MainMenu_Deselect
 *
 *  Description:   Deselect the mainmenu and all contained menus.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_MainMenu_Deselect(
        NEUIK_MainMenu   * mm)
{
    int ctr = 0;
    NEUIK_Menu  * m;

    if (mm->isActive)
    {
        mm->isActive = 0;
        if (mm->menus != NULL)
        {
            for (ctr = 0;; ctr++)
            {
                m = (NEUIK_Menu*)mm->menus[ctr];
                if (m == NULL)
                {
                    break;
                }

                if (m->selected)
                {
                    NEUIK_Menu_Deselect(m);
                }
            }
        }
    }

}


