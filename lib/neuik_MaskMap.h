/*******************************************************************************
 * Copyright (c) 2014-2019, Michael Leimon <leimon@gmail.com>
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
#ifndef NEUIK_MASKMAP_H
#define NEUIK_MASKMAP_H

#include "neuik_internal.h"

/*----------------------------------------------------------------------------*/
/* Typedef(s)                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct {
	neuik_Object   objBase;   /* this structure is requied to be an neuik object */
	int            sizeW;     /* width of MaskMap  (px) */
	int            sizeH;     /* height of MaskMap (px) */
	char         * mapData;   /* the 2d map indicating which pixels are masked */
	int            nRegAlloc; /* Number region zones allocated; for GetUnmasked */
	int          * regStart;  /* Start points of the regions zones */
	int          * regEnd;    /* End points of the regions zones */
} neuik_MaskMap;

/*----------------------------------------------------------------------------*/
/* Function Prototype(s)                                                      */
/*----------------------------------------------------------------------------*/
int 
	neuik_NewMaskMap(
			neuik_MaskMap ** mapPtr);

int 
	neuik_MakeMaskMap(
			neuik_MaskMap ** mapPtr,
			int              width,
			int              height);

int 
	neuik_MaskMap_Free(
			neuik_MaskMap * map);

int 
	neuik_MaskMap_SetSize(
			neuik_MaskMap * map, 
			int             width,
			int             height);

int 
	neuik_MaskMap_MaskPoint(
			neuik_MaskMap * map, 
			int             x,
			int             y);

int 
	neuik_MaskMap_MaskLine(
			neuik_MaskMap * map, 
			int             x1,
			int             y1,
			int             x2,
			int             y2);

int 
	neuik_MaskMap_GetUnmaskedRegionsOnHLine(
			neuik_MaskMap  * map, 
			int              y,
			int            * nRegions,
			const int     ** rStart,
			const int     ** rEnd);

int 
	neuik_MaskMap_GetUnmaskedRegionsOnVLine(
			neuik_MaskMap  * map, 
			int              x,
			int            * nRegions,
			const int     ** rStart,
			const int     ** rEnd);


#endif /* NEUIK_MASKMAP_H */
