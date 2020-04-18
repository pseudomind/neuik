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
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "NEUIK_defs.h"
#include "neuik_MaskMap.h"
#include "NEUIK_error.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__MaskMap(void **);
int neuik_Object_Free__MaskMap(void *);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_MaskMap_BaseFuncs = {
    /* Init(): Class initialization (in most cases will not be needed) */
    NULL, /* (unused) */
    /* New(): Allocate and Initialize the object */
    neuik_Object_New__MaskMap,
    /* Copy(): Copy the contents of one object into another */
    NULL,
    /* Free(): Free the allocated memory of an object */
    neuik_Object_Free__MaskMap,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_MaskMap
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_MaskMap()
{
    int            eNum       = 0; /* which error to report (if any) */
    static char    funcName[] = "neuik_RegisterClass_MaskMap";
    static char  * errMsgs[]  = {"", // [0] no error
        "NEUIK library must be initialized first.",    // [1]
        "Failed to register `MaskMap` object class .", // [2]
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
        "neuik_MaskMap",             // className
        "The neuik_MaskMap Object.", // classDescription
        neuik__Set_NEUIK,            // classSet
        NULL,                        // superClass
        &neuik_MaskMap_BaseFuncs,    // baseFuncs
        NULL,                        // classFuncs
        &neuik__Class_MaskMap))      // newClass
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
 *  Name:          neuik_Object_New
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__MaskMap(
    void  ** mapPtr)
{
    return neuik_NewMaskMap((neuik_MaskMap **)mapPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_NewMaskMap
 *
 *  Description:   Allocates and initializes values for a new MaskMap.
 *
 *  Returns:       NULL if error otherwise, returns a pointer to a valid 
 *                 MaskMap. 
 *
 ******************************************************************************/
int neuik_NewMaskMap(
    neuik_MaskMap ** mapPtr)
{
    int             eNum       = 0; /* which error to report (if any) */
    neuik_MaskMap * map        = NULL;
    static char     funcName[] = "neuik_NewMaskMap";
    static char   * errMsgs[]  = {"", // [0] no error
        "Output Argument `mapPtr` is NULL.",     // [1]
        "Failure to allocate memory.",           // [2]
    };

    if (mapPtr == NULL)
    {
        eNum = 1;
        goto out;
    }

    (*mapPtr) = (neuik_MaskMap*) malloc(sizeof(neuik_MaskMap));
    map = (*mapPtr);
    if (map == NULL)
    {
        eNum = 2;
        goto out;
    }

    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_MaskMap, 
        NULL,
        &(map->objBase));

    /* initialize pointers to NULL */
    map->mapData  = NULL;
    map->regStart = NULL;
    map->regEnd   = NULL;

    /* set default values */
    map->sizeW     = 0;
    map->sizeH     = 0;
    map->nRegAlloc = 20;

    /*------------------------------------------------------------------------*/
    /* Perform initial allocation.                                            */
    /*------------------------------------------------------------------------*/
    map->regStart = malloc(map->nRegAlloc*sizeof(int));
    if (map->regStart == NULL)
    {
        eNum = 2;
        goto out;
    }
    map->regEnd = malloc(map->nRegAlloc*sizeof(int));
    if (map->regEnd == NULL)
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


int neuik_Object_Free__MaskMap(
    void * mapPtr)
{
    return neuik_MaskMap_Free((neuik_MaskMap *)mapPtr);
}


/*******************************************************************************
 *
 *  Name:          neuik_MakeMaskMap
 *
 *  Description:   Allocates and initializes values for a new MaskMap. In
 *                 addition, this function will allocate a map of the specified
 *                 size.
 *
 *  Returns:       NULL if error otherwise, returns a pointer to a valid 
 *                 MaskMap. 
 *
 ******************************************************************************/
int neuik_MakeMaskMap(
    neuik_MaskMap ** mapPtr,
    int              width,
    int              height)
{
    int             eNum       = 0; /* which error to report (if any) */
    int             aSize      = 0; /* allocation size */
    neuik_MaskMap * map        = NULL;
    static char     funcName[] = "neuik_MakeMaskMap";
    static char   * errMsgs[]  = {"", // [0] no error
        "Output Argument `mapPtr` is NULL.",                // [1]
        "", // [2]
        "Failure to allocate memory.",                      // [3]
        "Argument `width` invalid;  value (<=0) supplied.", // [4]
        "Argument `height` invalid; value (<=0) supplied.", // [5]
    };

    if (mapPtr == NULL)
    {
        eNum = 1;
        goto out;
    }
    if (width <= 0)
    {
        /* invalid width */
        eNum = 4;
        goto out;
    }
    if (height <= 0)
    {
        /* invalid height */
        eNum = 5;
        goto out;
    }

    (*mapPtr) = (neuik_MaskMap*) malloc(sizeof(neuik_MaskMap));
    map = (*mapPtr);
    if (map == NULL)
    {
        eNum = 3;
        goto out;
    }

    neuik_GetObjectBaseOfClass(
        neuik__Set_NEUIK, 
        neuik__Class_MaskMap, 
        NULL,
        &(map->objBase));

    /* initialize pointers to NULL */
    map->mapData  = NULL;
    map->regStart = NULL;
    map->regEnd   = NULL;

    /* set default values */
    map->sizeW     = 0;
    map->sizeH     = 0;
    map->nRegAlloc = 20;

    /*------------------------------------------------------------------------*/
    /* Perform initial allocation.                                            */
    /*------------------------------------------------------------------------*/
    map->regStart = malloc(map->nRegAlloc*sizeof(int));
    if (map->regStart == NULL)
    {
        eNum = 3;
        goto out;
    }
    map->regEnd = malloc(map->nRegAlloc*sizeof(int));
    if (map->regEnd == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Allocate and set sizing information.                                   */
    /*------------------------------------------------------------------------*/
    map->sizeW = width;
    map->sizeH = height;

    aSize = width*height;
    map->mapData = malloc(aSize*sizeof(char));
    if (map->mapData == NULL)
    {
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Initialize all mask map values with zeros.                             */
    /*------------------------------------------------------------------------*/
    memset(map->mapData, 0, aSize);
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
 *  Name:          neuik_MaskMap_Free
 *
 *  Description:   Free all of the resources loaded by the MaskMap.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_MaskMap_Free(
    neuik_MaskMap * map) /* (in,out) the object to free */
{
    int            eNum       = 0;
    static char    funcName[] = "neuik_MaskMap_Free";
    static char  * errMsgs[]  = {"", // [0] no error
        "Argument `map` is NULL.",                          // [1]
        "Argument `map` does not implement MaskMap class.", // [2]
    };

    if (map == NULL)
    {
        eNum = 1;
        goto out;
    }

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 2;
        goto out;
    }


    /*------------------------------------------------------------------------*/
    /* Free all memory that was dynamically allocated for this object         */
    /*------------------------------------------------------------------------*/
    if (map->mapData != NULL)
    {
        free(map->mapData);
    }
    if (map->regStart != NULL)
    {
        free(map->regStart);
    }
    if (map->regEnd != NULL)
    {
        free(map->regEnd);
    }

    free(map);
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
 *  Name:          neuik_MaskMap_InvertValues
 *
 *  Description:   Switch the values used for all points (i.e., 0->1 and 1->0).
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_InvertValues(
    neuik_MaskMap * map)
{
    int           ctr   = 0; /* iteration counter */
    int           aSize = 0; /* allocation size */
    int           eNum  = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_InvertValues";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "map->mapData is NULL; was the mask size set?",     // [2]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }
    if (map->mapData == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* This is a proper rect (non-zero w & h).                                */
    /*------------------------------------------------------------------------*/
    aSize = map->sizeW*map->sizeH;
    for (ctr = 0; ctr < aSize; ctr++)
    {
        if (map->mapData[ctr] == 0)
        {
            map->mapData[ctr] = 1;
        }
        else
        {
            map->mapData[ctr] = 0;
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_PrintValues
 *
 *  Description:   Print the values of the contained points to stdout.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_PrintValues(
    neuik_MaskMap * map)
{
    int           xCtr  = 0; /* iteration counter */
    int           yCtr  = 0; /* iteration counter */
    int           eNum  = 0; /* which error to report (if any) */
    int           pos   = 0; /* position of point within mapData */
    static char   funcName[] = "neuik_MaskMap_PrintValues";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "map->mapData is NULL; was the mask size set?",     // [2]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }
    if (map->mapData == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Print out the values line by line.                                     */
    /*------------------------------------------------------------------------*/
    for (yCtr = 0; yCtr < map->sizeH; yCtr++)
    {
        for (xCtr = 0; xCtr < map->sizeW; xCtr++)
        {
            pos = map->sizeW*yCtr + xCtr;
            if (map->mapData[pos] == 0)
            {
                printf("0");
            }
            else
            {
                printf("1");
            }
        }
        printf("\n");
    }
    printf("\n");
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_SetSize
 *
 *  Description:   Set the outer (x,y) dimensions of a MaskMap.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_SetSize(
    neuik_MaskMap * map, 
    int             width,
    int             height)
{
    int           aSize = 0; /* allocation size */
    int           eNum  = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_SetSize";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "Argument `width` invalid;  value (<=0) supplied.", // [2]
        "Argument `height` invalid; value (<=0) supplied.", // [3]
        "Failure to allocate memory.",                      // [4]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (width <= 0)
    {
        /* invalid width */
        eNum = 2;
        goto out;
    }
    map->sizeW = width;
    if (height <= 0)
    {
        /* invalid height */
        eNum = 3;
        goto out;
    }
    map->sizeH = height;

    if (map->mapData != NULL)
    {
        free(map->mapData);
    }

    aSize = width*height;
    map->mapData = malloc(aSize*sizeof(char));
    if (map->mapData == NULL)
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Initialize all mask map values with zeros.                             */
    /*------------------------------------------------------------------------*/
    memset(map->mapData, 0, aSize);
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
 *  Name:          neuik_MaskMap_Resize
 *
 *  Description:   Change the outer (x,y) dimensions of a MaskMap while 
 *                 preserving as much of the existing maskData as possible.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_Resize(
    neuik_MaskMap * map, 
    int             width,
    int             height)
{
    int           ctr     = 0;    /* counter (used for loops) */
    int           colCtr  = 0;    /* column counter */
    int           rowCtr  = 0;    /* row counter */
    int           aSize   = 0;    /* allocation size */
    int           eNum    = 0;    /* which error to report (if any) */
    int           pos     = 0;    /* position of point within mapData */
    int           oldW    = 0;    /* The old maskMap width */
    int           oldH    = 0;    /* The old maskMap height */
    char          oldVal;         /* Old mask value at a point */
    char        * oldData = NULL; /* copy of mask data; FREE AT EXIT */
    static char   funcName[] = "neuik_MaskMap_Resize";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "Argument `width` invalid;  value (<=0) supplied.", // [2]
        "Argument `height` invalid; value (<=0) supplied.", // [3]
        "Failure to allocate memory.",                      // [4]
        "Failure to reallocate memory.",                    // [5]
    };

    /*------------------------------------------------------------------------*/
    /* Check for some potential input issues before continuing.               */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    oldW = map->sizeW;
    oldH = map->sizeH;

    if (width <= 0)
    {
        /* invalid width */
        eNum = 2;
        goto out;
    }
    if (height <= 0)
    {
        /* invalid height */
        eNum = 3;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check if this resize command actually results in a change of size.     */
    /*------------------------------------------------------------------------*/
    if (oldW == width && oldH == height)
    {
        /* Resize called with the current size; return */
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Copy the memory address used for the current mapData array.            */
    /*------------------------------------------------------------------------*/
    oldData = map->mapData;

    /*------------------------------------------------------------------------*/
    /* Resize `map->mapData`. Since there is a dimensional change of the map  */
    /* there is no benefit to preserving the current data. Therefore a free() */
    /* & malloc() will be used in favor of a realloc(), as this will result   */
    /* in better performance.                                                 */
    /*------------------------------------------------------------------------*/
    aSize = width*height;
    map->mapData = malloc(aSize*sizeof(char));
    if (map->mapData == NULL)
    {
        eNum = 4;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Set all of the initial values to zero before copying over the old mask */
    /* data.                                                                  */
    /*------------------------------------------------------------------------*/
    memset(map->mapData, 0, aSize);

    /*------------------------------------------------------------------------*/
    /* Copy over the values from the maskData copy into the new maskData.     */
    /*------------------------------------------------------------------------*/
    for (rowCtr = 0; rowCtr < oldH; rowCtr++)
    {
        if (colCtr >= width)
        {
            /*----------------------------------------------------------------*/
            /* The new maskMap height is smaller and this is out of bounds.   */
            /*----------------------------------------------------------------*/
            break;
        }

        for (colCtr = 0; colCtr < oldW; colCtr++)
        {
            if (colCtr >= width)
            {
                /*------------------------------------------------------------*/
                /* The new maskMap width is smaller and this is out of        */
                /* bounds; continue copying on next row of map.               */
                /*------------------------------------------------------------*/
                break;
            }
            pos = oldW*rowCtr + colCtr;
            oldVal = oldData[pos];

            map->mapData[ctr] = oldVal;
        }
    }
    map->sizeW = width;
    map->sizeH = height;
out:
    if (oldData != NULL) free(oldData);

    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
        eNum = 1;
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_MaskAll
 *
 *  Description:   Set the entire mask map as masked. Masked points are used to
 *                 identify portions of an image that should not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_MaskAll(
    neuik_MaskMap * map)
{
    int           aSize = 0; /* allocation size */
    int           eNum  = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_MaskAll";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "map->mapData is NULL; was the mask size set?",     // [2]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }
    if (map->mapData == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the allocate size.                                           */
    /*------------------------------------------------------------------------*/
    aSize = map->sizeW*map->sizeH;

    /*------------------------------------------------------------------------*/
    /* Initialize all mask map values with ones.                              */
    /*------------------------------------------------------------------------*/
    memset(map->mapData, 1, aSize);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_UnmaskAll
 *
 *  Description:   Set the entire mask map as unmasked. Masked points are used
 *                 to identify portions of an image that should not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_UnmaskAll(
    neuik_MaskMap * map)
{
    int           aSize = 0; /* allocation size */
    int           eNum  = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_UnmaskAll";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "map->mapData is NULL; was the mask size set?",     // [2]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }
    if (map->mapData == NULL)
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Calculate the allocate size.                                           */
    /*------------------------------------------------------------------------*/
    aSize = map->sizeW*map->sizeH;

    /*------------------------------------------------------------------------*/
    /* Initialize all mask map values with zeros.                             */
    /*------------------------------------------------------------------------*/
    memset(map->mapData, 0, aSize);
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_MaskPoint
 *
 *  Description:   Flag a point within the map as masked. Masked points are used
 *                 to identify portions of an image that should not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_MaskPoint(
    neuik_MaskMap * map, 
    int             x,
    int             y)
{
    int           pos  = 0; /* position of point within mapData */
    int           eNum = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_MaskPoint";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "Argument `x` invalid;  value (<0) supplied.",      // [2]
        "Argument `y` invalid; value (<0) supplied.",       // [3]
        "Argument `x` invalid; exceeds mask bounds.",       // [4]
        "Argument `y` invalid; exceeds mask bounds..",      // [5]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (x < 0)
    {
        /* invalid x-value (<0) */
        eNum = 2;
        goto out;
    }
    else if (x >= map->sizeW)
    {
        /* x-value beyond bounds */
        eNum = 4;
        goto out;
    }
    if (y < 0)
    {
        /* invalid y-value */
        eNum = 3;
        goto out;
    }
    else if (y >= map->sizeH)
    {
        /* y-value beyond bounds */
        eNum = 5;
        goto out;
    }


    pos = map->sizeW*y + x;
    map->mapData[pos] = 1;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_UnmaskPoint
 *
 *  Description:   Flag a point within the map as unmasked. Masked points are
 *                 used to identify portions of an image that should not be 
 *                 rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_UnmaskPoint(
    neuik_MaskMap * map, 
    int             x,
    int             y)
{
    int           pos  = 0; /* position of point within mapData */
    int           eNum = 0; /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_UnmaskPoint";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.", // [1]
        "Argument `x` invalid;  value (<0) supplied.",      // [2]
        "Argument `y` invalid; value (<0) supplied.",       // [3]
        "Argument `x` invalid; exceeds mask bounds.",       // [4]
        "Argument `y` invalid; exceeds mask bounds..",      // [5]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (x < 0)
    {
        /* invalid x-value (<0) */
        eNum = 2;
        goto out;
    }
    else if (x >= map->sizeW)
    {
        /* x-value beyond bounds */
        eNum = 4;
        goto out;
    }
    if (y < 0)
    {
        /* invalid y-value */
        eNum = 3;
        goto out;
    }
    else if (y >= map->sizeH)
    {
        /* y-value beyond bounds */
        eNum = 5;
        goto out;
    }


    pos = map->sizeW*y + x;
    map->mapData[pos] = 0;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_SetMaskLine
 *
 *  Description:   Set the mask setting for a line of points within the map. 
 *                 Masked points are used to identify portions of an image that
 *                 should not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_SetMaskLine(
    neuik_MaskMap * map, 
    int             maskVal, /* 0 (unmaksed) or 1 (masked) */
    int             x1,
    int             y1,
    int             x2,
    int             y2)
{
    int           pos   = 0;   /* position of point within mapData */
    int           pos0  = 0;   /* position of point (x1,y1) within mapData */
    int           eNum  = 0;   /* which error to report (if any) */
    int           idx   = 0;   /* delta x (x2 - x1); as an integer */
    int           idy   = 0;   /* delta y (y2 - y1); as an integer */
    double        dx    = 0.0; /* delta x (x2 - x1) */
    double        dy    = 0.0; /* delta y (y2 - y1) */
    double        dxInt = 0.0; /* dx resulting from a single loop interval */
    double        dyInt = 0.0; /* dy resulting from a single loop interval */
    double        hyp   = 0.0; /* length of the hypotenuse */
    double        fCtr  = 0.0; /* float counter */
    static char   funcName[] = "neuik_MaskMap_SetMaskLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.",  // [1]
        "Argument `x1` invalid; value (<0) supplied.",       // [2]
        "Argument `y1` invalid; value (<0) supplied.",       // [3]
        "Argument `x1` invalid; exceeds mask bounds.",       // [4]
        "Argument `y1` invalid; exceeds mask bounds..",      // [5]
        "Argument `x2` invalid; value (<0) supplied.",       // [6]
        "Argument `y2` invalid; value (<0) supplied.",       // [7]
        "Argument `x2` invalid; exceeds mask bounds.",       // [8]
        "Argument `y2` invalid; exceeds mask bounds..",      // [9]
        "Argument `maskVal` invalid; value must be 0 or 1.", // [10]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (!(maskVal == 0 || maskVal == 1))
    {
        eNum = 10;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check for coordinate argument input errors (invalid/out-of-bounds).    */
    /*------------------------------------------------------------------------*/
    /* Check the x1,y1 values.                                                */
    /*------------------------------------------------------------------------*/
    if (x1 < 0)
    {
        /* invalid x1-value (<0) */
        eNum = 2;
        goto out;
    }
    else if (x1 >= map->sizeW)
    {
        /* x1-value beyond bounds */
        eNum = 4;
        goto out;
    }
    if (y1 < 0)
    {
        /* invalid y1-value */
        eNum = 3;
        goto out;
    }
    else if (y1 >= map->sizeH)
    {
        /* y1-value beyond bounds */
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check the x2,y2 values.                                                */
    /*------------------------------------------------------------------------*/
    if (x2 < 0)
    {
        /* invalid x2-value (<0) */
        eNum = 6;
        goto out;
    }
    else if (x2 >= map->sizeW)
    {
        /* x2-value beyond bounds */
        eNum = 8;
        goto out;
    }
    if (y2 < 0)
    {
        /* invalid y2-value */
        eNum = 3;
        goto out;
    }
    else if (y2 >= map->sizeH)
    {
        /* y2-value beyond bounds */
        eNum = 9;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Mask a line of values between point (x1,y1) and (x2,y2).               */
    /*------------------------------------------------------------------------*/
    idx = x2 - x1;
    idy = y2 - y1;
    dx = (double)(idx);
    dy = (double)(idy);

    if (idx == 0 && idy == 0)
    {
        /*--------------------------------------------------------------------*/
        /* This line is actually just a point.                                */
        /*--------------------------------------------------------------------*/
        pos = map->sizeW*y1 + x1;
        map->mapData[pos] = 1;
        goto out;
    }

    if (idx == 0)
    {
        hyp   = dy;
        dyInt = 1.0;
        if (idy < 0)
        {
            dyInt = -dyInt;
        }
    }
    else if (idy == 0)
    {
        hyp = dx;
        dxInt = 1.0;
        if (idx < 0)
        {
            dxInt = -dxInt;
        }
    }
    else
    {
        hyp = sqrt(dx*dx + dy*dy);
        dxInt = dx/hyp;
        dyInt = dy/hyp;
    }

    /*------------------------------------------------------------------------*/
    /* Mark the first and final points of the line first.                     */
    /*------------------------------------------------------------------------*/
    pos0 = map->sizeW*y1 + x1;
    pos = pos0;
    map->mapData[pos] = maskVal;

    pos = map->sizeW*y2 + x2;
    map->mapData[pos] = maskVal;

    /*------------------------------------------------------------------------*/
    /* Mark the rest of the points on the line.                               */
    /*------------------------------------------------------------------------*/
    for (fCtr = 1.0; fCtr < hyp; fCtr += 1.0)
    {
        pos = pos0 + map->sizeW*(int)(fCtr*dyInt) + (int)(fCtr*dxInt);
        map->mapData[pos] = maskVal;
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_SetUnboundedMaskLine
 *
 *  Description:   Set the mask setting for a line of points within the map. 
 *                 Masked points are used to identify portions of an image that
 *                 should not be rendered. The unbounded variant of this 
 *                 function does not check perform bounds checking on the line
 *                 to be (un)masked. Instead the individual pixels of the 
 *                 resulting line are bounds checked and are applied only if 
 *                 they are actually within the mask bounds.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_SetUnboundedMaskLine(
    neuik_MaskMap * map, 
    int             maskVal, /* 0 (unmaksed) or 1 (masked) */
    int             x1,
    int             y1,
    int             x2,
    int             y2)
{
    int           pos   = 0;   /* position of point within mapData */
    int           pos0  = 0;   /* position of point (x1,y1) within mapData */
    int           ptX   = 0;   /* x-axis position of a point */
    int           ptY   = 0;   /* y-axis position of a point */
    int           eNum  = 0;   /* which error to report (if any) */
    int           idx   = 0;   /* delta x (x2 - x1); as an integer */
    int           idy   = 0;   /* delta y (y2 - y1); as an integer */
    double        dx    = 0.0; /* delta x (x2 - x1) */
    double        dy    = 0.0; /* delta y (y2 - y1) */
    double        dxInt = 0.0; /* dx resulting from a single loop interval */
    double        dyInt = 0.0; /* dy resulting from a single loop interval */
    double        hyp   = 0.0; /* length of the hypotenuse */
    double        fCtr  = 0.0; /* float counter */
    static char   funcName[] = "neuik_MaskMap_SetUnboundedMaskLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.",  // [1]
        "Argument `maskVal` invalid; value must be 0 or 1.", // [2]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (!(maskVal == 0 || maskVal == 1))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Mask a line of values between point (x1,y1) and (x2,y2).               */
    /*------------------------------------------------------------------------*/
    idx = x2 - x1;
    idy = y2 - y1;
    dx = (double)(idx);
    dy = (double)(idy);

    if (idx == 0 && idy == 0)
    {
        /*--------------------------------------------------------------------*/
        /* This line is actually just a point.                                */
        /*--------------------------------------------------------------------*/
        if (!(y1 >= 0 && y1 < map->sizeH && x1 >= 0 && x1 < map->sizeW))
        {
            /* this point lies within the mask bounds */
            pos = map->sizeW*y1 + x1;
            map->mapData[pos] = 1;
        }
        goto out;
    }

    if (idx == 0)
    {
        hyp   = dy;
        dyInt = 1.0;
        if (idy < 0)
        {
            dyInt = -dyInt;
        }
    }
    else if (idy == 0)
    {
        hyp = dx;
        dxInt = 1.0;
        if (idx < 0)
        {
            dxInt = -dxInt;
        }
    }
    else
    {
        hyp = sqrt(dx*dx + dy*dy);
        dxInt = dx/hyp;
        dyInt = dy/hyp;
    }

    /*------------------------------------------------------------------------*/
    /* Mark the first and final points of the line first.                     */
    /*------------------------------------------------------------------------*/
    if (y1 >= 0 && y1 < map->sizeH && x1 >= 0 && x1 < map->sizeW)
    {
        /* this point lies within the mask bounds */
        pos0 = map->sizeW*y1 + x1;
        pos = pos0;
        map->mapData[pos] = maskVal;
    }

    if (y2 >= 0 && y2 < map->sizeH && x2 >= 0 && x2 < map->sizeW)
    {
        /* this point lies within the mask bounds */
        pos = map->sizeW*y2 + x2;
        map->mapData[pos] = maskVal;
    }

    /*------------------------------------------------------------------------*/
    /* Mark the rest of the points on the line.                               */
    /*------------------------------------------------------------------------*/
    for (fCtr = 1.0; fCtr < hyp; fCtr += 1.0)
    {
        ptX = x1 + (int)(fCtr*dxInt);
        ptY = y1 + (int)(fCtr*dyInt);
        if (ptY >= 0 && ptY < map->sizeH && ptX >= 0 && ptX < map->sizeW)
        {
            /* this point lies within the mask bounds */
            pos = map->sizeW*ptY + ptX;
            map->mapData[pos] = maskVal;
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_MaskLine
 *
 *  Description:   Flag a line of points within the map as masked. Masked points
 *                 are used to identify portions of an image that should not be 
 *                 rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_MaskLine(
    neuik_MaskMap * map, 
    int             x1,
    int             y1,
    int             x2,
    int             y2)
{
    int           eNum  = 0;   /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_MaskLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_MaskMap_SetMaskLine()`.", // [1]
    };

    if (neuik_MaskMap_SetMaskLine(map, 1, x1, y1, x2, y2))
    {
        eNum = 1;
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_UnmaskLine
 *
 *  Description:   Flag a line of points within the map as unmasked. Masked 
 *                 points are used to identify portions of an image that should 
 *                 not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_UnmaskLine(
    neuik_MaskMap * map, 
    int             x1,
    int             y1,
    int             x2,
    int             y2)
{
    int           eNum  = 0;   /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_UnmaskLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_MaskMap_SetMaskLine()`.", // [1]
    };

    if (neuik_MaskMap_SetMaskLine(map, 0, x1, y1, x2, y2))
    {
        eNum = 1;
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_UnmaskUnboundedLine
 *
 *  Description:   Flag a line of points within the map as unmasked. Masked 
 *                 points are used to identify portions of an image that should 
 *                 not be rendered. The unbounded variant of this function does
 *                 not check perform bounds checking on the line to be 
 *                 unmasked. Instead the individual pixels of the resulting line
 *                 are bounds checked and are applied only if they are actually
 *                 within the mask bounds.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_UnmaskUnboundedLine(
    neuik_MaskMap * map, 
    int             x1,
    int             y1,
    int             x2,
    int             y2)
{
    int           eNum  = 0;   /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_UnmaskUnboundedLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_MaskMap_SetUnboundedMaskLine()`.", // [1]
    };

    if (neuik_MaskMap_SetUnboundedMaskLine(map, 0, x1, y1, x2, y2))
    {
        eNum = 1;
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_SetMaskRect
 *
 *  Description:   Set the mask setting for a rect of points within the map. 
 *                 Masked points are used to identify portions of an image that
 *                 should not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_SetMaskRect(
    neuik_MaskMap * map,
    int             maskVal, /* 0 (unmaksed) or 1 (masked) */
    int             x,
    int             y,
    int             w,
    int             h)
{
    int           pos   = 0;   /* position of point within mapData */
    int           eNum  = 0;   /* which error to report (if any) */
    int           xCtr  = 0;
    int           yCtr  = 0;
    int           xf    = 0;   /* final x-position for the rect */
    int           yf    = 0;   /* final y-position for the rect */
    static char   funcName[] = "neuik_MaskMap_SetMaskRect";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.",  // [1]
        "Argument `x` invalid; value (<0) supplied.",        // [2]
        "Argument `x` invalid; exceeds mask bounds.",        // [3]
        "Argument `y` invalid; value (<0) supplied.",        // [4]
        "Argument `y` invalid; exceeds mask bounds..",       // [5]
        "Argument `w` invalid; value (<=0) supplied.",       // [6]
        "Argument `h` invalid; value (<=0) supplied.",       // [7]
        "Argument `maskVal` invalid; value must be 0 or 1.", // [8]
    };

    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (!(maskVal == 0 || maskVal == 1))
    {
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check for coordinate argument input errors (invalid/out-of-bounds).    */
    /*------------------------------------------------------------------------*/
    /* Check the x,y values.                                                  */
    /*------------------------------------------------------------------------*/
    if (x < 0)
    {
        /* invalid x-value (<0) */
        eNum = 2;
        goto out;
    }
    else if (x >= map->sizeW)
    {
        /* x-value beyond bounds */
        eNum = 3;
        goto out;
    }
    if (y < 0)
    {
        /* invalid y-value */
        eNum = 4;
        goto out;
    }
    else if (y >= map->sizeH)
    {
        /* y-value beyond bounds */
        eNum = 5;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check the w,h values.                                                  */
    /*------------------------------------------------------------------------*/
    if (w < 0)
    {
        /* invalid w-value (<0) */
        eNum = 6;
        goto out;
    }
    if (h < 0)
    {
        /* invalid h-value */
        eNum = y;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Determine the actual final bounds of the rect.                         */
    /*------------------------------------------------------------------------*/
    xf = x + w;
    if (xf >= map->sizeW)
    {
        xf = map->sizeW - 1;
    }

    yf = y + h;
    if (yf >= map->sizeH)
    {
        yf = map->sizeH - 1;
    }

    if (w == 0 && h == 0)
    {
        /*--------------------------------------------------------------------*/
        /* This rect is actually just a point.                                */
        /*--------------------------------------------------------------------*/
        pos = map->sizeW*y + x;
        map->mapData[pos] = maskVal;
        goto out;
    }
    else if (w == 0 && h > 0)
    {
        /*--------------------------------------------------------------------*/
        /* This rect is actually just a vertical line.                        */
        /*--------------------------------------------------------------------*/
        for (yCtr = y; yCtr <= yf; yCtr++)
        {
            pos = map->sizeW*yCtr + x;
            map->mapData[pos] = maskVal;
        }
        goto out;
    }
    else if (w > 0 && h == 0)
    {
        /*--------------------------------------------------------------------*/
        /* This rect is actually just a horizontal line.                      */
        /*--------------------------------------------------------------------*/
        for (xCtr = x; xCtr <= xf; xCtr++)
        {
            pos = map->sizeW*y + xCtr;
            map->mapData[pos] = maskVal;
        }
        goto out;
    }
    else
    {
        /*--------------------------------------------------------------------*/
        /* This is a proper rect (non-zero w & h).                            */
        /*--------------------------------------------------------------------*/
        for (yCtr = y; yCtr <= yf; yCtr++)
        {
            for (xCtr = x; xCtr <= xf; xCtr++)
            {
                pos = map->sizeW*yCtr + xCtr;
                map->mapData[pos] = maskVal;
            }
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_MaskRect
 *
 *  Description:   Flag a rect of points within the map as masked. Masked points
 *                 are used to identify portions of an image that should not be 
 *                 rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_MaskRect(
    neuik_MaskMap * map, 
    int             x,
    int             y,
    int             w,
    int             h)
{
    int           eNum  = 0;   /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_MaskRect";
    static char * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_MaskMap_SetMaskRect()`.", // [1]
    };

    if (neuik_MaskMap_SetMaskRect(map, 1, x, y, w, h))
    {
        eNum = 1;
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_UnmaskRect
 *
 *  Description:   Flag a rect of points within the map as unmasked. Masked 
 *                 pointsare used to identify portions of an image that should 
 *                 not be rendered.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_UnmaskRect(
    neuik_MaskMap * map, 
    int             x,
    int             y,
    int             w,
    int             h)
{
    int           eNum  = 0;   /* which error to report (if any) */
    static char   funcName[] = "neuik_MaskMap_UnmaskRect";
    static char * errMsgs[]  = {"", // [0] no error
        "Failure in `neuik_MaskMap_SetMaskRect()`.", // [1]
    };

    if (neuik_MaskMap_SetMaskRect(map, 0, x, y, w, h))
    {
        eNum = 1;
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_FillFromLoc
 *
 *  Description:   Fill a mask with data from another mask at a specified 
 *                 location. The location specified is the upper-left point of
 *                 the region to be copied from the source mask.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_FillFromLoc(
    neuik_MaskMap * destMap, 
    neuik_MaskMap * srcMap, 
    int             x,
    int             y)
{
    int           rPos  = 0;   /* read position of point within src mapData */
    int           wPos  = 0;   /* write position of point within dest mapData */
    int           eNum  = 0;   /* which error to report (if any) */
    int           xCtr  = 0;
    int           yCtr  = 0;
    int           xf    = 0;   /* final x-position for the rect */
    int           yf    = 0;   /* final y-position for the rect */
    static char   funcName[] = "neuik_MaskMap_FillFromLoc";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `destMap` does not implement MaskMap class.", // [1]
        "Argument `srcMap` does not implement MaskMap class.",  // [2]
        "Argument `x` invalid; value (<0) supplied.",           // [3]
        "Argument `x` invalid; exceeds srcMap bounds.",         // [4]
        "Argument `y` invalid; value (<0) supplied.",           // [5]
        "Argument `y` invalid; exceeds srcMap bounds..",        // [6]
        "x + destMapWidth; exceeds srcMap bounds.",             // [7]
        "y + destMapHeight; exceeds srcMap bounds.",            // [8]
    };

    if (!neuik_Object_IsClass(destMap, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }
    if (!neuik_Object_IsClass(srcMap, neuik__Class_MaskMap))
    {
        eNum = 2;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Check for coordinate argument input errors (invalid/out-of-bounds).    */
    /*------------------------------------------------------------------------*/
    /* Check the x,y values.                                                  */
    /*------------------------------------------------------------------------*/
    if (x < 0)
    {
        /* invalid x-value (<0) */
        eNum = 3;
        goto out;
    }
    else if (x >= srcMap->sizeW)
    {
        /* x-value beyond bounds */
        eNum = 4;
        goto out;
    }
    if (y < 0)
    {
        /* invalid y-value */
        eNum = 5;
        goto out;
    }
    else if (y >= srcMap->sizeH)
    {
        /* y-value beyond bounds */
        eNum = 6;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Make sure that the source has enough data to fill the mask from the    */
    /* specified location.                                                    */
    /*------------------------------------------------------------------------*/
    xf = x + destMap->sizeW;
    if (xf > srcMap->sizeW)
    {
        /* final x-value beyond source bounds */
        eNum = 7;
        goto out;
    }

    yf = y + destMap->sizeH;
    if (yf > srcMap->sizeH)
    {
        /* final y-value beyond source bounds */
        eNum = 8;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Copy over the data...                                                  */
    /*------------------------------------------------------------------------*/
    for (yCtr = y; yCtr < yf; yCtr++)
    {
        for (xCtr = x; xCtr < xf; xCtr++)
        {
            rPos = srcMap->sizeW*yCtr + xCtr;
            wPos = destMap->sizeW*(yCtr-y) + (xCtr-x);
            destMap->mapData[wPos] = srcMap->mapData[rPos];
        }
    }
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_GetUnmaskedRegionsOnHLine
 *
 *  Description:   Identify and return the first and final potions (along the
 *                 x-axis) of all the unmasked regions of a horizontal line. 
 *                 Argument `nRegions` captures the number of regions; 
 *                 Argument `rStart` captures all the x0 values for the regions;
 *                 Argument `rEnd` captures the xf values for the regions.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_GetUnmaskedRegionsOnHLine(
    neuik_MaskMap * map, 
    int             y,        /* y-offset corresponding to HLine of interest */
    int           * nRegions, /* captures the number of regions */
    const int    ** rStart,   /* captures all the x0 values for the regions */
    const int    ** rEnd)     /* captures the xf values for the regions */
{
    int eNum     = 0; /* which error to report (if any) */
    int ctr      = 0;
    int mapPos   = 0;
    int inRegion = FALSE;
    int regCount = 0;
    int x0       = 0;
    int xf       = 0;
    static char   funcName[] = "neuik_MaskMap_GetUnmaskedRegionsOnHLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.",          // [1]
        "Return argument `nRegions` is NULL.",                       // [2]
        "Return argument `rStart` is NULL.",                         // [3]
        "Return argument `rEnd` is NULL.",                           // [4]
        "MaskMap size not set; set with `neuik_MaskMap_SetSize()`.", // [5]
        "Argument `y` invalid; a value (<0) was supplied.",          // [6]
        "Argument `y` invalid; exceeds mask bounds.",                // [7]
        "Failure to reallocate memory.",                             // [8]
    };

    /*------------------------------------------------------------------------*/
    /* Check for potential issues before investigating further.               */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (nRegions == NULL)
    {
        eNum = 2;
        goto out;
    }

    if (rStart == NULL)
    {
        eNum = 3;
        goto out;
    }

    if (rEnd == NULL)
    {
        eNum = 4;
        goto out;
    }

    if (map->sizeW == 0 || map->sizeH == 0)
    {
        /* The size of the MaskMap appears to not be set */
        eNum = 5;
        goto out;
    }

    if (y < 0)
    {
        /* invalid y-value */
        eNum = 6;
        goto out;
    }
    if (y >= map->sizeH)
    {
        /* y-value beyond bounds */
        eNum = 7;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Determine the number of regions required (for allocation)              */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < map->sizeW; ctr++)
    {
        mapPos = y * map->sizeW + ctr;
        if (map->mapData[mapPos] == 0)
        {
            if (inRegion) continue;

            /* else: This is the start of an active region */
            x0 = ctr;
            inRegion = TRUE;
        }
        else if (inRegion)
        {
            /*----------------------------------------------------------------*/
            /* An active region stopped on the previous point.                */
            /*----------------------------------------------------------------*/
            inRegion = FALSE;
            regCount++;
        }
    }
    if (inRegion)
    {
        /*--------------------------------------------------------------------*/
        /* The current active region stopped at the end of the map.           */
        /*--------------------------------------------------------------------*/
        inRegion = FALSE;
        regCount++;
    }

    /*------------------------------------------------------------------------*/
    /* Determine if there is enough memory allocated for the region data. If  */
    /* more is needed, reallocate.                                            */
    /*------------------------------------------------------------------------*/
    if (regCount > map->nRegAlloc)
    {
        map->nRegAlloc = regCount + 20;

        map->regStart = realloc(map->regStart, map->nRegAlloc*sizeof(int));
        if (map->regStart == NULL)
        {
            eNum = 8;
            goto out;
        }
        map->regEnd = realloc(map->regEnd, map->nRegAlloc*sizeof(int));
        if (map->regEnd == NULL)
        {
            eNum = 8;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Store the region start/stops in the appropriate locations              */
    /*------------------------------------------------------------------------*/
    regCount = 0;
    for (ctr = 0; ctr < map->sizeW; ctr++)
    {
        mapPos = y * map->sizeW + ctr;
        if (map->mapData[mapPos] == 0)
        {
            if (inRegion) continue;

            /* else: This is the start of an active region */
            x0 = ctr;
            inRegion = TRUE;
        }
        else if (inRegion)
        {
            /*----------------------------------------------------------------*/
            /* An active region stopped on the previous point.                */
            /*----------------------------------------------------------------*/
            xf = ctr -1;
            inRegion = FALSE;

            map->regStart[regCount] = x0;
            map->regEnd[regCount]   = xf;
            regCount++;
        }
    }
    if (inRegion)
    {
        /*--------------------------------------------------------------------*/
        /* The current active region stopped at the end of the map.           */
        /*--------------------------------------------------------------------*/
        xf = ctr -1;

        map->regStart[regCount] = x0;
        map->regEnd[regCount]   = xf;
        regCount++;
    }

    /*------------------------------------------------------------------------*/
    /* Set the return values.                                                 */
    /*------------------------------------------------------------------------*/
    *nRegions = regCount;
    *rStart   = map->regStart;
    *rEnd     = map->regEnd;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_MaskMap_GetUnmaskedRegionsOnVLine
 *
 *  Description:   Identify and return the first and final potions (along the
 *                 y-axis) of all the unmasked regions of a vertical line. 
 *                 Argument `nRegions` captures the number of regions; 
 *                 Argument `rStart` captures all the y0 values for the regions;
 *                 Argument `rEnd` captures the yf values for the regions.
 *
 *  Returns:       A non-zero value if there was an error.
 *
 ******************************************************************************/
int neuik_MaskMap_GetUnmaskedRegionsOnVLine(
    neuik_MaskMap * map, 
    int             x,        /* x-offset corresponding to VLine of interest */
    int           * nRegions, /* captures the number of regions */
    const int    ** rStart,   /* captures all the y0 values for the regions */
    const int    ** rEnd)     /* captures the yf values for the regions */
{
    int eNum     = 0; /* which error to report (if any) */
    int ctr      = 0;
    int mapPos   = 0;
    int inRegion = FALSE;
    int regCount = 0;
    int y0       = 0;
    int yf       = 0;
    static char   funcName[] = "neuik_MaskMap_GetUnmaskedRegionsOnVLine";
    static char * errMsgs[]  = {"", // [0] no error
        "Argument `map` does not implement MaskMap class.",          // [1]
        "Return argument `nRegions` is NULL.",                       // [2]
        "Return argument `rStart` is NULL.",                         // [3]
        "Return argument `rEnd` is NULL.",                           // [4]
        "MaskMap size not set; set with `neuik_MaskMap_SetSize()`.", // [5]
        "Argument `x` invalid; a value (<0) was supplied.",          // [6]
        "Argument `x` invalid; exceeds mask bounds.",                // [7]
        "Failure to reallocate memory.",                             // [8]
    };

    /*------------------------------------------------------------------------*/
    /* Check for potential issues before investigating further.               */
    /*------------------------------------------------------------------------*/
    if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
    {
        eNum = 1;
        goto out;
    }

    if (nRegions == NULL)
    {
        eNum = 2;
        goto out;
    }

    if (rStart == NULL)
    {
        eNum = 3;
        goto out;
    }

    if (rEnd == NULL)
    {
        eNum = 4;
        goto out;
    }

    if (map->sizeW == 0 || map->sizeH == 0)
    {
        /* The size of the MaskMap appears to not be set */
        eNum = 5;
        goto out;
    }

    if (x < 0)
    {
        /* invalid x-value */
        eNum = 6;
        goto out;
    }
    if (x >= map->sizeW)
    {
        /* x-value beyond bounds */
        eNum = 7;
        goto out;
    }

    /*------------------------------------------------------------------------*/
    /* Determine the number of regions required (for allocation)              */
    /*------------------------------------------------------------------------*/
    for (ctr = 0; ctr < map->sizeH; ctr++)
    {
        mapPos = ctr * map->sizeW + x;
        if (map->mapData[mapPos] == 0)
        {
            if (inRegion) continue;

            /* else: This is the start of an active region */
            y0 = ctr;
            inRegion = TRUE;
        }
        else if (inRegion)
        {
            /*----------------------------------------------------------------*/
            /* An active region stopped on the previous point.                */
            /*----------------------------------------------------------------*/
            inRegion = FALSE;
            regCount++;
        }
    }
    if (inRegion)
    {
        /*--------------------------------------------------------------------*/
        /* The current active region stopped at the end of the map.           */
        /*--------------------------------------------------------------------*/
        inRegion = FALSE;
        regCount++;
    }

    /*------------------------------------------------------------------------*/
    /* Determine if there is enough memory allocated for the region data. If  */
    /* more is needed, reallocate.                                            */
    /*------------------------------------------------------------------------*/
    if (regCount > map->nRegAlloc)
    {
        map->nRegAlloc = regCount + 20;

        map->regStart = realloc(map->regStart, map->nRegAlloc*sizeof(int));
        if (map->regStart == NULL)
        {
            eNum = 8;
            goto out;
        }
        map->regEnd = realloc(map->regEnd, map->nRegAlloc*sizeof(int));
        if (map->regEnd == NULL)
        {
            eNum = 8;
            goto out;
        }
    }

    /*------------------------------------------------------------------------*/
    /* Store the region start/stops in the appropriate locations              */
    /*------------------------------------------------------------------------*/
    regCount = 0;
    for (ctr = 0; ctr < map->sizeH; ctr++)
    {
        mapPos = ctr * map->sizeW + x;
        if (map->mapData[mapPos] == 0)
        {
            if (inRegion) continue;

            /* else: This is the start of an active region */
            y0 = ctr;
            inRegion = TRUE;
        }
        else if (inRegion)
        {
            /*----------------------------------------------------------------*/
            /* An active region stopped on the previous point.                */
            /*----------------------------------------------------------------*/
            yf = ctr -1;
            inRegion = FALSE;

            map->regStart[regCount] = y0;
            map->regEnd[regCount]   = yf;
            regCount++;
        }
    }
    if (inRegion)
    {
        /*--------------------------------------------------------------------*/
        /* The current active region stopped at the end of the map.           */
        /*--------------------------------------------------------------------*/
        yf = ctr -1;

        map->regStart[regCount] = y0;
        map->regEnd[regCount]   = yf;
        regCount++;
    }

    /*------------------------------------------------------------------------*/
    /* Set the return values.                                                 */
    /*------------------------------------------------------------------------*/
    *nRegions = regCount;
    *rStart   = map->regStart;
    *rEnd     = map->regEnd;
out:
    if (eNum > 0)
    {
        NEUIK_RaiseError(funcName, errMsgs[eNum]);
    }

    return eNum;
}

