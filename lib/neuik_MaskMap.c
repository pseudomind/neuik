/*******************************************************************************
 * Copyright (c) 2014-2019, Michael Leimon <leimon@gmail.com>
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
	int             ctr        = 0; /* counter (used for loops) */
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
	for (ctr = 0; ctr < aSize; ctr++)
	{
		map->mapData[ctr] = 0;
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
	int           ctr   = 0; /* counter (used for loops) */
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
	for (ctr = 0; ctr < aSize; ctr++)
	{
		map->mapData[ctr] = 0;
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
	static char   funcName[] = "neuik_MaskMap_MaskLine";
	static char * errMsgs[]  = {"", // [0] no error
		"Argument `map` does not implement MaskMap class.", // [1]
		"Argument `x1` invalid; value (<0) supplied.",      // [2]
		"Argument `y1` invalid; value (<0) supplied.",      // [3]
		"Argument `x1` invalid; exceeds mask bounds.",      // [4]
		"Argument `y1` invalid; exceeds mask bounds..",     // [5]
		"Argument `x2` invalid; value (<0) supplied.",      // [6]
		"Argument `y2` invalid; value (<0) supplied.",      // [7]
		"Argument `x2` invalid; exceeds mask bounds.",      // [8]
		"Argument `y2` invalid; exceeds mask bounds..",     // [9]
	};

	if (!neuik_Object_IsClass(map, neuik__Class_MaskMap))
	{
		eNum = 1;
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
	map->mapData[pos] = 1;

	pos = map->sizeW*y2 + x2;
	map->mapData[pos] = 1;

	/*------------------------------------------------------------------------*/
	/* Mark the rest of the points on the line.                               */
	/*------------------------------------------------------------------------*/
	for (fCtr = 1.0; fCtr < hyp; fCtr += 1.0)
	{
		pos = pos0 + map->sizeW*(int)(fCtr*dyInt) + (int)(fCtr*dxInt);
		map->mapData[pos] = 1;
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
	int inRegion = 0;
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
		"Argument `y` invalid; exceeds mask bounds..",               // [7]
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
			inRegion = 1;
		}
		else if (inRegion)
		{
			/*----------------------------------------------------------------*/
			/* An active region stopped on the previous point.                */
			/*----------------------------------------------------------------*/
			xf = ctr -1;
			inRegion = 0;
			regCount++;
		}
	}
	if (inRegion)
	{
		/*--------------------------------------------------------------------*/
		/* The current active region stopped at the end of the map.           */
		/*--------------------------------------------------------------------*/
		xf = ctr -1;
		inRegion = 0;
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
			inRegion = 1;
		}
		else if (inRegion)
		{
			/*----------------------------------------------------------------*/
			/* An active region stopped on the previous point.                */
			/*----------------------------------------------------------------*/
			xf = ctr -1;
			inRegion = 0;

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
		inRegion = 0;

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
