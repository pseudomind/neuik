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
#include <stdlib.h>
#include <string.h>

#include "NEUIK_colors.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"
#include "NEUIK_FontSet.h"
#include "NEUIK_ToggleButtonConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ToggleButtonConfig(void ** cfg);
int neuik_Object_Copy__ToggleButtonConfig(void * dst, const void * src);
int neuik_Object_Free__ToggleButtonConfig(void * cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ToggleButtonConfig_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ToggleButtonConfig,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__ToggleButtonConfig,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ToggleButtonConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ToggleButtonConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ToggleButtonConfig()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_ToggleButtonConfig";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",               // [1]
        "Failed to register `ToggleButtonConfig` object class .", // [2]
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
        "ToggleButtonConfig",                         // className
        "Configuration for the ToggleButton Object.", // classDescription
        neuik__Set_NEUIK,                             // classSet
        NULL,                                         // superClass
        &neuik_ToggleButtonConfig_BaseFuncs,          // baseFuncs
        NULL,                                         // classFuncs
        &neuik__Class_ToggleButtonConfig))            // newClass
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
 *  Name:          NEUIK_GetDefaultToggleButtonConfig
 *
 *  Description:   Returns a pointer to the initialized default ToggleButton
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_ToggleButtonConfig; NULL if 
 *                 error.
 *
 ******************************************************************************/
NEUIK_ToggleButtonConfig * NEUIK_GetDefaultToggleButtonConfig()
{
    int                               eNum           = 0;
    static int                        isInitialized  = 0;
    static char                     * dFontName      = NULL;
    NEUIK_ToggleButtonConfig        * rvCfg          = NULL;
    /* default ButtonConfig */
    static NEUIK_ToggleButtonConfig   dCfg = {
        {0, 0, NULL, NULL, NULL}, // neuik_Object       objBase
        NULL,                     // NEUIK_FontSet    * fontSet
        11,                       // int                fontSize
        0,                        // int                fontBold
        0,                        // int                fontItalic
        NULL,                     // char             * fontName
        COLOR_LBLACK,             // SDL_Color          fgColor
        COLOR_LWHITE,             // SDL_Color          fgColorPressed
        COLOR_GRAY,               // SDL_Color          borderColor
        COLOR_DGRAY,              // SDL_Color          borderColorDark
        15,                       // int                EmWidth
    };
    static char   funcName[] = "NEUIK_GetDefaultToggleButtonConfig";
    static char * errMsgs[] = {"", // [0] no error
        "Failure in GetDefaultFontSet().", // [1]
        "Failure in FontSet_GetFont().",   // [2]
        "Failure in String_Duplicate().",  // [3]
        "Failed to allocate memory().",    // [4]
    };

    if (!isInitialized)
    {
        isInitialized = 1;

        neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ToggleButtonConfig, 
            NULL,
            &(dCfg.objBase));

        /* Look for the first default font that is supported */
        dCfg.fontSet = NEUIK_GetDefaultFontSet(&dFontName);
        if (dCfg.fontSet == NULL)
        {
            eNum = 1;
            goto out;
        }

        String_Duplicate(&(dCfg.fontName), dFontName);
        if (dCfg.fontName == NULL)
        {
            eNum = 3;
            goto out;
        }

        /* Finally attempt to load the font */
        if (NEUIK_FontSet_GetFont(dCfg.fontSet, dCfg.fontSize,
            dCfg.fontBold, dCfg.fontItalic) == NULL)
        {
            eNum = 2;
            goto out;
        }
        rvCfg = &dCfg;
    }
    else
    {
        rvCfg = &dCfg;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return rvCfg;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New__ToggleButtonConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__ToggleButtonConfig(
    void ** cfgPtr)
{
    return NEUIK_NewToggleButtonConfig((NEUIK_ToggleButtonConfig **)cfgPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewToggleButtonConfig
 *
 *  Description:   Allocate memory and set default values for ToggleButtonConfig.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewToggleButtonConfig(
        NEUIK_ToggleButtonConfig ** cfgPtr)
{
    int                        eNum       = 0;
    NEUIK_ToggleButtonConfig * cfg        = NULL;
    static char                funcName[] = "NEUIK_NewToggleButtonConfig";
    static char              * errMsgs[]  = {"", // [0] no error
        "Output Argument cfgPtr is NULL.",        // [1]
        "Failure to allocate memory.",            // [2]
        "Failure in ToggleButtonConfig_Copy().",  // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_ToggleButtonConfig*) malloc(sizeof(NEUIK_ToggleButtonConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of ButtonConfig                            */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_ToggleButtonConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new ButtonConfig             */
    /*------------------------------------------------------------------------*/
    if (NEUIK_ToggleButtonConfig_Copy((cfg), NEUIK_GetDefaultToggleButtonConfig()))
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
 *  Name:          neuik_Object_Copy__ToggleButtonConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__ToggleButtonConfig(
    void        * dst,
    const void  * src)
{
    return NEUIK_ToggleButtonConfig_Copy(
        (NEUIK_ToggleButtonConfig *)dst, (const NEUIK_ToggleButtonConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ToggleButton_CopyConfig
 *
 *  Description:   Copy the data in a ToggleButtonConfig to that used in the 
 *                 struct.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_ToggleButtonConfig_Copy(
    NEUIK_ToggleButtonConfig       * dst,
    const NEUIK_ToggleButtonConfig * src)
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "NEUIK_ToggleButtonConfig_Copy";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `src` is invalid or an incorrect type.", // [1]
        "Argument `dst` is invalid or an incorrect type.", // [2]
        "ToggleButtonConfig->fontName is NULL.",           // [3]
        "Failure in String_Duplicate.",                    // [4]
    };

    if (!neuik_Object_IsClass(src, neuik__Class_ToggleButtonConfig))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst, neuik__Class_ToggleButtonConfig))
    {
        eNum = 2;
        goto out;
    }

    dst->fontSet    = src->fontSet;
    dst->fontSize   = src->fontSize;
    dst->fontBold   = src->fontBold;
    dst->fontItalic = src->fontItalic;

    if (src->fontName == NULL)
    {
        eNum = 3;
        goto out;
    }
    String_Duplicate(&(dst->fontName), src->fontName);
    if (dst->fontName == NULL)
    {
        eNum = 4;
        goto out;
    }

    dst->fgColor         = src->fgColor;
    dst->fgColorPressed  = src->fgColorPressed;
    dst->borderColor     = src->borderColor;
    dst->borderColorDark = src->borderColorDark;
    dst->fontEmWidth     = src->fontEmWidth;
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
 *  Name:          neuik_Object_Free__ToggleButtonConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__ToggleButtonConfig(
    void * cfgPtr)
{
    int                        eNum       = 0;
    NEUIK_ToggleButtonConfig * cfg        = NULL;
    static char                funcName[] = "neuik_Object_Free__ToggleButtonConfig";
    static char              * errMsgs[]  = {"", // [0] no error
        "Argument `cfgPtr` is NULL.",                         // [1]
        "Argument `cfgPtr` is invalid or an incorrect type.", // [2]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    cfg = (NEUIK_ToggleButtonConfig*)cfgPtr;

    if (!neuik_Object_IsClass(cfg, neuik__Class_ToggleButtonConfig))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if (cfg->fontName != NULL) free(cfg->fontName);

    free(cfg);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

