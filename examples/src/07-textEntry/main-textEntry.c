#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


int main()
{
	int               rv           = 0;
	NEUIK_Window    * mw           = NULL;
	NEUIK_VGroup    * vg0          = NULL; /* contains the display and horiz. group */
	NEUIK_HGroup    * hg0          = NULL; /* JUSTIFY_LEFT   */
	NEUIK_HGroup    * hg1          = NULL; /* JUSTIFY_CENTER */
	NEUIK_HGroup    * hg2          = NULL; /* JUSTIFY_RIGHT  */
	NEUIK_Label     * label_left   = NULL;
	NEUIK_Label     * label_center = NULL;
	NEUIK_Label     * label_right  = NULL;
	NEUIK_TextEntry * te_left      = NULL;
	NEUIK_TextEntry * te_center    = NULL;
	NEUIK_TextEntry * te_right     = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-textentry");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "Justification/TextEntry Example");


	/*------------------------------------------------------------------------*/
	/* Create and configure the text labels                                   */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_left, "JUSTIFY_LEFT:");
	NEUIK_Element_Configure(label_left, "FillAll", "HJustify=left", NULL);

	NEUIK_MakeLabel(&label_center, "JUSTIFY_CENTER:");
	NEUIK_Element_Configure(label_center, "FillAll", "HJustify=center", NULL);

	NEUIK_MakeLabel(&label_right, "JUSTIFY_RIGHT:");
	NEUIK_Element_Configure(label_right, "FillAll", "HJustify=right", NULL);

	/*------------------------------------------------------------------------*/
	/* Create and configure the text entry boxes                              */
	/*------------------------------------------------------------------------*/
	NEUIK_NewTextEntry(&te_left);
	NEUIK_Element_Configure(te_left, "FillAll", "HJustify=left", NULL);

	NEUIK_NewTextEntry(&te_center);
	NEUIK_Element_Configure(te_center, "FillAll", NULL);
	NEUIK_TextEntry_Configure(te_center, "HJustify=center", NULL);

	NEUIK_NewTextEntry(&te_right);
	NEUIK_Element_Configure(te_right, "FillAll", NULL);
	NEUIK_TextEntry_Configure(te_right, "HJustify=right", NULL);


	/*------------------------------------------------------------------------*/
	/* Create and load the items into horizontal groups                       */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHGroup(&hg0);
	NEUIK_Element_Configure(hg0, "FillAll", NULL);
	NEUIK_Container_AddElements(hg0, label_left, te_left, NULL);
	NEUIK_HGroup_SetHSpacing(hg0, 3);

	NEUIK_NewHGroup(&hg1);
	NEUIK_Element_Configure(hg1, "FillAll", NULL);
	NEUIK_Container_AddElements(hg1, label_center, te_center, NULL);
	NEUIK_HGroup_SetHSpacing(hg1, 3);

	NEUIK_NewHGroup(&hg2);
	NEUIK_Element_Configure(hg2, "FillAll", NULL);
	NEUIK_Container_AddElements(hg2, label_right, te_right, NULL);
	NEUIK_HGroup_SetHSpacing(hg2, 3);

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "HFill", "PadAll=5", NULL);
	NEUIK_Container_AddElements(vg0, hg0, hg1, hg2, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 3);

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
