#include <stdio.h>
#include <NEUIK.h>


void cbFunc_SetBGColor_Black(
	void *window)
{
	NEUIK_WindowConfig *cfg;
	NEUIK_Color clr = {0, 0, 0, 255};

	if (window == NULL)
	{
		printf("Unable to set BGColor: *Window is NULL.\n");
		return;
	}
	cfg = NEUIK_Window_GetConfig((NEUIK_Window*)(window));
	if (cfg == NULL)
	{
		printf("Unable to set BGColor: *WindowConfig is NULL.\n");
		return;
	}
	NEUIK_WindowConfig_SetBGColor(cfg, clr);
}

void cbFunc_SetBGColor_Blue(
	void *window)
{
	NEUIK_WindowConfig *cfg;
	NEUIK_Color clr = {0, 0, 200, 255};

	if (window == NULL)
	{
		printf("Unable to set BGColor: *Window is NULL.\n");
		return;
	}
	cfg = NEUIK_Window_GetConfig((NEUIK_Window*)(window));
	if (cfg == NULL)
	{
		printf("Unable to set BGColor: *WindowConfig is NULL.\n");
		return;
	}
	NEUIK_WindowConfig_SetBGColor(cfg, clr);
}

void cbFunc_SetBGColor_White(
	void *window)
{
	NEUIK_WindowConfig *cfg;
	NEUIK_Color clr = {255, 255, 255, 255};

	if (window == NULL)
	{
		printf("Unable to set BGColor: *Window is NULL.\n");
		return;
	}
	cfg = NEUIK_Window_GetConfig((NEUIK_Window*)(window));
	if (cfg == NULL)
	{
		printf("Unable to set BGColor: *WindowConfig is NULL.\n");
		return;
	}
	NEUIK_WindowConfig_SetBGColor(cfg, clr);
}

int main()
{
	NEUIK_Window       *mw     = NULL;
	NEUIK_MainMenu     *mmenu;
	NEUIK_Menu         *mItem1 = NULL;
	NEUIK_Menu         *mItem2 = NULL;
	NEUIK_Menu         *mItem3 = NULL;
	NEUIK_Menu         *mItem4 = NULL;
	NEUIK_MenuItem     *ms1_1  = NULL;
	NEUIK_MenuItem     *ms1_2  = NULL;
	NEUIK_MenuItem     *ms1_3  = NULL;
	NEUIK_MenuItem     *ms1_4  = NULL;
	NEUIK_MenuItem     *ms1_5  = NULL;
	NEUIK_MenuItem     *ms2_1  = NULL;
	NEUIK_MenuItem     *ms2_2  = NULL;
	NEUIK_MenuItem     *ms2_3  = NULL;
	NEUIK_MenuItem     *ms3_1  = NULL;
	NEUIK_MenuItem     *ms3_2  = NULL;
	NEUIK_MenuItem     *ms3_3  = NULL;


	if(NEUIK_Init())
	{
		NEUIK_BacktraceErrors();
		return 1;
	}

	mw = NEUIK_NewWindow();
	NEUIK_Window_SetTitle(mw, "Blank Window Demo");

	/*------------------------------------------------------------------------*/

	mItem1 = NEUIK_NewMenu("File");
	ms1_1 = NEUIK_NewMenuItem("New File");
	ms1_2 = NEUIK_NewMenuItem("Open");
	ms1_3 = NEUIK_NewMenuItem("Open Recent");
	ms1_4 = NEUIK_NewMenuItem("Save");
	ms1_5 = NEUIK_NewMenuItem("Save As");
	NEUIK_Menu_AddMenuItem(mItem1, ms1_1);
	NEUIK_Menu_AddMenuItem(mItem1, ms1_2);
	NEUIK_Menu_AddMenuItem(mItem1, ms1_3);
	NEUIK_Menu_AddMenuItem(mItem1, ms1_4);
	NEUIK_Menu_AddMenuItem(mItem1, ms1_5);

	mItem2 = NEUIK_NewMenu("Edit");
	ms2_1 = NEUIK_NewMenuItem("Copy");
	ms2_2 = NEUIK_NewMenuItem("Cut");
	ms2_3 = NEUIK_NewMenuItem("Paste");
	NEUIK_Menu_AddMenuItem(mItem2, ms2_1);
	NEUIK_Menu_AddMenuItem(mItem2, ms2_2);
	NEUIK_Menu_AddMenuItem(mItem2, ms2_3);
	mItem2->selected = 1;
	ms2_2->selected = 1;

	mItem3 = NEUIK_NewMenu("Set BGColor");
	ms3_1 = NEUIK_NewMenuItem("Black");
	NEUIK_MenuItem_SetCallbackFunc(ms3_1, cbFunc_SetBGColor_Black, NULL, NULL);
	ms3_2 = NEUIK_NewMenuItem("White");
	NEUIK_MenuItem_SetCallbackFunc(ms3_2, cbFunc_SetBGColor_White, NULL, NULL);
	ms3_3 = NEUIK_NewMenuItem("Blue");
	NEUIK_MenuItem_SetCallbackFunc(ms3_3, cbFunc_SetBGColor_Blue, NULL, NULL);
	NEUIK_Menu_AddMenuItem(mItem3, ms3_1);
	NEUIK_Menu_AddMenuItem(mItem3, ms3_2);
	NEUIK_Menu_AddMenuItem(mItem3, ms3_3);

	mItem4 = NEUIK_NewMenu("Find");

	mmenu = NEUIK_NewMainMenu(mw->win, NULL, 1);
	mmenu->isActive = 1;
	NEUIK_MainMenu_AddMenu(mmenu, mItem1);
	NEUIK_MainMenu_AddMenu(mmenu, mItem2);
	NEUIK_MainMenu_AddMenu(mmenu, mItem3);
	NEUIK_MainMenu_AddMenu(mmenu, mItem4);

	NEUIK_Window_SetMainMenu(mw, mmenu);
	/*------------------------------------------------------------------------*/

	NEUIK_Window_Create(mw);
	NEUIK_EventLoop(1);
	if (NEUIK_HasErrors()) NEUIK_BacktraceErrors();

	printf("Hello World!\n");


	NEUIK_Quit();
	return 1;
}
