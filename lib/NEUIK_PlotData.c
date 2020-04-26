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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NEUIK_error.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_PlotData.h"
#include "NEUIK_PlotData_internal.h"
#include "NEUIK_render.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_PlotData_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__PlotData,
    /* Copy(): Copy the contents of one object into another */
    neuik_Object_Copy__PlotData,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__PlotData,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_PlotData
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_PlotData()
{
    int           eNum       = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_RegisterClass_PlotData";
    static char * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",     // [1]
        "Failed to register `PlotData` object class .", // [2]
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
        "PlotData",                             // className
        "Stores a set of X,Y datapoint pairs.", // classDescription
        neuik__Set_NEUIK,                       // classSet
        NULL,                                   // superClass
        &neuik_PlotData_BaseFuncs,              // baseFuncs
        NULL,                                   // classFuncs
        &neuik__Class_PlotData))                // newClass
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
 *  Name:          neuik_Object_New__PlotData
 *
 *  Description:   Allocate memory and set default values for PlotData.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__PlotData(
    void ** ptr)  /* Address of where it initialize object. */
{
    int              eNum       = 0;
    NEUIK_PlotData * pd         = NULL;
    static char      funcName[] = "neuik_Object_New__PlotData";
    static char    * errMsgs[]  = {"", // [0] no error
        "Output Argument `ptr` is NULL.", // [1]
        "Failure to allocate memory.",    // [2]
    };

    if (ptr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*ptr) = (NEUIK_PlotData*) malloc(sizeof(NEUIK_PlotData));
    pd = (*ptr);
    if (pd == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set the object base to that of PlotData                                */
    /*------------------------------------------------------------------------*/
    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_PlotData, 
        NULL,
        &(pd->objBase));

    /*------------------------------------------------------------------------*/
    /* Initialize values within the new PlotData                              */
    /*------------------------------------------------------------------------*/
    pd->uniqueName = NULL;
    pd->stateMod   = 0;
    pd->nAlloc     = 0;
    pd->nPoints    = 0;
    pd->nUsed      = 0;
    pd->precision  = 0;
    pd->boundsSet  = FALSE;
    pd->data_32    = NULL; /* (at time of free) free pointer if non-NULL */
    pd->data_64    = NULL; /* (at time of free) free pointer if non-NULL */
    /*------------------*/
    pd->bounds_32.x_min = 0.0;
    pd->bounds_32.x_max = 0.0;
    pd->bounds_32.y_min = 0.0;
    pd->bounds_32.y_max = 0.0;
    /*------------------*/
    pd->bounds_64.x_min = 0.0;
    pd->bounds_64.x_max = 0.0;
    pd->bounds_64.y_min = 0.0;
    pd->bounds_64.y_max = 0.0;
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
 *  Name:          neuik_Object_Free__PlotData
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__PlotData(
    void * ptr)
{
    int              eNum       = 0;    /* which error to report (if any) */
    NEUIK_PlotData * pd         = NULL;
    static char      funcName[] = "neuik_Object_Free__PlotData";
    static char    * errMsgs[]  = {"", // [0] no error
        "Argument `ptr` is NULL.",                  // [1]
        "Argument `ptr` is not of PlotData class.", // [2]
        "Failure in function `neuik_Object_Free`.", // [3]
    };

    if (ptr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(ptr, neuik__Class_PlotData))
    {
        eNum = 2;
        goto out;
    }
    pd = (NEUIK_PlotData*)ptr;

    /*------------------------------------------------------------------------*/
    /* The object is what it says it is and it is still allocated.            */
    /*------------------------------------------------------------------------*/
    if(neuik_Object_Free(pd->objBase.superClassObj))
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Free other memory allocated by this object.                            */
    /*------------------------------------------------------------------------*/
    if (pd->uniqueName != NULL) free(pd->uniqueName);
    if (pd->data_32 != NULL)    free(pd->data_32);
    if (pd->data_64 != NULL)    free(pd->data_64);

    /*------------------------------------------------------------------------*/
    /* Free the memory used for the object structure itself.                  */
    /*------------------------------------------------------------------------*/
    free(pd);
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
 *  Name:          neuik_Object_Copy__PlotData
 *
 *  Description:   Copy the data from one PlotData object to another.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Copy__PlotData(
    void       * dst_ptr,
    const void * src_ptr)
{
    int              ctr        = 0;
    NEUIK_PlotData * dst        = NULL;
    NEUIK_PlotData * src        = NULL;
    int              eNum       = 0; /* which error to report (if any) */
    static char      funcName[] = "neuik_Object_Copy__PlotData";
    static char    * errMsgs[]  = {"", // [0] no error
        "Argument `src_ptr` is invalid or an incorrect type.", // [1]
        "Argument `dst_ptr` is invalid or an incorrect type.", // [2]
        "Failure in `String_Duplicate()`.",                    // [3]
        "Failure to allocate memory.",                         // [4]
    };

    if (!neuik_Object_IsClass(src_ptr, neuik__Class_PlotData))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(dst_ptr, neuik__Class_PlotData))
    {
        eNum = 2;
        goto out;
    }

    dst = (NEUIK_PlotData *)(dst_ptr);
    src = (NEUIK_PlotData *)(src_ptr);

    /*------------------------------------------------------------------------*/
    /* First, copy over the values that do not require memory allocation.     */
    /*------------------------------------------------------------------------*/
    dst->nAlloc    = src->nAlloc;
    dst->nPoints   = src->nPoints;
    dst->nUsed     = src->nUsed;
    dst->precision = src->precision;
    dst->boundsSet = src->boundsSet;
    /*------------------*/
    dst->bounds_32.x_min = src->bounds_32.x_min;
    dst->bounds_32.x_max = src->bounds_32.x_max;
    dst->bounds_32.y_min = src->bounds_32.y_min;
    dst->bounds_32.y_max = src->bounds_32.y_max;
    /*------------------*/
    dst->bounds_64.x_min = src->bounds_64.x_min;
    dst->bounds_64.x_max = src->bounds_64.x_max;
    dst->bounds_64.y_min = src->bounds_64.y_min;
    dst->bounds_64.y_max = src->bounds_64.y_max;

    /*------------------------------------------------------------------------*/
    /* Now copy over allocated data value(s) if allocated.                    */
    /*------------------------------------------------------------------------*/
    if (src->uniqueName != NULL)
    {
        String_Duplicate(&dst->uniqueName, src->uniqueName);
        if (dst->uniqueName == NULL)
        {
            eNum = 3;
            goto out;
        }
    }

    if (src->data_32 != NULL && src->nAlloc > 0)
    {
        dst->data_32 = malloc(src->nAlloc*sizeof(float));
        if (dst->data_32 == NULL)
        {
            eNum = 4;
            goto out;
        }
        for (ctr = 0; ctr < src->nAlloc; ctr++)
        {
            dst->data_32[ctr] = src->data_32[ctr];
        }
    }
    if (src->data_64 != NULL && src->nAlloc > 0)
    {
        dst->data_64 = malloc(src->nAlloc*sizeof(double));
        if (dst->data_64 == NULL)
        {
            eNum = 4;
            goto out;
        }
        for (ctr = 0; ctr < src->nAlloc; ctr++)
        {
            dst->data_64[ctr] = src->data_64[ctr];
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
 *  Name:          NEUIK_NewPlotData
 *
 *  Description:   Create and return a pointer to a new NEUIK_NewPlotData.
 *
 *  Returns:       NULL if there is an error; otherwise a valid pointer.
 *
 ******************************************************************************/
int NEUIK_NewPlotData(
    NEUIK_PlotData ** pdPtr,
    const char      * uniqueName) /* unique name for this plot data. */
{
    NEUIK_PlotData * pd         = NULL;
    int              eNum       = 0; /* which error to report (if any) */
    static char      funcName[] = "NEUIK_NewPlotData";
    static char    * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_Object_New__PlotData()`.", // [1]
        "Failure in `String_Duplicate()`.",           // [2]
    };

    if (neuik_Object_New__PlotData((void**)pdPtr))
    {
        eNum = 1;
        goto out;
    }
    pd = *pdPtr;

    if (uniqueName != NULL)
    {
        String_Duplicate(&(pd->uniqueName), uniqueName);
        if (pd->uniqueName == NULL)
        {
            eNum = 2;
            goto out;
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
 *  Name:          NEUIK_MakePlotData
 *
 *  Description:   Create a new NEUIK_PlotData with specified precision.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_MakePlotData(
    NEUIK_PlotData ** pdPtr,      /* Address of where it initialize object. */
    const char      * uniqueName, /* unique name for this plot data. */
    int               precision)  /* Must be `32` (32bit) or `64` (64bit).  */
{
    int              eNum       = 0;
    NEUIK_PlotData * pd         = NULL;
    static char      funcName[] = "NEUIK_MakePlotData";
    static char    * errMsgs[]  = {"", // [0] no error
        "Output Argument `pdPtr` is NULL.",                        // [1]
        "Argument `precision` is invalid (must be `32` or `64`).", // [2]
        "Failure in `neuik_Object_New__PlotData()`.",              // [3]
        "Failure in `String_Duplicate()`.",                        // [4]
    };

    if (pdPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!(precision == 32 || precision == 64))
    {
        eNum = 2;
        goto out;
    }

    if (neuik_Object_New__PlotData((void**)pdPtr))
    {
        eNum = 3;
        goto out;
    }
    pd = *pdPtr;

    if (uniqueName != NULL)
    {
        String_Duplicate(&(pd->uniqueName), uniqueName);
        if (pd->uniqueName == NULL)
        {
            eNum = 4;
            goto out;
        }
    }

    pd->precision = precision;
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
 *  Name:          neuik_GetDoubleArrayFromFields
 *
 *  Description:   Get an float array from a whitespace separated values in a 
 *                 string.
 *
 *                 NOTE: This function allocates memory within `arrayPtr` which
 *                 needs to be freed elsewhere.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_GetDoubleArrayFromFields(
    double       ** arrayPtr, /* (out) The resulting allocated double array. */
    unsigned int  * arrayLen, /* (out) length of the resulting array */
    const char    * srcStr)   /* whitespace separated value string */
{
    int      ctr       = 0;
    int      isWS      = FALSE; /* current active char is whitespace */
    int      inValue   = FALSE; /* within a contiguous value substring */
    int      storeCtr  = 0;
    int      valCount  = 0;
    int      srcStrLen = 0;
    char     aChar     = 0;    /* current active char */
    double   nextFloat = 0;
    double * newArray  = NULL;
    char   * str0      = NULL;
    char   * srcStrCpy = NULL; /* free on exit */
    /*------------------------------------------------------------------------*/
    int           eNum       = 0;
    static char   funcName[] = "neuik_GetDoubleArrayFromFields";
    static char * errMsgs[]  = {"", // [0] no error
        "Output argument `arrayPtr` is NULL.", // [1]
        "Output argument `arrayLen` is NULL.", // [2]
        "Argument `srcStr` is NULL.",          // [3]
        "Failure to allocate memory.",         // [4]
        "Failed to scan double value.",        // [5]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
    if (arrayPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (arrayLen == NULL)
    {
        eNum = 2;
        goto out;
    }
    if (srcStr == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create copies of the inputs which need to be processed.                */
    /*------------------------------------------------------------------------*/
    srcStrLen = strlen(srcStr);
    srcStrCpy = malloc(1 + sizeof(char)*srcStrLen);
    if (srcStrCpy == NULL)
    {
        eNum = 4;
        goto out;
    }
    strcpy(srcStrCpy, srcStr);

    /*------------------------------------------------------------------------*/
    /* Determine the total number of values in the array.                     */
    /*------------------------------------------------------------------------*/
    str0 = srcStrCpy;
    for (ctr = 0; ctr < srcStrLen; ctr++)
    {
        aChar = str0[ctr];
        isWS = FALSE;
        if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')
        {
            /*----------------------------------------------------------------*/
            /* The current active char is a whitespace character.             */
            /*----------------------------------------------------------------*/
            isWS = TRUE;
        }

        if (isWS && inValue)
        {
            /*----------------------------------------------------------------*/
            /* This is a whitespace character following the end of a value.   */
            /*----------------------------------------------------------------*/
            inValue = FALSE;
        }
        else if (!isWS && !inValue)
        {
            /*----------------------------------------------------------------*/
            /* The start of a value substring.                                */
            /*----------------------------------------------------------------*/
            inValue = TRUE;
            valCount++;
        }
    }
    *arrayLen = valCount;

    /*------------------------------------------------------------------------*/
    /* Allocate memory and set the value.                                     */
    /*------------------------------------------------------------------------*/
    *arrayPtr = malloc(sizeof(double)*valCount);
    if (*arrayPtr == NULL)
    {
        eNum = 4;
        goto out;
    }
    newArray = *arrayPtr;

    /*------------------------------------------------------------------------*/
    /* Extract the values from the string and store them in the array.        */
    /*------------------------------------------------------------------------*/
    str0 = srcStrCpy;
    for (ctr = 0; ctr < srcStrLen; ctr++)
    {
        aChar = srcStrCpy[ctr];
        isWS = FALSE;
        if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')
        {
            /*----------------------------------------------------------------*/
            /* The current active char is a whitespace character.             */
            /*----------------------------------------------------------------*/
            isWS = TRUE;
        }

        if (isWS && inValue)
        {
            /*----------------------------------------------------------------*/
            /* This is a whitespace character following the end of a value.   */
            /*----------------------------------------------------------------*/
            inValue = FALSE;
            srcStrCpy[ctr] = '\0';
            if (strchr(str0, 'e'))
            {
                if (sscanf(str0, "%le", &nextFloat) != 1)
                {
                    eNum = 5;
                    goto out;
                }
            }
            else if (strchr(str0, 'E'))
            {
                if (sscanf(str0, "%lE", &nextFloat) != 1)
                {
                    eNum = 5;
                    goto out;
                }
            }
            else if (sscanf(str0, "%lf", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
            newArray[storeCtr] = nextFloat;
            storeCtr++;
        }
        else if (!isWS && !inValue)
        {
            /*----------------------------------------------------------------*/
            /* The start of a value substring.                                */
            /*----------------------------------------------------------------*/
            inValue = TRUE;
            str0 = srcStrCpy + ctr;
        }
    }

    if (inValue)
    {
        /*--------------------------------------------------------------------*/
        /* There is a final value in the field string; capture it here.       */
        /*--------------------------------------------------------------------*/
        if (strchr(str0, 'e'))
        {
            if (sscanf(str0, "%le", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
        }
        else if (strchr(str0, 'E'))
        {
            if (sscanf(str0, "%lE", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
        }
        else if (sscanf(str0, "%lf", &nextFloat) != 1)
        {
            eNum = 5;
            goto out;
        }
        newArray[storeCtr] = nextFloat;
        storeCtr++;
    }
out:
    if (srcStrCpy != NULL) free(srcStrCpy);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_GetFloatArrayFromFields
 *
 *  Description:   Get an float array from a whitespace separated values in a 
 *                 string.
 *
 *                 NOTE: This function allocates memory within `arrayPtr` which
 *                 needs to be freed elsewhere.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_GetFloatArrayFromFields(
    float        ** arrayPtr, /* (out) The resulting allocated float array. */
    unsigned int  * arrayLen, /* (out) length of the resulting array */
    const char    * srcStr)   /* whitespace separated value string */
{
    int     ctr       = 0;
    int     isWS      = FALSE; /* current active char is whitespace */
    int     inValue   = FALSE; /* within a contiguous value substring */
    int     storeCtr  = 0;
    int     valCount  = 0;
    int     srcStrLen = 0;
    char    aChar     = 0;    /* current active char */
    float   nextFloat = 0;
    float * newArray  = NULL;
    char  * str0      = NULL;
    char  * srcStrCpy = NULL; /* free on exit */
    /*------------------------------------------------------------------------*/
    int           eNum       = 0;
    static char   funcName[] = "neuik_GetFloatArrayFromFields";
    static char * errMsgs[]  = {"", // [0] no error
        "Output argument `arrayPtr` is NULL.", // [1]
        "Output argument `arrayLen` is NULL.", // [2]
        "Argument `srcStr` is NULL.",          // [3]
        "Failure to allocate memory.",         // [4]
        "Failed to scan float value.",         // [5]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
    if (arrayPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (arrayLen == NULL)
    {
        eNum = 2;
        goto out;
    }
    if (srcStr == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Create copies of the inputs which need to be processed.                */
    /*------------------------------------------------------------------------*/
    srcStrLen = strlen(srcStr);
    srcStrCpy = malloc(1 + sizeof(char)*srcStrLen);
    if (srcStrCpy == NULL)
    {
        eNum = 4;
        goto out;
    }
    strcpy(srcStrCpy, srcStr);

    /*------------------------------------------------------------------------*/
    /* Determine the total number of values in the array.                     */
    /*------------------------------------------------------------------------*/
    str0 = srcStrCpy;
    for (ctr = 0; ctr < srcStrLen; ctr++)
    {
        aChar = str0[ctr];
        isWS = FALSE;
        if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')
        {
            /*----------------------------------------------------------------*/
            /* The current active char is a whitespace character.             */
            /*----------------------------------------------------------------*/
            isWS = TRUE;
        }

        if (isWS && inValue)
        {
            /*----------------------------------------------------------------*/
            /* This is a whitespace character following the end of a value.   */
            /*----------------------------------------------------------------*/
            inValue = FALSE;
        }
        else if (!isWS && !inValue)
        {
            /*----------------------------------------------------------------*/
            /* The start of a value substring.                                */
            /*----------------------------------------------------------------*/
            inValue = TRUE;
            valCount++;
        }
    }
    *arrayLen = valCount;

    /*------------------------------------------------------------------------*/
    /* Allocate memory and set the value.                                     */
    /*------------------------------------------------------------------------*/
    *arrayPtr = malloc(sizeof(float)*valCount);
    if (*arrayPtr == NULL)
    {
        eNum = 4;
        goto out;
    }
    newArray = *arrayPtr;

    /*------------------------------------------------------------------------*/
    /* Extract the values from the string and store them in the array.        */
    /*------------------------------------------------------------------------*/
    str0 = srcStrCpy;
    for (ctr = 0; ctr < srcStrLen; ctr++)
    {
        aChar = srcStrCpy[ctr];
        isWS = FALSE;
        if (aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r')
        {
            /*----------------------------------------------------------------*/
            /* The current active char is a whitespace character.             */
            /*----------------------------------------------------------------*/
            isWS = TRUE;
        }

        if (isWS && inValue)
        {
            /*----------------------------------------------------------------*/
            /* This is a whitespace character following the end of a value.   */
            /*----------------------------------------------------------------*/
            inValue = FALSE;
            srcStrCpy[ctr] = '\0';
            if (strchr(str0, 'e'))
            {
                if (sscanf(str0, "%e", &nextFloat) != 1)
                {
                    eNum = 5;
                    goto out;
                }
            }
            else if (strchr(str0, 'E'))
            {
                if (sscanf(str0, "%E", &nextFloat) != 1)
                {
                    eNum = 5;
                    goto out;
                }
            }
            else if (sscanf(str0, "%f", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
            newArray[storeCtr] = nextFloat;
            storeCtr++;
        }
        else if (!isWS && !inValue)
        {
            /*----------------------------------------------------------------*/
            /* The start of a value substring.                                */
            /*----------------------------------------------------------------*/
            inValue = TRUE;
            str0 = srcStrCpy + ctr;
        }
    }

    if (inValue)
    {
        /*--------------------------------------------------------------------*/
        /* There is a final value in the field string; capture it here.       */
        /*--------------------------------------------------------------------*/
        if (strchr(str0, 'e'))
        {
            if (sscanf(str0, "%e", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
        }
        else if (strchr(str0, 'E'))
        {
            if (sscanf(str0, "%E", &nextFloat) != 1)
            {
                eNum = 5;
                goto out;
            }
        }
        else if (sscanf(str0, "%f", &nextFloat) != 1)
        {
            eNum = 5;
            goto out;
        }
        newArray[storeCtr] = nextFloat;
        storeCtr++;
    }
out:
    if (srcStrCpy != NULL) free(srcStrCpy);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_PlotData_SetValuesFromString
 *
 *  Description:   Supply a new set of X,Y data point values. The argument 
 *                 `valStr` is a whitespace separated list of float values.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_PlotData_SetValuesFromString(
    NEUIK_PlotData * pd,
    int              precision,
    const char     * valStr)
{
    int            isXval     = FALSE;
    unsigned int   ctr        = 0;
    unsigned int   arrayLen   = 0;
    float          nextVal    = 0.0;  /* the next value in the array */
    float        * f32Array   = NULL; /* free value before returning */
    double         nextF64Val = 0.0;  /* the next value in the array */
    double       * f64Array   = NULL; /* free value before returning */
    /*------------------------------------------------------------------------*/
    int           eNum       = 0;
    static char   funcName[] = "NEUIK_PlotData_SetValuesFromString";
    static char * errMsgs[]  = {"", // [0] no error
        "Output Argument `pd` is NULL.",                                    // [1]
        "Argument `valStr` is NULL.",                                       // [2]
        "Failure in `neuik_GetFloatArrayFromFields()`.",                    // [3]
        "Argument `valStr` must contain an even number of values.",         // [4]
        "Argument `valStr` must have values sorted by ascending X values.", // [5]
        "Argument `precision` has invalid value; must be `32` or `64`.",    // [6]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
    if (pd == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (valStr == NULL)
    {
        eNum = 2;
        goto out;
    }


    /*------------------------------------------------------------------------*/
    /* Read in the float values from the string.                              */
    /*------------------------------------------------------------------------*/
    switch (precision)
    {
    case 32:
        if (neuik_GetFloatArrayFromFields(&f32Array, &arrayLen, valStr))
        {
            eNum = 3;
            goto out;
        }
        break;
    case 64:
        if (neuik_GetDoubleArrayFromFields(&f64Array, &arrayLen, valStr))
        {
            eNum = 3;
            goto out;
        }
        break;
    default:
        /*--------------------------------------------------------------------*/
        /* Unsupported value provided for precision.                          */
        /*--------------------------------------------------------------------*/
        eNum = 6;
        goto out;
        break;
    }

    /* Make sure that there is an even number of float values provided.       */
    if (arrayLen % 2 == 1)
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Free previously allocated PlotData values (if present).                */
    /*------------------------------------------------------------------------*/
    if (pd->data_32 != NULL)
    {
        free(pd->data_32);
        pd->data_32         = NULL;
        pd->bounds_32.x_min = 0.0;
        pd->bounds_32.x_max = 0.0;
        pd->bounds_32.y_min = 0.0;
        pd->bounds_32.y_max = 0.0;
    }
    if (pd->data_64 != NULL)
    {
        free(pd->data_64);
        pd->data_64         = NULL;
        pd->bounds_64.x_min = 0.0;
        pd->bounds_64.x_max = 0.0;
        pd->bounds_64.y_min = 0.0;
        pd->bounds_64.y_max = 0.0;
    }
    pd->boundsSet = FALSE;

    /*------------------------------------------------------------------------*/
    /* Determine/Set the PlotData bounds from these values.                   */
    /*------------------------------------------------------------------------*/
    isXval = TRUE;
    switch (precision)
    {
    case 32:
        /*--------------------------------------------------------------------*/
        /* Store values as 32 bit floats.                                     */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < arrayLen; ctr++)
        {
            nextVal = f32Array[ctr];
            if (isXval)
            {
                /*------------------------------------------------------------*/
                /* Data is expected to be provided as a list of X,Y pairs.    */
                /* This section processes the bounds for the X-axis values.   */
                /*------------------------------------------------------------*/
                if (pd->boundsSet)
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point So this section is for the second point onwards. */
                    /*--------------------------------------------------------*/
                    if (nextVal < pd->bounds_32.x_min)
                    {
                        /*----------------------------------------------------*/
                        /* This would indicate that a subsequent data point   */
                        /* has an earlier position on the X-axis (i.e., data  */
                        /* points were not sorted properly).                  */
                        /*----------------------------------------------------*/
                        eNum = 5;
                        goto out;
                    }
                    else if (nextVal < pd->bounds_32.x_max)
                    {
                        /*----------------------------------------------------*/
                        /* This would indicate that a subsequent data point   */
                        /* has an earlier position on the X-axis (i.e., data  */
                        /* points were not sorted properly).                  */
                        /*----------------------------------------------------*/
                        eNum = 5;
                        goto out;
                    }
                    pd->bounds_32.x_max = nextVal;
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point.This section is for processing the first data    */
                    /* point.                                                 */
                    /*--------------------------------------------------------*/
                    pd->bounds_32.x_min = nextVal;
                    pd->bounds_32.x_max = nextVal;
                }
                isXval = FALSE;
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Data is expected to be provided as a list of X,Y pairs.    */
                /* This section processes the bounds for the Y-axis values.   */
                /*------------------------------------------------------------*/
                if (pd->boundsSet)
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first.   */
                    /* point. So this section is for the second point         */
                    /* onwards.                                               */
                    /*--------------------------------------------------------*/
                    if (nextVal < pd->bounds_32.y_min)
                    {
                        pd->bounds_32.y_min = nextVal;
                    }
                    if (nextVal > pd->bounds_32.y_max)
                    {
                        pd->bounds_32.y_max = nextVal;
                    }
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point. This section is for processing the first data   */
                    /* point.                                                 */
                    /*--------------------------------------------------------*/
                    pd->bounds_32.y_min = nextVal;
                    pd->bounds_32.y_max = nextVal;
                }
                isXval        = TRUE;
                pd->boundsSet = TRUE;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Set the values within the PlotData object.                         */
        /*--------------------------------------------------------------------*/
        pd->stateMod++;
        pd->nAlloc    = arrayLen;
        pd->nPoints   = arrayLen/2;
        pd->nUsed     = arrayLen;
        pd->precision = 32;
        pd->data_32   = f32Array;
        break;

    case 64:
        /*--------------------------------------------------------------------*/
        /* Store values as 64 bit doubles.                                    */
        /*--------------------------------------------------------------------*/
        for (ctr = 0; ctr < arrayLen; ctr++)
        {
            nextF64Val = f64Array[ctr];
            if (isXval)
            {
                /*------------------------------------------------------------*/
                /* Data is expected to be provided as a list of X,Y pairs.    */
                /* This section processes the bounds for the X-axis values.   */
                /*------------------------------------------------------------*/
                if (pd->boundsSet)
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point So this section is for the second point onwards. */
                    /*--------------------------------------------------------*/
                    if (nextF64Val < pd->bounds_64.x_min)
                    {
                        /*----------------------------------------------------*/
                        /* This would indicate that a subsequent data point   */
                        /* has an earlier position on the X-axis (i.e., data  */
                        /* points were not sorted properly).                  */
                        /*----------------------------------------------------*/
                        eNum = 5;
                        goto out;
                    }
                    else if (nextF64Val < pd->bounds_64.x_max)
                    {
                        /*----------------------------------------------------*/
                        /* This would indicate that a subsequent data point   */
                        /* has an earlier position on the X-axis (i.e., data  */
                        /* points were not sorted properly).                  */
                        /*----------------------------------------------------*/
                        eNum = 5;
                        goto out;
                    }
                    pd->bounds_64.x_max = nextF64Val;
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point.This section is for processing the first data    */
                    /* point.                                                 */
                    /*--------------------------------------------------------*/
                    pd->bounds_64.x_min = nextF64Val;
                    pd->bounds_64.x_max = nextF64Val;
                }
                isXval = FALSE;
            }
            else
            {
                /*------------------------------------------------------------*/
                /* Data is expected to be provided as a list of X,Y pairs.    */
                /* This section processes the bounds for the Y-axis values.   */
                /*------------------------------------------------------------*/
                if (pd->boundsSet)
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first.   */
                    /* point. So this section is for the second point         */
                    /* onwards.                                               */
                    /*--------------------------------------------------------*/
                    if (nextF64Val < pd->bounds_64.y_min)
                    {
                        pd->bounds_64.y_min = nextF64Val;
                    }
                    if (nextF64Val > pd->bounds_64.y_max)
                    {
                        pd->bounds_64.y_max = nextF64Val;
                    }
                }
                else
                {
                    /*--------------------------------------------------------*/
                    /* Bounds are initially set after processing the first    */
                    /* point. This section is for processing the first data   */
                    /* point.                                                 */
                    /*--------------------------------------------------------*/
                    pd->bounds_64.y_min = nextF64Val;
                    pd->bounds_64.y_max = nextF64Val;
                }
                isXval        = TRUE;
                pd->boundsSet = TRUE;
            }
        }

        /*--------------------------------------------------------------------*/
        /* Set the values within the PlotData object.                         */
        /*--------------------------------------------------------------------*/
        pd->stateMod++;
        pd->nAlloc    = arrayLen;
        pd->nPoints   = arrayLen/2;
        pd->nUsed     = arrayLen;
        pd->precision = 64;
        pd->data_64   = f64Array;
        break;
    }
out:
    if (eNum > 0)
    {
        if (f32Array != NULL) free(f32Array);
        if (f64Array != NULL) free(f64Array);

        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          NEUIK_PlotData_WriteValuesToASCIIFile
 *
 *  Description:   Write out the values contained within a PlotData object.
 *                 Optionally this data file can include additional header 
 *                 information.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_PlotData_WriteValuesToASCIIFile(
    NEUIK_PlotData * pd,
    const char     * fileName,
    int              writeHeader) /* If a PlotData header should be written. */
{
    int    ctr     = 0;
    int    posCtr  = 0;
    FILE * outFile = NULL;
    static char cmtBarLn[] =
        "#------------------------------------------------------------------------------#\n";

    /*------------------------------------------------------------------------*/
    int           eNum       = 0;
    static char   funcName[] = "NEUIK_PlotData_WriteValuesToASCIIFile";
    static char * errMsgs[]  = {"", // [0] no error 
        "Argument `pd` is NULL.",                        // [1]
        "Argument `pd` is not of PlotData class.",       // [2]
        "Argument `fileName` is NULL.",                  // [3]
        "Argument `fileName` supplied an empty string.", // [4]
        "Failed to open file for writing.",              // [5]
        "Internal 32-bit float data array is NULL.",     // [6]
        "Internal 64-bit float data array is NULL.",     // [7]
    };

    /*------------------------------------------------------------------------*/
    /* Check for errors before continuing.                                    */
    /*------------------------------------------------------------------------*/
    if (pd == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(pd, neuik__Class_PlotData))
    {
        eNum = 2;
        goto out;
    }
    if (fileName == NULL)
    {
        eNum = 3;
        goto out;
    }
    if (strlen(fileName) == 0)
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Attempt to open the file for writing.                                  */
    /*------------------------------------------------------------------------*/
    outFile = fopen(fileName, "w");
    if (outFile == NULL)
    {
        eNum = 5;
        goto out;
    }

    if (writeHeader)
    {
        fprintf(outFile, cmtBarLn);
        fprintf(outFile, "# NEUIK_PlotData -- ASCII\n");
        fprintf(outFile, "# uniqueName : `%s`\n", pd->uniqueName);
        fprintf(outFile, "# precision  : %d\n", pd->precision);
        fprintf(outFile, "# nPoints    : %d\n", pd->nPoints);
    }


    switch (pd->precision)
    {
        case 32:
            if (pd->data_32 == NULL)
            {
                eNum = 6;
                goto out;
            }
            if (writeHeader)
            {
                fprintf(outFile, "# x_min      : % 16.10e\n",
                    pd->bounds_32.x_min);
                fprintf(outFile, "# x_max      : % 16.10e\n",
                    pd->bounds_32.x_max);
                fprintf(outFile, "# y_min      : % 16.10e\n",
                    pd->bounds_32.y_min);
                fprintf(outFile, "# y_max      : % 16.10e\n",
                    pd->bounds_32.y_max);
                fprintf(outFile, cmtBarLn);
            }

            for (ctr = 0; ctr < pd->nPoints; ctr++)
            {
                fprintf(outFile, "% 16.10e % 16.10e\n", 
                    pd->data_32[posCtr],
                    pd->data_32[posCtr+1]);
                posCtr += 2;
            }
            break;
        case 64:
            if (pd->data_64 == NULL)
            {
                eNum = 7;
                goto out;
            }
            if (writeHeader)
            {
                fprintf(outFile, "# x_min      : % 18.12e\n",
                    pd->bounds_64.x_min);
                fprintf(outFile, "# x_max      : % 18.12e\n",
                    pd->bounds_64.x_max);
                fprintf(outFile, "# y_min      : % 18.12e\n",
                    pd->bounds_64.y_min);
                fprintf(outFile, "# y_max      : % 18.12e\n",
                    pd->bounds_64.y_max);
                fprintf(outFile, cmtBarLn);
            }

            for (ctr = 0; ctr < pd->nPoints; ctr++)
            {
                fprintf(outFile, "% 18.12e % 18.12e\n", 
                    pd->data_64[posCtr],
                    pd->data_64[posCtr+1]);
                posCtr += 2;
            }
            break;
        default:
            fprintf(outFile, cmtBarLn);
            break;
    }
    fflush(outFile);
    fclose(outFile);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}

