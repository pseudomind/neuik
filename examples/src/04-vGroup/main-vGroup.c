#include <stdio.h>
#include <NEUIK.h>

void cbSetBGColor(
	void * window,
	void * color_str_ptr)
{
	char * color_str;
	char   cfg_str[50];

	color_str = (char*)color_str_ptr;

	if (window == NULL)
	{
		printf("Unable to set BGColor: *Window is NULL.\n");
		return;
	}
	if (color_str == NULL)
	{
		printf("Unable to set BGColor: A color string is required.\n");
		return;
	}

	sprintf(cfg_str, "BGColor=%s", color_str);
	NEUIK_Window_Configure((NEUIK_Window*)window, cfg_str, NULL);
}

void cbFunc_Btn_PushMe(
	void *window)
{
	printf("Button `Push Me!` pushed!!!\n");
	NEUIK_Window_SetTitle(window, "The Button was pushed!");
}

int main()
{
	int             rv   = 0;
	NEUIK_Window  * mw   = NULL;
	NEUIK_VGroup  * vg   = NULL;
	NEUIK_Label   * lbl  = NULL;
	NEUIK_Button  * btn1 = NULL;
	NEUIK_Button  * btn2 = NULL;
	NEUIK_Button  * btn3 = NULL;
	NEUIK_Button  * btn4 = NULL;
	NEUIK_Button  * btn5 = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-vGroup");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "Buttons in a VGroup.");

	NEUIK_MakeLabel(&lbl, "Here are a bunch of Buttons:");

	NEUIK_MakeButton(&btn1, "Update Title!");
	NEUIK_Element_Configure(btn1, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn1, "OnClicked", cbFunc_Btn_PushMe, NULL, NULL);

	NEUIK_MakeButton(&btn2, "Set BG Black");
	NEUIK_Element_Configure(btn2, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn2, "OnClicked", cbSetBGColor, "0,0,0,255", NULL);

	NEUIK_MakeButton(&btn3, "Set BG Blue");
	NEUIK_Element_Configure(btn3, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn3, "OnClicked", cbSetBGColor, "0,0,200,255", NULL);

	NEUIK_MakeButton(&btn4, "Set BG White");
	NEUIK_Element_Configure(btn4, "FillAll", "VScale=2.0", NULL);
	NEUIK_Element_SetCallback(btn4, "OnClicked", cbSetBGColor, "255,255,255,255", NULL);

	NEUIK_MakeButton(&btn5, "Set BG Pink");
	NEUIK_Element_Configure(btn5, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn5, "OnClicked", cbSetBGColor, "255,200,200,255", NULL);

	NEUIK_NewVGroup(&vg);
	NEUIK_Element_Configure(vg, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(vg, lbl, btn1, btn2, btn3, btn4, btn5, NULL);
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
