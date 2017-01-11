/* example -- hide and show NEUIK elements using a combobox */
#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


/* This function will show the specified element when called */
void cbFunc_OnActivated_Show(
		void *  window,
		void *  elem,
		void *  unused)
{
	NEUIK_Element_Configure(elem, "Show", NULL);
}

/* This function will hide the specified element when called */
void cbFunc_OnDeactivated_Hide(
		void *  window,
		void *  elem,
		void *  unused)
{
	NEUIK_Element_Configure(elem, "!Show", NULL);
}


int main()
{
	NEUIK_Window     * mw      = NULL;
	NEUIK_VGroup     * vg0     = NULL;
	NEUIK_Label      * label_1 = NULL;
	NEUIK_PopupMenu  * puMenu  = NULL;

	if(NEUIK_Init())
	{
		NEUIK_BacktraceErrors();
		return 1;
	}

	mw = NEUIK_NewWindow();
	NEUIK_Window_SetSize(mw, 300, 150);
	NEUIK_Window_SetTitle(mw, "Show/Hide Elements -- example");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text labels and buttons                       */
	/*------------------------------------------------------------------------*/
	label_1 = NEUIK_NewLabel("PopupMenu");

	puMenu = NEUIK_NewPopupMenu();
	NEUIK_PopupMenu_AddItem(puMenu, "Copy", NULL);
	NEUIK_PopupMenu_AddItem(puMenu, "Cut", NULL);
	NEUIK_PopupMenu_AddItem(puMenu, "Paste", NULL);

	vg0 = NEUIK_NewVGroup();
	NEUIK_Element_AddElements(vg0, label_1, puMenu, NULL);
	NEUIK_Window_SetElement(mw, vg0);

	/*------------------------------------------------------------------------*/
	/* Check that no errors were experienced before starting the event loop   */
	/*------------------------------------------------------------------------*/
	if (NEUIK_HasErrors())
	{
		printf("Errors Ocurred before starting EventLoop...\n");
		NEUIK_BacktraceErrors();
		NEUIK_Quit();
		return 1;
	}

	NEUIK_Window_Create(mw);
	NEUIK_EventLoop(1);
	if (NEUIK_HasErrors()) NEUIK_BacktraceErrors();

	NEUIK_Quit();
	return 0;
}
