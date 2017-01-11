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
#ifndef NEUIK_CLASSES_H
#define NEUIK_CLASSES_H

#include "neuik_internal.h"

/*----------------------------------------------------------------------------*/
/* Registered class implementations                                           */
/*----------------------------------------------------------------------------*/
extern neuik_Set   * neuik__Set_NEUIK;
extern neuik_Class * neuik__Class_Window;
extern neuik_Class * neuik__Class_WindowConfig;
extern neuik_Class * neuik__Class_Element;
extern neuik_Class * neuik__Class_HGroup;
extern neuik_Class * neuik__Class_VGroup;
extern neuik_Class * neuik__Class_FlowGroup;
extern neuik_Class * neuik__Class_Image;
extern neuik_Class * neuik__Class_ImageConfig;
extern neuik_Class * neuik__Class_ListGroup;
extern neuik_Class * neuik__Class_ListRow;
extern neuik_Class * neuik__Class_Button;
extern neuik_Class * neuik__Class_ButtonConfig;
extern neuik_Class * neuik__Class_ComboBox;
extern neuik_Class * neuik__Class_ComboBoxConfig;
extern neuik_Class * neuik__Class_Label;
extern neuik_Class * neuik__Class_LabelConfig;
extern neuik_Class * neuik__Class_Line;
extern neuik_Class * neuik__Class_ProgressBar;
extern neuik_Class * neuik__Class_ProgressBarConfig;
extern neuik_Class * neuik__Class_TextEdit;
extern neuik_Class * neuik__Class_TextEditConfig;
extern neuik_Class * neuik__Class_TextEntry;
extern neuik_Class * neuik__Class_TextEntryConfig;
extern neuik_Class * neuik__Class_ToggleButton;
extern neuik_Class * neuik__Class_ToggleButtonConfig;
extern neuik_Class * neuik__Class_Container;
extern neuik_Class * neuik__Class_Frame;
extern neuik_Class * neuik__Class_Stack;

/*----------------------------------------------------------------------------*/
/* Registerd Virtual Functions                                                */
/*----------------------------------------------------------------------------*/
extern neuik_VirtualFunc neuik_Element_vfunc_CaptureEvent;
extern neuik_VirtualFunc neuik_Element_vfunc_IsShown;
extern neuik_VirtualFunc neuik_Element_vfunc_SetWindowPointer;

/*----------------------------------------------------------------------------*/
/* Registration functions for class implementations                           */
/*----------------------------------------------------------------------------*/
int neuik_RegisterClass_Window();
int neuik_RegisterClass_WindowConfig();
int neuik_RegisterClass_Element();
int neuik_RegisterClass_Container();
int neuik_RegisterClass_HGroup();
int neuik_RegisterClass_VGroup();
int neuik_RegisterClass_FlowGroup();
int neuik_RegisterClass_Image();
int neuik_RegisterClass_ImageConfig();
int neuik_RegisterClass_ListGroup();
int neuik_RegisterClass_ListRow();
int neuik_RegisterClass_Frame();
int neuik_RegisterClass_Button();
int neuik_RegisterClass_ButtonConfig();
int neuik_RegisterClass_ComboBox();
int neuik_RegisterClass_ComboBoxConfig();
int neuik_RegisterClass_Label();
int neuik_RegisterClass_LabelConfig();
int neuik_RegisterClass_Line();
int neuik_RegisterClass_ProgressBar();
int neuik_RegisterClass_ProgressBarConfig();
int neuik_RegisterClass_TextEdit();
int neuik_RegisterClass_TextEditConfig();
int neuik_RegisterClass_TextEntry();
int neuik_RegisterClass_TextEntryConfig();
int neuik_RegisterClass_ToggleButton();
int neuik_RegisterClass_ToggleButtonConfig();
int neuik_RegisterClass_Stack();


#endif /* NEUIK_CLASSES_H */
