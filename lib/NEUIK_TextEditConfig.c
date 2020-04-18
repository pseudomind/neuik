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
#include "NEUIK_defs.h"
#include "NEUIK_FontSet.h"
#include "NEUIK_TextEditConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__TextEditConfig(void ** cfg);
int neuik_Object_Copy__TextEditConfig(void * dst, const void * src);
int neuik_Object_Free__TextEditConfig(void * cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_TextEditConfig_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__TextEditConfig,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__TextEditConfig,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__TextEditConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_TextEditConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_TextEditConfig()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_TextEditConfig";
    static char * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",           // [1]
        "Failed to register `TextEditConfig` object class .", // [2]
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
        "NEUIK_TextEditConfig",                     // className
        "Configuration for NEUIK_TextEdit Object.", // classDescription
        neuik__Set_NEUIK,                            // classSet
        NULL,                                        // superClass
        &neuik_TextEditConfig_BaseFuncs,            // baseFuncs
        NULL,                                        // classFuncs
        &neuik__Class_TextEditConfig))              // newClass
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
 *  Name:          NEUIK_GetDefaultTextEditConfig
 *
 *  Description:   Returns a pointer to the initialized default TextEdit
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_TextEditConfig; NULL if error.
 *
 ******************************************************************************/
NEUIK_TextEditConfig * NEUIK_GetDefaultTextEditConfig()
{
    int                    eNum           = 0;
    static int             isInitialized  = 0;
    static char          * dFontName      = NULL;
    static char          * dFontNameMS    = NULL;
    NEUIK_TextEditConfig * rvCfg          = NULL;
    /* default TextEditConfig */
    static NEUIK_TextEditConfig   dCfg = {
        {0, 0, NULL, NULL, NULL}, // neuik_Object objBase
        NULL,                     // NEUIK_FontSet * fontSet
        NULL,                     // NEUIK_FontSet * fontSetMS
        11,                       // int             fontSize
        FALSE,                    // int             fontBold
        FALSE,                    // int             fontItalic
        FALSE,                    // int             fontMono
        NULL,                     // char          * fontName
        NULL,                     // char          * fontNameMS
        COLOR_WHITE,              // SDL_Color       bgColor
        COLOR_LBLACK,             // SDL_Color       fgColor
        COLOR_LBLUE,              // SDL_Color       bgColorHl
        COLOR_WHITE,              // SDL_Color       fgColorHl
        COLOR_DBLUE,              // SDL_Color       bgColorSelect
        COLOR_GRAY,               // SDL_Color       borderColor
        COLOR_DGRAY,              // SDL_Color       borderColorDark
        COLOR_GRAY,               // SDL_Color       bgScrollColor
        COLOR_DGRAY,              // SDL_Color       scrollSliderColor
        NEUIK_VJUSTIFY_CENTER,    // int             textVJustify
        NEUIK_HJUSTIFY_LEFT,      // int             textHJustify
        15,                       // int             EmWidth
        NEUIK_RESTRICT_NONE,      // int             restriction
        NULL,                     // char          * restrict_str
        10,                       // int             emptySpaces
    };
    static char   funcName[] = "NEUIK_GetDefaultTextEditConfig";
    static char * errMsgs[] = {"", // [0] no error
        "Failure in GetDefaultFontSet().", // [1]
        "Failure in FontSet_GetFont().",   // [2]
        "Failure in String_Duplicate().",  // [3]
    };

    if (!isInitialized)
    {
        isInitialized = 1;

        neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_TextEditConfig, 
            NULL,
            &(dCfg.objBase));

        /*--------------------------------------------------------------------*/
        /* Load the default standard font.                                    */
        /*--------------------------------------------------------------------*/
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

        /*--------------------------------------------------------------------*/
        /* Load the default monospaced font.                                  */
        /*--------------------------------------------------------------------*/
        /* Look for the first default monospaced font that is supported */
        dCfg.fontSetMS = NEUIK_GetDefaultMSFontSet(&dFontNameMS);
        if (dCfg.fontSetMS == NULL)
        {
            eNum = 1;
            goto out;
        }

        String_Duplicate(&(dCfg.fontNameMS), dFontNameMS);
        if (dCfg.fontNameMS == NULL)
        {
            eNum = 3;
            goto out;
        }

        /* Finally attempt to load the font */
        if (NEUIK_FontSet_GetFont(dCfg.fontSetMS, dCfg.fontSize,
            dCfg.fontBold, dCfg.fontItalic) == NULL)
        {
            eNum = 2;
            goto out;
        }
    }
    rvCfg = &dCfg;

out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return rvCfg;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New__TextEditConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__TextEditConfig(
    void ** cfgPtr)
{
    int                    eNum       = 0;
    NEUIK_TextEditConfig * cfg        = NULL;
    static char            funcName[] = "neuik_Object_New__TextEditConfig";
    static char          * errMsgs[]  = {"", // [0] no error
        "Output Argument cfgPtr is NULL.",   // [1]
        "Failure to allocate memory.",       // [2]
        "Failure in TextEditConfig_Copy().", // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_TextEditConfig*) malloc(sizeof(NEUIK_TextEditConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of TextEditConfig                         */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_TextEditConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new TextEditConfig          */
    /*------------------------------------------------------------------------*/
    if (NEUIK_TextEditConfig_Copy((cfg), NEUIK_GetDefaultTextEditConfig()))
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
 *  Name:          NEUIK_NewTextEditConfig
 *
 *  Description:   Allocate memory and set default values for TextEditConfig.
 *
 *                 This is only a wrapper function.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewTextEditConfig(
        NEUIK_TextEditConfig ** cfgPtr)
{
    return neuik_Object_New__TextEditConfig((void**)cfgPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_Copy__TextEditConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__TextEditConfig(
    void        * dst,
    const void  * src)
{
    return NEUIK_TextEditConfig_Copy(
        (NEUIK_TextEditConfig *)dst, (const NEUIK_TextEditConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_TextEditConfig_Copy
 *
 *  Description:   Copy the data in a TextEditConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_TextEditConfig_Copy(
    NEUIK_TextEditConfig       * dst,
    const NEUIK_TextEditConfig * src)
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "NEUIK_TextEditConfig_Copy";
    static char  * errMsgs[]  = {"", // [0] no error
        "Argument `src` is invalid or an incorrect type.", // [1]
        "Argument `dst` is invalid or an incorrect type.", // [2]
        "TextEditConfig->fontName is NULL.",               // [3]
        "Failure in String_Duplicate.",                    // [4]
    };

    if (!neuik_Object_IsClass(src, neuik__Class_TextEditConfig))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst, neuik__Class_TextEditConfig))
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
    String_Duplicate(&(dst->fontNameMS), src->fontNameMS);
    if (dst->fontName == NULL)
    {
        eNum = 4;
        goto out;
    }

    if (src->restrict_str != NULL)
    {
        String_Duplicate(&(dst->restrict_str), src->restrict_str);
        if (dst->restrict_str == NULL)
        {
            eNum = 4;
            goto out;
        }
    }
    else
    {
        dst->restrict_str = NULL;
    }

    dst->fontSet           = src->fontSet;
    dst->fontSetMS         = src->fontSetMS;
    dst->fontSize          = src->fontSize;
    dst->fontBold          = src->fontBold;
    dst->fontItalic        = src->fontItalic;
    dst->fontMono          = src->fontMono;
    dst->bgColor           = src->bgColor;
    dst->fgColor           = src->fgColor;
    dst->bgColorHl         = src->bgColorHl;
    dst->fgColorHl         = src->fgColorHl;
    dst->bgColorSelect     = src->bgColorSelect;
    dst->borderColor       = src->borderColor;
    dst->borderColorDark   = src->borderColorDark;
    dst->bgScrollColor     = src->bgScrollColor;
    dst->scrollSliderColor = src->scrollSliderColor;
    dst->textVJustify      = src->textVJustify;
    dst->textHJustify      = src->textHJustify;
    dst->fontEmWidth       = src->fontEmWidth;
    dst->restriction       = src->restriction;
    dst->emptySpaces       = src->emptySpaces;
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
 *  Name:          neuik_Object_Free__TextEditConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__TextEditConfig(
    void * cfgPtr)
{
    int                    eNum       = 0;
    NEUIK_TextEditConfig * cfg        = NULL;
    static char            funcName[] = "neuik_Object_Free__TextEditConfig";
    static char          * errMsgs[]  = {"", // [0] no error
        "Argument `cfgPtr` is NULL.",                          // [1]
        "Argument `*cfgPtr` is invalid or an incorrect type.", // [2]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    cfg = (NEUIK_TextEditConfig*)cfgPtr;

    if (!neuik_Object_IsClass(cfg, neuik__Class_TextEditConfig))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if (cfg->fontName != NULL)     free(cfg->fontName);
    if (cfg->restrict_str != NULL) free(cfg->restrict_str);

    free(cfg);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

