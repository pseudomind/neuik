#include <stdio.h>
#include <NEUIK.h>

int main()
{
	int              rv    = 0;
	NEUIK_Window   * mw    = NULL;
	NEUIK_CelGroup * cg    = NULL;
	NEUIK_Button   * btnA  = NULL;
	NEUIK_Button   * btnB  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-celgroup");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "NEUIK - CelGroup example");

	NEUIK_MakeButton(&btnA, "This button contains a very very very long string!!!");
	NEUIK_MakeButton(&btnB, "A small Button...");

	NEUIK_NewCelGroup(&cg);
	NEUIK_Container_AddElements(cg, btnA, btnB, NULL);

	NEUIK_Window_SetElement(mw, cg);

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
