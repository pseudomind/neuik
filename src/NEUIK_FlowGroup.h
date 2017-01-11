/*******************************************************************************
 * Copyright (c) 2014-2017, Michael Leimon <leimon@gmail.com>
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
#ifndef NEUIK_FLOWGROUP_H
#define NEUIK_FLOWGROUP_H

#include "NEUIK_Element.h"


typedef enum {
	NEUIK_FLOWGROUP_FILLDIRN_LEFT_TO_RIGHT,
	NEUIK_FLOWGROUP_FILLDIRN_RIGHT_TO_LEFT,
	NEUIK_FLOWGROUP_FILLDIRN_TOP_TO_BOTTOM,
	NEUIK_FLOWGROUP_FILLDIRN_BOTTOM_TO_TOP,
} NEUIK_FlowGroup_FillDirn;


typedef struct {
		neuik_Object             objBase;   /* this structure is requied to be an neuik object */
		int                      selected;
		int                      isActive;
		int                      HSpacing;
		int                      VSpacing;
		NEUIK_FlowGroup_FillDirn FillFirst;
		NEUIK_FlowGroup_FillDirn FillSecond;
		int                      maxHItems; // 0 = no limit
		int                      maxVItems; // 0 = no limit
} NEUIK_FlowGroup;

int NEUIK_NewFlowGroup(NEUIK_FlowGroup **);

void 
	NEUIK_FlowGroup_Deselect(
			NEUIK_FlowGroup  * fg);


#endif /* NEUIK_FLOWGROUP_H */
