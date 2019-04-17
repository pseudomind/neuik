#include <stdio.h>
#include <NEUIK.h>

void cbFunc_Btn_PushMe(
	void *window)
{
	printf("Button `Push Me!` pushed!!!\n");
	NEUIK_Window_SetTitle(window, "The Button was pushed!");
}

int main()
{
	int                 rv    = 0;
	NEUIK_Window      * mw    = NULL;
	NEUIK_Transformer * trans = NULL;
	NEUIK_Button      * btn   = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-hasButton");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "This window has a button.");

	NEUIK_MakeButton(&btn, "Push Me!");
	NEUIK_Element_SetCallback(btn, "OnClicked", cbFunc_Btn_PushMe, NULL, NULL);

	NEUIK_NewTransformer(&trans);
	NEUIK_Transformer_Configure(trans, "Rotation=270.0", NULL);
	// NEUIK_Transformer_Configure(trans, "Rotation=180.0", NULL);
	// NEUIK_Transformer_Configure(trans, "Rotation=90.0", NULL);
	NEUIK_Container_SetElement(trans, btn);

	NEUIK_Window_SetElement(mw, trans);

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
