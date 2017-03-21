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
#ifndef NEUIK_H
#define NEUIK_H

/*----------------------------------------------------------------------------*/
/* This is just a meta-include which will include all of the headers needed   */
/* by external programs.                                                      */
/*----------------------------------------------------------------------------*/
int NEUIK_Object_Free(
	void ** objPtr);

#include "NEUIK_Button.h"
#include "NEUIK_ButtonConfig.h"
#include "NEUIK_Container.h"
#include "NEUIK_Callback.h"
#include "NEUIK_ComboBox.h"
#include "NEUIK_Element.h"
#include "NEUIK_error.h"
#include "NEUIK_Event.h"
#include "NEUIK_Frame.h"
#include "NEUIK_Image.h"
#include "NEUIK_Label.h"
#include "NEUIK_LabelConfig.h"
#include "NEUIK_Line.h"
#include "NEUIK_ListGroup.h"
#include "NEUIK_ListRow.h"
#include "NEUIK_neuik.h"
#include "NEUIK_ProgressBar.h"
// #include "NEUIK_PopupMenu.h"
#include "NEUIK_Stack.h"
#include "NEUIK_StockImage.h"
#include "NEUIK_TextEdit.h"
#include "NEUIK_TextEntry.h"
#include "NEUIK_ToggleButton.h"
#include "NEUIK_ToggleButtonConfig.h"
#include "NEUIK_HGroup.h"
#include "NEUIK_VGroup.h"
#include "NEUIK_FlowGroup.h"
#include "NEUIK_Window.h"
#include "NEUIK_WindowConfig.h"


#endif /* NEUIK_H */
