#include <stdio.h>
#include <NEUIK.h>

#pragma message("TODO/FIXME: The VGroup isn't getting the TopPadding that it deserves.")

void cbFunc_SetActiveElem(
	void * window,
	void * stkPtr,
	void * elem)
{
	NEUIK_Stack * stk;

	stk = (NEUIK_Stack *)stkPtr;

	NEUIK_Stack_SetActiveElement(stk, elem);
}


int main()
{
	int             rv      = 0;
	NEUIK_Window  * mw      = NULL;
	NEUIK_VGroup  * mVG     = NULL;
	NEUIK_VGroup  * s1_vg   = NULL;
	NEUIK_Button  * s1_btn1 = NULL;
	NEUIK_Button  * s1_btn2 = NULL;
	NEUIK_Button  * s1_btn3 = NULL;
	NEUIK_Button  * s1_btn4 = NULL;
	NEUIK_Button  * s1_btn5 = NULL;
	NEUIK_Label   * s2_lbl  = NULL;
	NEUIK_Stack   * stk     = NULL;
	NEUIK_Frame   * frame   = NULL;
	NEUIK_HGroup  * selHG   = NULL;
	NEUIK_Label   * selLbl1 = NULL;
	NEUIK_Label   * selLbl2 = NULL;
	NEUIK_Button  * selBtn1 = NULL;
	NEUIK_Button  * selBtn2 = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-stack");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "NEUIK_Stack example.");

	NEUIK_MakeButton(&s1_btn1, "NonFunctional Button 1");
	NEUIK_Element_Configure(s1_btn1, "FillAll", NULL);

	NEUIK_MakeButton(&s1_btn2, "NonFunctional Button 2");
	NEUIK_Element_Configure(s1_btn2, "FillAll", NULL);

	NEUIK_MakeButton(&s1_btn3, "NonFunctional Button 3");
	NEUIK_Element_Configure(s1_btn3, "FillAll", NULL);

	NEUIK_MakeButton(&s1_btn4, "NonFunctional Button 4");
	NEUIK_Element_Configure(s1_btn4, "FillAll", "VScale=2.0", NULL);

	NEUIK_MakeButton(&s1_btn5, "NonFunctional Button 5");
	NEUIK_Element_Configure(s1_btn5, "FillAll", NULL);

	NEUIK_NewVGroup(&s1_vg);
	NEUIK_Element_Configure(s1_vg, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(s1_vg, s1_btn1, s1_btn2, s1_btn3, s1_btn4, s1_btn5, NULL);
	NEUIK_VGroup_SetVSpacing(s1_vg, 3);

	NEUIK_MakeLabel(&s2_lbl, "Just a single label...");

	NEUIK_NewStack(&stk);
	NEUIK_Container_AddElements(stk, s1_vg, s2_lbl, NULL);
	NEUIK_Stack_SetActiveElement(stk, s1_vg);

	NEUIK_NewFrame(&frame);
	NEUIK_Container_SetElement(frame, stk);

	/* create the control button hGroup */
	NEUIK_MakeLabel(&selLbl1, "Push one of the following buttons to");
	NEUIK_MakeLabel(&selLbl2, "change the active stack element:");
	NEUIK_MakeButton(&selBtn1, "Set elem[0] as active");
	NEUIK_Element_SetCallback(selBtn1, "OnClicked", cbFunc_SetActiveElem, stk, s1_vg);
	NEUIK_MakeButton(&selBtn2, "Set elem[1] as active");
	NEUIK_Element_SetCallback(selBtn2, "OnClicked", cbFunc_SetActiveElem, stk, s2_lbl);

	NEUIK_NewHGroup(&selHG);
	NEUIK_Container_AddElements(selHG, selBtn1, selBtn2, NULL);

	/* Add elements to the main VGroup */
	NEUIK_NewVGroup(&mVG);
	NEUIK_Container_AddElements(mVG, selLbl1, selLbl2, selHG, frame, NULL);
	NEUIK_VGroup_SetVSpacing(mVG, 5);

	NEUIK_Window_SetElement(mw, mVG);
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
