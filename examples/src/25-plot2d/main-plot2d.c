#include <NEUIK.h>

int main()
{
	int            rv  = 0;
	NEUIK_Window * mw  = NULL;
	NEUIK_Plot2D * plt = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-plot2D");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 640, 480);	
	NEUIK_Window_SetTitle(mw, "Plot2D Element");
	NEUIK_Window_Configure(mw, "Resizable", NULL);

	NEUIK_NewPlot2D(&plt);
	NEUIK_Plot_SetTitle(plt, "New Title");
	NEUIK_Element_Configure(plt, "FillAll", "PadAll=10", NULL);

	NEUIK_Window_SetElement(mw, plt);

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
