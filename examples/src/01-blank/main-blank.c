#include <stdio.h>
#include <NEUIK.h>


int main()
{
	NEUIK_Window *mw = NULL;

	if(NEUIK_Init())
	{
		NEUIK_BacktraceErrors();
		return 1;
	}

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "Blank Window Demo");
	NEUIK_Window_Create(mw);

	NEUIK_EventLoop(1);
	if (NEUIK_HasErrors()) {
		NEUIK_BacktraceErrors();
	}

	NEUIK_Quit();
	return 1;
}
