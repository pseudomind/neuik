/* progress bar example */
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "neuik.h"
#include "error.h"
#include "Window.h"
#include "render.h"
#include "elements/Label.h"
#include "elements/VGroup.h"


int backgroundTask(
		void * elapsedLblPtr)
{
	char           timeStr[100];
	int            thisTime   = 0;
	NEUIK_Label  * elapsedLbl = NULL;

	/* this is the only background task being run */
	elapsedLbl = (NEUIK_Label *)(elapsedLblPtr);
	printf("Starting `backgroundTask\n");

	for (;;)
	{
		SDL_Delay(1000);
		thisTime += 1;
		sprintf(timeStr, "%d", thisTime);
		NEUIK_Label_SetText(elapsedLbl, timeStr);
	}

	return 0;
}


void cbFunc_OnWindowCreate(
		void *  window,
		void *  elapsedLbl,
		void *  unused)
{
	SDL_Thread    * thread = NULL;

	printf("cbFunc_OnWindowCreate called\n");

	thread = SDL_CreateThread(&backgroundTask, "backgroundTask", elapsedLbl);
	SDL_DetachThread(thread);
}


int main()
{
	NEUIK_Window       * mw         = NULL;
	NEUIK_VGroup       * vg0        = NULL;
	NEUIK_Label        * label_top  = NULL;
	NEUIK_Label        * label_elap = NULL;
	NEUIK_Label        * label_bot  = NULL;

	if(NEUIK_Init())
	{
		NEUIK_BacktraceErrors();
		return 1;
	}

	mw = NEUIK_NewWindow();
	NEUIK_Window_SetSize(mw, 300, 150);
	NEUIK_Window_SetTitle(mw, "Time since started -- example");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text label                                    */
	/*------------------------------------------------------------------------*/
	label_top = NEUIK_NewLabel("This program started");

	label_elap = NEUIK_NewLabel("0");

	label_bot = NEUIK_NewLabel("seconds ago.");

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	vg0 = NEUIK_NewVGroup();
	NEUIK_Element_AddElements(vg0, label_top, label_elap, label_bot, NULL);
	if (vg0 != NULL) vg0->VSpacing = 10;


	NEUIK_Window_SetCallback(mw, "OnCreated", cbFunc_OnWindowCreate, label_elap, NULL);
	if (mw != NULL) mw->elem = vg0;


	/*------------------------------------------------------------------------*/
	/* Check that no errors were experienced before starting the event loop   */
	/*------------------------------------------------------------------------*/
	if (NEUIK_HasErrors())
	{
		printf("Errors Ocurred before starting EventLoop...\n");
		NEUIK_BacktraceErrors();
		NEUIK_Quit();
		return 1;
	}

	NEUIK_Window_Create(mw);
	NEUIK_EventLoop(1);
	if (NEUIK_HasErrors()) NEUIK_BacktraceErrors();

	printf("Hello World!\n");


	NEUIK_Quit();
	return 1;
}
