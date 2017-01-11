#include <NEUIK.h>

int main()
{
	int            rv  = 0;
	NEUIK_Window * mw  = NULL;
	NEUIK_Image * img = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-hasImage");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetTitle(mw, "This window contains an image.");

	NEUIK_MakeImage(&img, "app_crashed_64x64.png");
	NEUIK_Window_SetElement(mw, img);

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
