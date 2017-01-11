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

#include "neuik_classes.h"

/*----------------------------------------------------------------------------*/
/* Registered class implementations                                           */
/*----------------------------------------------------------------------------*/
neuik_Set   * neuik__Set_NEUIK                = NULL;
neuik_Class * neuik__Class_Window             = NULL;
neuik_Class * neuik__Class_WindowConfig       = NULL;
neuik_Class * neuik__Class_Element            = NULL;
neuik_Class * neuik__Class_Button             = NULL;
neuik_Class * neuik__Class_ButtonConfig       = NULL;
neuik_Class * neuik__Class_ComboBox           = NULL;
neuik_Class * neuik__Class_ComboBoxConfig     = NULL;
neuik_Class * neuik__Class_Label              = NULL;
neuik_Class * neuik__Class_LabelConfig        = NULL;
neuik_Class * neuik__Class_Line               = NULL;
neuik_Class * neuik__Class_ProgressBar        = NULL;
neuik_Class * neuik__Class_ProgressBarConfig  = NULL;
neuik_Class * neuik__Class_TextEdit           = NULL;
neuik_Class * neuik__Class_TextEditConfig     = NULL;
neuik_Class * neuik__Class_TextEntry          = NULL;
neuik_Class * neuik__Class_TextEntryConfig    = NULL;
neuik_Class * neuik__Class_ToggleButton       = NULL;
neuik_Class * neuik__Class_ToggleButtonConfig = NULL;
neuik_Class * neuik__Class_Container          = NULL;
neuik_Class * neuik__Class_HGroup             = NULL;
neuik_Class * neuik__Class_VGroup             = NULL;
neuik_Class * neuik__Class_FlowGroup          = NULL;
neuik_Class * neuik__Class_Image              = NULL;
neuik_Class * neuik__Class_ImageConfig        = NULL;
neuik_Class * neuik__Class_ListGroup          = NULL;
neuik_Class * neuik__Class_ListRow            = NULL;
neuik_Class * neuik__Class_Frame              = NULL;
neuik_Class * neuik__Class_Stack              = NULL;


/*----------------------------------------------------------------------------*/
/* Registerd Virtual Functions                                                */
/*----------------------------------------------------------------------------*/
neuik_VirtualFunc neuik_Element_vfunc_CaptureEvent     = NULL;
neuik_VirtualFunc neuik_Element_vfunc_IsShown          = NULL;
neuik_VirtualFunc neuik_Element_vfunc_SetWindowPointer = NULL;
