#include <NEUIK.h>

const char SAMPLE_SINE_FUNC[] = 
	"0	0\n"
	"1	0.370820393249937\n"
	"2	0.822899353209462\n"
	"3	1.29442719099992\n"
	"4	1.71190172933128\n"
	"5	2\n"
	"6	2.09232433584934\n"
	"7	1.94164078649987\n"
	"8	1.52824165596043\n"
	"9	0.865247584249853\n"
	"10	3.67394039744206E-16\n"
	"11	-0.98885438199983\n"
	"12	-1.99846985779441\n"
	"13	-2.91246117974981\n"
	"14	-3.61401476192158\n"
	"15	-4\n"
	"16	-3.99443736843965\n"
	"17	-3.55967477524977\n"
	"18	-2.70381216054538\n"
	"19	-1.48328157299975\n"
	"20	-1.22464679914735E-15\n"
	"21	1.60688837074973\n"
	"22	3.17404036237935\n"
	"23	4.53049516849971\n"
	"24	5.51612779451189\n"
	"25	6\n"
	"26	5.89655040102995\n"
	"27	5.17770876399967\n"
	"28	3.87938266513032\n"
	"29	2.10131556174964\n";


int main()
{
	int              rv  = 0;
	NEUIK_Window   * mw  = NULL;
	NEUIK_Plot2D   * plt = NULL;
	NEUIK_PlotData * pd  = NULL;

	if (NEUIK_Init())
	{
		goto out;
	}
	NEUIK_SetAppName("neuik-example-plot2D");

	NEUIK_NewWindow(&mw);
	NEUIK_Window_SetSize(mw, 640, 480);	
	NEUIK_Window_SetTitle(mw, "Plot2D Element");
	NEUIK_Window_Configure(mw, "Resizable", NULL);

	NEUIK_NewPlot2D(&plt);
	NEUIK_Plot2D_Configure(plt,
		"xAxisNumTics=5",
		"yAxisNumTics=6",
		"xAxisRange=0.0,30.0",
		"yAxisRange=-4.0,10.0",
		NULL);

	NEUIK_Plot_SetTitle(plt, "New Title");
	NEUIK_Element_Configure(plt, "FillAll", "PadAll=10", NULL);

	NEUIK_MakePlotData(&pd, "sampleSineFunc", 64);
	NEUIK_PlotData_SetValuesFromString(pd, 32, SAMPLE_SINE_FUNC);
	NEUIK_PlotData_WriteValuesToASCIIFile(pd, "sampleSineFunc.pltdat", TRUE);

	NEUIK_Plot2D_AddPlotData(plt, pd, "sineFunc");

	NEUIK_Window_SetElement(mw, plt);

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
