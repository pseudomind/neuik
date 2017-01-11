/* progress bar example */
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


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
	int             rv         = 0;
	NEUIK_Window  * mw         = NULL;
	NEUIK_VGroup  * vg0        = NULL;
	NEUIK_Label   * label_top  = NULL;
	NEUIK_Label   * label_elap = NULL;
	NEUIK_Label   * label_bot  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-timeSinceStarted");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 300, 150);
	NEUIK_Window_SetTitle(mw, "Time since started -- example");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text label                                    */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_top, "This program started");
	NEUIK_MakeLabel(&label_elap, "0");
	NEUIK_MakeLabel(&label_bot, "seconds ago.");

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewVGroup(&vg0);
	NEUIK_Container_AddElements(vg0, label_top, label_elap, label_bot, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

	NEUIK_Window_SetCallback(mw, "OnCreated", cbFunc_OnWindowCreate, label_elap, NULL);
	NEUIK_Window_SetElement(mw, vg0);

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

	if (!NEUIK_HasErrors())
	{
		NEUIK_EventLoop(1);
	}
out:
	if (NEUIK_HasErrors())
	{
		rv = 1;
		NEUIK_BacktraceErrors();
	}

	NEUIK_Quit();
	return rv;
}
