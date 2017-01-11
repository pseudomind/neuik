#include <stdio.h>
#include <NEUIK.h>

void cb_ConfigureWindow(
	void * window,
	void * cfgStrPtr)
{
	const char * cfgStr;

	if (window == NULL || cfgStrPtr == NULL)
	{
		printf("cb_ConfigureWindow: Invalid Args.\n");
		return;
	}

	cfgStr = (const char*)(cfgStrPtr);
	NEUIK_Window_Configure(window, cfgStr, NULL);
}

int main()
{
	int                   rv    = 0;
	NEUIK_Window        * mw    = NULL;
	NEUIK_VGroup        * vg    = NULL;
	NEUIK_ToggleButton  * tbtn1 = NULL;
	NEUIK_ToggleButton  * tbtn2 = NULL;
	NEUIK_ToggleButton  * tbtn3 = NULL;
	NEUIK_Button        * btn4  = NULL;
	NEUIK_Button        * btn5  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-windowConfigure");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "NEUIK_Example: Configure Window");

	NEUIK_MakeToggleButton(&tbtn1, "Window Resizable");
	NEUIK_Element_Configure(tbtn1, "FillAll", NULL);
	/* set the on activated callback */
	NEUIK_Element_SetCallback(tbtn1, "OnActivated", 
		cb_ConfigureWindow, "Resizable", NULL);
	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(tbtn1, "OnDeactivated", 
		cb_ConfigureWindow, "!Resizable", NULL);

	NEUIK_MakeToggleButton(&tbtn2, "Window Borderless");
	NEUIK_Element_Configure(tbtn2, "FillAll", NULL);
	/* set the on activated callback */
	NEUIK_Element_SetCallback(tbtn2, "OnActivated", 
		cb_ConfigureWindow, "Borderless", NULL);
	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(tbtn2, "OnDeactivated", 
		cb_ConfigureWindow, "!Borderless", NULL);

	NEUIK_MakeToggleButton(&tbtn3, "Window Fullscreen");
	NEUIK_Element_Configure(tbtn3, "FillAll", NULL);
	/* set the on activated callback */
	NEUIK_Element_SetCallback(tbtn3, "OnActivated", 
		cb_ConfigureWindow, "Fullscreen", NULL);
	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(tbtn3, "OnDeactivated", 
		cb_ConfigureWindow, "!Fullscreen", NULL);

	NEUIK_MakeButton(&btn4, "Window Maximize");
	NEUIK_Element_Configure(btn4, "FillAll", "VScale=2.0", NULL);
	NEUIK_Element_SetCallback(btn4, "OnClicked",
		cb_ConfigureWindow, "Maximize", NULL);

	NEUIK_MakeButton(&btn5, "Window Minimize");
	NEUIK_Element_Configure(btn5, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn5, "OnClicked",
		cb_ConfigureWindow, "Minimize", NULL);

	NEUIK_NewVGroup(&vg);
	NEUIK_Element_Configure(vg, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(vg, tbtn1, tbtn2, tbtn3, btn4, btn5, NULL);
	NEUIK_VGroup_SetVSpacing(vg, 3);

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
