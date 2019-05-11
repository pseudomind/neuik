#include <stdio.h>
#include <string.h>
#include <NEUIK.h>

int    ClearOnNextOp = 0;
double LastValue     = 0.0;
char   LastOp        = 0;
char   Buffer[4096];


void cbBtnPushed(
	void * window,
	void * btnDisplay,
	void * opChar)
{
	char         op;
	double       result  = 0.0;
	double       thisVal = 0.0;
	const char * dispStr = NULL;
	char         dispBuf[2048];

	if (window == NULL)
	{
		printf("cbBtnPushed: *Window is NULL.\n");
		return;
	}
	if (btnDisplay == NULL)
	{
		printf("cbBtnPushed: A color string is required.\n");
		return;
	}
	if (opChar == NULL)
	{
		printf("cbBtnPushed: An operation character is required.\n");
		return;
	}

	op = *(char*)(opChar);

	if (ClearOnNextOp)
	{
		/*--------------------------------------------------------------------*/
		/* After a `+,-,*,/` is used, the next operation will clear the       */
		/* display before continuing.                                         */
		/*--------------------------------------------------------------------*/
		ClearOnNextOp = 0;
		NEUIK_Button_SetText(btnDisplay, "");
	}
	dispStr = NEUIK_Button_GetText(btnDisplay);

	switch (op) 
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			//----------------------------------------------------------------//
			// A number button, add it to the end of the display.             //
			//----------------------------------------------------------------//
			sprintf(dispBuf, "%s%c", dispStr, op);
			NEUIK_Button_SetText(btnDisplay, dispBuf);
			break;
		case '.':
			if (!strchr(dispStr, op))
			{
				/*------------------------------------------------------------*/
				/* Only add a period if it did not already contain one.       */
				/*------------------------------------------------------------*/
				sprintf(dispBuf, "%s%c", dispStr, op);
				NEUIK_Button_SetText(btnDisplay, dispBuf);
			}
			break;
		case 'C':
			NEUIK_Button_SetText(btnDisplay, "");
			LastOp    = 0;
			LastValue = 0.0;
			break;
		case '+':
			LastOp = '+';
			sscanf(dispStr, "%lf", &LastValue);
			ClearOnNextOp = 1;
			break;
		case '-':
			LastOp = '-';
			sscanf(dispStr, "%lf", &LastValue);
			ClearOnNextOp = 1;
			break;
		case '*':
			LastOp = '*';
			sscanf(dispStr, "%lf", &LastValue);
			ClearOnNextOp = 1;
			break;
		case '/':
			LastOp = '/';
			sscanf(dispStr, "%lf", &LastValue);
			ClearOnNextOp = 1;
			break;
		case '=':
			if (LastOp == 0)
			{
				return;
			}
			sscanf(dispStr, "%lf", &thisVal);

			switch (LastOp) 
			{
				case '+':
					result = LastValue + thisVal;
					break;
				case '-':
					result = LastValue - thisVal;
					break;
				case '*':
					result = LastValue * thisVal;
					break;
				case '/':
					if (thisVal == 0.0)
					{
						NEUIK_Button_SetText(btnDisplay, 
							"ERROR: Attempted Div By Zero");
						return;
					}
					result = LastValue / thisVal;
					break;
			}
			sprintf(dispBuf, "%g", result);
			NEUIK_Button_SetText(btnDisplay, dispBuf);
			break;
	}
}

int main()
{
	int                 rv    = 0;
	NEUIK_Window      * mw    = NULL;
	NEUIK_Transformer * tr    = NULL;
	NEUIK_HGroup      * hg    = NULL;
	NEUIK_VGroup      * vg0   = NULL; /* contains the display and horiz. group */
	NEUIK_VGroup      * vg1   = NULL;
	NEUIK_VGroup      * vg2   = NULL;
	NEUIK_VGroup      * vg3   = NULL;
	NEUIK_VGroup      * vg4   = NULL;

	NEUIK_Button      * btn0   = NULL;
	NEUIK_Button      * btn1   = NULL;
	NEUIK_Button      * btn2   = NULL;
	NEUIK_Button      * btn3   = NULL;
	NEUIK_Button      * btn4   = NULL;
	NEUIK_Button      * btn5   = NULL;
	NEUIK_Button      * btn6   = NULL;
	NEUIK_Button      * btn7   = NULL;
	NEUIK_Button      * btn8   = NULL;
	NEUIK_Button      * btn9   = NULL;
	/* special buttons */
	NEUIK_Button      * btnDot = NULL;
	NEUIK_Button      * btnClr = NULL;

	NEUIK_Button      * btnPlus    = NULL;
	NEUIK_Button      * btnMinus   = NULL;
	NEUIK_Button      * btnMult    = NULL;
	NEUIK_Button      * btnDiv     = NULL;
	NEUIK_Button      * btnEqual   = NULL;
	NEUIK_Button      * btnDisplay = NULL;

	char dFontSize[] = "FontSize=18";
	char dFontBlue[] = "FontColor=0,0,200,0";


	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-calculator");

	/*------------------------------------------------------------------------*/
	/* Numeric Buttons                                                        */
	/*------------------------------------------------------------------------*/

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "Basic Calculator");
	// NEUIK_Window_SetSize(mw, 20, 20);
	NEUIK_Window_Configure(mw, "BGColor=180,180,180,0", NULL);

	NEUIK_NewButton(&btnDisplay);
	NEUIK_Button_Configure(btnDisplay, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btnDisplay, "HFill", NULL);


	NEUIK_MakeButton(&btn0, "0");
	NEUIK_Button_Configure(btn0, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn0, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn0, "OnClicked", cbBtnPushed, btnDisplay, "0");


	NEUIK_MakeButton(&btn1, "1");
	NEUIK_Button_Configure(btn1, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn1, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn1, "OnClicked", cbBtnPushed, btnDisplay, "1");


	NEUIK_MakeButton(&btn2, "2");
	NEUIK_Button_Configure(btn2, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn2, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn2, "OnClicked", cbBtnPushed, btnDisplay, "2");


	NEUIK_MakeButton(&btn3, "3");
	NEUIK_Button_Configure(btn3, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn3, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn3, "OnClicked", cbBtnPushed, btnDisplay, "3");


	NEUIK_MakeButton(&btn4, "4");
	NEUIK_Button_Configure(btn4, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn4, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn4, "OnClicked", cbBtnPushed, btnDisplay, "4");


	NEUIK_MakeButton(&btn5, "5");
	NEUIK_Button_Configure(btn5, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn5, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn5, "OnClicked", cbBtnPushed, btnDisplay, "5");


	NEUIK_MakeButton(&btn6, "6");
	NEUIK_Button_Configure(btn6, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn6, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn6, "OnClicked", cbBtnPushed, btnDisplay, "6");


	NEUIK_MakeButton(&btn7, "7");
	NEUIK_Button_Configure(btn7, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn7, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn7, "OnClicked", cbBtnPushed, btnDisplay, "7");


	NEUIK_MakeButton(&btn8, "8");
	NEUIK_Button_Configure(btn8, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn8, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn8, "OnClicked", cbBtnPushed, btnDisplay, "8");


	NEUIK_MakeButton(&btn9, "9");
	NEUIK_Button_Configure(btn9, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btn9, "FillAll", NULL);
	NEUIK_Element_SetCallback(btn9, "OnClicked", cbBtnPushed, btnDisplay, "9");

	/*------------------------------------------------------------------------*/
	/* Special Buttons                                                        */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeButton(&btnDot, ".");
	NEUIK_Button_Configure(btnDot, dFontSize, "FontBold", NULL);
	NEUIK_Element_Configure(btnDot, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnDot, "OnClicked", cbBtnPushed, btnDisplay, ".");


	NEUIK_MakeButton(&btnClr, "C");
	NEUIK_Button_Configure(btnClr, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnClr, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnClr, "OnClicked", cbBtnPushed, btnDisplay, "C");


	NEUIK_MakeButton(&btnPlus, "+");
	NEUIK_Button_Configure(btnPlus, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnPlus, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnPlus, "OnClicked", cbBtnPushed, btnDisplay, "+");


	NEUIK_MakeButton(&btnMinus, "-");
	NEUIK_Button_Configure(btnMinus, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnMinus, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnMinus, "OnClicked", cbBtnPushed, btnDisplay, "-");


	NEUIK_MakeButton(&btnMult, "*");
	NEUIK_Button_Configure(btnMult, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnMult, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnMult, "OnClicked", cbBtnPushed, btnDisplay, "*");


	NEUIK_MakeButton(&btnDiv, "/");
	NEUIK_Button_Configure(btnDiv, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnDiv, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnDiv, "OnClicked", cbBtnPushed, btnDisplay, "/");


	NEUIK_MakeButton(&btnEqual, "=");
	NEUIK_Button_Configure(btnEqual, dFontSize, dFontBlue, NULL);
	NEUIK_Element_Configure(btnEqual, "FillAll", NULL);
	NEUIK_Element_SetCallback(btnEqual, "OnClicked", cbBtnPushed, btnDisplay, "=");


	NEUIK_NewVGroup(&vg1);
	NEUIK_Element_Configure(vg1, "FillAll", "HScale=3.0", NULL);
	NEUIK_Container_AddElements(vg1, btn7, btn4, btn1, btn0, NULL);
	NEUIK_VGroup_SetVSpacing(vg1, 3);


	NEUIK_NewVGroup(&vg2);
	NEUIK_Element_Configure(vg2, "FillAll", "HScale=3.0", NULL);
	NEUIK_Container_AddElements(vg2, btn8, btn5, btn2, btnDot, NULL);
	NEUIK_VGroup_SetVSpacing(vg2, 3);


	NEUIK_NewVGroup(&vg3);
	NEUIK_Element_Configure(vg3, "FillAll", "HScale=3.0", NULL);
	NEUIK_Container_AddElements(vg3, btn9, btn6, btn3, btnClr, NULL);
	NEUIK_VGroup_SetVSpacing(vg3, 3);


	NEUIK_NewVGroup(&vg4);
	NEUIK_Element_Configure(vg4, "FillAll", "HScale=2.0", NULL);
	NEUIK_Container_AddElements(vg4, btnDiv, btnMult, btnMinus, btnPlus, btnEqual, NULL);
	NEUIK_VGroup_SetVSpacing(vg4, 3);


	NEUIK_NewHGroup(&hg);
	NEUIK_Element_Configure(hg, "FillAll", NULL);
	NEUIK_Container_AddElements(hg, vg1, vg2, vg3, vg4, NULL);
	NEUIK_HGroup_SetHSpacing(hg, 3);


	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(vg0, btnDisplay, hg, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

	NEUIK_NewTransformer(&tr);
	NEUIK_Element_Configure(tr, "FillAll", NULL);
	NEUIK_Transformer_Configure(tr, "Rotation=90.0", NULL);
	NEUIK_Container_SetElement(tr, vg0);

	NEUIK_Window_SetElement(mw, tr);
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
