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
#include <SDL.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

#include "NEUIK_error.h"
#include "NEUIK_render.h"
#include "NEUIK_structs_basic.h"
#include "NEUIK_colors.h"
#include "NEUIK_GridLayout.h"
#include "NEUIK_Element_internal.h"
#include "NEUIK_Container.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__GridLayout(void **);
int neuik_Object_Free__GridLayout(void *);

neuik_EventState neuik_Element_CaptureEvent__GridLayout(
	NEUIK_Element, SDL_Event*);
int neuik_Element_GetMinSize__GridLayout(NEUIK_Element, RenderSize*);
int neuik_Element_Render__GridLayout(
	NEUIK_Element, RenderSize*, RenderLoc*, SDL_Renderer*, SDL_Surface*, int);
int neuik_Element_SetWindowPointer__GridLayout(NEUIK_Element, void*);
int neuik_Element_IsShown__GridLayout(NEUIK_Element);

/*----------------------------------------------------------------------------*/
/* neuik_Element    Function Table                                            */
/*----------------------------------------------------------------------------*/
NEUIK_Element_FuncTable neuik_GridLayout_FuncTable = {
	/* GetMinSize(): Get the minimum required size for the element  */
	neuik_Element_GetMinSize__GridLayout,

	/* Render(): Redraw the element */
	neuik_Element_Render__GridLayout,

	/* CaptureEvent(): Determine if this element caputures a given event */
	neuik_Element_CaptureEvent__GridLayout,

	/* Defocus(): This function will be called when an element looses focus */
	NULL,
};


/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_GridLayout_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__GridLayout,
	/* Copy(): Copy the contents of one object into another */
	NULL,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__GridLayout,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_GridLayout
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_GridLayout()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_GridLayout";
	static char  * errMsgs[]  = {"",                                       // [0] no error
		"NEUIK library must be initialized first.",                        // [1]
		"Failed to register `GridLayout` object class .",                  // [2]
		"Failed to register `Element_IsShown` virtual function.",          // [3]
		"Failed to register `Element_SetWindowPointer` virtual function.", // [4]
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
		"NEUIK_GridLayout",                                                     // className
		"An element container which aligns items vertically and horizontally.", // classDescription
		neuik__Set_NEUIK,                                                       // classSet
		neuik__Class_Container,                                                 // superClass
		&neuik_GridLayout_BaseFuncs,                                            // baseFuncs
		NULL,                                                                   // classFuncs
		&neuik__Class_GridLayout))                                              // newClass
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Register virtual function implementations                              */
	/*------------------------------------------------------------------------*/
	if (neuik_VirtualFunc_RegisterImplementation(
		&neuik_Element_vfunc_IsShown,
		neuik__Class_GridLayout,
		neuik_Element_IsShown__GridLayout))
	{
		eNum = 3;
		goto out;
	}

	if (neuik_VirtualFunc_RegisterImplementation(
		&neuik_Element_vfunc_SetWindowPointer,
		neuik__Class_GridLayout,
		neuik_Element_SetWindowPointer__GridLayout))
	{
		eNum = 4;
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
 *  Name:          neuik_Object_New__GridLayout
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__GridLayout(
	void ** gridPtr)
{
	int                eNum       = 0;
	NEUIK_Container  * cont       = NULL;
	NEUIK_GridLayout * grid       = NULL;
	NEUIK_Element    * sClassPtr  = NULL;
	static char        funcName[] = "neuik_Object_New__GridLayout";
	static char      * errMsgs[]  = {"",                                    // [0] no error
		"Output Argument `gridPtr` is NULL.",                               // [1]
		"Failure to allocate memory.",                                      // [2]
		"Failure in `neuik_GetObjectBaseOfClass`.",                         // [3]
		"Failure in function `neuik.NewElement`.",                          // [4]
		"Failure in function `neuik_Element_SetFuncTable`.",                // [5]
		"Argument `gridPtr` caused `neuik_Object_GetClassObject` to fail.", // [6]
		"Failure in `NEUIK_Element_SetBackgroundColorTransparent`.",        // [7]
	};

	if (gridPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	(*gridPtr) = (NEUIK_GridLayout*) malloc(sizeof(NEUIK_GridLayout));
	grid = *gridPtr;
	if (grid == NULL)
	{
		eNum = 2;
		goto out;
	}

	/* Allocation successful */
	grid->HSpacing    = 1;
	grid->VSpacing    = 1;
	grid->isActive    = 0;
	grid->xDim        = 0;
	grid->yDim        = 0;
	grid->squareElems = 0;

	/*------------------------------------------------------------------------*/
	/* Successful allocation of Memory -- Create Base Class Object            */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(
			neuik__Set_NEUIK, 
			neuik__Class_GridLayout, 
			NULL,
			&(grid->objBase)))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Create first level Base SuperClass Object                              */
	/*------------------------------------------------------------------------*/
	sClassPtr = (NEUIK_Element *) &(grid->objBase.superClassObj);
	if (neuik_Object_New(neuik__Class_Container, sClassPtr))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Element_SetFuncTable(grid, &neuik_GridLayout_FuncTable))
	{
		eNum = 5;
		goto out;
	}

	if (neuik_Object_GetClassObject(grid, neuik__Class_Container, (void**)&cont))
	{
		eNum = 6;
		goto out;
	}
	cont->cType        = NEUIK_CONTAINER_NO_DEFAULT_ADD_SET;
	cont->shownIfEmpty = 0;

	/*------------------------------------------------------------------------*/
	/* Set the default element background redraw styles.                      */
	/*------------------------------------------------------------------------*/
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "normal"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "selected"))
	{
		eNum = 7;
		goto out;
	}
	if (NEUIK_Element_SetBackgroundColorTransparent(cont, "hovered"))
	{
		eNum = 7;
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
 *  Name:          NEUIK_NewGridLayout
 *
 *  Description:   Create and return a pointer to a new NEUIK_GridLayout.
 *
 *                 Wrapper function to neuik_Object_New__GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewGridLayout(
	NEUIK_GridLayout ** gridPtr)
{
	return neuik_Object_New__GridLayout((void**)gridPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_MakeGridLayout
 *
 *  Description:   Create a new NEUIK_GridLayout with the specified dimensions.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_MakeGridLayout(
	NEUIK_GridLayout ** gridPtr,
	unsigned int        xDim,
	unsigned int        yDim)
{
	int           eNum       = 0;
	static char   funcName[] = "NEUIK_MakeGridLayout";
	static char * errMsgs[]  = {"",                              // [0] no error
		"Failure in function `NEUIK_NewGridLayout`.",            // [1]
		"Failure in function `NEUIK_GridLayout_SetDimensions`.", // [2]
	};

	if (NEUIK_NewGridLayout(gridPtr))
	{
		eNum = 1;
		goto out;
	}
	if (NEUIK_GridLayout_SetDimensions(*gridPtr, xDim, yDim))
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
 *  Name:          NEUIK_GridLayout_SetDimensions
 *
 *  Description:   Set the outer (x/y) dimensions of a NEUIK_GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_SetDimensions(
	NEUIK_GridLayout * grid,
	unsigned int       xDim,
	unsigned int       yDim)
{
	int               eNum       = 0;
	int               ctr        = 0;
	int               finalInd   = 0;
	NEUIK_Element     elem       = NULL;
	NEUIK_Container * cBase      = NULL;
	static char       funcName[] = "NEUIK_GridLayout_SetDimensions";
	static char     * errMsgs[]  = {"",                                   // [0] no error
		"Argument `grid` caused `neuik_Object_GetClassObject` to fail.",  // [1]
		"Failure in function `neuik_Object_Free`.",                       // [2]
		"Failure to allocate memory.",                                    // [3]
		"Failure to reallocate memory.",                                  // [4]
	};

	if (neuik_Object_GetClassObject(grid, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 1;
		goto out;
	}

	finalInd = grid->xDim * grid->yDim;

	/*------------------------------------------------------------------------*/
	/* Conditionally free memory of contained elements before continuing.     */
	/*------------------------------------------------------------------------*/
	if (cBase->elems != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Free any old allocated elements before updating GridLayout dims.   */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < finalInd; ctr++)
		{
			/*----------------------------------------------------------------*/
			/* A GridLayout is different from other containers in that NULL   */
			/* values are permitted within the elems array. The total number  */
			/* of contained elems is defined as xDim * yDim.                  */
			/*----------------------------------------------------------------*/
			elem = cBase->elems[ctr];
			if (elem == NULL) continue;

			if(neuik_Object_Free(elem))
			{
				eNum = 2;
				goto out;
			}

			cBase->elems[ctr] = NULL;
		}
	}

	finalInd = xDim * yDim;

	/*------------------------------------------------------------------------*/
	/* Allocate/Reallocate the Container->elems array.                        */
	/*------------------------------------------------------------------------*/
	if (cBase->elems == NULL)
	{
		/*--------------------------------------------------------------------*/
		/* elems array currently unallocated; allocate now                    */
		/*--------------------------------------------------------------------*/
		cBase->elems = (NEUIK_Element*)malloc((finalInd+1)*sizeof(NEUIK_Element));
		if (cBase->elems == NULL)
		{
			eNum = 3;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Set all of the contained pointer values to NULL before continuing. */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < finalInd; ctr++)
		{
			cBase->elems[ctr] = NULL;
		}

		cBase->n_allocated = finalInd;
		cBase->n_used      = 0;
	}
	else if (cBase->n_allocated < finalInd)
	{
		/*--------------------------------------------------------------------*/
		/* The new size is larger than the old, reallocate memory.            */
		/* This pointer array will be null terminated.                        */
		/*--------------------------------------------------------------------*/
		cBase->elems = (NEUIK_Element*)realloc(cBase->elems, 
			(finalInd+1)*sizeof(NEUIK_Element));
		if (cBase->elems == NULL)
		{
			eNum = 4;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Set all of the contained pointer values to NULL before continuing. */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < finalInd; ctr++)
		{
			cBase->elems[ctr] = NULL;
		}

		cBase->n_allocated = finalInd;
		cBase->n_used      = 0;
	}

	/*------------------------------------------------------------------------*/
	/* Store the new overall GridLayout dimensions.                           */
	/*------------------------------------------------------------------------*/
	grid->xDim = xDim;
	grid->yDim = yDim;
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
 *  Name:          neuik_GridLayout_Free
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_Free__GridLayout(
	void  * gridPtr)
{
	int                eNum       = 0;    /* which error to report (if any) */
	NEUIK_GridLayout * grid       = NULL;
	static char        funcName[] = "neuik_Object_Free__GridLayout";
	static char      * errMsgs[]  = {"",                 // [0] no error
		"Argument `gridPtr` is NULL.",                   // [1]
		"Argument `gridPtr` is not of Container class.", // [2]
		"Failure in function `neuik_Object_Free`.",      // [3]
	};

	if (gridPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(gridPtr, neuik__Class_GridLayout))
	{
		eNum = 2;
		goto out;
	}
	grid = (NEUIK_GridLayout*)gridPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if(neuik_Object_Free(grid->objBase.superClassObj))
	{
		eNum = 3;
		goto out;
	}

	free(grid);
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
 *  Name:          NEUIK_GridLayout_SetHSpacing
 *
 *  Description:   Set the horizontal spacing parameter within a GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_SetHSpacing(
	NEUIK_GridLayout * grid,
	int                spacing)
{
	RenderSize     rSize;
	RenderLoc      rLoc;
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_GridLayout_SetHSpacing";
	static char  * errMsgs[]  = {"",                        // [0] no error
		"Argument `grid` is not of GridLayout class.",      // [1]
		"Argument `spacing` can not be negative.",          // [2]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (spacing < 0)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no effective change in spacing; don't do anything          */
	/*------------------------------------------------------------------------*/
	if (spacing == grid->HSpacing) goto out;

	grid->HSpacing = spacing;
	if (neuik_Element_GetSizeAndLocation(grid, &rSize, &rLoc))
	{
		eNum = 3;
		goto out;
	}
	neuik_Element_RequestRedraw(grid, rLoc, rSize);
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
 *  Name:          NEUIK_GridLayout_SetVSpacing
 *
 *  Description:   Set the vertical spacing parameter within a GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_SetVSpacing(
	NEUIK_GridLayout * grid,
	int                spacing)
{
	RenderSize     rSize;
	RenderLoc      rLoc;
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_GridLayout_SetVSpacing";
	static char  * errMsgs[]  = {"",                        // [0] no error
		"Argument `grid` is not of GridLayout class.",      // [1]
		"Argument `spacing` can not be negative.",          // [2]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (spacing < 0)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no effective change in spacing; don't do anything          */
	/*------------------------------------------------------------------------*/
	if (spacing == grid->VSpacing) goto out;

	grid->VSpacing = spacing;
	if (neuik_Element_GetSizeAndLocation(grid, &rSize, &rLoc))
	{
		eNum = 3;
		goto out;
	}
	neuik_Element_RequestRedraw(grid, rLoc, rSize);
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
 *  Name:          NEUIK_GridLayout_SetSpacing
 *
 *  Description:   Set the horizontal/vertical spacing parameters within a 
 *                 GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_SetSpacing(
	NEUIK_GridLayout * grid,
	int                spacing)
{
	RenderSize     rSize;
	RenderLoc      rLoc;
	int            eNum       = 0;    /* which error to report (if any) */
	static char    funcName[] = "NEUIK_GridLayout_SetSpacing";
	static char  * errMsgs[]  = {"",                        // [0] no error
		"Argument `grid` is not of GridLayout class.",      // [1]
		"Argument `spacing` can not be negative.",          // [2]
		"Failure in `neuik_Element_GetSizeAndLocation()`.", // [3]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (spacing < 0)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* if there is no effective change in spacing; don't do anything          */
	/*------------------------------------------------------------------------*/
	if (spacing == grid->HSpacing && spacing == grid->VSpacing) goto out;

	grid->HSpacing = spacing;
	grid->VSpacing = spacing;
	if (neuik_Element_GetSizeAndLocation(grid, &rSize, &rLoc))
	{
		eNum = 3;
		goto out;
	}
	neuik_Element_RequestRedraw(grid, rLoc, rSize);
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
 *  Name:          NEUIK_GridLayout_GetElementAt
 *
 *  Description:   Return a pointer to the element stored within an x/y location
 *                 of a GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_GetElementAt(
	NEUIK_GridLayout * grid,
	unsigned int       xLoc,
	unsigned int       yLoc,
	NEUIK_Element    * elem)
{
	int                 eNum       = 0;    /* which error to report (if any) */
	int                 offset     = 0;
	NEUIK_Container   * cBase      = NULL;
	static char         funcName[] = "NEUIK_GridLayout_GetElementAt";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `grid` is not of GridLayout class.",                   // [1]
		"Argument `grid` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Output Argument `elem` is NULL.",                               // [3]
		"Argument `xLoc` is beyond specified `xDim` of GridLayout.",     // [4]
		"Argument `yLoc` is beyond specified `yDim` of GridLayout.",     // [5]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(grid, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 2;
		goto out;
	}
	if (elem == NULL)
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check that specified location is within GridLayout bounds.             */
	/*------------------------------------------------------------------------*/
	if (xLoc >= grid->xDim)
	{
		eNum = 4;
		goto out;
	}
	if (yLoc >= grid->yDim)
	{
		eNum = 5;
		goto out;
	}

	offset = xLoc + yLoc*(grid->xDim);
	*elem = cBase->elems[offset];
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
 *  Name:          NEUIK_GridLayout_GetElementPos
 *
 *  Description:   Return the x/y location corresponding to where the element 
 *                 is stored within a GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int	NEUIK_GridLayout_GetElementPos(
	NEUIK_GridLayout * grid,
	NEUIK_Element      elem,
	int              * hasElem,
	int              * xLoc,
	int              * yLoc)
{
	int                 eNum       = 0;    /* which error to report (if any) */
	int                 ctr        = 0;
	int                 nAlloc     = 0;
	NEUIK_Container   * cBase      = NULL;
	static char         funcName[] = "NEUIK_GridLayout_GetElementPos";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `grid` is not of GridLayout class.",                   // [1]
		"Argument `grid` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Argument `elem` is NULL.",                                      // [3]
		"Output Argument `hasElem` is NULL.",                            // [4]
		"Output Argument `xLoc` is NULL.",                               // [5]
		"Output Argument `yLoc` is NULL.",                               // [6]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(grid, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 2;
		goto out;
	}
	if (elem == NULL)
	{
		eNum = 3;
		goto out;
	}
	if (hasElem == NULL)
	{
		eNum = 4;
		goto out;
	}
	if (xLoc == NULL)
	{
		eNum = 5;
		goto out;
	}
	if (yLoc == NULL)
	{
		eNum = 6;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set a default set of values for unlocated elements.                    */
	/*------------------------------------------------------------------------*/
	*hasElem = 0;
	*xLoc    = 0;
	*yLoc    = 0;

	nAlloc = grid->xDim*grid->yDim;

	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		if (cBase->elems[ctr] == elem)
		{
			*hasElem = 1;
			*xLoc = ctr % grid->xDim;
			*yLoc = (ctr - (*xLoc)) / grid->xDim;
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
 *  Name:          NEUIK_GridLayout_SetElementAt
 *
 *  Description:   Set the element stored within an x/y location of a 
 *                 GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_GridLayout_SetElementAt(
	NEUIK_GridLayout * grid,
	unsigned int       xLoc,
	unsigned int       yLoc,
	NEUIK_Element      elem)
{
	int                 eNum       = 0;    /* which error to report (if any) */
	int                 offset     = 0;
	NEUIK_ElementBase * eBase      = NULL;
	NEUIK_Container   * cBase      = NULL;
	static char         funcName[] = "NEUIK_GridLayout_SetElementAt";
	static char       * errMsgs[]  = {"",                                // [0] no error
		"Argument `grid` is not of GridLayout class.",                   // [1]
		"Argument `grid` caused `neuik_Object_GetClassObject` to fail.", // [2]
		"Argument `elem` does not implement Element class.",             // [3]
		"Argument `xLoc` is beyond specified `xDim` of GridLayout.",     // [4]
		"Argument `yLoc` is beyond specified `yDim` of GridLayout.",     // [5]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	if (neuik_Object_GetClassObject(grid, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 2;
		goto out;
	}
	if (!neuik_Object_ImplementsClass(elem, neuik__Class_Element))
	{
		eNum = 3;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Check that specified location is within GridLayout bounds.             */
	/*------------------------------------------------------------------------*/
	if (xLoc >= grid->xDim)
	{
		eNum = 4;
		goto out;
	}
	if (yLoc >= grid->yDim)
	{
		eNum = 5;
		goto out;
	}

	offset = xLoc + yLoc*(grid->xDim);
	cBase->elems[offset] = elem;

	/*------------------------------------------------------------------------*/
	/* Set the Window and Parent Element pointers                             */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(grid, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 2;
		goto out;
	}
	if (eBase->eSt.window != NULL)
	{
		neuik_Element_SetWindowPointer(elem, eBase->eSt.window);
	}
	neuik_Element_SetParentPointer(elem, grid);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}


void neuik_GridLayout_Configure_capture_segv(
	int sig_num)
{
	static char funcName[] = "NEUIK_GridLayout_Configure";
	static char errMsg[] = 
		"SIGSEGV (segmentation fault) captured; is call `NULL` terminated?";

	NEUIK_RaiseError(funcName, errMsg);
	NEUIK_BacktraceErrors();
	exit(1);
}

/*******************************************************************************
 *
 *  Name:          NEUIK_GridLayout_Configure
 *
 *  Description:   Allows the user to set a number of configurable parameters.
 *
 *                 NOTE: This list of named sets must be terminated by a NULL 
 *                 pointer
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_GridLayout_Configure(
	NEUIK_GridLayout * grid,
	const char       * set0,
	...)
{
	int          ctr;
	int          nCtr;
	int          eNum      = 0; /* which error to report (if any) */
	int          doRedraw  = 0;
	int          isBool;
	int          boolVal   = 0;
	int          typeMixup;
	RenderSize   rSize     = {0, 0};
	RenderLoc    rLoc      = {0, 0};;
	char         buf[4096];
	va_list      args;
	char       * strPtr    = NULL;
	char       * name      = NULL;
	const char * set       = NULL;
	/*------------------------------------------------------------------------*/
	/* If a `name=value` string with an unsupported name is found, check to   */
	/* see if a boolName was mistakenly used instead.                         */
	/*------------------------------------------------------------------------*/
	static char * boolNames[] = {
		"SquareElems",
		NULL,
	};
	/*------------------------------------------------------------------------*/
	/* If a boolName string with an unsupported name is found, check to see   */
	/* if a supported nameValue type was mistakenly used instead.             */
	/*------------------------------------------------------------------------*/
	static char * valueNames[] = {
		NULL,
	};
	static char   funcName[] = "NEUIK_GridLayout_Configure";
	static char * errMsgs[] = {"",                              // [0] no error
		"Argument `grid` does not implement GridLayout class.", // [1]
		"`name=value` string is too long.",                     // [2]
		"Invalid `name=value` string.",                         // [3]
		"ValueType name used as BoolType, skipping.",           // [4]
		"BoolType name unknown, skipping.",                     // [5]
		"NamedSet.name is NULL, skipping..",                    // [6]
		"NamedSet.name is blank, skipping..",                   // [7]
		"BoolType name used as ValueType, skipping.",           // [8]
		"NamedSet.name type unknown, skipping.",                // [9]
		"Failure in `neuik_Element_GetSizeAndLocation()`.",     // [10]
	};

	if (!neuik_Object_IsClass(grid, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	set = set0;

	va_start(args, set0);

	for (ctr = 0;; ctr++)
	{
		isBool = 0;
		name   = NULL;

		if (set == NULL) break;

		#ifndef NO_NEUIK_SIGNAL_TRAPPING
			signal(SIGSEGV, neuik_GridLayout_Configure_capture_segv);
		#endif

		if (strlen(set) > 4095)
		{
			#ifndef NO_NEUIK_SIGNAL_TRAPPING
				signal(SIGSEGV, NULL);
			#endif
			NEUIK_RaiseError(funcName, errMsgs[2]);
			set = va_arg(args, const char *);
			continue;
		}
		else
		{
			#ifndef NO_NEUIK_SIGNAL_TRAPPING
				signal(SIGSEGV, NULL);
			#endif
			strcpy(buf, set);
			/* Find the equals and set it to '\0' */
			strPtr = strchr(buf, '=');
			if (strPtr == NULL)
			{
				/*------------------------------------------------------------*/
				/* Bool type configuration (or a mistake)                     */
				/*------------------------------------------------------------*/
				if (buf[0] == 0)
				{
					NEUIK_RaiseError(funcName, errMsgs[3]);
				}

				isBool  = 1;
				boolVal = 1;
				name    = buf;
				if (buf[0] == '!')
				{
					boolVal = 0;
					name    = buf + 1;
				}
			}
			else
			{
				*strPtr = 0;
				strPtr++;
				if (*strPtr == 0)
				{
					/* `name=value` string is missing a value */
					NEUIK_RaiseError(funcName, errMsgs[3]);
					set = va_arg(args, const char *);
					continue;
				}
				name  = buf;
			}
		}

		if (isBool)
		{
			/*----------------------------------------------------------------*/
			/* Check for boolean parameter setting.                           */
			/*----------------------------------------------------------------*/
			if (!strcmp("SquareElems", name))
			{
				if (grid->squareElems == boolVal) continue;

				/* else: The previous setting was changed */
				grid->squareElems = boolVal;
				doRedraw = 1;
			}
			else 
			{
				/*------------------------------------------------------------*/
				/* Bool parameter not found; may be mixup or mistake .        */
				/*------------------------------------------------------------*/
				typeMixup = 0;
				for (nCtr = 0;; nCtr++)
				{
					if (valueNames[nCtr] == NULL) break;

					if (!strcmp(valueNames[nCtr], name))
					{
						typeMixup = 1;
						break;
					}
				}

				if (typeMixup)
				{
					/* A value type was mistakenly used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[4]);
				}
				else
				{
					/* An unsupported name was used as a bool type */
					NEUIK_RaiseError(funcName, errMsgs[5]);
				}
			}
		}
		else
		{
			if (name == NULL)
			{
				NEUIK_RaiseError(funcName, errMsgs[6]);
			}
			else if (name[0] == 0)
			{
				NEUIK_RaiseError(funcName, errMsgs[7]);
			}
			else
			{
				typeMixup = 0;
				for (nCtr = 0;; nCtr++)
				{
					if (boolNames[nCtr] == NULL) break;

					if (!strcmp(boolNames[nCtr], name))
					{
						typeMixup = 1;
						break;
					}
				}

				if (typeMixup)
				{
					/* A bool type was mistakenly used as a value type */
					NEUIK_RaiseError(funcName, errMsgs[8]);
				}
				else
				{
					/* An unsupported name was used as a value type */
					NEUIK_RaiseError(funcName, errMsgs[9]);
				}
			}
		}

		/* before starting */
		set = va_arg(args, const char *);
	}
	va_end(args);
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}
	if (doRedraw) {
		if (neuik_Element_GetSizeAndLocation(grid, &rSize, &rLoc))
		{
			eNum = 10;
			goto out;
		}
		neuik_Element_RequestRedraw(grid, rLoc, rSize);
	}

	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_IsShown__GridLayout    (redefined-vfunc)
 *
 *  Description:   This function reports whether or not an element is currently
 *                 being shown.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if element is shown, 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_IsShown__GridLayout(
	NEUIK_Element  gridElem)
{
	int                 isShown  = 0;
	int                 ctr      = 0;
	int                 nAlloc   = 0;
	NEUIK_Element       elem;
	NEUIK_ElementBase * eBase;
	NEUIK_Container   * cBase = NULL;
	NEUIK_GridLayout  * grid  = NULL;
	static int          nRecurse = 0; /* number of times recursively called */

	nRecurse++;
	if (nRecurse > NEUIK_MAX_RECURSION)
	{
		/*--------------------------------------------------------------------*/
		/* This is likely a case of appears to be runaway recursion; report   */
		/* an error to the user.                                              */
		/*--------------------------------------------------------------------*/
		neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
		goto out;
	}

	if (!neuik_Object_IsClass(gridElem, neuik__Class_GridLayout))
	{
		goto out;
	}
	grid = (NEUIK_GridLayout *)(gridElem);

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Container, (void**)&cBase))
	{
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* First check if this element is being shown.                            */
	/*------------------------------------------------------------------------*/
	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Element, (void**)&eBase))
	{
		goto out;
	}

	if (!eBase->eCfg.Show) goto out;

	/*------------------------------------------------------------------------*/
	/* Check if the GridLayout has valid dimensions first...                  */
	/*------------------------------------------------------------------------*/
	nAlloc = grid->xDim*grid->yDim;
	if (nAlloc == 0)
	{
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Examine the contained elements to see if any of them are being shown.  */
	/*------------------------------------------------------------------------*/
	if (cBase->elems == NULL) goto out;

	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		elem = cBase->elems[ctr];
		if (elem == NULL) continue;
		
		if (NEUIK_Element_IsShown(elem))
		{
			if (neuik_HasFatalError())
			{
				goto out;
			}
			isShown = 1;
			break;
		}
		if (neuik_HasFatalError())
		{
			goto out;
		}
	}
out:
	nRecurse--;
	return isShown;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_GetMinSize__GridLayout
 *
 *  Description:   Returns the rendered size of a given GridLayout.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Element_GetMinSize__GridLayout(
	NEUIK_Element   gridElem,
	RenderSize    * rSize)
{
	// static int             nCalled    = 0;
	int                    eNum       = 0; /* which error to report (if any) */
	int                    tempH;
	int                    tempW;
	int                    nAlloc     = 0;
	int                    rowCtr     = 0;
	int                    colCtr     = 0;
	int                    offset     = 0;
	int                    ctr        = 0;
	int                    maxMin     = 0;    // Max Min (of widths and heights)
	RenderSize           * rs         = NULL;
	NEUIK_Element          elem       = NULL;
	NEUIK_ElementBase    * eBase      = NULL;
	NEUIK_ElementConfig  * eCfg       = NULL;
	int                  * allMaxMinH = NULL; // Free upon returning; 
	                                          // The max min width (per row)
	int                  * allMaxMinW = NULL; // Free upon returning; 
	                                          // The max min width (per column)
	int                  * elemsValid = NULL; // Free upon returning.
	int                  * elemsShown = NULL; // Free upon returning.
	RenderSize           * elemsMinSz = NULL; // Free upon returning.
	NEUIK_ElementConfig ** elemsCfg   = NULL; // Free upon returning.

	static RenderSize     rsZero      = {0, 0};
	NEUIK_Container      * cont       = NULL;
	NEUIK_GridLayout     * grid       = NULL;
	static char            funcName[] = "neuik_Element_GetMinSize__GridLayout";
	static char          * errMsgs[]  = {"",                                 // [0] no error
		"Argument `gridElem` is not of GridLayout class.",                   // [1]
		"Element_GetMinSize Failed.",                                        // [2]
		"Element_GetConfig returned NULL.",                                  // [3]
		"Argument `gridElem` caused `neuik_Object_GetClassObject` to fail.", // [4]
		"Failure to allocate memory.",                                       // [5]
	};

	rSize->w = 0;
	rSize->h = 0;

	/*------------------------------------------------------------------------*/
	/* Check for problems before proceeding                                   */
	/*------------------------------------------------------------------------*/
	if (!neuik_Object_IsClass(gridElem, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	grid = (NEUIK_GridLayout*)gridElem;

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 4;
		goto out;
	}
	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 4;
		goto out;
	}

	if (cont->elems == NULL) {
		/* there are no UI elements contained by this GridLayout */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for the calculated maximum minimum values.             */
	/*------------------------------------------------------------------------*/
	allMaxMinW = malloc(grid->xDim*sizeof(int));
	if (allMaxMinW == NULL)
	{
		eNum = 5;
		goto out;
	}
	allMaxMinH = malloc(grid->yDim*sizeof(int));
	if (allMaxMinH == NULL)
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Zero out the initial maximum minimum values.                           */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->xDim; ctr++)
	{
		allMaxMinW[ctr] = 0;
	}
	for (ctr = 0; ctr < grid->yDim; ctr++)
	{
		allMaxMinH[ctr] = 0;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for lists of contained element properties.             */
	/*------------------------------------------------------------------------*/
	nAlloc = grid->xDim*grid->yDim;
	elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
	if (elemsCfg == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsValid = malloc(nAlloc*sizeof(int));
	if (elemsValid == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsShown = malloc(nAlloc*sizeof(int));
	if (elemsShown == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
	if (elemsMinSz == NULL)
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Store the current properties for the contained elements.               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		/*--------------------------------------------------------------------*/
		/* A GridLayout is different from other containers in that NULL       */
		/* values are permitted within the elems array. The total number of   */
		/* contained elems is defined as xDim * yDim.                         */
		/*--------------------------------------------------------------------*/
		elemsValid[ctr] = 0;
		elem = cont->elems[ctr];
		if (elem == NULL) continue;
		elemsValid[ctr] = 1;

		elemsShown[ctr] = NEUIK_Element_IsShown(elem);
		if (!elemsShown[ctr]) continue;

		elemsCfg[ctr] = neuik_Element_GetConfig(elem);
		if (elemsCfg[ctr] == NULL)
		{
			eNum = 3;
			goto out;
		}

		elemsMinSz[ctr] = rsZero;
		if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
		{
			eNum = 2;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the maximum minimum heights required for all of the rows.    */
	/* (i.e., for each row of elements, determine the maximum value of the    */
	/* minimum heights required (among elements in the row)).                 */
	/*------------------------------------------------------------------------*/
	for (rowCtr = 0; rowCtr < grid->yDim; rowCtr++)
	{
		for (colCtr = 0; colCtr < grid->xDim; colCtr++)
		{
			offset = colCtr + rowCtr*(grid->xDim);

			if (!elemsValid[offset]) continue; /* no elem at this location */
			if (!elemsShown[offset]) continue; /* this elem isn't shown */

			eCfg = elemsCfg[offset];
			rs   = &elemsMinSz[offset];

			tempH = rs->h + (eCfg->PadTop + eCfg->PadBottom);
			if (tempH > allMaxMinH[rowCtr])
			{
				allMaxMinH[rowCtr] = tempH;
			}
			if (tempH > maxMin)
			{
				maxMin = tempH;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the maximum minimum widths required for all of the columns.  */
	/* (i.e., for each column of elements, determine the maximum value of the */
	/* minimum widths required (among elements in the column)).               */
	/*------------------------------------------------------------------------*/
	for (colCtr = 0; colCtr < grid->xDim; colCtr++)
	{
		for (rowCtr = 0; rowCtr < grid->yDim; rowCtr++)
		{
			offset = colCtr + rowCtr*(grid->xDim);

			if (!elemsValid[offset]) continue; /* no elem at this location */
			if (!elemsShown[offset]) continue; /* this elem isn't shown */

			eCfg = elemsCfg[offset];
			rs   = &elemsMinSz[offset];

			tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
			if (tempW > allMaxMinW[colCtr])
			{
				allMaxMinW[colCtr] = tempW;
			}
			if (tempW > maxMin)
			{
				maxMin = tempW;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Determine the required minimum width.                                  */
	/*------------------------------------------------------------------------*/
	if (grid->squareElems)
	{
		/*--------------------------------------------------------------------*/
		/* Square element sizing is required.                                 */
		/*--------------------------------------------------------------------*/
		rSize->w = grid->xDim*maxMin;
		if (grid->xDim > 1)
		{
			rSize->w += grid->HSpacing*(grid->xDim - 1);
		}
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* Square element sizing is not required.                             */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < grid->xDim; ctr++)
		{
			rSize->w += allMaxMinW[ctr];
		}
		if (grid->xDim > 1)
		{
			rSize->w += grid->HSpacing*(grid->xDim - 1);
		}
	}

	/*------------------------------------------------------------------------*/
	/* Determine the required minimum height.                                 */
	/*------------------------------------------------------------------------*/
	if (grid->squareElems)
	{
		/*--------------------------------------------------------------------*/
		/* Square element sizing is required.                                 */
		/*--------------------------------------------------------------------*/
		rSize->h = grid->yDim*maxMin;
		if (grid->yDim > 1)
		{
			rSize->h += grid->VSpacing*(grid->yDim - 1);
		}
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* Square element sizing is not required.                             */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < grid->yDim; ctr++)
		{
			rSize->h += allMaxMinH[ctr];
		}
		if (grid->yDim > 1)
		{
			rSize->h += grid->VSpacing*(grid->yDim - 1);
		}
	}
out:
	if (allMaxMinW != NULL) free(allMaxMinW);
	if (allMaxMinH != NULL) free(allMaxMinH);
	if (elemsCfg != NULL)   free(elemsCfg);
	if (elemsShown != NULL) free(elemsShown);
	if (elemsValid != NULL) free(elemsValid);
	if (elemsMinSz != NULL) free(elemsMinSz);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	// printf("[%3d] GridLayout MinSize= [%d,%d]\n", nCalled, rSize->w, rSize->h);
	// nCalled++;
	return eNum;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_Render__GridLayout
 *
 *  Description:   Renders a single button as an SDL_Texture*.
 *
 *  Returns:       NULL if there is a problem, otherwise a valid SDL_Texture*.
 *
 ******************************************************************************/
int neuik_Element_Render__GridLayout(
	NEUIK_Element   gridElem,
	RenderSize    * rSize,    /* in/out the size the tex occupies when complete */
	RenderLoc     * rlMod,    /* A relative location modifier (for rendering) */
	SDL_Renderer  * xRend,    /* the external renderer to prepare the texture for */
	SDL_Surface   * xSurf,    /* the external surface (used for transp. bg) */
	int             mock)     /* If true; calculate sizes/locations but don't draw */
{
	int                    nAlloc        = 0;
	int                    tempH         = 0;
	int                    tempW         = 0;
	int                    rowCtr        = 0;
	int                    colCtr        = 0;
	int                    offset        = 0;
	int                    ctr           = 0;
	int                    xPos          = 0;
	int                    yPos          = 0;
	int                    xFree         = 0; // px of space free for hFill elems
	int                    yFree         = 0; // px of space free for vFill elems
	int                    dH            = 0; // Change in height [px]
	int                    dW            = 0; // Change in width [px]
	int                    eNum          = 0; // which error to report (if any)
	int                    nHFill        = 0; // number of cols which can HFill
	int                    nVFill        = 0; // number of rows which can VFill
	int                    reqResizeH    = 0; // required resize height
	int                    reqResizeW    = 0; // required resize width
	int                    hfillColsMinW = 0; // min width for all hFill cols
	int                    hfillMaxMinW  = 0; // largest minimum col width 
	int                    vfillRowsMinH = 0; // min height for all vFill rows
	int                    vfillMaxMinH  = 0; // largest minimum row height 
	                                          // among vertically filling rows.
	int                  * allHFill      = NULL; // Free upon returning; 
	                                             // Cols fills vertically? (per col)
	int                  * allVFill      = NULL; // Free upon returning; 
	                                             // Row fills vertically? (per row)
	int                  * allMaxMinH    = NULL; // Free upon returning; 
	                                             // The max min width (per row)
	int                  * allMaxMinW    = NULL; // Free upon returning; 
	                                             // The max min width (per column)
	int                  * rendColW      = NULL; // Free upon returning; 
	                                             // Rendered col width (per column)
	int                  * rendRowH      = NULL; // Free upon returning; 
	                                             // Rendered row height (per row)
	int                  * elemsValid    = NULL; // Free upon returning.
	int                  * elemsShown    = NULL; // Free upon returning.
	RenderSize           * elemsMinSz    = NULL; // Free upon returning.
	NEUIK_ElementConfig ** elemsCfg      = NULL; // Free upon returning.
	RenderLoc              rl            = {0, 0};
	RenderLoc              rlRel         = {0, 0}; /* renderloc relative to parent */
	SDL_Rect               rect          = {0, 0, 0, 0};
	static RenderSize      rsZero        = {0, 0};
	RenderSize             rsMin         = {0, 0};
	RenderSize           * rs            = NULL;
	SDL_Renderer         * rend          = NULL;
	NEUIK_Container      * cont          = NULL;
	NEUIK_ElementBase    * eBase         = NULL;
	NEUIK_Element          elem          = NULL;
	NEUIK_ElementConfig  * eCfg          = NULL;
	NEUIK_GridLayout     * grid          = NULL;
	static char            funcName[]    = "neuik_Element_Render__GridLayout";
	static char          * errMsgs[]     = {"", // [0] no error
		"Argument `gridElem` is not of GridLayout class.",                   // [1]
		"Failure in `neuik_Element_Render()`",                               // [2]
		"Element_GetConfig returned NULL.",                                  // [3]
		"Element_GetMinSize Failed.",                                        // [4]
		"Failure to allocate memory.",                                       // [5]
		"Invalid specified `rSize` (negative values).",                      // [6]
		"", // [7]
		"Argument `gridElem` caused `neuik_Object_GetClassObject` to fail.", // [8]
		"Failure in neuik_Element_RedrawBackground().",                      // [9]
	};

	if (!neuik_Object_IsClass(gridElem, neuik__Class_GridLayout))
	{
		eNum = 1;
		goto out;
	}
	grid = (NEUIK_GridLayout*)gridElem;

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 8;
		goto out;
	}
	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Container, (void**)&cont))
	{
		eNum = 8;
		goto out;
	}

	if (rSize->w < 0 || rSize->h < 0)
	{
		eNum = 6;
		goto out;
	}

	eBase->eSt.rend = xRend;
	rend = eBase->eSt.rend;

	/*------------------------------------------------------------------------*/
	/* Redraw the background surface before continuing.                       */
	/*------------------------------------------------------------------------*/
	if (!mock)
	{
		if (neuik_Element_RedrawBackground(gridElem, xSurf, rlMod, NULL))
		{
			eNum = 9;
			goto out;
		}
	}
	rl = eBase->eSt.rLoc;

	if (cont->elems == NULL) {
		/* there are no UI elements contained by this GridLayout */
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for the calculated maximum minimum values, the         */
	/* VFill/HFill flags, and for the rendered row/column heights/widths.     */
	/*------------------------------------------------------------------------*/
	allMaxMinW = malloc(grid->xDim*sizeof(int));
	if (allMaxMinW == NULL)
	{
		eNum = 5;
		goto out;
	}
	allMaxMinH = malloc(grid->yDim*sizeof(int));
	if (allMaxMinH == NULL)
	{
		eNum = 5;
		goto out;
	}
	allHFill = malloc(grid->xDim*sizeof(int));
	if (allHFill == NULL)
	{
		eNum = 5;
		goto out;
	}
	allVFill = malloc(grid->yDim*sizeof(int));
	if (allVFill == NULL)
	{
		eNum = 5;
		goto out;
	}
	rendRowH = malloc(grid->yDim*sizeof(int));
	if (rendRowH == NULL)
	{
		eNum = 5;
		goto out;
	}
	rendColW = malloc(grid->xDim*sizeof(int));
	if (rendColW == NULL)
	{
		eNum = 5;
		goto out;
	}


	/*------------------------------------------------------------------------*/
	/* Zero out the initial maximum minimum values and HFill/VFill flags.     */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->xDim; ctr++)
	{
		allMaxMinW[ctr] = 0;
		allHFill[ctr]   = 0;
	}
	for (ctr = 0; ctr < grid->yDim; ctr++)
	{
		allMaxMinH[ctr] = 0;
		allVFill[ctr]   = 0;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for lists of contained element properties.             */
	/*------------------------------------------------------------------------*/
	nAlloc = grid->xDim*grid->yDim;
	elemsCfg = malloc(nAlloc*sizeof(NEUIK_ElementConfig*));
	if (elemsCfg == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsValid = malloc(nAlloc*sizeof(int));
	if (elemsValid == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsShown = malloc(nAlloc*sizeof(int));
	if (elemsShown == NULL)
	{
		eNum = 5;
		goto out;
	}
	elemsMinSz = malloc(nAlloc*sizeof(RenderSize));
	if (elemsMinSz == NULL)
	{
		eNum = 5;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Store the current properties for the contained elements.               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < nAlloc; ctr++)
	{
		/*--------------------------------------------------------------------*/
		/* A GridLayout is different from other containers in that NULL       */
		/* values are permitted within the elems array. The total number of   */
		/* contained elems is defined as xDim * yDim.                         */
		/*--------------------------------------------------------------------*/
		elemsValid[ctr] = 0;
		elem = cont->elems[ctr];
		if (elem == NULL) continue;
		elemsValid[ctr] = 1;

		elemsShown[ctr] = NEUIK_Element_IsShown(elem);
		if (!elemsShown[ctr]) continue;

		elemsCfg[ctr] = neuik_Element_GetConfig(elem);
		if (elemsCfg[ctr] == NULL)
		{
			eNum = 3;
			goto out;
		}

		elemsMinSz[ctr] = rsZero;
		if (neuik_Element_GetMinSize(elem, &elemsMinSz[ctr]))
		{
			eNum = 2;
			goto out;
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the maximum minimum heights required for all of the rows.    */
	/* (i.e., for each row of elements, determine the maximum value of the    */
	/* minimum heights required (among elements in the row)).                 */
	/*------------------------------------------------------------------------*/
	for (rowCtr = 0; rowCtr < grid->yDim; rowCtr++)
	{
		for (colCtr = 0; colCtr < grid->xDim; colCtr++)
		{
			offset = colCtr + rowCtr*(grid->xDim);

			if (!elemsValid[offset]) continue; /* no elem at this location */
			if (!elemsShown[offset]) continue; /* this elem isn't shown */

			eCfg = elemsCfg[offset];
			rs   = &elemsMinSz[offset];

			tempH = rs->h + (eCfg->PadTop + eCfg->PadBottom);
			if (tempH > allMaxMinH[rowCtr])
			{
				allMaxMinH[rowCtr] = tempH;
			}

			/*----------------------------------------------------------------*/
			/* Check if the element can fill vertically and if so, mark the   */
			/* whole row as one that can fill vertically.                     */
			/*----------------------------------------------------------------*/
			if (eCfg->VFill)
			{
				allVFill[rowCtr] = 1;
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the maximum minimum widths required for all of the columns.  */
	/* (i.e., for each column of elements, determine the maximum value of the */
	/* minimum widths required (among elements in the column)).               */
	/*------------------------------------------------------------------------*/
	for (colCtr = 0; colCtr < grid->xDim; colCtr++)
	{
		for (rowCtr = 0; rowCtr < grid->yDim; rowCtr++)
		{
			offset = colCtr + rowCtr*(grid->xDim);

			if (!elemsValid[offset]) continue; /* no elem at this location */
			if (!elemsShown[offset]) continue; /* this elem isn't shown */

			eCfg = elemsCfg[offset];
			rs   = &elemsMinSz[offset];

			tempW = rs->w + (eCfg->PadLeft + eCfg->PadRight);
			if (tempW > allMaxMinW[colCtr])
			{
				allMaxMinW[colCtr] = tempW;
			}

			/*----------------------------------------------------------------*/
			/* Check if the element can fill horizontally and if so, mark the */
			/* whole column as one that can fill horizontally.                */
			/*----------------------------------------------------------------*/
			if (eCfg->HFill)
			{
				allHFill[colCtr] = 1;
			}
		}
	}

	/*========================================================================*/
	/* Calculation of rendered column widths (accounts for HFill).            */
	/*========================================================================*/
	/* Determine the required minimum width and the total number of columns   */
	/* which can fill horizontally.                                           */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->xDim; ctr++)
	{
		rsMin.w += allMaxMinW[ctr];
		nHFill += allHFill[ctr];
	}
	if (grid->xDim > 1)
	{
		rsMin.w += grid->HSpacing*(grid->xDim - 1);
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the space occupied by all HFill columns and determine the    */
	/* value of the largest minimum column width among horizontally filling   */
	/* columns.                                                               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->xDim; ctr++)
	{
		if (allHFill[ctr])
		{
			hfillColsMinW += allMaxMinW[ctr];
			if (hfillMaxMinW < allMaxMinW[ctr])
			{
				hfillMaxMinW = allMaxMinW[ctr];
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the amount of currently unused horizontal space (beyond min) */
	/*------------------------------------------------------------------------*/
	xFree = rSize->w - rsMin.w;

	/*------------------------------------------------------------------------*/
	/* Check if there is enough unused horizontal space to bring all HFill    */
	/* columns to the same width.                                             */
	/*------------------------------------------------------------------------*/
	reqResizeW = nHFill*hfillMaxMinW - hfillColsMinW; // required resize width
	if (xFree >= reqResizeW)
	{
		/*--------------------------------------------------------------------*/
		/* There is enough space; get all HFill cols to the same size first.  */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < grid->xDim; ctr++)
		{
			rendColW[ctr] = allMaxMinW[ctr];
			if (allHFill[ctr])
			{
				rendColW[ctr] = hfillMaxMinW;
			}
		}

		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining hSpace between the hFill columns.      */
		/*--------------------------------------------------------------------*/
		xFree -= reqResizeW;

		dW = (int)(floor( (float)(xFree)/(float)(nHFill) ));
		if (dW > 0)
		{
			/*----------------------------------------------------------------*/
			/* Increase the width of hFill columns all by the same quantity.  */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->xDim; ctr++)
			{
				if (allHFill[ctr])
				{
					rendColW[ctr] += dW;
					xFree -= dW;
				}
			}
		}

		if (xFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining hSpace to the hFill one pixel at a    */
			/* time (starting from the left column and moving right).         */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->xDim; ctr++)
			{
				if (allHFill[ctr])
				{
					rendColW[ctr] += 1;
					xFree -= 1;
					if (xFree == 0)
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining hSpace between the hFill columns.      */
		/*--------------------------------------------------------------------*/
		while (xFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining hSpace to the hFill one pixel at a    */
			/* time (starting from the left column and moving right).         */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->xDim; ctr++)
			{
				if (allHFill[ctr] && rendColW[ctr] < hfillMaxMinW)
				{
					rendColW[ctr] += 1;
					xFree -= 1;
					if (xFree == 0)
					{
						break;
					}
				}
			}
		}
	}

	/*========================================================================*/
	/* Calculation of rendered row heights (accounts for VFill).              */
	/*========================================================================*/
	/* Determine the required minimum height and the total number of rows     */
	/* which can fill vertically.                                             */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->yDim; ctr++)
	{
		rsMin.h += allMaxMinH[ctr];
		nVFill += allVFill[ctr];
	}
	if (grid->yDim > 1)
	{
		rsMin.h += grid->VSpacing*(grid->yDim - 1);
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the space occupied by all VFill rows and determine the value */
	/* of the largest minimum row height among vertically filling rows.       */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr < grid->yDim; ctr++)
	{
		if (allVFill[ctr])
		{
			vfillRowsMinH += allMaxMinH[ctr];
			if (vfillMaxMinH < allMaxMinH[ctr])
			{
				vfillMaxMinH = allMaxMinH[ctr];
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/* Calculate the amount of currently unused vertical space (beyond min).  */
	/*------------------------------------------------------------------------*/
	yFree = rSize->h - rsMin.h;

	/*------------------------------------------------------------------------*/
	/* Check if there is enough unused vertical space to bring all VFill rows */
	/* to the same height.                                                    */
	/*------------------------------------------------------------------------*/
	reqResizeH = nVFill*vfillMaxMinH - vfillRowsMinH; // required resize height
	if (yFree >= reqResizeH)
	{
		/*--------------------------------------------------------------------*/
		/* There is enough space; get all VFill rows to the same size first.  */
		/*--------------------------------------------------------------------*/
		for (ctr = 0; ctr < grid->yDim; ctr++)
		{
			rendRowH[ctr] = allMaxMinH[ctr];
			if (allVFill[ctr])
			{
				rendRowH[ctr] = vfillMaxMinH;
			}
		}

		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining vSpace between the vFill rows.         */
		/*--------------------------------------------------------------------*/
		yFree -= reqResizeH;

		dH = (int)(floor( (float)(yFree)/(float)(nVFill) ));
		if (dH > 0)
		{
			/*----------------------------------------------------------------*/
			/* Increase the height of vFill rows all by the same quantity.    */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->yDim; ctr++)
			{
				if (allVFill[ctr])
				{
					rendRowH[ctr] += dH;
					yFree -= dH;
				}
			}
		}

		if (yFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining vSpace to the vFill one pixel at a    */
			/* time (starting from the top row and moving down).              */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->yDim; ctr++)
			{
				if (allVFill[ctr])
				{
					rendRowH[ctr] += 1;
					yFree -= 1;
					if (yFree == 0)
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		/*--------------------------------------------------------------------*/
		/* Evenly divide the remaining vSpace between the vFill rows.         */
		/*--------------------------------------------------------------------*/
		while (yFree > 0)
		{
			/*----------------------------------------------------------------*/
			/* Distribute the remaining vSpace to the vFill one pixel at a    */
			/* time (starting from the top row and moving down).              */
			/*----------------------------------------------------------------*/
			for (ctr = 0; ctr < grid->yDim; ctr++)
			{
				if (allVFill[ctr] && rendRowH[ctr] < vfillMaxMinH)
				{
					rendRowH[ctr] += 1;
					yFree -= 1;
					if (yFree == 0)
					{
						break;
					}
				}
			}
		}
	}

	/*========================================================================*/
	/* Render and place the child elements                                    */
	/*========================================================================*/
	yPos = 0;
	for (rowCtr = 0; rowCtr < grid->yDim; rowCtr++)
	{
		xPos = 0;
		if (rowCtr > 0)
		{
			yPos += rendRowH[rowCtr-1] + grid->VSpacing;
		}

		for (colCtr = 0; colCtr < grid->xDim; colCtr++)
		{
			offset = colCtr + rowCtr*(grid->xDim);

			if (colCtr > 0)
			{
				xPos += rendColW[colCtr-1] + grid->HSpacing;
			}

			if (!elemsValid[offset]) continue; /* no elem at this location */
			if (!elemsShown[offset]) continue; /* this elem isn't shown */

			elem = cont->elems[offset];
			eCfg = elemsCfg[offset];
			rs   = &elemsMinSz[offset];

			tempH = rendRowH[rowCtr];
			tempW = rendColW[colCtr];

			/*----------------------------------------------------------------*/
			/* Check for and apply if necessary Horizontal and Vertical fill. */
			/*----------------------------------------------------------------*/
			if (allHFill[colCtr])
			{
				rs->w = tempW - (eCfg->PadLeft + eCfg->PadRight);
			}
			if (allVFill[rowCtr])
			{
				rs->h = tempH - (eCfg->PadTop + eCfg->PadBottom);
			}

			/*----------------------------------------------------------------*/
			/* Update the stored location before rendering the element. This  */
			/* is necessary as the location of this object will propagate to  */
			/* its child objects.                                             */
			/*----------------------------------------------------------------*/
			switch (eCfg->HJustify)
			{
				case NEUIK_HJUSTIFY_DEFAULT:
					switch (cont->HJustify)
					{
						case NEUIK_HJUSTIFY_LEFT:
							rect.x = xPos + eCfg->PadLeft;
							break;
						case NEUIK_HJUSTIFY_CENTER:
						case NEUIK_HJUSTIFY_DEFAULT:
							rect.x = (xPos + rendColW[colCtr]/2) - (tempW/2);
							break;
						case NEUIK_HJUSTIFY_RIGHT:
							rect.x = (xPos + rendColW[colCtr]) - 
								(rs->w + eCfg->PadRight);
							break;
					}
					break;
				case NEUIK_HJUSTIFY_LEFT:
					rect.x = xPos + eCfg->PadLeft;
					break;
				case NEUIK_HJUSTIFY_CENTER:
					rect.x = (xPos + rendColW[colCtr]/2) - (tempW/2);
					break;
				case NEUIK_HJUSTIFY_RIGHT:
					rect.x = (xPos + rendColW[colCtr]) - 
						(rs->w + eCfg->PadRight);
					break;
			}
			switch (eCfg->VJustify)
			{
				case NEUIK_VJUSTIFY_DEFAULT:
					switch (cont->VJustify)
					{
						case NEUIK_VJUSTIFY_TOP:
							rect.y = yPos + eCfg->PadTop;
							break;
						case NEUIK_VJUSTIFY_CENTER:
						case NEUIK_VJUSTIFY_DEFAULT:
							rect.y = (yPos + rendRowH[rowCtr]/2) - (tempH/2);
							break;
						case NEUIK_VJUSTIFY_BOTTOM:
							rect.y = (yPos + rendRowH[rowCtr]) - 
								(rs->h + eCfg->PadBottom);
							break;
					}
					break;
				case NEUIK_VJUSTIFY_TOP:
					rect.y = yPos + eCfg->PadTop;
					break;
				case NEUIK_VJUSTIFY_CENTER:
					rect.y = (yPos + rendRowH[rowCtr]/2) - (tempH/2);
					break;
				case NEUIK_VJUSTIFY_BOTTOM:
					rect.y = (yPos + rendRowH[rowCtr]) - 
						(rs->h + eCfg->PadBottom);
					break;
			}

			rect.w = rendColW[colCtr];
			rect.h = rendRowH[rowCtr];
			rl.x = (eBase->eSt.rLoc).x + rect.x;
			rl.y = (eBase->eSt.rLoc).y + rect.y;
			rlRel.x = rect.x;
			rlRel.y = rect.y;
			neuik_Element_StoreSizeAndLocation(elem, *rs, rl, rlRel);

			if (neuik_Element_NeedsRedraw(elem))
			{
				if (neuik_Element_Render(elem, rs, rlMod, rend, xSurf, mock))
				{
					eNum = 2;
					goto out;
				}
			}
		}
	}
out:
	if (!mock) eBase->eSt.doRedraw = 0;

	if (elemsCfg   != NULL) free(elemsCfg);
	if (elemsShown != NULL) free(elemsShown);
	if (elemsValid != NULL) free(elemsValid);
	if (elemsMinSz != NULL) free(elemsMinSz);
	if (allMaxMinW != NULL) free(allMaxMinW);
	if (allMaxMinH != NULL) free(allMaxMinH);
	if (rendColW   != NULL) free(rendColW);
	if (rendRowH   != NULL) free(rendRowH);
	if (allHFill   != NULL) free(allHFill);
	if (allVFill   != NULL) free(allVFill);

	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

/*******************************************************************************
 *
 *  Name:          neuik_Element_CaptureEvent__GridLayout
 *
 *  Description:   A virtual function reimplementation of the function
 *                 neuik_Element_CaptureEvent.
 *
 *  Returns:       1 if the event was captured; 0 otherwise.
 *
 ******************************************************************************/
neuik_EventState neuik_Element_CaptureEvent__GridLayout(
	NEUIK_Element   gridElem, 
	SDL_Event     * ev)
{
	int                ctr        = 0;
	int                finalInd   = 0;
	neuik_EventState   evCaputred = NEUIK_EVENTSTATE_NOT_CAPTURED;
	NEUIK_Element      elem       = NULL;
	NEUIK_Container  * cBase      = NULL;
	NEUIK_GridLayout * grid       = NULL;

	if (neuik_Object_GetClassObject_NoError(
		gridElem, neuik__Class_GridLayout, (void**)&grid)) goto out;

	if (neuik_Object_GetClassObject_NoError(
		gridElem, neuik__Class_Container, (void**)&cBase)) goto out;

	if (cBase->elems != NULL)
	{
		finalInd = grid->xDim * grid->yDim;
		for (ctr = 0; ctr < finalInd; ctr++)
		{
			/*----------------------------------------------------------------*/
			/* A GridLayout is different from other containers in that NULL   */
			/* values are permitted within the elems array. The total number  */
			/* of contained elems is defined as xDim * yDim.                  */
			/*----------------------------------------------------------------*/
			elem = cBase->elems[ctr];
			if (elem == NULL) continue;

			if (!NEUIK_Element_IsShown(elem)) continue;

			evCaputred = neuik_Element_CaptureEvent(elem, ev);
			if (evCaputred == NEUIK_EVENTSTATE_OBJECT_FREED)
			{
				goto out;
			}
			if (evCaputred == NEUIK_EVENTSTATE_CAPTURED)
			{
				neuik_Element_SetActive(gridElem, 1);
				goto out;
			}
		}
	}
out:
	return evCaputred;
}


/*******************************************************************************
 *
 *  Name:          neuik_Element_SetWindowPointer__GridLayout (redefined-vfunc)
 *
 *  Description:   Set the Window Pointer for an object.
 *
 *                 This operation is a virtual function redefinition.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
******************************************************************************/
int neuik_Element_SetWindowPointer__GridLayout (
	NEUIK_Element   gridElem, 
	void          * win)
{
	int                  eNum     = 0;
	int                  ctr      = 0;
	int                  finalInd = 0;
	NEUIK_Element        elem     = NULL; 
	NEUIK_ElementBase  * eBase    = NULL;
	NEUIK_Container    * cBase    = NULL;
	NEUIK_GridLayout   * grid     = NULL;
	static int           nRecurse = 0; /* number of times recursively called */
	static char          funcName[] = "neuik_Element_SetWindowPointer__GridLayout";
	static char        * errMsgs[]  = {"",                                    // [0] no error
		"Argument `gridElem` caused `neuik_Object_GetClassObject` to fail.",  // [1]
		"Child Element caused `SetWindowPointer` to fail.",                   // [2]
		"Argument `win` does not implement Window class.",                    // [3]
	};

	nRecurse++;
	if (nRecurse > NEUIK_MAX_RECURSION)
	{
		/*--------------------------------------------------------------------*/
		/* This is likely a case of appears to be runaway recursion; report   */
		/* an error to the user.                                              */
		/*--------------------------------------------------------------------*/
		neuik_Fatal = NEUIK_FATALERROR_RUNAWAY_RECURSION;
		goto out;
	}

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_GridLayout, (void**)&grid))
	{
		eNum = 1;
		goto out;
	}

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Container, (void**)&cBase))
	{
		eNum = 1;
		goto out;
	}

	if (cBase->elems != NULL)
	{
		/*--------------------------------------------------------------------*/
		/* Propagate this information to contained UI Elements                */
		/*--------------------------------------------------------------------*/
		finalInd = grid->xDim * grid->yDim;
		for (ctr = 0; ctr < finalInd; ctr++)
		{
			/*----------------------------------------------------------------*/
			/* A GridLayout is different from other containers in that NULL   */
			/* values are permitted within the elems array. The total number  */
			/* of contained elems is defined as xDim * yDim.                  */
			/*----------------------------------------------------------------*/
			elem = cBase->elems[ctr];
			if (elem == NULL) continue;

			if (neuik_Element_SetWindowPointer(elem, win))
			{
				eNum = 2;
				goto out;
			}
		}
	}

	if (neuik_Object_GetClassObject(gridElem, neuik__Class_Element, (void**)&eBase))
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_ImplementsClass(win, neuik__Class_Window))
	{
		eNum = 3;
		goto out;
	}

	eBase->eSt.window = win;
out:
	nRecurse--;
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return eNum;
}
