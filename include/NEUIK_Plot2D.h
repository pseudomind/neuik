/*******************************************************************************
 * Copyright (c) 2014-2020, Michael Leimon <leimon@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#ifndef NEUIK_PLOT2D_H
#define NEUIK_PLOT2D_H

#include "NEUIK_Event.h"
#include "NEUIK_Element.h"
#include "NEUIK_Canvas.h"
#include "NEUIK_GridLayout.h"
#include "NEUIK_HGroup.h"
#include "NEUIK_PlotData.h"
#include "NEUIK_VGroup.h"

typedef struct {
	int nTicmarks;     /* -1 = AUTO; else: >=2 is valid. */
	int showGridlines; /* Draw inner ticmark gridlines in plot? */
} neuik_Plot2DAxisCfg;

typedef struct {
		neuik_Object          objBase;   /* this structure is requied to be an neuik object */
		NEUIK_Canvas        * drawing_background;
		NEUIK_GridLayout    * drawing_ticmarks;        /* A 2x2 gridlayout. */
		NEUIK_Canvas        * drawing_ticmarks_plot_area;
		NEUIK_VGroup        * drawing_y_axis_ticmarks; /* The y-axis ticmark labels */
		NEUIK_HGroup        * drawing_x_axis_ticmarks; /* The x-axis ticmark labels */
		neuik_Plot2DAxisCfg   xAxisCfg;      /* Configuration of x axis for plot */
		neuik_Plot2DAxisCfg   yAxisCfg;      /* Configuration of y axis for plot */
		NEUIK_Color           colorGridline; /* Color to use for the gridlines */
} NEUIK_Plot2D;

int 
	NEUIK_NewPlot2D(
			NEUIK_Plot2D ** plt);

int 
	NEUIK_Plot2D_AddPlotData(
			NEUIK_Plot2D   * plt,
			NEUIK_PlotData * data,
			const char     * label);

int 
	NEUIK_Plot2D_RemovePlotData(
			NEUIK_Plot2D * plt,
			const char   * uniqueName);

int 
	NEUIK_Plot2D_SetPlotDataLabel(
			NEUIK_Plot2D * plt,
			const char   * uniqueName,
			const char   * label);

int
	NEUIK_Plot2D_Configure(
			NEUIK_Plot2D * plot2d,
			const char   * set0,
			...);

#endif /* NEUIK_PLOT2D_H */
