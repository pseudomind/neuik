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

#include "NEUIK_Callback.h"
#include "NEUIK_error.h"

#define N_BINDING_STACK       250
#define N_BINDING_STACK_FINAL 249

static unsigned int neuik_bindingStack[N_BINDING_STACK];
static int          neuik_bindingStack_Start      = 0;
static int          neuik_bindingStack_Stop       = 0;
// static int          neuik_bindingStack_ReadBlock  = 0;


/*******************************************************************************
 *
 *  Name:          NEUIK_NewCallbackTable
 *
 *  Description:   Return a prepared NEUIK_CallbackTable.
 *
 *  Returns:       A NULLed out NEUIK_CallbackTable.
 *
 ******************************************************************************/
NEUIK_CallbackTable NEUIK_NewCallbackTable()
{
    NEUIK_CallbackTable  cbt;

    cbt.CustomEvents  = NULL;
    cbt.OnClick       = NULL;
    cbt.OnClicked     = NULL;
    cbt.OnCreated     = NULL;
    cbt.OnHover       = NULL;
    cbt.OnMouseEnter  = NULL;
    cbt.OnMouseLeave  = NULL;
    cbt.OnMouseOver   = NULL;
    cbt.OnSelected    = NULL;
    cbt.OnDeselected  = NULL;
    cbt.OnActivated   = NULL;
    cbt.OnDeactivated = NULL;
    cbt.OnTextChanged = NULL;
    cbt.OnExpanded    = NULL;
    cbt.OnCollapsed   = NULL;
    cbt.OnCursorMoved = NULL;

    return cbt;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewCallback
 *
 *  Description:   Create and return a pointer to a new NEUIK_Callback.
 *
 *  Returns:       NULL if error, otherwise a valid NEUIK_Callback.
 *
 ******************************************************************************/
NEUIK_Callback * NEUIK_NewCallback(
    void  * cbFunc, 
    void  * cbArg1, 
    void  * cbArg2)
{
    int              eNum       = 0; /* which error to report (if any) */
    NEUIK_Callback * cb         = NULL;
    static char      funcName[] = "NEUIK_NewCallback";
    static char    * errMsgs[]  = {"", // [0] no error
        "Failure to allocate memory.", // [1]
    };

    cb = (NEUIK_Callback*) malloc(sizeof(NEUIK_Callback));
    if (cb == NULL)
    {
        eNum = 1;
        goto out;
    }

    cb->cbFn              = cbFunc;
    cb->cbArg1            = cbArg1;
    cb->cbArg2            = cbArg2;
    cb->isBindingCallback = 0;
    cb->bindID            = 0;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return cb;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewBindingCallback
 *
 *  Description:   Create and return a pointer to a new (Binding) Callback.
 *
 *  Returns:       NULL if error, otherwise a valid NEUIK_Callback.
 *
 ******************************************************************************/
NEUIK_Callback * NEUIK_NewBindingCallback(
    unsigned int bindID)
{
    int              eNum       = 0; /* which error to report (if any) */
    NEUIK_Callback * cb         = NULL;
    static char      funcName[] = "NEUIK_NewBindingCallback";
    static char    * errMsgs[]  = {"", // [0] no error
        "Failure to allocate memory.", // [1]
    };

    cb = (NEUIK_Callback*) malloc(sizeof(NEUIK_Callback));
    if (cb == NULL)
    {
        eNum = 1;
        goto out;
    }

    cb->isBindingCallback = 1;
    cb->bindID            = bindID;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return cb;
}

/*******************************************************************************
 *
 *  Name:          neuik_PushBindingCallbackToStack
 *
 *  Description:   Push the callback Binding ID on to the Binding Callback 
 *                 stack.
 *
 ******************************************************************************/
void neuik_PushBindingCallbackToStack(
    unsigned int bindID)
{
    int           nAttempts;
    int           eNum        = 0; /* which error to report (if any) */
    static int    maxAttempts = 25;
    static int    writeBlock  = 0;
    static char   funcName[]  = "neuik_PushBindingCallbackToStack";
    static char * errMsgs[]   = {"",                       // [0] no error
        "Obtaining a write block took too long.",          // [1]
        "Obtaining an opening in the stack took to long.", // [2]
    };

    /*------------------------------------------------------------------------*/
    /* Wait until we can obtain a write block on the callback stack           */
    /*------------------------------------------------------------------------*/
    if (writeBlock)
    {
        for (nAttempts = 0; nAttempts < maxAttempts; nAttempts++) 
        {
            SDL_Delay(1);
            if (!writeBlock)
            {
                writeBlock = 1;
                break;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Too many failed attempts at obtaining a write block.               */
        /*--------------------------------------------------------------------*/
        if (nAttempts == maxAttempts)
        {
            eNum = 1;
            goto out;
        }
    }
    else
    {
        writeBlock = 1;
    }
    /*------------------------------------------------------------------------*/
    /* Write block obtained on the callback stack                             */
    /*------------------------------------------------------------------------*/
    /* First check to make sure that the stack is not full                    */
    /*------------------------------------------------------------------------*/
    if ((neuik_bindingStack_Stop == 0 
            && neuik_bindingStack_Start == N_BINDING_STACK_FINAL) ||
        (neuik_bindingStack_Stop == neuik_bindingStack_Start - 1))
    {
        for (nAttempts = 0; nAttempts < maxAttempts; nAttempts++) 
        {
            SDL_Delay(1);
            if (!((neuik_bindingStack_Stop == 0 
                    && neuik_bindingStack_Start == N_BINDING_STACK_FINAL) ||
                (neuik_bindingStack_Stop == neuik_bindingStack_Start - 1)))
            {
                break;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Too many failed attempts at waiting for an opening in the stack.   */
        /*--------------------------------------------------------------------*/
        if (nAttempts == maxAttempts)
        {
            eNum = 2;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* There is an opening on the stack, write the binding ID there.          */
    /*------------------------------------------------------------------------*/
    neuik_bindingStack[neuik_bindingStack_Stop] = bindID;
    neuik_bindingStack_Stop++;

    if (neuik_bindingStack_Stop > N_BINDING_STACK_FINAL)
    {
        neuik_bindingStack_Stop = 0;
    }   
    writeBlock = 0;
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }
}

/*******************************************************************************
 *
 *  Name:          NEUIK_PopBindingCallbackFromStack
 *
 *  Description:   Pop a callback Binding ID from the Binding Callback stack.
 *                 stack.
 *
 *  Returns:       1 if a valid bindID was returned; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_PopBindingCallbackFromStack(
    unsigned int * bindID)
{
    int           nAttempts;
    int           isValid     = 0;
    int           eNum        = 0; /* which error to report (if any) */
    static int    maxAttempts = 25;
    static int    readBlock   = 0;
    static char   funcName[]  = "NEUIK_PopBindingCallbackFromStack";
    static char * errMsgs[]   = {"",             // [0] no error
        "Obtaining a read block took too long.", // [1]
    };

    /*------------------------------------------------------------------------*/
    /* Wait until we can obtain a read block on the callback stack            */
    /*------------------------------------------------------------------------*/
    if (readBlock)
    {
        for (nAttempts = 0; nAttempts < maxAttempts; nAttempts++) 
        {
            SDL_Delay(1);
            if (!readBlock)
            {
                readBlock = 1;
                break;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Too many failed attempts at obtaining a read block.                */
        /*--------------------------------------------------------------------*/
        if (nAttempts == maxAttempts)
        {
            eNum = 1;
            goto out;
        }
    }
    else
    {
        readBlock = 1;
    }

    /*------------------------------------------------------------------------*/
    /* Check to see if there are actually any bindIDs in the stack.           */
    /*------------------------------------------------------------------------*/
    if (neuik_bindingStack_Start  == neuik_bindingStack_Stop)
    {
        readBlock = 0;
        goto out;
    }
    else
    {
        isValid   = 1;
        (*bindID) = neuik_bindingStack[neuik_bindingStack_Start];
        neuik_bindingStack_Start++;
        if (neuik_bindingStack_Start > N_BINDING_STACK_FINAL)
        {
            neuik_bindingStack_Start = 0;
        }
        readBlock = 0;
    }
out:
    if (eNum != 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }
    return isValid;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_WaitForBindingCallback
 *
 *  Description:   Wait for a valid Binding ID to be poped from the Binding 
 *                 Callback stack.
 *
 *  Returns:       The Binding ID that was popped from the stack.
 *
 ******************************************************************************/
unsigned int NEUIK_WaitForBindingCallback(
    unsigned int msSleep)
{
    unsigned int bindID;

    for (;;)
    {
        if (NEUIK_PopBindingCallbackFromStack(&bindID))
        {
            return bindID;
        }
        SDL_Delay(msSleep);
    }
}



/*******************************************************************************
 *
 *  Name:          NEUIK_Callback_Trigger
 *
 *  Description:   Execute the callback funciton with the callback args.
 *
 ******************************************************************************/
void NEUIK_Callback_Trigger(
    NEUIK_Callback  * cb,
    void            * win)
{
    if (cb != NULL)
    {
        if (cb->isBindingCallback)
        {
            neuik_PushBindingCallbackToStack(cb->bindID);
        }
        else if (cb->cbFn != NULL)
        {
            cb->cbFn(win, cb->cbArg1, cb->cbArg2);
        }
    }
}



