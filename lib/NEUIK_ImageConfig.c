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
#include "NEUIK_ImageConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__ImageConfig(void ** cfg);
int neuik_Object_Copy__ImageConfig(void * dst, const void * src);
int neuik_Object_Free__ImageConfig(void * cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ImageConfig_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__ImageConfig,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__ImageConfig,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__ImageConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ImageConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_ImageConfig()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_ImageConfig";
    static char * errMsgs[]  = {"",                       // [0] no error
        "NEUIK library must be initialized first.",        // [1]
        "Failed to register `ImageConfig` object class .", // [2]
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
        "NEUIK_ImageConfig",                     // className
        "Configuration for NEUIK_Image Object.", // classDescription
        neuik__Set_NEUIK,                        // classSet
        NULL,                                    // superClass
        &neuik_ImageConfig_BaseFuncs,            // baseFuncs
        NULL,                                    // classFuncs
        &neuik__Class_ImageConfig))              // newClass
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
 *  Name:          NEUIK_GetDefaultImageConfig
 *
 *  Description:   Returns a pointer to the initialized default Image
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_ImageConfig; NULL if error.
 *
 ******************************************************************************/
NEUIK_ImageConfig * NEUIK_GetDefaultImageConfig()
{
    static int                 isInitialized  = 0;
    NEUIK_ImageConfig        * rvCfg          = NULL;
    /* default ImageConfig */
    static NEUIK_ImageConfig   dCfg = {
        {0, 0, NULL, NULL, NULL}, // neuik_Object objBase
        COLOR_LGRAY,              // SDL_Color       bgColor
        COLOR_DBLUE,              // SDL_Color       bgColorSelect
    };

    if (!isInitialized)
    {
        isInitialized = 1;

        neuik_GetObjectBaseOfClass(
            neuik__Set_NEUIK, 
            neuik__Class_ImageConfig, 
            NULL,
            &(dCfg.objBase));
        rvCfg = &dCfg;
    }
    else
    {
        rvCfg = &dCfg;
    }

    return rvCfg;
}


/*******************************************************************************
 *
 *  Name:          neuik_Object_New__ImageConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__ImageConfig(
    void ** cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_ImageConfig * cfg        = NULL;
    static char         funcName[] = "neuik_Object_New__ImageConfig";
    static char       * errMsgs[]  = {"",  // [0] no error
        "Output Argument cfgPtr is NULL.", // [1]
        "Failure to allocate memory.",     // [2]
        "Failure in ImageConfig_Copy().",  // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_ImageConfig*) malloc(sizeof(NEUIK_ImageConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of ImageConfig                             */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_ImageConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new ImageConfig              */
    /*------------------------------------------------------------------------*/
    if (NEUIK_ImageConfig_Copy((cfg), NEUIK_GetDefaultImageConfig()))
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
 *  Name:          NEUIK_NewImageConfig
 *
 *  Description:   Allocate memory and set default values for ImageConfig.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewImageConfig(
    NEUIK_ImageConfig ** cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_ImageConfig * cfg        = NULL;
    static char         funcName[] = "NEUIK_NewImageConfig";
    static char       * errMsgs[]  = {"",    // [0] no error
        "Output Argument `cfgPtr` is NULL.", // [1]
        "Failure to allocate memory.",       // [2]
        "Failure in ImageConfig_Copy().",    // [3]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*cfgPtr) = (NEUIK_ImageConfig*) malloc(sizeof(NEUIK_ImageConfig));
    cfg = (*cfgPtr);
    if (cfg == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of ImageConfig                             */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_ImageConfig, 
        NULL,
        &(cfg->objBase));

    /*------------------------------------------------------------------------*/
    /* Copy the default config settings into the new ImageConfig              */
    /*------------------------------------------------------------------------*/
    if (NEUIK_ImageConfig_Copy((cfg), NEUIK_GetDefaultImageConfig()))
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
 *  Name:          neuik_Object_Copy__ImageConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__ImageConfig(
    void        * dst,
    const void  * src)
{
    return NEUIK_ImageConfig_Copy(
        (NEUIK_ImageConfig *)dst, (const NEUIK_ImageConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ImageConfig_Copy
 *
 *  Description:   Copy the data in a ImageConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_ImageConfig_Copy(
    NEUIK_ImageConfig       * dst,
    const NEUIK_ImageConfig * src)
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "NEUIK_ImageConfig_Copy";
    static char  * errMsgs[]  = {"",                       // [0] no error
        "Argument `src` is invalid or an incorrect type.", // [1]
        "Argument `dst` is invalid or an incorrect type.", // [2]
    };

    if (!neuik_Object_IsClass(src, neuik__Class_ImageConfig))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst, neuik__Class_ImageConfig))
    {
        eNum = 2;
        goto out;
    }

    dst->bgColor         = src->bgColor;
    dst->bgColorSelect   = src->bgColorSelect;
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
 *  Name:          neuik_Object_Free__ImageConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__ImageConfig(
    void * cfgPtr)
{
    int                 eNum       = 0;
    NEUIK_ImageConfig * cfg        = NULL;
    static char         funcName[] = "neuik_Object_Free__ImageConfig";
    static char       * errMsgs[]  = {"",                      // [0] no error
        "Argument `cfgPtr` is NULL.",                          // [1]
        "Argument `*cfgPtr` is invalid or an incorrect type.", // [2]
    };

    if (cfgPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    cfg = (NEUIK_ImageConfig*)cfgPtr;

    if (!neuik_Object_IsClass(cfg, neuik__Class_ImageConfig))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    free(cfg);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

