#include <NEUIK.h>

int main()
{
	int            rv   = 0;
	NEUIK_Window * mw   = NULL;
	NEUIK_Canvas * cnvs = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-canvas");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 640, 480);	
	NEUIK_Window_SetTitle(mw, "Canvas Element");

	NEUIK_NewCanvas(&cnvs);
	NEUIK_Element_Configure(cnvs, "FillAll", "PadAll=10", NULL);
	NEUIK_Canvas_SetDrawColor(cnvs, 255, 255, 255, 255);
	NEUIK_Canvas_Fill(cnvs);
	NEUIK_Canvas_SetDrawColor(cnvs, 255, 0, 0, 255);
	NEUIK_Canvas_MoveTo(cnvs, 0, 0);
	NEUIK_Canvas_DrawPoint(cnvs);
	NEUIK_Canvas_MoveTo(cnvs, 20, 20);
	NEUIK_Canvas_DrawPoint(cnvs);
	NEUIK_Canvas_MoveTo(cnvs, 40, 40);
	NEUIK_Canvas_DrawPoint(cnvs);
	NEUIK_Canvas_MoveTo(cnvs, 60, 60);
	NEUIK_Canvas_DrawPoint(cnvs);
	NEUIK_Canvas_MoveTo(cnvs, 1, 0);
	NEUIK_Canvas_DrawLine(cnvs, 200, 0);
	NEUIK_Canvas_MoveTo(cnvs, 100, 100);
	NEUIK_Canvas_SetTextSize(cnvs, 11);
	NEUIK_Canvas_DrawText(cnvs, "HELLO World");

	NEUIK_Window_SetElement(mw, cnvs);

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
