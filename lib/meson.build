sdl2       = dependency('SDL2')
sdl2_ttf   = dependency('SDL2_ttf')
sdl2_image = dependency('SDL2_image')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

all_deps   = [sdl2, sdl2_ttf, sdl2_image, m_dep]

libneuik_srcs = [
	'NEUIK_neuik.c',
	'neuik_classes.c',
	'neuik_internal.c',
	'neuik_MaskMap.c',
	'neuik_TextBlock.c',
	'NEUIK_error.c',
	'NEUIK_error_CrashReporter.c',
	'NEUIK_render.c',
	'NEUIK_Callback.c',
	'NEUIK_Event.c',
	'NEUIK_Fill.c',
	'NEUIK_FontSet.c',
	'NEUIK_Window.c',
	'NEUIK_WindowConfig.c',
	'NEUIK_Canvas.c',
	'NEUIK_Container.c',
	'NEUIK_Element.c',
	'NEUIK_Frame.c',
	'NEUIK_FlowGroup.c',
	'NEUIK_GridLayout.c',
	'NEUIK_Image.c',
	'NEUIK_ImageConfig.c',
	'NEUIK_Label.c',
	'NEUIK_LabelConfig.c',
	'NEUIK_Line.c',
	'NEUIK_ListGroup.c',
	'NEUIK_ListRow.c',
	'NEUIK_Button.c',
	'NEUIK_ButtonConfig.c',
	'NEUIK_CelGroup.c',
	'NEUIK_ComboBox.c',
	'NEUIK_ComboBoxConfig.c',
	'NEUIK_Plot.c',
	'NEUIK_Plot2D.c',
	'NEUIK_ProgressBar.c',
	'NEUIK_ProgressBarConfig.c',
	'NEUIK_Stack.c',
	'NEUIK_TextEdit.c',
	'NEUIK_TextEdit_CaptureEvent.c',
	'NEUIK_TextEditConfig.c',
	'NEUIK_TextEntry.c',
	'NEUIK_TextEntry_CaptureEvent.c',
	'NEUIK_TextEntryConfig.c',
	'NEUIK_ToggleButton.c',
	'NEUIK_ToggleButtonConfig.c',
	'NEUIK_Transformer.c',
	'NEUIK_HGroup.c',
	'NEUIK_VGroup.c',
	'neuik_StockImage_app_crashed.c',
	'neuik_StockImage_neuik_icon.c',
	'neuik_StockImage_neuik_logo.c',
	'MainMenu.c',
	'Menu.c',
	'MenuConfig.c',
	'MenuItem.c',
]

SOURCES_OSX = [
	'NEUIK_platform_darwin.c',
	'NEUIK_FontSet_darwin.c',
]

SOURCES_LINUX = [
	'NEUIK_platform_linux.c',
	'NEUIK_FontSet_linux.c',
]

SOURCES_WINDOWS = [
	'NEUIK_platform_windows.c',
	'NEUIK_FontSet_windows.c',
]

libneuik_srcs += SOURCES_LINUX
# srcs.extend(SOURCES_LINUX)

inc = include_directories('.')

# static_library('neuik', 
# 	libneuik_srcs, 
# 	dependencies : all_deps, 
# 	include_directories : inc,
# 	install : true)

libneuik = library('neuik', 
	libneuik_srcs, 
	dependencies : all_deps, 
	include_directories : inc,
	install : true)

pkg = import('pkgconfig')
pkg.generate(libraries: [ libneuik ],
	version : meson.project_version(),
	name : 'neuik',
	description : 'GUI Toolkit',
	subdirs: 'neuik')