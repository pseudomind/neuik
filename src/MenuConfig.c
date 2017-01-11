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
#include <SDL.h>
#include <SDL_ttf.h>

#include "NEUIK_colors.h"
#include "NEUIK_render.h"
#include "NEUIK_error.h"

#include "MenuConfig.h"


NEUIK_MenuConfig * NEUIK_GetDefaultMenuConfig()
{
	static int     isInitialized = 0;
	static char  * dFontName     = NULL;
	static NEUIK_MenuConfig dCfg = {  /* default MenuConfig*/
		NULL,         // NEUIK_FontSet * fontSet
		11,           // int fontSize
		NULL,         // char* fontName
		COLOR_LGRAY,  // SDL_Color bgColor
		COLOR_LBLACK, // SDL_Color fgColor
		COLOR_DBLUE,  // SDL_Color bgColorSelect
		COLOR_WHITE,  // SDL_Color fgColorSelect
		COLOR_GRAY,   // SDL_Color sepColor
		COLOR_DGRAY,  // SDL_Color sepColorDark
		20,           // int Height
		15,           // int EmWidth
	};
	NEUIK_MenuConfig * rvCfg      = NULL;
	int                eNum       = 0;
	static char        funcName[] = "NEUIK_GetDefaultMenuConfig";
	static char      * errMsgs[]  = {"",   // [0] no error
		"Failure in GetDefaultFontSet().", // [1]
		"Failure in FontSet_GetFont().",   // [2]
	};

	if (!isInitialized)
	{
		isInitialized = 1;

		dCfg.fontSet = NEUIK_GetDefaultFontSet(&dFontName);
		if (dCfg.fontSet == NULL)
		{
			eNum = 1;
			goto out;
		}

		/* Finally attempt to load the font */
		if (NEUIK_FontSet_GetFont(dCfg.fontSet, dCfg.fontSize, 0, 0) == NULL)
		{
			eNum = 2;
			goto out;
		}
		rvCfg = &dCfg;
	}

out:
	if (eNum > 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
	}

	return rvCfg;
}



// int NEUIK_MenuConfig_SetFont(
// 		NEUIK_MenuConfig * mCfg,
// 		const char       * fontName,
// 		int                fontSize)
// {
// 	int           eNum = 0;
// 	static char   funcName[] = "NEUIK_MenuConfig_SetFont";
// 	static char * errMsgs[] = {"",       // [0] no error
// 		"Failure in GetFontSet().",      // [1]
// 		"Failure in FontSet_GetFont().", // [2]
// 	};

// 	mCfg->fontSet = NEUIK_GetFontSet(fontName);
// 	if (mCfg->fontSet == NULL)
// 	{
// 		eNum = 1;
// 		goto out;
// 	}

// 	if (NEUIK_FontSet_GetFont(mCfg->fontSet, mCfg->fontSize, 0, 0) == NULL)
// 	{
// 		eNum = 2;
// 		goto out;
// 	}

// out:
// 	if (eNum > 0)
// 	{
// 		NEUIK_RaiseError(funcName, errMsgs[eNum]);
// 	}

// 	return eNum;
// }
