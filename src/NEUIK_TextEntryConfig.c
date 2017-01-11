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
#include "NEUIK_colors.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"
#include "NEUIK_defs.h"
#include "NEUIK_FontSet.h"
#include "NEUIK_TextEntryConfig.h"
#include "neuik_internal.h"
#include "neuik_classes.h"

extern int neuik__isInitialized;

/*----------------------------------------------------------------------------*/
/* Internal Function Prototypes                                               */
/*----------------------------------------------------------------------------*/
int neuik_Object_New__TextEntryConfig(void ** cfg);
int neuik_Object_Copy__TextEntryConfig(void * dst, const void * src);
int neuik_Object_Free__TextEntryConfig(void ** cfg);

/*----------------------------------------------------------------------------*/
/* neuik_Object    Function Table                                             */
/*----------------------------------------------------------------------------*/
neuik_Class_BaseFuncs  neuik_TextEntryConfig_BaseFuncs = {
	/* Init(): Class initialization (in most cases will not be needed) */
	NULL, /* (unused) */
	/* New(): Allocate and Initialize the object */
	neuik_Object_New__TextEntryConfig,
	/* Copy(): Copy the contents of one object into another */
	neuik_Object_Copy__TextEntryConfig,
	/* Free(): Free the allocated memory of an object */
	neuik_Object_Free__TextEntryConfig,
};


/*******************************************************************************
 *
 *  Name:          neuik_RegisterClass_TextEntryConfig
 *
 *  Description:   Register this class with the NEUIK runtime.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_RegisterClass_TextEntryConfig()
{
	int           eNum       = 0; /* which error to report (if any) */
	static char   funcName[] = "neuik_RegisterClass_TextEntryConfig";
	static char * errMsgs[]  = {"",                            // [0] no error
		"NEUIK library must be initialized first.",            // [1]
		"Failed to register `TextEntryConfig` object class .", // [2]
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
		"NEUIK_TextEntryConfig",                     // className
		"Configuration for NEUIK_TextEntry Object.", // classDescription
		neuik__Set_NEUIK,                            // classSet
		NULL,                                        // superClass
		&neuik_TextEntryConfig_BaseFuncs,            // baseFuncs
		NULL,                                        // classFuncs
		&neuik__Class_TextEntryConfig))              // newClass
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
 *  Name:          NEUIK_GetDefaultTextEntryConfig
 *
 *  Description:   Returns a pointer to the initialized default TextEntry
 *                 configuration.
 *
 *  Returns:       A pointer to the default NEUIK_TextEntryConfig; NULL if error.
 *
 ******************************************************************************/
NEUIK_TextEntryConfig * NEUIK_GetDefaultTextEntryConfig()
{
	int                            eNum           = 0;
	static int                     isInitialized  = 0;
	static char                  * dFontName      = NULL;
	NEUIK_TextEntryConfig        * rvCfg          = NULL;
	/* default TextEntryConfig */
	static NEUIK_TextEntryConfig   dCfg = {
		{0, 0, NULL, NULL, NULL}, // neuik_Object objBase
		NULL,                     // NEUIK_FontSet * fontSet
		11,                       // int             fontSize
		0,                        // int             fontBold
		0,                        // int             fontItalic
		NULL,                     // char          * fontName
		COLOR_WHITE,              // SDL_Color       bgColor
		COLOR_LBLACK,             // SDL_Color       fgColor
		COLOR_LBLUE,              // SDL_Color       bgColorHl
		COLOR_WHITE,              // SDL_Color       fgColorHl
		COLOR_DBLUE,              // SDL_Color       bgColorSelect
		COLOR_GRAY,               // SDL_Color       borderColor
		COLOR_DGRAY,              // SDL_Color       borderColorDark
		NEUIK_VJUSTIFY_CENTER,    // int             textVJustify
		NEUIK_HJUSTIFY_LEFT,      // int             textHJustify
		15,                       // int             EmWidth
		NEUIK_RESTRICT_NONE,      // int             restriction
		NULL,                     // char          * restrict_str
		10,                       // int             emptySpaces
	};
	static char   funcName[] = "NEUIK_GetDefaultTextEntryConfig";
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
			neuik__Class_TextEntryConfig, 
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
 *  Name:          neuik_Object_New__TextEntryConfig
 *
 *  Description:   An implementation of the neuik_Object_New method.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int neuik_Object_New__TextEntryConfig(
	void ** cfgPtr)
{
	int                 eNum       = 0;
	NEUIK_TextEntryConfig * cfg        = NULL;
	static char         funcName[] = "neuik_Object_New__TextEntryConfig";
	static char       * errMsgs[]  = {"",     // [0] no error
		"Output Argument cfgPtr is NULL.",    // [1]
		"Failure to allocate memory.",        // [2]
		"Failure in TextEntryConfig_Copy().", // [3]
	};

	if (cfgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}

	(*cfgPtr) = (NEUIK_TextEntryConfig*) malloc(sizeof(NEUIK_TextEntryConfig));
	cfg = (*cfgPtr);
	if (cfg == NULL)
	{
		eNum = 2;
		goto out;
	}

	/*------------------------------------------------------------------------*/
	/* Set the object base to that of TextEntryConfig                         */
	/*------------------------------------------------------------------------*/
	neuik_GetObjectBaseOfClass(
		neuik__Set_NEUIK, 
		neuik__Class_TextEntryConfig, 
		NULL,
		&(cfg->objBase));

	/*------------------------------------------------------------------------*/
	/* Copy the default config settings into the new TextEntryConfig          */
	/*------------------------------------------------------------------------*/
	if (NEUIK_TextEntryConfig_Copy((cfg), NEUIK_GetDefaultTextEntryConfig()))
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
 *  Name:          NEUIK_NewTextEntryConfig
 *
 *  Description:   Allocate memory and set default values for TextEntryConfig.
 *
 *                 This is only a wrapper function.
 *
 *  Returns:       1 if there is an error; 0 otherwise.
 *
 ******************************************************************************/
int NEUIK_NewTextEntryConfig(
		NEUIK_TextEntryConfig ** cfgPtr)
{
	return neuik_Object_New__TextEntryConfig((void**)cfgPtr);
}

/*******************************************************************************
 *
 *  Name:          neuik_Object_Copy__TextEntryConfig
 *
 *  Description:   An implementation of the neuik_Object_Copy method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Copy__TextEntryConfig(
	void        * dst,
	const void  * src)
{
	return NEUIK_TextEntryConfig_Copy(
		(NEUIK_TextEntryConfig *)dst, (const NEUIK_TextEntryConfig *)src);
}


/*******************************************************************************
 *
 *  Name:          NEUIK_TextEntryConfig_Copy
 *
 *  Description:   Copy the data in a TextEntryConfig to that used in the struct.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int NEUIK_TextEntryConfig_Copy(
	NEUIK_TextEntryConfig       * dst,
	const NEUIK_TextEntryConfig * src)
{
	int            eNum       = 0; /* which error to report (if any) */
	static char    funcName[] = "NEUIK_TextEntryConfig_Copy";
	static char  * errMsgs[]  = {"",                       // [0] no error
		"Argument `src` is invalid or an incorrect type.", // [1]
		"Argument `dst` is invalid or an incorrect type.", // [2]
		"TextEntryConfig->fontName is NULL.",              // [3]
		"Failure in String_Duplicate.",                    // [4]
	};

	if (!neuik_Object_IsClass(src, neuik__Class_TextEntryConfig))
	{
		eNum = 1;
		goto out;
	}
	if (!neuik_Object_IsClass(dst, neuik__Class_TextEntryConfig))
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

	dst->fontSet         = src->fontSet;
	dst->fontSize        = src->fontSize;
	dst->fontBold        = src->fontBold;
	dst->fontItalic      = src->fontItalic;
	dst->bgColor         = src->bgColor;
	dst->fgColor         = src->fgColor;
	dst->bgColorHl       = src->bgColorHl;
	dst->fgColorHl       = src->fgColorHl;
	dst->bgColorSelect   = src->bgColorSelect;
	dst->borderColor     = src->borderColor;
	dst->borderColorDark = src->borderColorDark;
	dst->textVJustify    = src->textVJustify;
	dst->textHJustify    = src->textHJustify;
	dst->fontEmWidth     = src->fontEmWidth;
	dst->restriction     = src->restriction;
	dst->emptySpaces     = src->emptySpaces;
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
 *  Name:          neuik_Object_Free__TextEntryConfig
 *
 *  Description:   An implementation of the neuik_Object_Free method.
 *
 *  Returns:       Non-zero if an error occurs.
 *
 ******************************************************************************/
int neuik_Object_Free__TextEntryConfig(
	void  ** cfgPtr)
{
	int                     eNum       = 0;
	NEUIK_TextEntryConfig * cfg        = NULL;
	static char             funcName[] = "neuik_Object_Free__TextEntryConfig";
	static char           * errMsgs[]  = {"",                  // [0] no error
		"Argument `cfgPtr` is NULL.",                          // [1]
		"Argument `*cfgPtr` is invalid or an incorrect type.", // [2]
	};

	if (cfgPtr == NULL)
	{
		eNum = 1;
		goto out;
	}
	cfg = (*cfgPtr);

	if (!neuik_Object_IsClass(cfg, neuik__Class_TextEntryConfig))
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
	(*cfgPtr) = NULL;
out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

