/*******************************************************************************
 * Copyright (c) 2014-2021, Michael Leimon <mike@leimon.net>
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
#ifndef NEUIK_PLOT_H
#define NEUIK_PLOT_H

#include "NEUIK_Element.h"
#include "NEUIK_HGroup.h"


typedef enum {
		NEUIK_PLOTRANGECONFIG_UNSET, 
		NEUIK_PLOTRANGECONFIG_AUTO, 
		NEUIK_PLOTRANGECONFIG_SPECIFIED} 
neuik_PlotRangeConfig;

typedef struct {
	char        * uniqueName;
	char        * label;
	int           lineThickness;
	int           lineColorSpecified;
	NEUIK_Color   lineColor;
} neuik_PlotDataConfig;

typedef struct {
		neuik_Object            objBase;     /* this structure is requied to be an neuik object */
		NEUIK_Element         * title;
		NEUIK_Element         * x_label;
		NEUIK_Element         * y_label;
		NEUIK_Element         * y_label_trans;
		NEUIK_HGroup          * hg_data;
		NEUIK_Element         * drawing_background;
		NEUIK_Element         * drawing_ticmarks;
		NEUIK_Element         * drawing;
		NEUIK_Element         * legend;
		NEUIK_Element         * visual;
		NEUIK_Object          * data_sets;    /* data_set of this plot */
		neuik_PlotDataConfig  * data_configs; /* config for the associated data sets */
		unsigned int            n_allocated;  /* number of data_set slots allocated */
		unsigned int            n_used;       /* number of data_set slots in use */
		neuik_PlotRangeConfig   x_range_cfg;  /* specifies plot range determination */
		double                  x_range_min;
		double                  x_range_max;
		neuik_PlotRangeConfig   y_range_cfg;  /* specifies plot range determination */
		double                  y_range_min;
		double                  y_range_max;
} NEUIK_Plot;

int
	NEUIK_Plot_SetTitle(
			NEUIK_Element   plot,
			const char    * text);

int
	NEUIK_Plot_SetXAxisLabel(
			NEUIK_Element   plot,
			const char    * text);

int
	NEUIK_Plot_SetYAxisLabel(
			NEUIK_Element   plot,
			const char    * text);

int
	NEUIK_Plot_AddXTic(
			NEUIK_Plot * plot,
			double       tic);

int
	NEUIK_Plot_AddXTicAliased(
			NEUIK_Plot * plot,
			double       tic,
			const char * alias);

 int
	NEUIK_Plot_AddYTic(
			NEUIK_Plot * plot,
			double       tic);

int
	NEUIK_Plot_AddYTicAliased(
			NEUIK_Plot * plot,
			double       tic,
			const char * alias);

#endif /* NEUIK_PLOT_H */
