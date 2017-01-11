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
	NEUIK_Window    * mw          = NULL;
	NEUIK_VGroup    * vg0         = NULL;
	NEUIK_VGroup    * vgSHLabels  = NULL;
	NEUIK_VGroup    * vgSHButtons = NULL;
	NEUIK_HGroup    * shItems     = NULL;
	NEUIK_ComboBox  * shCBox      = NULL;
	NEUIK_Label     * label_1     = NULL;
	NEUIK_Label     * label_2     = NULL;
	NEUIK_Label     * label_3     = NULL;
	NEUIK_Button    * btn_1       = NULL;
	NEUIK_Button    * btn_2       = NULL;
	NEUIK_Button    * btn_3       = NULL;
	NEUIK_Frame     * shFrame     = NULL;

	if(NEUIK_Init())
	{
		NEUIK_BacktraceErrors();
		return 1;
	}

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 300, 150);
	NEUIK_Window_SetTitle(mw, "Show/Hide Elements -- example");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text labels and buttons                       */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_1, "These are labels");
	NEUIK_MakeLabel(&label_2, "within a single");
	NEUIK_MakeLabel(&label_3, "vertical group.");

	NEUIK_NewVGroup(&vgSHLabels);
	NEUIK_Element_Configure(vgSHLabels, "!Show", NULL);
	NEUIK_Container_AddElements(vgSHLabels, label_1, label_2, label_3, NULL);

	// shCBox = NEUIK_NewComboBox("Show Labels", "Show Buttons", NULL);
	NEUIK_MakeComboBox(&shCBox, "Show Labels");

	// shBtn_Labels  = NEUIK_NewToggleButton("Show Labels");

	// /* set the on activated callback */
	// NEUIK_Element_SetCallback(shBtn_Labels, "OnActivated", 
	// 	cbFunc_OnActivated_Show, vgSHLabels, NULL);

	// /* set the on deActivated callback */
	// NEUIK_Element_SetCallback(shBtn_Labels, "OnDeactivated", 
	// 	cbFunc_OnDeactivated_Hide, vgSHLabels, NULL);

	NEUIK_MakeButton(&btn_1, "Button 1");
	NEUIK_MakeButton(&btn_2, "Button 2");
	NEUIK_MakeButton(&btn_3, "Button 3");

	printf("btn1 %ld, btn2 %ld, btn3 %ld\n", (long)btn_1, (long)btn_2, (long)btn_3);
	NEUIK_NewVGroup(&vgSHButtons);
	NEUIK_Element_Configure(vgSHButtons, "!Show", NULL);
	NEUIK_Container_AddElements(vgSHButtons, btn_1, btn_2, btn_3, NULL);

	// shBtn_Buttons = NEUIK_NewToggleButton("Show Buttons");

	// /* set the on activated callback */
	// NEUIK_Element_SetCallback(shBtn_Buttons, "OnActivated", 
	// 	cbFunc_OnActivated_Show, vgSHButtons, NULL);

	// /* set the on deActivated callback */
	// NEUIK_Element_SetCallback(shBtn_Buttons, "OnDeactivated", 
	// 	cbFunc_OnDeactivated_Hide, vgSHButtons, NULL);


	NEUIK_NewHGroup(&shItems);
	NEUIK_Container_AddElements(shItems, vgSHLabels, vgSHButtons, NULL);

	NEUIK_NewFrame(&shFrame);
	NEUIK_Container_SetElement(shFrame, shItems);

	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "VJustify=top", NULL);
	NEUIK_Container_AddElements(vg0, shCBox, shFrame, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

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
