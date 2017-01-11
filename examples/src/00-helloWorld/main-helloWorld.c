#include <NEUIK.h>

int main()
{
	NEUIK_Window * mw;
	NEUIK_Label  * lbl;

	NEUIK_Init(); /* Initialize the NEUIK library */

	NEUIK_NewWindow(&mw);  /* Create a new Window */
	NEUIK_Window_SetTitle(mw, "Quick Demo");
	NEUIK_Window_SetSize(mw, 200, 100);

	NEUIK_MakeLabel(&lbl, "Hello World"); /* Create a new Label */
	/* set the label as the element contained by the window */
	NEUIK_Window_SetElement(mw, lbl);

	NEUIK_Window_Create(mw); /* Create and show the window */
	NEUIK_EventLoop(1); /* Start capturing events 1=killOnError */

	NEUIK_Quit();
	return 1;
}
