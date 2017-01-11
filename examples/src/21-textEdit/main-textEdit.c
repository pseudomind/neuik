#include <NEUIK.h>

void cbFunc_SetText(
	void * window,
	void * tePtr)
{
	const char text[] =
		"[1] The quick brown fox jumps over the lazy dog.\n"
		"[2] The quick brown fox jumps over the lazy dog.\n"
		"[3] The quick brown fox jumps over the lazy dog.\n";

	NEUIK_TextEdit_SetText(tePtr, text);
}

int main()
{
	int            rv        = 0;
	NEUIK_Window * mw        = NULL;

	NEUIK_Label  * lblEdit   = NULL;
	NEUIK_Label  * lblSelect = NULL;
	NEUIK_Label  * lblView   = NULL;

	NEUIK_Button * btnSetText = NULL;

	NEUIK_TextEdit * teEdit   = NULL;
	NEUIK_TextEdit * teSelect = NULL;
	NEUIK_TextEdit * teView   = NULL;

	NEUIK_VGroup * vgMain    = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-textEdit");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 640, 480);	
	NEUIK_Window_SetTitle(mw, "TextEdit demonstration");

	NEUIK_MakeLabel(&lblEdit, "The Contents of this TextEdit can be changed");
	NEUIK_Element_Configure(lblEdit, "HJustify=left", NULL);

	NEUIK_MakeLabel(&lblSelect, "The Contents of this TextEdit can only be selected");
	NEUIK_Element_Configure(lblSelect, "HJustify=left", NULL);

	NEUIK_MakeLabel(&lblView, "The Contents of this TextEdit can only be viewed");
	NEUIK_Element_Configure(lblView, "HJustify=left", NULL);

	NEUIK_MakeTextEdit(&teEdit, 
		"[1] The quick brown fox jumps over the lazy dog.\n"
		"[2] The quick brown fox jumps over the lazy dog.\n"
		"[3] The quick brown fox jumps over the lazy dog.\n");
	NEUIK_Element_Configure(teEdit, "FillAll", NULL);

	NEUIK_MakeTextEdit(&teSelect, 
		"[a] The quick brown fox jumps over the lazy dog.\n"
		"[b] The quick brown fox jumps over the lazy dog.\n"
		"[c] The quick brown fox jumps over the lazy dog.\n");
	NEUIK_Element_Configure(teSelect, "FillAll", NULL);

	// NEUIK_MakeTextEdit(&teView, 
	// 	"[A] The quick brown fox jumps over the lazy dog.\n"
	// 	"[B] The quick brown fox jumps over the lazy dog.\n"
	// 	"[C] The quick brown fox jumps over the lazy dog.\n");

	// NEUIK_MakeTextEdit(&teView, "");
	NEUIK_NewTextEdit(&teView);
	NEUIK_Element_Configure(teView, "FillAll", NULL);

	NEUIK_MakeButton(&btnSetText, "Set Text");
	NEUIK_Element_Configure(btnSetText, "HJustify=left", NULL);
	NEUIK_Element_SetCallback(btnSetText, "OnClicked", cbFunc_SetText, teView, NULL);

	NEUIK_NewVGroup(&vgMain);
	// NEUIK_Element_Configure(vgMain, "HFill", "VFill", NULL);
	NEUIK_Element_Configure(vgMain, "FillAll", "PadAll=10", NULL);
	NEUIK_Container_AddElements(vgMain, 
		lblEdit, teEdit, lblSelect, teSelect, lblView, teView, btnSetText, NULL);

	NEUIK_Window_SetElement(mw, vgMain);

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
