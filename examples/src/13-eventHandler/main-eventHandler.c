/* NEUIK example -- custom event handler */
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


int OnScreenValue = 10;

/*----------------------------------------------------------------------------*/
/* This is a special event handling function to capture `+` or `-` key press  */
/* events.                                                                    */
/*----------------------------------------------------------------------------*/
int * eHFunc_Win(
	void            * container, 
	ptrTo_SDL_Event   ev,
	int             * captured, 
	void            * lblPtr, 
	void            * unused)
{
	int                  ctr;
	int                  doUpdate = 0;
	char                 aChar;
	char                 numStr[20];
	SDL_Event          * e        = NULL; 
	SDL_TextInputEvent * tEEv;
	NEUIK_Label        * lblVal;

	(*captured) = 0;

	e = (SDL_Event*)ev;
	switch (e->type)
	{
	case SDL_TEXTINPUT:
		tEEv  = (SDL_TextInputEvent*)(e);
		for (ctr = 0;; ctr++)
		{
			aChar = tEEv->text[ctr];
			if (aChar == '\0') break;

			switch (aChar)
			{
				case '+':
					doUpdate    = 1;
					(*captured) = 1;
					OnScreenValue++;
					break;

				case '-':
					doUpdate    = 1;
					(*captured) = 1;
					OnScreenValue--;
					break;
			}

		}

		if (doUpdate)
		{
			sprintf(numStr, "%d", OnScreenValue);
			lblVal = (NEUIK_Label*)lblPtr;
			NEUIK_Label_SetText(lblVal, numStr);
		}
	}

	/* no error */
	return 0;
}

int main()
{
	int            rv         = 0;
	NEUIK_Window * mw         = NULL;
	NEUIK_VGroup * vg0        = NULL;
	NEUIK_Label  * label_top  = NULL;
	NEUIK_Label  * label_val  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-eventHandler");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 300, 150);
	NEUIK_Window_SetTitle(mw, "NEUIK example -- EventHandler");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text label                                    */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_top, "Enter `+` or `-` chars to change the value below");
	NEUIK_MakeLabel(&label_val, "10");

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewVGroup(&vg0);
	NEUIK_Container_AddElements(vg0, label_top, label_val, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

	NEUIK_Window_SetEventHandler(mw, "After", eHFunc_Win, label_val, NULL);
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

	SDL_StartTextInput();
	NEUIK_Window_Create(mw);

	if (!NEUIK_HasErrors())
	{
		NEUIK_EventLoop(1);
	}
out:
	SDL_StopTextInput();
	if (NEUIK_HasErrors())
	{
		rv = 1;
		NEUIK_BacktraceErrors();
	}

	NEUIK_Quit();
	return rv;
}