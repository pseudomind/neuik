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
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string.h>

#include "neuik_internal.h"
#include "neuik_classes.h"
#include "NEUIK_neuik.h"
#include "NEUIK_error.h"

int           neuik__isInitialized = 0;
neuik_SetID   neuik__SetID_NEUIK   = -1;
int           neuik__AppNameSet    = 0;
char          neuik__AppName[2048];

int NEUIK_Init()
{
	int           eNum  = 0;
	int           rvErr = 0;
	static char   funcName[] = "NEUIK_Init";
	static char * errMsgs[] = {"",          // [0] no error
		"Failed to Initialize SDL2.",       // [1]
		"Failed to Initialize SDL2_ttf.",   // [2]
		"Failed to Initialize SDL2_image.", // [3]
		"Failed to Register Class Set.",    // [4]
	};

	if (!neuik__isInitialized)
	{
		/*--------------------------------------------------------------------*/
		/* Only initialize if not already initialized                         */
		/*--------------------------------------------------------------------*/

		/*--------------------------------------------------------------------*/
		/* Initialize the requisite SDL libraries                             */
		/*--------------------------------------------------------------------*/
		if (SDL_Init(SDL_INIT_VIDEO))
		{
			NEUIK_RaiseError(funcName, SDL_GetError());
			eNum = 1;
			goto out;
		}
		if (TTF_Init() ==  -1)
		{
			NEUIK_RaiseError(funcName, TTF_GetError());
			eNum = 2;
			goto out;
		}
		if (IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF) ==  -1)
		{
			NEUIK_RaiseError(funcName, IMG_GetError());
			eNum = 3;
			goto out;
		}

		/*--------------------------------------------------------------------*/
		/* Register the "NEUIK" class set                                     */
		/*--------------------------------------------------------------------*/
		rvErr = neuik_RegisterClassSet("NEUIK", "Base NEUIK Object Set", &neuik__Set_NEUIK);
		if (rvErr)
		{
			eNum = 4;
			goto out;
		}

		neuik__isInitialized = 1;

		/*--------------------------------------------------------------------*/
		/* Register the "NEUIK" set classes                                   */
		/*--------------------------------------------------------------------*/
		neuik_RegisterClass_WindowConfig();
		neuik_RegisterClass_Window();
		neuik_RegisterClass_Element();
		neuik_RegisterClass_Container();
		neuik_RegisterClass_Canvas();
		neuik_RegisterClass_CelGroup();
		neuik_RegisterClass_HGroup();
		neuik_RegisterClass_VGroup();
		neuik_RegisterClass_FlowGroup();
		neuik_RegisterClass_GridLayout();
		neuik_RegisterClass_Image();
		neuik_RegisterClass_ImageConfig();
		neuik_RegisterClass_ListGroup();
		neuik_RegisterClass_ListRow();
		neuik_RegisterClass_Frame();
		neuik_RegisterClass_ButtonConfig();
		neuik_RegisterClass_Button();
		neuik_RegisterClass_ComboBoxConfig();
		neuik_RegisterClass_ComboBox();
		neuik_RegisterClass_ToggleButtonConfig();
		neuik_RegisterClass_ToggleButton();
		neuik_RegisterClass_LabelConfig();
		neuik_RegisterClass_Label();
		neuik_RegisterClass_Fill();
		neuik_RegisterClass_Line();
		neuik_RegisterClass_TextEditConfig();
		neuik_RegisterClass_TextEdit();
		neuik_RegisterClass_TextEntryConfig();
		neuik_RegisterClass_TextEntry();
		neuik_RegisterClass_Transformer();
		neuik_RegisterClass_Plot();
		neuik_RegisterClass_Plot2D();
		neuik_RegisterClass_ProgressBarConfig();
		neuik_RegisterClass_ProgressBar();
		neuik_RegisterClass_Stack();


		neuik_RegisterClass_MaskMap();
	}
out:
	if (eNum != 0)
	{
		NEUIK_RaiseError(funcName, errMsgs[eNum]);
		eNum = 1;
	}

	return eNum;
}

void NEUIK_Quit()
{
	if (neuik__isInitialized)
	{
		IMG_Quit();
		TTF_Quit();
		SDL_Quit();
	}
	neuik__isInitialized = 0;
}

int NEUIK_SetAppName(
	const char * appName)
{
	int rvErr = 0;

	if (!neuik__isInitialized || appName == NULL) 
	{
		rvErr = 1;
		goto out;
	}

	if (strlen(appName) > 2047)
	{
		rvErr = 1;
		goto out;
	}

	strcpy(neuik__AppName, appName);
	neuik__AppNameSet = 1;
out:
	return rvErr;
}
