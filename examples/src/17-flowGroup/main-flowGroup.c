/* example -- flow group demonstration */
#include <NEUIK.h>

int main()
{
	int               rv   = 0;
	NEUIK_Window    * mw   = NULL;
	NEUIK_FlowGroup * fg   = NULL;
	NEUIK_Button    * btn1 = NULL;
	NEUIK_Button    * btn2 = NULL;
	NEUIK_Button    * btn3 = NULL;
	NEUIK_Button    * btn4 = NULL;
	NEUIK_Button    * btn5 = NULL;
	NEUIK_Button    * btn6 = NULL;
	NEUIK_Button    * btn7 = NULL;
	NEUIK_Button    * btn8 = NULL;
	NEUIK_Button    * btn9 = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-flowGroup");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "NEUIK example -- FlowGroup");
	NEUIK_Window_Configure(mw, "Resizable", NULL);

	NEUIK_MakeButton(&btn1, "Button #1");
	NEUIK_MakeButton(&btn2, "Button #2");
	NEUIK_MakeButton(&btn3, "Button #3");
	NEUIK_MakeButton(&btn4, "Button #4");
	NEUIK_MakeButton(&btn5, "Button #5");
	NEUIK_MakeButton(&btn6, "Button #6");
	NEUIK_MakeButton(&btn7, "Button #7");
	NEUIK_MakeButton(&btn8, "Button #8");
	NEUIK_MakeButton(&btn9, "Button #9");

	NEUIK_NewFlowGroup(&fg);
	NEUIK_Element_Configure(fg, "FillAll", "PadAll=1", NULL);
	NEUIK_Container_AddElements(fg,
		btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9, NULL);

	NEUIK_Window_SetElement(mw, fg);
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
