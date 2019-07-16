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
#ifndef NEUIK_CONTAINER_H
#define NEUIK_CONTAINER_H

#include "NEUIK_Element.h"


typedef enum {
		NEUIK_CONTAINER_UNSET, 
		NEUIK_CONTAINER_SINGLE, 
		NEUIK_CONTAINER_MULTI,
		NEUIK_CONTAINER_NO_DEFAULT_ADD_SET} 
neuik_Container_Type;


typedef struct {
		neuik_Object           objBase;      /* this structure is requied to be an neuik object */
		NEUIK_Element        * elems;        /* child elements of this container */
		unsigned int           n_allocated;  /* number of element slots allocated */
		unsigned int           n_used;       /* number of element slots in use */
		neuik_Container_Type   cType;        /* identify the container as single or multi */
		int                    shownIfEmpty; /* [bool] whether container is visible without children shown */
		int                    redrawAll;    /* [bool] if all child elements must be redrawn */
		enum neuik_VJustify    VJustify;     /* Vertical   justification */
		enum neuik_HJustify    HJustify;     /* Horizontal justification */
} NEUIK_Container;


int 
	NEUIK_Container_AddElement(
			NEUIK_Element cont,
			NEUIK_Element elem);

int 
	NEUIK_Container_AddElements(
			NEUIK_Element cont,
			NEUIK_Element elem0,
			...);

int
	NEUIK_Container_Configure(
			NEUIK_Element   cont,
			const char    * set0,
			...);

int 
	NEUIK_Container_GetElementCount(
			NEUIK_Element   cont,
			int           * elemCount);

int
	NEUIK_Container_RemoveElement(
			NEUIK_Element cont,
			NEUIK_Element elem);

int
	NEUIK_Container_SetElement(
			NEUIK_Element cont,
			NEUIK_Element elem);

int
	NEUIK_Container_DeleteElements(
			NEUIK_Element cont);


#endif /* NEUIK_CONTAINER_H */
