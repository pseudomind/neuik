#!/usr/bin/env python2
# encoding: utf-8

import os

NAME        = "neuik"
LIB_DIR     = ".." + os.sep + "lib" + os.sep
SRC_DIR     = "src" + os.sep

SOURCES     = [
	SRC_DIR + "NEUIK_neuik.c",
	SRC_DIR + "neuik_classes.c",
	SRC_DIR + "neuik_internal.c",
	SRC_DIR + "neuik_TextBlock.c",
	SRC_DIR + "NEUIK_error.c",
	SRC_DIR + "NEUIK_error_CrashReporter.c",
	SRC_DIR + "NEUIK_render.c",
	SRC_DIR + "NEUIK_Callback.c",
	SRC_DIR + "NEUIK_Event.c",
	SRC_DIR + "NEUIK_FontSet.c",
	SRC_DIR + "NEUIK_Window.c",
	SRC_DIR + "NEUIK_WindowConfig.c",

	SRC_DIR + "NEUIK_Container.c",
	SRC_DIR + "NEUIK_Element.c",
	SRC_DIR + "NEUIK_Frame.c",
	SRC_DIR + "NEUIK_FlowGroup.c",
	SRC_DIR + "NEUIK_Image.c",
	SRC_DIR + "NEUIK_ImageConfig.c",
	SRC_DIR + "NEUIK_Label.c",
	SRC_DIR + "NEUIK_LabelConfig.c",
	SRC_DIR + "NEUIK_Line.c",
	SRC_DIR + "NEUIK_ListGroup.c",
	SRC_DIR + "NEUIK_ListRow.c",
	SRC_DIR + "NEUIK_Button.c",
	SRC_DIR + "NEUIK_ButtonConfig.c",
	SRC_DIR + "NEUIK_ComboBox.c",
	SRC_DIR + "NEUIK_ComboBoxConfig.c",
	# SRC_DIR + "NEUIK_PopupMenu.c",
	# SRC_DIR + "NEUIK_PopupMenuConfig.c",
	SRC_DIR + "NEUIK_ProgressBar.c",
	SRC_DIR + "NEUIK_ProgressBarConfig.c",
	SRC_DIR + "NEUIK_Stack.c",
	SRC_DIR + "NEUIK_TextEdit.c",
	SRC_DIR + "NEUIK_TextEditConfig.c",
	SRC_DIR + "NEUIK_TextEntry.c",
	SRC_DIR + "NEUIK_TextEntryConfig.c",
	SRC_DIR + "NEUIK_ToggleButton.c",
	SRC_DIR + "NEUIK_ToggleButtonConfig.c",
	SRC_DIR + "NEUIK_HGroup.c",
	SRC_DIR + "NEUIK_VGroup.c",

	SRC_DIR + "neuik_StockImage_app_crashed.c",

	SRC_DIR + "MainMenu.c",
	SRC_DIR + "Menu.c",
	SRC_DIR + "MenuConfig.c",
	SRC_DIR + "MenuItem.c",

]

SOURCES_OSX = [
	SRC_DIR + "NEUIK_platform_darwin.c",
	SRC_DIR + "NEUIK_FontSet_darwin.c",
]

SOURCES_LINUX = [
	SRC_DIR + "NEUIK_platform_linux.c",
	SRC_DIR + "NEUIK_FontSet_linux.c",
]

SOURCES_WINDOWS = [
	SRC_DIR + "NEUIK_platform_windows.c",
	SRC_DIR + "NEUIK_FontSet_windows.c",
]
