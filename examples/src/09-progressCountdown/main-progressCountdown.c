/* progress bar example */
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <NEUIK.h>


int TaskActive = 0;

typedef struct {
		NEUIK_Element progBar;
		NEUIK_Element progMsg;
		int           taskTime; /* [s] duration of the operation */
} bgTask;


int backgroundTask(
		void * thisTask)
{
	double               flTime    = 0.0;
	double               thisTime  = 0.0;
	double               frac      = 0.0;
	int                  timeStep  = 50; /* [ms] */
	NEUIK_Label        * statusMsg = NULL;
	NEUIK_ProgressBar  * pBar      = NULL;

	bgTask *             the_task  = (bgTask *) thisTask;

	TaskActive = 1;
	pBar      = the_task->progBar; 
	statusMsg = (NEUIK_Label *) the_task->progMsg;
	flTime    = (double) the_task->taskTime;

	for (;;)
	{
		frac = thisTime/flTime;
		if (frac >= 1.0)
		{
			NEUIK_ProgressBar_SetFraction(pBar, 1.0);
			goto out;
		}

		NEUIK_ProgressBar_SetFraction(pBar, frac);

		SDL_Delay(timeStep);
		thisTime += (double)(timeStep)/1000.0;
	}

out:
	NEUIK_Label_SetText(statusMsg, "");
	TaskActive = 0;
	return 0;
}


void cbFunc_Btn_Start(
		void *  window,
		void *  workTime,
		void ** outp)
{
	void          * progBar;
	NEUIK_Label   * statusMsg;
	SDL_Thread    * thread = NULL;
	static char     taskMsg[200];
	static bgTask   task;

	progBar   = outp[0];
	statusMsg = (NEUIK_Label *)(outp[1]);

	if (!TaskActive)
	{
		task.taskTime = *(int *)(workTime);
		task.progBar  = progBar;
		task.progMsg  = statusMsg;

		sprintf(taskMsg, "%ds countdown in progress...", task.taskTime);
		NEUIK_Label_SetText(statusMsg, taskMsg);

		thread = SDL_CreateThread(&backgroundTask, "backgroundTask", &task);
		SDL_DetachThread(thread);
	}
}


int main()
{
	int                  rv         = 0;
	int                  time10s    = 10;
	int                  time30s    = 30;
	int                  time60s    = 60;
	NEUIK_Window       * mw         = NULL;
	NEUIK_VGroup       * vg0        = NULL; /* contains instrcution text */
	NEUIK_HGroup       * hg0        = NULL; /* contains the countdown buttons */
	NEUIK_Label        * label_inst = NULL;
	NEUIK_Label        * label_cnt  = NULL;
	NEUIK_Label        * label_msg  = NULL;
	NEUIK_Button       * btn_10s    = NULL;
	NEUIK_Button       * btn_30s    = NULL;
	NEUIK_Button       * btn_60s    = NULL;
	NEUIK_ProgressBar  * progBar    = NULL;
	NEUIK_Line         * hlineRes   = NULL;
	NEUIK_Frame        * oFrame     = NULL;
	void               * outp[2];

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-progressCountdown");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 400, 200);
	NEUIK_Window_SetTitle(mw, "ProgressBar -- Countdown example");

	/*------------------------------------------------------------------------*/
	/* Create and configure the text label                                    */
	/*------------------------------------------------------------------------*/
	NEUIK_MakeLabel(&label_inst, 
		"Click on one of the following buttons to start a countdown:");
	NEUIK_Element_Configure(label_inst, "FillAll", NULL);
	NEUIK_MakeLabel(&label_cnt, "Countdown for:");
	NEUIK_Element_Configure(label_cnt, "!FillAll", NULL);
	NEUIK_NewLabel(&label_msg);
	NEUIK_Element_Configure(label_msg, "HFill", "!VFill", NULL);

	/*------------------------------------------------------------------------*/
	/* Create and configure the progress bar                                  */
	/*------------------------------------------------------------------------*/
	NEUIK_NewProgressBar(&progBar);
	NEUIK_Element_Configure(progBar, "HFill", "!VFill", NULL);

	/*------------------------------------------------------------------------*/
	/* Create and configure the countdown buttons                             */
	/*------------------------------------------------------------------------*/
	outp[0] = progBar;
	outp[1] = label_msg;

	NEUIK_MakeButton(&btn_10s, "10s");
	NEUIK_Element_Configure(btn_10s, "!HFill", "VFill", NULL);
	NEUIK_Element_SetCallback(btn_10s, "OnClicked", cbFunc_Btn_Start, &time10s, outp);

	NEUIK_MakeButton(&btn_30s, "30s");
	NEUIK_Element_Configure(btn_30s, "!HFill", "VFill", NULL);
	NEUIK_Element_SetCallback(btn_30s, "OnClicked", cbFunc_Btn_Start, &time30s, outp);

	NEUIK_MakeButton(&btn_60s, "60s");
	NEUIK_Element_Configure(btn_60s, "!HFill", "VFill", NULL);
	NEUIK_Element_SetCallback(btn_60s, "OnClicked", cbFunc_Btn_Start, &time60s, outp);


	/*------------------------------------------------------------------------*/
	/* Create the HLine                                                       */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHLine(&hlineRes);
	NEUIK_Line_SetThickness(hlineRes, 1);

	/*------------------------------------------------------------------------*/
	/* Create and load the items into horizontal groups                       */
	/*------------------------------------------------------------------------*/
	NEUIK_NewHGroup(&hg0);
	NEUIK_Container_AddElements(hg0, label_cnt, btn_10s, btn_30s ,btn_60s , NULL);
	NEUIK_HGroup_SetHSpacing(hg0, 3);

	/*------------------------------------------------------------------------*/
	/* Create and load the horizontal groups into the vertical group          */
	/*------------------------------------------------------------------------*/
	NEUIK_NewVGroup(&vg0);
	NEUIK_Element_Configure(vg0, "FillAll", "PadAll=5", NULL);
	NEUIK_Container_AddElements(vg0, label_inst, hg0, progBar, hlineRes, label_msg, NULL);
	NEUIK_VGroup_SetVSpacing(vg0, 10);

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
