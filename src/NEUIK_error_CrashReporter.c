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
#include <stdio.h>
#include <string.h>

#include "NEUIK.h"
#include "NEUIK_Event_internal.h"

extern int          inGUIBacktrace;
extern int          errorsOmitted;
extern const int    maxErrors;
extern const char * errorList[11];
extern const char * funcNameList[11];
extern int          errorDuplicates[11];
extern int          neuik__AppNameSet;
extern char         neuik__AppName[2048];

void NEUIK_BacktraceErrors_CLI()
{
	int eNum = 1;
	int ctr;

	printf("NEUIK Error(s) Backtrace:\n\n");

	for (ctr = maxErrors; ctr >= 0; ctr--)
	{
		if (errorList[ctr] != NULL)
		{
			printf("  [%d]  %s: %s\n", eNum, funcNameList[ctr], errorList[ctr]);
			if (errorDuplicates[ctr] > 0)
			{
				if (errorDuplicates[ctr] == 1)
				{
					printf("  ^^^  This message is repeated one time.");
				}
				else
				{
					printf("  ^^^  This message is repeated %d times.", 
						errorDuplicates[ctr]);
				}
			}
			printf("\n");

			eNum++;
		}
	}
	if (errorsOmitted > 0)
	{
		printf("%d errors were omitted.\n", errorsOmitted);
	}
}

/* This function will show the specified element when called */
void neuik__btErrors_cbFunc_SetShown(
		void *  window,
		void *  elem,
		void *  shownPtr)
{
	int shown;

	shown = *(int*)(shownPtr);

	if (!shown)
	{
		NEUIK_Element_Configure(elem, "!Show", NULL);
	}
	else
	{
		NEUIK_Element_Configure(elem, "Show", NULL);
	}
}

int NEUIK_BacktraceErrors_GUI()
{
	int             eNum    = 1;
	int             ctr     = 0;
	int             nErrs   = -1;
	int             btFail  = 1;    /* backtrace failed to display */
	int             isFalse = 0;
	int             isTrue  = 1;
	char            buf[2048];
	NEUIK_Window  * btWin   = NULL;
	NEUIK_Label   * btTitle = NULL;
	NEUIK_Label   * btMsg0  = NULL;
	NEUIK_Label   * btMsg1  = NULL;
	NEUIK_Label   * btMsg3  = NULL;
	NEUIK_VGroup  * vg      = NULL;
	NEUIK_VGroup  * vgOuter = NULL;
	NEUIK_Line    * hLn0    = NULL;
	NEUIK_Line    * hLn1    = NULL;

	NEUIK_Image   * imgAppCrashed = NULL;

	NEUIK_HGroup       * hgTitleBar     = NULL;
	NEUIK_HGroup       * hgDetails      = NULL;
	NEUIK_ToggleButton * btnDetails     = NULL;
	NEUIK_Button       * btnCopyDetails = NULL;

	NEUIK_Frame   * errFrame = NULL;
	NEUIK_VGroup  * vgErrs   = NULL;
	NEUIK_Label  ** errMsgs  = NULL;

	inGUIBacktrace = 1;
	/*------------------------------------------------------------------------*/
	/* The following must be unset if the GUI error backtrace window is to    */
	/* have any chance of working.                                            */
	/*------------------------------------------------------------------------*/
	neuik_Fatal = NEUIK_FATALERROR_NO_ERROR;

	/*------------------------------------------------------------------------*/
	/* Initialize the NEUIK library in case it wasn't already initialized.    */
	/*------------------------------------------------------------------------*/
	if(NEUIK_Init())
	{
		/*--------------------------------------------------------------------*/
		/* Failed to initialize NEUIK, fallback to CLI backtrace.             */
		/*--------------------------------------------------------------------*/
		goto out;
	}

	if (NEUIK_NewWindow(&btWin)) goto out;
	NEUIK_Window_SetTitle(btWin, "NEUIK Crash Reporter");
	NEUIK_Window_SetSize(btWin, 20, 20);
	NEUIK_Window_Configure(btWin, "Resizable", NULL);

	if (NEUIK_MakeLabel(&btTitle, "Crash Report")) goto out;
	NEUIK_Label_Configure(btTitle, "FontBold", "FontSize=18", NULL);
	NEUIK_Element_Configure(btTitle, "HFill", "HJustify=left", "VJustify=center", NULL);

	if (NEUIK_MakeImage_FromStock(&imgAppCrashed, NEUIK_STOCKIMAGE_APP_CRASHED)) goto out;

	if (NEUIK_NewHGroup(&hgTitleBar)) goto out;
	NEUIK_Element_Configure(hgTitleBar, "HFill", "PadRight=50", NULL);
	NEUIK_Container_AddElements(hgTitleBar, btTitle, imgAppCrashed, NULL);

	if (NEUIK_NewHLine(&hLn0)) goto out;
	NEUIK_Element_Configure(hLn0, "PadTop=5", "PadBottom=5", NULL);

	if (neuik__AppNameSet)
	{
		sprintf(buf, "Unfortunately the application named `%s`,",
			neuik__AppName);
	}
	else
	{
		strcpy(buf, "Unfortunately the unnamed application *** Set name using `NEUIK_SetAppName()` ***,");
	}
	if (NEUIK_MakeLabel(&btMsg0, buf)) goto out;
	NEUIK_Element_Configure(btMsg0, "HJustify=left", NULL);

	if (NEUIK_MakeLabel(&btMsg1, "has just crashed.")) goto out;
	NEUIK_Element_Configure(btMsg1, "HJustify=left", "PadBottom=10", NULL);

	if (NEUIK_MakeLabel(&btMsg3, "The details of the crash can be accessed below.")) goto out;
	NEUIK_Element_Configure(btMsg3, "HJustify=left", NULL);

	if (NEUIK_NewHLine(&hLn1)) goto out;
	NEUIK_Element_Configure(hLn1, "PadTop=5", "PadBottom=5", NULL);

	if (NEUIK_MakeToggleButton(&btnDetails, "Show Crash Details")) goto out;
	if (NEUIK_MakeButton(&btnCopyDetails, "Copy to Clipboard")) goto out;
	if (NEUIK_NewHGroup(&hgDetails)) goto out;

	NEUIK_HGroup_SetHSpacing(hgDetails, 10);
	NEUIK_Container_AddElements(hgDetails, btnDetails, btnCopyDetails, NULL); 

	/*------------------------------------------------------------------------*/
	/* Count the total number of error messages                               */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr <= maxErrors; ctr++)
	{
		if (errorList[ctr] != NULL) continue;
		nErrs = ctr;
		break;
	}
	if (nErrs == -1)
	{
		nErrs = maxErrors;
	}

	for (ctr = 0; ctr <= maxErrors; ctr++)
	{
		/*--------------------------------------------------------------------*/
		/* Add an additional line for each `occurred N times message`         */
		/*--------------------------------------------------------------------*/
		if (errorList[ctr] == NULL) break;
		if (errorDuplicates[ctr] > 0) nErrs++;
	}

	/*------------------------------------------------------------------------*/
	/* Allocate memory for the Error Message labels                           */
	/*------------------------------------------------------------------------*/
	errMsgs = (NEUIK_Label **) malloc((nErrs+1)*sizeof(NEUIK_Label *));
	if (errMsgs == NULL) goto out;
	errMsgs[nErrs] = NULL;

	if (NEUIK_NewVGroup(&vgErrs)) goto out;

	/*------------------------------------------------------------------------*/
	/* Create the various error messages and add them to the vgErrs           */
	/*------------------------------------------------------------------------*/
	for (ctr = 0; ctr <= maxErrors; ctr++)
	{
		if (errorList[ctr] == NULL) break;

		sprintf(buf, "  [%d]  %s: %s", eNum, funcNameList[ctr], errorList[ctr]);
		if (NEUIK_MakeLabel(&errMsgs[ctr], buf)) goto out;
		NEUIK_Element_Configure(errMsgs[ctr], "HJustify=left", NULL);

		NEUIK_Container_AddElement(vgErrs, errMsgs[ctr]);

		if (errorDuplicates[ctr] > 0)
		{
			if (errorDuplicates[ctr] == 1)
			{
				strcpy(buf, "  ^^^  This message is repeated one time.");
			}
			else
			{
				sprintf(buf, "  ^^^  This message is repeated %d times.", 
					errorDuplicates[ctr]);
			}
			if (NEUIK_MakeLabel(&errMsgs[ctr], buf)) goto out;
			NEUIK_Element_Configure(errMsgs[ctr], "HJustify=left", NULL);

			NEUIK_Container_AddElement(vgErrs, errMsgs[ctr]);
		}
		eNum++;
	}

	if (NEUIK_NewFrame(&errFrame)) goto out;
	NEUIK_Element_Configure(errFrame, "FillAll", "!Show", NULL);
	NEUIK_Container_SetElement(errFrame, vgErrs);

	/* set the on activated callback */
	NEUIK_Element_SetCallback(btnDetails, "OnActivated",
		neuik__btErrors_cbFunc_SetShown, errFrame, &isTrue);

	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(btnDetails, "OnDeactivated",
		neuik__btErrors_cbFunc_SetShown, errFrame, &isFalse);


	if (NEUIK_NewVGroup(&vg)) goto out;
	NEUIK_Element_Configure(vg, "HFill", NULL);
	NEUIK_Container_AddElements(vg, 
		hgTitleBar, hLn0, btMsg0, btMsg1, btMsg3, hLn1, hgDetails, NULL);

	if (NEUIK_NewVGroup(&vgOuter)) goto out;
	NEUIK_Element_Configure(vgOuter, "FillAll", "PadAll=10", "PadTop=0", NULL);
	NEUIK_VGroup_SetVSpacing(vgOuter, 5);
	NEUIK_Container_AddElements(vgOuter, vg, errFrame, NULL);

	NEUIK_Window_SetElement(btWin, vgOuter);
	NEUIK_Window_Create(btWin);

	if (NEUIK_HasErrors()) goto out;
	NEUIK_EventLoop(1);
out:
	if (!NEUIK_HasErrors()) btFail = 0;
	if (errMsgs != NULL) free(errMsgs);

	inGUIBacktrace = 0;
	return btFail;
}

void NEUIK_BacktraceErrors()
{
	/*------------------------------------------------------------------------*/
	/* Close all of the currently open windows before starting up the crash   */
	/* report window.                                                         */
	/*------------------------------------------------------------------------*/
	neuik_FreeAllWindows();

	/*------------------------------------------------------------------------*/
	/* Try to use the GUI Crash Reporter first and fall back to the CLI       */
	/* version it does not work for some reason.                              */
	/*------------------------------------------------------------------------*/
	if (NEUIK_BacktraceErrors_GUI())
	{
		/*--------------------------------------------------------------------*/
		/* The GUI backtrace failed...                                        */
		/*--------------------------------------------------------------------*/
		NEUIK_BacktraceErrors_CLI();
	}
}
