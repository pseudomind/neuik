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
#include <stdlib.h>

#include "NEUIK_error.h"
#include "NEUIK_Event.h"
#include "NEUIK_Event_internal.h"
#include "NEUIK_Window.h"
#include "NEUIK_Window_internal.h"

extern int neuik__Report_About;

/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct {
    int             cap;
    NEUIK_Window ** windows;
} neuik_WindowArray;

/*----------------------------------------------------------------------------*/
/* Local Globals                                                              */
/*----------------------------------------------------------------------------*/
int windowArrayInit = 1;
neuik_WindowArray neuik_windows;


/*******************************************************************************
 *
 *  Name:          neuik_FreeAllWindows
 *
 *  Description:   Free all registered windows.
 *
 *  Returns:       0 = No Error; 1 otherwise. 
 *
 ******************************************************************************/
int neuik_FreeAllWindows()
{
    int           ctr;
    int           eNum = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_FreeAllWindows";
    static char * errMsgs[] = {"",        // [0] no error
        "Failed to free a NEUIK_Window.", // [1]
    };

    /*------------------------------------------------------------------------*/
    /* The window pointer array is not initialized (do nothing)               */
    /*------------------------------------------------------------------------*/
    if (windowArrayInit)
    {
        eNum = 0;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Find any valid window pointer and free the associated resources.       */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;ctr < neuik_windows.cap; ctr++)
    {
        if (neuik_windows.windows[ctr] != NULL)
        {
            if (NEUIK_Window_Free(neuik_windows.windows[ctr]))
            {
                eNum = 1;
                goto out;
            }
            neuik_windows.windows[ctr] = NULL;
        }
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
 *  Name:          neuik_RegisterWindow
 *
 *  Description:   Register (add to a tracked array) a window for event handling
 *                 purposes.
 *
 *  Returns:       0 = No Error; 1 otherwise. 
 *
 ******************************************************************************/
int neuik_RegisterWindow(
    NEUIK_Window * w)
{
    int           wasSet = 0;
    int           ctr;
    int           eNum = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterWindow";
    static char * errMsgs[] = {"",                          // [0] no error
        "Failed allocate memory for windowArray.",          // [1]
        "TODO Array reallocation not currently supported.", // [2]
    };

    /*------------------------------------------------------------------------*/
    /* Initialize the window pointer array (first run only)                   */
    /*------------------------------------------------------------------------*/
    if (windowArrayInit)
    {
        windowArrayInit = 0;
        neuik_windows.cap = 11;
        neuik_windows.windows = (NEUIK_Window**) malloc(11*sizeof(NEUIK_Window*));
        if (neuik_windows.windows == NULL)
        {
            eNum = 1;
            goto out;
        }

        for (ctr = 0;ctr < neuik_windows.cap; ctr++)
        {
            neuik_windows.windows[ctr] = NULL;
        }
    }
    if (w == NULL)
    {
        /*--------------------------------------------------------------------*/
        /* This may happen if an error occurred prior to the creation of the  */
        /* initial window                                                     */
        /*--------------------------------------------------------------------*/
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Find an index in which to store the window pointer                     */
    /*------------------------------------------------------------------------*/
    for (ctr = 0;ctr < neuik_windows.cap; ctr++)
    {
        if (neuik_windows.windows[ctr] == NULL)
        {
            w->winID = ctr;
            neuik_windows.windows[ctr] = w;
            wasSet = 1;
            break;
        }
    }

    if (!wasSet)
    {
        eNum = 2;
        goto out;
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
 *  Name:          NEUIK_EventLoop
 *
 *  Description:   Start the event handling loop.
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_EventLoop(
    int killOnError) /* (1/0): whether or not NEUIK_HasErrors kills this loop */
{
    int                     ctr;
    int                     checkCtr   = 0;
    int                     checkMax   = 5;
    int                     evCaptured = 0;
    int                     didRedraw  = 0;
    static int              first      = 1;
    SDL_Event               event;
    NEUIK_Window          * win;

    if (NEUIK_HasErrors()) 
    {
        NEUIK_BacktraceErrors();
        NEUIK_ClearErrors();
        if (killOnError) goto out;
    }
    
    if (neuik__Report_About == 1)
    {
        neuik__Report_About = 2;
        printf(
            "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
            "| Developed using NEUIK (Nuclear Engineer's User Interface Kit) |\n"
            "|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\n"
            "| NOTE: The NEUIK project was started in 2014 by Michael Leimon |\n"
            "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }

    for (;;) {
        didRedraw = 0;
        SDL_PumpEvents();
        for (checkCtr = 0; checkCtr < checkMax; checkCtr++)
        {
            if (!SDL_PollEvent(&event))
            {
                /*------------------------------------------------------------*/
                /* No further events to handle, begin redraw.                 */
                /*------------------------------------------------------------*/
                break;
            }
            /*----------------------------------------------------------------*/
            /* Otherwise, there is an event to handle                         */
            /*----------------------------------------------------------------*/
            switch (event.type)
            {
            case SDL_QUIT:
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* Check the windows to see if they can capture this event        */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL)
                {
                    break;
                }

                evCaptured = NEUIK_Window_CaptureEvent(win, &event);
                if (NEUIK_HasErrors()) 
                {
                    NEUIK_BacktraceErrors();
                    NEUIK_ClearErrors();
                    if (killOnError) goto out;
                }
                if (evCaptured)
                {
                    break;
                }
            }           
        }

        if (!first)
        {
            /*----------------------------------------------------------------*/
            /* Only redraw windows which need to be redrawn                   */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL) break;

                if (win->doRedraw)
                {
                    NEUIK_Window_Redraw(win);
                    if (NEUIK_HasErrors()) 
                    {
                        NEUIK_BacktraceErrors();
                        NEUIK_ClearErrors();
                        if (killOnError) goto out;
                    }
                    didRedraw = 1;
                }
                if (win->updateTitle)
                {
                    SDL_SetWindowTitle(win->win, win->title);
                    win->updateTitle = 0;
                }
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* First time in in the event loop, Redraw all the windows        */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL) break;

                if (win->doRedraw)
                {
                    NEUIK_Window_Redraw(win);
                    if (NEUIK_HasErrors()) 
                    {
                        NEUIK_BacktraceErrors();
                        NEUIK_ClearErrors();
                        if (killOnError) goto out;
                    }
                    didRedraw = 1;
                }
            }
            first = 0;
        }


        if (!didRedraw)
        {
            /*----------------------------------------------------------------*/
            /* There were no events handle, just wait for a brief moment.     */
            /*----------------------------------------------------------------*/

            SDL_Delay(4);
            continue;
        }
    }
out:
    return;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_EventLoopNoErrHandling
 *
 *  Description:   Start the event handling loop (don't check for errors).
 *
 *  Returns:       Nothing!
 *
 ******************************************************************************/
void NEUIK_EventLoopNoErrHandling()
{
    int                     ctr;
    int                     checkCtr   = 0;
    int                     checkMax   = 5;
    int                     evCaptured = 0;
    int                     didRedraw  = 0;
    static int              first      = 1;
    SDL_Event               event;
    NEUIK_Window          * win;

    for (;;) {
        SDL_PumpEvents();
        for (checkCtr = 0; checkCtr < checkMax; checkCtr++)
        {
            if (!SDL_PollEvent(&event))
            {
                /*------------------------------------------------------------*/
                /* No further events to handle, begin redraw.                 */
                /*------------------------------------------------------------*/
                break;
            }
            /*----------------------------------------------------------------*/
            /* Otherwise, there is an event to handle                         */
            /*----------------------------------------------------------------*/
            switch (event.type)
            {
            case SDL_QUIT:
                goto out;
            }

            /*----------------------------------------------------------------*/
            /* Check the windows to see if they can capture this event        */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL)
                {
                    break;
                }

                evCaptured = NEUIK_Window_CaptureEvent(win, &event);
                if (evCaptured) 
                {
                    break;
                }
            }           
        }


        if (!first)
        {
            /*----------------------------------------------------------------*/
            /* Only redraw windows which need to be redrawn                   */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL) break;

                if (win->doRedraw)
                {
                    NEUIK_Window_Redraw(win);
                    didRedraw = 1;
                }
                if (win->updateTitle)
                {
                    SDL_SetWindowTitle(win->win, win->title);
                    win->updateTitle = 0;
                }
            }
        }
        else
        {
            /*----------------------------------------------------------------*/
            /* First time in in the event loop, Redraw all the windows        */
            /*----------------------------------------------------------------*/
            for (ctr=0;;ctr++)
            {
                win = neuik_windows.windows[ctr];
                if (win == NULL) break;

                if (win->doRedraw)
                {
                    NEUIK_Window_Redraw(win);
                    didRedraw = 1;
                }
            }
            first = 0;
        }

        if (!didRedraw)
        {
            /*----------------------------------------------------------------*/
            /* There were no events handle, just wait for a brief moment.     */
            /*----------------------------------------------------------------*/

            SDL_Delay(4);
            continue;
        }
    }
out:
    return;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewEventHandler
 *
 *  Description:   Create and return a pointer to a new NEUIK_EventHandler.
 *
 *  Returns:       NULL if error, otherwise a valid NEUIK_EventHandler.
 *
 ******************************************************************************/
NEUIK_EventHandler * NEUIK_NewEventHandler(
    void * eHFunc, 
    void * eHArg1, 
    void * eHArg2)
{
    int                   eNum = 0; /* which error to report (if any) */
    NEUIK_EventHandler  * eH   = NULL;
    static char           funcName[] = "NEUIK_NewEventHandler";
    static char         * errMsgs[] = {"",                // [0] no error
        "Failure to allocate memory.\n", // [1]
    };

    eH = (NEUIK_EventHandler*) malloc(sizeof(NEUIK_EventHandler));
    if (eH == NULL)
    {
        eNum = 1;
        goto out;
    }

    eH->eHFn   = eHFunc;
    eH->eHArg1 = eHArg1;
    eH->eHArg2 = eHArg2;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eH;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewEventHandlerTable
 *
 *  Description:   Return a prepared NEUIK_NewEventHandlerTable.
 *
 *  Returns:       A NULLed out NEUIK_NewEventHandlerTable.
 *
 ******************************************************************************/
NEUIK_EventHandlerTable NEUIK_NewEventHandlerTable()
{
    NEUIK_EventHandlerTable  eht;

    eht.Before   = NULL;
    eht.After    = NULL;
    eht.Override = NULL;

    return eht;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_EventHandler_Capture
 *
 *  Description:   Attempt to capture an SDL_Event using a specified EventHandler.
 *
 *  Returns:       Non-zero if there is an error; zero otherwise.
 *
 ******************************************************************************/
int NEUIK_EventHandler_Capture(
    NEUIK_EventHandler  * eH,
    void                * container,
    int                 * captured,
    ptrTo_SDL_Event       ev)
{
    int errRV = 0;

    if (eH != NULL)
    {
        if (eH->eHFn != NULL)
        {
            errRV = eH->eHFn(container, ev, captured, eH->eHArg1, eH->eHArg2);
        }
    }

    return errRV;
}

