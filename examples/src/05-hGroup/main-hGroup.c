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

	sprintf(cfg_str, "BGColor=%s", (char*)color_str);
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
	NEUIK_HGroup  * hg   = NULL;
	NEUIK_Button  * btn1 = NULL;
	NEUIK_Button  * btn2 = NULL;
	NEUIK_Button  * btn3 = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-hGroup");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 640, 480);
	NEUIK_Window_SetTitle(mw, "Buttons in a HGroup.");

	NEUIK_MakeButton(&btn1, "Update Title!");
	NEUIK_Element_Configure(btn1, "FillAll", "PadTop=20", NULL);
	NEUIK_Element_SetCallback(btn1, "OnClicked", cbFunc_Btn_PushMe, NULL, NULL);

	NEUIK_MakeButton(&btn2, "Set BG Black");
	NEUIK_Element_Configure(btn2, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn2, "OnClicked", cbSetBGColor, "0,0,0,255", NULL);

	NEUIK_MakeButton(&btn3, "Set BG Blue");
	NEUIK_Element_Configure(btn3, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn3, "OnClicked", cbSetBGColor, "0,0,200,255", NULL);

	NEUIK_NewHGroup(&hg);
	NEUIK_Element_Configure(hg, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(hg, btn1, btn2, btn3, NULL);
	NEUIK_HGroup_SetHSpacing(hg, 3);

	NEUIK_Window_SetElement(mw, hg);
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
