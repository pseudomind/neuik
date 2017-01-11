/* example -- hide and show NEUIK elements */
#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


void cb_SetShown(
	void *  window,
	void *  elem,
	void *  isShown)
{
	if (window == NULL)
	{
		printf("cb_SetShown: *Window is NULL.\n");
		return;
	}
	if (elem == NULL)
	{
		printf("cb_SetShown: Elem is NULL.\n");
		return;
	}
	if (isShown == NULL)
	{
		printf("cb_SetShown: isShown Not provided.\n");
		return;
	}

	if (*(int*)(isShown))
	{
		NEUIK_Element_Configure(elem, "Show", NULL);
	}
	else
	{
		NEUIK_Element_Configure(elem, "!Show", NULL);
	}
}


int main()
{
	int             rv            = 0;
	int             showTrue      = 1;
	int             showFalse     = 0;
	NEUIK_Window  * mw            = NULL;
	NEUIK_VGroup  * vg0           = NULL;
	NEUIK_VGroup  * vgSHLabels    = NULL;
	NEUIK_VGroup  * vgSHButtons   = NULL;
	NEUIK_HGroup  * hgBtns        = NULL;
	NEUIK_HGroup  * shItems       = NULL;

	NEUIK_ToggleButton  * shBtn_Labels  = NULL;
	NEUIK_ToggleButton  * shBtn_Buttons = NULL;

	NEUIK_Label   * label_1       = NULL;
	NEUIK_Label   * label_2       = NULL;
	NEUIK_Label   * label_3       = NULL;

	NEUIK_Button  * btn_1         = NULL;
	NEUIK_Button  * btn_2         = NULL;
	NEUIK_Button  * btn_3         = NULL;

	NEUIK_Frame   * shFrame       = NULL;


	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-hideAndShow");

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

	NEUIK_MakeToggleButton(&shBtn_Labels, "Show Labels");

	/* set the on activated callback */
	NEUIK_Element_SetCallback(shBtn_Labels, "OnActivated", 
		cb_SetShown, vgSHLabels, &showTrue);

	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(shBtn_Labels, "OnDeactivated", 
		cb_SetShown, vgSHLabels, &showFalse);

	NEUIK_MakeButton(&btn_1, "Button 1");
	NEUIK_MakeButton(&btn_2, "Button 2");
	NEUIK_MakeButton(&btn_3, "Button 3");

	printf("btn1 %ld, btn2 %ld, btn3 %ld\n", (long)btn_1, (long)btn_2, (long)btn_3);
	NEUIK_NewVGroup(&vgSHButtons);
	NEUIK_Element_Configure(vgSHButtons, "!Show", NULL);
	NEUIK_Container_AddElements(vgSHButtons, btn_1, btn_2, btn_3, NULL);

	NEUIK_MakeToggleButton(&shBtn_Buttons, "Show Buttons");

	/* set the on activated callback */
	NEUIK_Element_SetCallback(shBtn_Buttons, "OnActivated", 
		cb_SetShown, vgSHButtons, &showTrue);

	/* set the on deActivated callback */
	NEUIK_Element_SetCallback(shBtn_Buttons, "OnDeactivated", 
		cb_SetShown, vgSHButtons, &showFalse);


	NEUIK_NewHGroup(&shItems);
	NEUIK_Container_AddElements(shItems, vgSHLabels, vgSHButtons, NULL);

	NEUIK_NewFrame(&shFrame);
	NEUIK_Container_SetElement(shFrame, shItems);

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHGroup(&hgBtns);
	NEUIK_Container_AddElements(hgBtns, shBtn_Labels, shBtn_Buttons, NULL);

	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "VJustify=top", NULL);
	NEUIK_Container_AddElements(vg0, hgBtns, shFrame, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

	NEUIK_Window_SetElement(mw, vg0);
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
