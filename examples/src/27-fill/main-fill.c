#include <stdio.h>
#include <NEUIK.h>

int main()
{
	int            rv    = 0;
	NEUIK_Window * mw    = NULL;
	NEUIK_VGroup * vg    = NULL;
	NEUIK_Button * btn1  = NULL;
	NEUIK_Fill   * vFill = NULL;
	NEUIK_Button * btn2  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-fill");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "The Fill Element...");

	NEUIK_MakeButton(&btn1, "Top Button");
	NEUIK_NewVFill(&vFill); /* this will expand to occupy vertical space only */
	NEUIK_MakeButton(&btn2, "Bottom Button");

	NEUIK_NewVGroup(&vg);
	NEUIK_Element_Configure(vg, "FillAll", NULL);
	NEUIK_Container_AddElements(vg, btn1, vFill, btn2, NULL);

	NEUIK_Window_SetElement(mw, vg);
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
