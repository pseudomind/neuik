/* example -- listgroup demonstration */
#include <stdio.h>
#include <NEUIK.h>


void cb_RowActivated(
	void *  window,
	void *  label)
{
	if (window == NULL)
	{
		printf("cb_RowActivated: *Window is NULL.\n");
		return;
	}
	if (label == NULL)
	{
		printf("cb_RowActivated: Label is NULL.\n");
		return;
	}

	printf("%s\n", NEUIK_Label_GetText(label));
}


int main()
{
	int               rv   = 0;
	NEUIK_Window    * mw   = NULL;
	NEUIK_ListGroup * lg   = NULL;
	NEUIK_Label     * lbl1 = NULL;
	NEUIK_Label     * lbl2 = NULL;
	NEUIK_Label     * lbl3 = NULL;
	NEUIK_Label     * lbl4 = NULL;
	NEUIK_Label     * lbl5 = NULL;
	NEUIK_ListRow   * row1 = NULL;
	NEUIK_ListRow   * row2 = NULL;
	NEUIK_ListRow   * row3 = NULL;
	NEUIK_ListRow   * row4 = NULL;
	NEUIK_ListRow   * row5 = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("NEUIK Example: ListGroup");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "NEUIK Example: ListGroup");
	NEUIK_Window_Configure(mw, "Resizable", NULL);

	NEUIK_MakeLabel(&lbl1, "Text Label #1");
	NEUIK_MakeLabel(&lbl2, "Text Label #2");
	NEUIK_MakeLabel(&lbl3, "Text Label #3");
	NEUIK_MakeLabel(&lbl4, "Text Label #4");
	NEUIK_MakeLabel(&lbl5, "Text Label #5");

	NEUIK_NewListRow(&row1);
	NEUIK_NewListRow(&row2);
	NEUIK_NewListRow(&row3);
	NEUIK_NewListRow(&row4);
	NEUIK_NewListRow(&row5);

	NEUIK_Container_AddElements(row1, lbl1, NULL);
	NEUIK_Element_SetCallback(row1, "OnActivated", cb_RowActivated, lbl1, NULL);

	NEUIK_Container_AddElements(row2, lbl2, NULL);
	NEUIK_Element_SetCallback(row2, "OnActivated", cb_RowActivated, lbl2, NULL);

	NEUIK_Container_AddElements(row3, lbl3, NULL);
	NEUIK_Element_SetCallback(row3, "OnActivated", cb_RowActivated, lbl3, NULL);

	NEUIK_Container_AddElements(row4, lbl4, NULL);
	NEUIK_Element_SetCallback(row4, "OnActivated", cb_RowActivated, lbl4, NULL);

	NEUIK_Container_AddElements(row5, lbl5, NULL);
	NEUIK_Element_SetCallback(row5, "OnActivated", cb_RowActivated, lbl5, NULL);

	NEUIK_NewListGroup(&lg);
	NEUIK_Element_Configure(lg, "FillAll", "PadAll=1", NULL);
	NEUIK_ListGroup_AddRows(lg, row1, row2, row3, row4, row5, NULL);

	NEUIK_Window_SetElement(mw, lg);
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
