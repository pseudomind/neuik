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
#include "NEUIK_WindowConfig.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_error.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__WindowConfig(void ** cfg);
int neuik_Object_Copy__WindowConfig(void * dst, const void * src);
int neuik_Object_Free__WindowConfig(void * cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_WindowConfig_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__WindowConfig,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__WindowConfig,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__WindowConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_WindowConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_WindowConfig()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_WindowConfig";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",         // [1]
        "Failed to register `WindowConfig` object class .", // [2]
    };

    if (!neuik__isInitialized)
    {
        eNum = 1;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Otherwise, register the object                                         */
    /*------------------------------------------------------------------------*/
    if (neuik_RegisterClass(
        "NEUIK_WindowConfig",                     // className
        "Configuration for NEUIK_Window Object.", // classDescription
        neuik__Set_NEUIK,                         // classSet
        NULL,                                     // superClass
        &neuik_WindowConfig_BaseFuncs,            // baseFuncs
        NULL,                                     // classFuncs
        &neuik__Class_WindowConfig))              // newClass
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
 *  Name:          NEUIK_GetDefaultWindowConfig
 *
 *  Description:   Returns a pointer to the initialized default Window 
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_WindowConfig.
 *
 ******************************************************************************/
NEUIK_WindowConfig * NEUIK_GetDefaultWindowConfig()
{
    static int                isInitialized = 0;
    static NEUIK_Color        clrBG         = COLOR_LLGRAY;
    static NEUIK_WindowConfig dCfg;

    if (!isInitialized) 
    {
        isInitialized = 1;

        neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_WindowConfig, 
            NULL,
            &(dCfg.objBase));

        dCfg.colorBG      = clrBG;
        dCfg.autoResizeW  = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
        dCfg.autoResizeH  = NEUIK_WINDOW_RESIZE_ONLY_EXPAND;
        dCfg.canResizeW   = NEUIK_WINDOW_RESIZE_ANY;
        dCfg.canResizeH   = NEUIK_WINDOW_RESIZE_ANY;
        dCfg.isResizable  = 0;
        dCfg.isBorderless = 0;
        dCfg.isFullscreen = 0;
        dCfg.isMaximized  = 0;
        dCfg.isMinimized  = 0;
    }

    return &dCfg;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__WindowConfig(
    void ** cfg)
{
    return NEUIK_NewWindowConfig((NEUIK_WindowConfig **)cfg);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_WindowConfig_New
 *
 *  Description:   Allocate memory and set default values for WindowConfig.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_NewWindowConfig(
    NEUIK_WindowConfig ** cfgPtr)
{
    int                  eNum       = 0;
    NEUIK_WindowConfig * cfg        = NULL;
    static char          funcName[] = "NEUIK_NewWindowConfig";
    static char        * errMsgs[]  = {"", // [0] no error
        "Output Argument cfgPtr is NULL.", // [1]
        "Failure to allocate memory.",     // [2]
        "Failure in WindowConfig_Copy().", // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_WindowConfig*) malloc(sizeof(NEUIK_WindowConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of WindowConfig                            */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_WindowConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new WindowConfig             */
    /*------------------------------------------------------------------------*/
    if (NEUIK_WindowConfig_Copy((cfg), NEUIK_GetDefaultWindowConfig()))
    {
        eNum = 3;
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
 *  Name:          neuik_Object_Copy__WindowConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__WindowConfig(
    void        * dst,
    const void  * src)
{
    return NEUIK_WindowConfig_Copy(
        (NEUIK_WindowConfig *)dst, (const NEUIK_WindowConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_Window_CopyConfig
 *
 *  Description:   Copy the data in a WindowConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_WindowConfig_Copy(
    NEUIK_WindowConfig        * dst,
    const NEUIK_WindowConfig  * src)
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "NEUIK_WindowConfig_Copy";
    static char  * errMsgs[]  = {"", // [0] no error
        "Argument `src` is invalid or an incorrect type.", // [1]
        "Argument `dst` is invalid or an incorrect type.", // [2]

    };

    if (!neuik_Object_IsClass(src, neuik__Class_WindowConfig))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst, neuik__Class_WindowConfig))
    {
        eNum = 2;
        goto out;
    }

    dst->colorBG      = src->colorBG;
    dst->autoResizeW  = src->autoResizeW;
    dst->autoResizeH  = src->autoResizeH;
    dst->canResizeW   = src->canResizeW;
    dst->canResizeH   = src->canResizeH;
    dst->isResizable  = src->isResizable;
    dst->isBorderless = src->isBorderless;
    dst->isFullscreen = src->isFullscreen;
    dst->isMaximized  = src->isMaximized;
    dst->isMinimized  = src->isMinimized;
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
 *  Name:          neuik_Object_Free__WindowConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__WindowConfig(
    void * cfg)
{
    return NEUIK_WindowConfig_Free((NEUIK_WindowConfig*)cfg);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_WindowConfig_Free
 *
 *  Description:   Free memory allocated for this object and NULL out pointer.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_WindowConfig_Free(
    NEUIK_WindowConfig * cfg)
{
    int           eNum       = 0;
    static char   funcName[] = "NEUIK_WindowConfig_Free";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `cfg` is NULL.",                               // [1]
        "Argument `cfg` does not implement WindowConfig class.", // [2]
    };

    if (cfg == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(cfg, neuik__Class_WindowConfig))
    {
        eNum = 2;
        goto out;
    }

    free(cfg);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


void NEUIK_WindowConfig_SetBGColor(
        NEUIK_WindowConfig  *wc,
        NEUIK_Color          clr)
{
    wc->colorBG = clr;
}

