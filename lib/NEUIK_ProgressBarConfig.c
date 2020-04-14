/*******************************************************************************
 * Copyright (c) 2014-2017, Michael Leimon <leimon@gmail.com>
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

#include "NEUIK_colors.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"
#include "NEUIK_FontSet.h"
#include "NEUIK_ProgressBarConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_Copy__ProgressBarConfig(void *, const void *);
int neuik_Object_New__ProgressBarConfig(void **);
int neuik_Object_Free__ProgressBarConfig(void *);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_ProgressBarConfig_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__ProgressBarConfig,
	/* Copy(): Copy the contents of one object into another */
	neuik_Object_Copy__ProgressBarConfig,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__ProgressBarConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_ProgressBarConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_RegisterClass_ProgressBarConfig()
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "neuik_RegisterClass_ProgressBarConfig";
	static char  * errMsgs[]  = {"",                             // [0] no error
		"NEUIK library must be initialized first.",              // [1]
		"Failed to register `ProgressBarConfig` object class .", // [2]
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
		"ProgressBarConfig",                     // className
		"The configuration for a progress bar.", // classDescription
		neuik__Set_NEUIK,                        // classSet
		neuik__Class_Element,                    // superClass
		&neuik_ProgressBarConfig_BaseFuncs,      // baseFuncs
		NULL,                                    // classFuncs
		&neuik__Class_ProgressBarConfig))        // newClass
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
 *  Name:          neuik_Object_New__ProgressBarConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_New__ProgressBarConfig(
	void ** cfgPtr)
{
	return NEUIK_NewProgressBarConfig((NEUIK_ProgressBarConfig **)cfgPtr);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_NewProgressBarConfig
 *
 *  Description:   Allocate memory and set default values for ProgressBarConfig.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_NewProgressBarConfig(
		NEUIK_ProgressBarConfig ** cfgPtr)
{
	int                       eNum       = 0;
	NEUIK_ProgressBarConfig * cfg        = NULL;
	static char               funcName[] = "NEUIK_NewProgressBarConfig";
	static char             * errMsgs[]  = {"",           // [0] no error
		"Output Argument cfgPtr is NULL.",                // [1]
		"Failure to allocate memory.",                    // [2]
		"Failure in ProgressBarConfig_Copy().",           // [3]
		"Failue in `neuik_Object_GetObjectBaseOfClass`.", // [4]
	};

	if (cfgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	(*cfgPtr) = (NEUIK_ProgressBarConfig*) malloc(sizeof(NEUIK_ProgressBarConfig));
	cfg = (*cfgPtr);
	if (cfg == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the object base to that of ProgressBarConfig                       */
	/*------------------------------------------------------------------------*/
	if (neuik_GetObjectBaseOfClass(neuik__Set_NEUIK, 
		neuik__Class_ProgressBarConfig, NULL, &(cfg->objBase)))
	{
		eNum = 4;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Copy the default config settings into the new ProgressBarConfig        */
	/*------------------------------------------------------------------------*/
	if (NEUIK_ProgressBarConfig_Copy((cfg), NEUIK_GetDefaultProgressBarConfig()))
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
 *  Name:          neuik_Object_Copy__ProgressBarConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__ProgressBarConfig(
	void        * dst,
	const void  * src)
{
	return NEUIK_ProgressBarConfig_Copy(
		(NEUIK_ProgressBarConfig *)dst, (const NEUIK_ProgressBarConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_ProgressBar_CopyConfig
 *
 *  Description:   Copy the data in a ProgressBarConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_ProgressBarConfig_Copy(
	NEUIK_ProgressBarConfig       * dst,
	const NEUIK_ProgressBarConfig * src)
{
	int            eNum       = 0; /* which error to report (if any) */
	int            csLen;
	int            ctr;
	static char    funcName[] = "NEUIK_ProgressBarConfig_Copy";
	static char  * errMsgs[]  = {"",                       // [0] no error
		"Argument `src` is invalid or an incorrect type.", // [1]
		"Argument `dst` is invalid or an incorrect type.", // [2]
		"ProgressBarConfig->fontName is NULL.",            // [3]
		"Failure in String_Duplicate.",                    // [4]
		"`src->gradCS` or `src->gradCSSelect` is NULL.",   // [5]
		"Failed to allocate memory.",                      // [6]
	};

	if (!neuik_Object_IsClass(src, neuik__Class_ProgressBarConfig))
	{
		eNum = 1;
		goto out;
	}
	if (!neuik_Object_IsClass(dst, neuik__Class_ProgressBarConfig))
	{
		eNum = 2;
		goto out;
	}
	else if (src->gradCS == NULL)
	{
		eNum = 5;
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

	/*--------------------------------------------------------------------*/
	/* Create the NULL-terminated ColorStop array for creating a gradient */
	/*--------------------------------------------------------------------*/
	/* first determine the length of the ColorStop array */
	for (csLen = 0;; csLen++)
	{
		if (src->gradCS[csLen] == NULL) break;
	}

	dst->gradCS = (NEUIK_ColorStop **)malloc((1+csLen)*sizeof(NEUIK_ColorStop*));
	if (dst->gradCS == NULL)
	{
		eNum = 6;
		goto out;
	}
	for (ctr = 0;; ctr++)
	{
		if (src->gradCS[ctr] == NULL)
		{
			/* this is the final NULL terminating pointer */
			dst->gradCS[ctr] = NULL;
			break;
		}

		/* otherwise allocate memory for a ColorStop and copy over values */
		dst->gradCS[ctr] = (NEUIK_ColorStop *)malloc(sizeof(NEUIK_ColorStop));
		if (dst->gradCS[ctr] == NULL)
		{
			eNum = 6;
			goto out;
		}
		dst->gradCS[ctr] = src->gradCS[ctr];
	}

	dst->bgColor         = src->bgColor;
	dst->fgColor         = src->fgColor;
	dst->progColorLight  = src->progColorLight;
	dst->progColorDark   = src->progColorDark ;
	dst->borderColor     = src->borderColor;
	dst->borderColorDark = src->borderColorDark;
	dst->fontEmWidth     = src->fontEmWidth;
	dst->decimalPlaces   = src->decimalPlaces;
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
 *  Name:          NEUIK_GetDefaultProgressBarConfig
 *
 *  Description:   Returns a pointer to the initialized default ProgressBar
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_ProgressBarConfig; NULL if error.
 *
 ******************************************************************************/
NEUIK_ProgressBarConfig * NEUIK_GetDefaultProgressBarConfig()
{
	int                              eNum           = 0;
	static int                       isInitialized  = 0;
	static char                    * dFontName      = NULL;
	static NEUIK_ColorStop           cs0 = {COLOR_PBAR_LBLUE, 0.0};
	static NEUIK_ColorStop           cs1 = {COLOR_PBAR_DBLUE, 1.0};
	NEUIK_ProgressBarConfig        * rvCfg          = NULL;
	/* default ProgressBarConfig */
	static NEUIK_ProgressBarConfig   dCfg = {
		{0, 0, NULL, NULL, NULL}, // neuik_Object       objBase
		NULL,                     // NEUIK_FontSet    * fontSet
		11,                       // int                fontSize
		0,                        // int                fontBold
		0,                        // int                fontItalic
		NULL,                     // char             * fontName
		NULL,                     // NEUIK_ColorStop ** gradCS;
		COLOR_LGRAY,              // SDL_Color          bgColor
		COLOR_LBLACK,             // SDL_Color          fgColor
		COLOR_PBAR_LBLUE,         // SDL_Color          progColorLight
		COLOR_PBAR_DBLUE,         // SDL_Color          progColorDark
		COLOR_GRAY,               // SDL_Color          borderColor
		COLOR_DGRAY,              // SDL_Color          borderColorDark
		15,                       // int                EmWidth
		0,                        // unsigned int       decimalPlaces
	};
	static char   funcName[] = "NEUIK_GetDefaultProgressBarConfig";
	static char * errMsgs[] = {"",                        // [0] no error
		"Failure in GetDefaultFontSet().",                // [1]
		"Failure in FontSet_GetFont().",                  // [2]
		"Failure in String_Duplicate().",                 // [3]
		"Failed to allocate memory().",                   // [4]
		"Failue in `neuik_Object_GetObjectBaseOfClass`.", // [5]
	};

	if (!isInitialized)
	{
		isInitialized = 1;

		if (neuik_GetObjectBaseOfClass(neuik__Set_NEUIK, 
			neuik__Class_ProgressBarConfig, NULL, &(dCfg.objBase)))
		{
			eNum = 5;
			goto out;
		}

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
		/* Create the NULL-terminated ColorStop array for creating a gradient */
		/*--------------------------------------------------------------------*/
		dCfg.gradCS = (NEUIK_ColorStop **)malloc(3*sizeof(NEUIK_ColorStop*));
		if (dCfg.gradCS == NULL)
		{
			eNum = 4;
			goto out;
		}

		dCfg.gradCS[0] = &cs0;
		dCfg.gradCS[1] = &cs1;
		dCfg.gradCS[2] = NULL;

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
 *  Name:          NEUIK_ProgressBarConfig_Free
 *
 *  Description:   Free memory allocated for this object and NULL out pointer.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__ProgressBarConfig(
	void * cfgPtr)
{
	int                       eNum       = 0;
	int                       ctr;
	NEUIK_ProgressBarConfig * cfg        = NULL;
	static char               funcName[] = "NEUIK_ProgressBarConfig_Free";
	static char             * errMsgs[]  = {"",                // [0] no error
		"Argument `cfgPtr` is NULL.",                          // [1]
		"Argument `*cfgPtr` is invalid or an incorrect type.", // [2]
	};

	if (cfgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	if (!neuik_Object_IsClass(cfgPtr, neuik__Class_ProgressBarConfig))
	{
		eNum = 2;
		goto out;
	}
	cfg = (NEUIK_ProgressBarConfig*)cfgPtr;

	/*------------------------------------------------------------------------*/
	/* The object is what it says it is and it is still allocated.            */
	/*------------------------------------------------------------------------*/
	if (cfg->fontName != NULL) free(cfg->fontName);

	if (cfg->gradCS != NULL)
	{
		for (ctr = 0;; ctr++)
		{
			/* look for the terminating NULL pointer */
			if (cfg->gradCS[ctr] == NULL) break;
			/* otherwise; free the colorStop */
			free(cfg->gradCS[ctr]);
		}
		free(cfg->gradCS);
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

