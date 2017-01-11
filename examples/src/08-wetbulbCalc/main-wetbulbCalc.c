#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


void cbFunc_TE_Act(
	void * window,
	void * teInp,
	void * teOutp)
{
	NEUIK_TextEntry *  outp    = NULL;
	NEUIK_TextEntry ** teArry  = NULL;

	outp   = (NEUIK_TextEntry *)teOutp;
	teArry = (NEUIK_TextEntry **)teInp;


	if (NEUIK_TextEntry_GetText(teArry[0]) != NULL && 
		NEUIK_TextEntry_GetText(teArry[1]) != NULL && 
		NEUIK_TextEntry_GetText(teArry[2]) != NULL)
	{
		if ((*NEUIK_TextEntry_GetText(teArry[0])) != '\0' &&
			(*NEUIK_TextEntry_GetText(teArry[1])) != '\0' &&
			(*NEUIK_TextEntry_GetText(teArry[2])) != '\0'
		)
			NEUIK_TextEntry_SetText(outp, "Do a Calculation...");
	}
}


int main()
{
	int                rv         = 0;
	NEUIK_Window     * mw         = NULL;

	NEUIK_VGroup     * vg0        = NULL; /* contains the display and horiz. group */
	NEUIK_HGroup     * hg0        = NULL; /* T_DB */
	NEUIK_HGroup     * hg1        = NULL; /* P    */
	NEUIK_HGroup     * hg2        = NULL; /* rH   */
	NEUIK_HGroup     * hg3        = NULL; /* T_WB */

	NEUIK_Label      * label_T_DB = NULL;
	NEUIK_Label      * label_P    = NULL;
	NEUIK_Label      * label_rH   = NULL;
	NEUIK_Label      * label_T_WB = NULL;

	NEUIK_TextEntry  * te_T_DB    = NULL;
	NEUIK_TextEntry  * te_P       = NULL;
	NEUIK_TextEntry  * te_rH      = NULL;
	NEUIK_TextEntry  * te_T_WB    = NULL;

	NEUIK_Line       * hlineRes   = NULL;

	NEUIK_Frame      * oFrame     = NULL;

	NEUIK_Element      teInp[3]   = {NULL, NULL, NULL};

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-wetbulbTempCalculator");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "Wetbulb Temperature Calculator");


	/*------------------------------------------------------------------------*/
	/* Create and configure the text labels                                   */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_T_DB, "T_DB (F) :");
	NEUIK_Element_Configure(label_T_DB, "FillAll", "HJustify=right", NULL);

	NEUIK_MakeLabel(&label_P, "P (psi) :");
	NEUIK_Element_Configure(label_P, "FillAll", "HJustify=right", NULL);

	NEUIK_MakeLabel(&label_rH, "rH (%) :");
	NEUIK_Element_Configure(label_rH, "FillAll", "HJustify=right", NULL);

	NEUIK_MakeLabel(&label_T_WB, "T_WB (F) :");
	NEUIK_Element_Configure(label_T_WB, "FillAll", "HJustify=right", NULL);

	/*------------------------------------------------------------------------*/
	/* Create and configure the text entry boxes                              */
	/*------------------------------------------------------------------------*/
	NEUIK_NewTextEntry(&te_T_DB);
	NEUIK_Element_Configure(te_T_DB, "FillAll", NULL);

	NEUIK_NewTextEntry(&te_P);
	NEUIK_Element_Configure(te_P, "FillAll", NULL);

	NEUIK_NewTextEntry(&te_rH);
	NEUIK_Element_Configure(te_rH, "FillAll", NULL);

	NEUIK_NewTextEntry(&te_T_WB);
	NEUIK_Element_Configure(te_T_WB, "FillAll", NULL);

	teInp[0] = te_T_DB;
	teInp[1] = te_P;
	teInp[2] = te_rH;

	NEUIK_Element_SetCallback(te_T_DB, "OnActivated", cbFunc_TE_Act, teInp, te_T_WB);
	NEUIK_Element_SetCallback(te_P, "OnActivated", cbFunc_TE_Act, teInp, te_T_WB);
	NEUIK_Element_SetCallback(te_rH, "OnActivated", cbFunc_TE_Act, teInp, te_T_WB);

	/*------------------------------------------------------------------------*/
	/* Create the HLine                                                       */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHLine(&hlineRes);
	NEUIK_Line_SetThickness(hlineRes, 1);

	/*------------------------------------------------------------------------*/
	/* Create and load the items into horizontal groups                       */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHGroup(&hg0);
	NEUIK_Element_Configure(hg0, "FillAll", "PadLeft=5", "PadRight=5", NULL);
	NEUIK_Container_AddElements(hg0, label_T_DB, te_T_DB, NULL);
	NEUIK_HGroup_SetHSpacing(hg0, 3);

	NEUIK_NewHGroup(&hg1);
	NEUIK_Element_Configure(hg1, "FillAll", "PadLeft=5", "PadRight=5", NULL);
	NEUIK_Container_AddElements(hg1, label_P, te_P, NULL);
	NEUIK_HGroup_SetHSpacing(hg1, 3);

	NEUIK_NewHGroup(&hg2);
	NEUIK_Element_Configure(hg2, "FillAll", "PadLeft=5", "PadRight=5", NULL);
	NEUIK_Container_AddElements(hg2, label_rH, te_rH, NULL);
	NEUIK_HGroup_SetHSpacing(hg2, 3);

	NEUIK_NewHGroup(&hg3);
	NEUIK_Element_Configure(hg3, "FillAll", "PadLeft=5", "PadRight=5", NULL);
	NEUIK_Container_AddElements(hg3, label_T_WB, te_T_WB, NULL);
	NEUIK_HGroup_SetHSpacing(hg3, 3);

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "FillAll", "PadTop=5", "PadBottom=5", NULL);
	NEUIK_Container_AddElements(vg0, hg0, hg1, hg2, hlineRes, hg3, oFrame, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 5);

	NEUIK_NewFrame(&oFrame);
	NEUIK_Element_Configure(oFrame, "HFill", "PadAll=10", NULL);
	NEUIK_Container_SetElement(oFrame, vg0);

	NEUIK_Window_SetElement(mw, oFrame);

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
