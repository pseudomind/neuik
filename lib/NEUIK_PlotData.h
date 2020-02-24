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
#ifndef NEUIK_PLOTDATA_H
#define NEUIK_PLOTDATA_H

#include "NEUIK_Element.h"


typedef struct {
		neuik_Object   objBase;    /* this structure is requied to be an neuik object */
		char         * uniqueName; /* unique name for this plot data. */
		unsigned int   nAlloc;     /* Number of allocated data slots */
		unsigned int   nPoints;    /* Number of X,Y datapoint pairs */
		unsigned int   nUsed;      /* Number of data slots in use */
		int            precision;  /* 32bit (32) or 64bit (64) */
		int            boundsSet;  /*  */
		float        * data_32;    /* Used for storing 32bit float values */
		double       * data_64;    /* Used for storing 64bit float values */
		struct {
			float x_min;
			float x_max;
			float y_min;
			float y_max;
		} bounds_32;
		struct {
			double x_min;
			double x_max;
			double y_min;
			double y_max;
		} bounds_64;
} NEUIK_PlotData;


int
	NEUIK_NewPlotData(
			NEUIK_PlotData ** pd,
			const char      * uniqueName);

int
	NEUIK_MakePlotData(
			NEUIK_PlotData ** pd,
			const char      * uniqueName,
			int               precision);

int 
	NEUIK_PlotData_Copy(
			NEUIK_PlotData       * dst,
			const NEUIK_PlotData * src);

int 
	NEUIK_PlotData_Free(
			NEUIK_PlotData * pd);

int
	NEUIK_PlotData_SetValuesFromString(
			NEUIK_PlotData * pd,
			const char     * valStr);

int
	NEUIK_PlotData_WriteValuesToASCIIFile(
			NEUIK_PlotData * pd,
			const char     * fileName,
			int              writeHeader);

#endif /* NEUIK_PLOTDATA_H */
