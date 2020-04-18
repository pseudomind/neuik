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
#include "NEUIK_colors.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"
#include "NEUIK_FontSet.h"
#include "NEUIK_LabelConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__LabelConfig(void ** cfg);
int neuik_Object_Copy__LabelConfig(void * dst, const void * src);
int neuik_Object_Free__LabelConfig(void * cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_LabelConfig_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__LabelConfig,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__LabelConfig,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__LabelConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_LabelConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_LabelConfig()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_LabelConfig";
    static char * errMsgs[]  = {"",                       // [0] no error
        "NEUIK library must be initialized first.",        // [1]
        "Failed to register `LabelConfig` object class .", // [2]
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
        "NEUIK_LabelConfig",                     // className
        "Configuration for NEUIK_Label Object.", // classDescription
        neuik__Set_NEUIK,                        // classSet
        NULL,                                    // superClass
        &neuik_LabelConfig_BaseFuncs,            // baseFuncs
        NULL,                                    // classFuncs
        &neuik__Class_LabelConfig))              // newClass
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
 *  Name:          NEUIK_GetDefaultLabelConfig
 *
 *  Description:   Returns a pointer to the initialized default Label
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_LabelConfig; NULL if error.
 *
 ******************************************************************************/
NEUIK_LabelConfig * NEUIK_GetDefaultLabelConfig()
{
    int                        eNum           = 0;
    static int                 isInitialized  = 0;
    static char              * dFontName      = NULL;
    NEUIK_LabelConfig        * rvCfg          = NULL;
    /* default LabelConfig */
    static NEUIK_LabelConfig   dCfg = {
        {0, 0, NULL, NULL, NULL}, // neuik_Object objBase
        NULL,                     // NEUIK_FontSet * fontSet
        11,                       // int             fontSize
        0,                        // int             fontBold
        0,                        // int             fontItalic
        NULL,                     // char          * fontName
        COLOR_LGRAY,              // SDL_Color       bgColor
        COLOR_LBLACK,             // SDL_Color       fgColor
        COLOR_DBLUE,              // SDL_Color       bgColorSelect
        COLOR_WHITE,              // SDL_Color       fgColorSelect
        COLOR_GRAY,               // SDL_Color       borderColor
        COLOR_DGRAY,              // SDL_Color       borderColorDark
        15,                       // int             EmWidth
    };
    static char   funcName[] = "NEUIK_GetDefaultLabelConfig";
    static char * errMsgs[] = {"",         // [0] no error
        "Failure in GetDefaultFontSet().", // [1]
        "Failure in FontSet_GetFont().",   // [2]
        "Failure in String_Duplicate().",  // [3]
    };

    if (!isInitialized)
    {
        isInitialized = 1;

        neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_LabelConfig, 
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
 *  Name:          neuik_Object_New__LabelConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__LabelConfig(
    void ** cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_LabelConfig * cfg        = NULL;
    static char         funcName[] = "neuik_Object_New__LabelConfig";
    static char       * errMsgs[]  = {"",  // [0] no error
        "Output Argument cfgPtr is NULL.", // [1]
        "Failure to allocate memory.",     // [2]
        "Failure in LabelConfig_Copy().",  // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_LabelConfig*) malloc(sizeof(NEUIK_LabelConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of LabelConfig                             */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_LabelConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new LabelConfig              */
    /*------------------------------------------------------------------------*/
    if (NEUIK_LabelConfig_Copy((cfg), NEUIK_GetDefaultLabelConfig()))
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
 *  Name:          NEUIK_NewLabelConfig
 *
 *  Description:   Allocate memory and set default values for LabelConfig.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewLabelConfig(
    NEUIK_LabelConfig ** cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_LabelConfig * cfg        = NULL;
    static char         funcName[] = "NEUIK_NewLabelConfig";
    static char       * errMsgs[]  = {"",    // [0] no error
        "Output Argument `cfgPtr` is NULL.", // [1]
        "Failure to allocate memory.",       // [2]
        "Failure in LabelConfig_Copy().",    // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_LabelConfig*) malloc(sizeof(NEUIK_LabelConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of LabelConfig                             */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_LabelConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new LabelConfig              */
    /*------------------------------------------------------------------------*/
    if (NEUIK_LabelConfig_Copy((cfg), NEUIK_GetDefaultLabelConfig()))
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
 *  Name:          neuik_Object_Copy__LabelConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__LabelConfig(
    void        * dst,
    const void  * src)
{
    return NEUIK_LabelConfig_Copy(
        (NEUIK_LabelConfig *)dst, (const NEUIK_LabelConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_LabelConfig_Copy
 *
 *  Description:   Copy the data in a LabelConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_LabelConfig_Copy(
    NEUIK_LabelConfig       * dst,
    const NEUIK_LabelConfig * src)
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "NEUIK_LabelConfig_Copy";
    static char  * errMsgs[]  = {"",                       // [0] no error
        "Argument `src` is invalid or an incorrect type.", // [1]
        "Argument `dst` is invalid or an incorrect type.", // [2]
        "LabelConfig->fontName is NULL.",                  // [3]
        "Failure in String_Duplicate.",                    // [4]
    };

    if (!neuik_Object_IsClass(src, neuik__Class_LabelConfig))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst, neuik__Class_LabelConfig))
    {
        eNum = 2;
        goto out;
    }

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

    dst->fontSet         = src->fontSet;
    dst->fontSize        = src->fontSize;
    dst->fontBold        = src->fontBold;
    dst->fontItalic      = src->fontItalic;
    dst->bgColor         = src->bgColor;
    dst->fgColor         = src->fgColor;
    dst->bgColorSelect   = src->bgColorSelect;
    dst->fgColorSelect   = src->fgColorSelect;
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
 *  Name:          neuik_Object_Free__LabelConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__LabelConfig(
    void * cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_LabelConfig * cfg        = NULL;
    static char         funcName[] = "neuik_Object_Free__LabelConfig";
    static char       * errMsgs[]  = {"",                      // [0] no error
        "Argument `cfgPtr` is NULL.",                          // [1]
        "Argument `*cfgPtr` is invalid or an incorrect type.", // [2]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    cfg = (NEUIK_LabelConfig*)cfgPtr;

    if (!neuik_Object_IsClass(cfg, neuik__Class_LabelConfig))
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

