project('libneuik', 'c', version : '0.0.10')

include_dirs = include_directories('include')

sdl2_dep       = dependency('SDL2')
sdl2_ttf_dep   = dependency('SDL2_ttf')
sdl2_image_dep = dependency('SDL2_image')

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : true)

all_deps = [
	sdl2_dep, 
	sdl2_ttf_dep, 
	sdl2_image_dep, 
	m_dep]

libneuik_srcs = [
	'lib/NEUIK_neuik.c',
	'lib/neuik_classes.c',
	'lib/neuik_internal.c',
	'lib/neuik_MaskMap.c',
	'lib/neuik_TextBlock.c',
	'lib/NEUIK_error.c',
	'lib/NEUIK_error_CrashReporter.c',
	'lib/NEUIK_render.c',
	'lib/NEUIK_Callback.c',
	'lib/NEUIK_Event.c',
	'lib/NEUIK_Fill.c',
	'lib/NEUIK_FontSet.c',
	'lib/NEUIK_Window.c',
	'lib/NEUIK_WindowConfig.c',
	'lib/NEUIK_Canvas.c',
	'lib/NEUIK_Container.c',
	'lib/NEUIK_Element.c',
	'lib/NEUIK_Frame.c',
	'lib/NEUIK_FlowGroup.c',
	'lib/NEUIK_GridLayout.c',
	'lib/NEUIK_Image.c',
	'lib/NEUIK_ImageConfig.c',
	'lib/NEUIK_Label.c',
	'lib/NEUIK_LabelConfig.c',
	'lib/NEUIK_Line.c',
	'lib/NEUIK_ListGroup.c',
	'lib/NEUIK_ListRow.c',
	'lib/NEUIK_Button.c',
	'lib/NEUIK_ButtonConfig.c',
	'lib/NEUIK_CelGroup.c',
	'lib/NEUIK_ComboBox.c',
	'lib/NEUIK_ComboBoxConfig.c',
	'lib/NEUIK_Plot.c',
	'lib/NEUIK_Plot2D.c',
	'lib/neuik_Plot2D_Render32.c',
	'lib/neuik_Plot2D_Render64.c',
	'lib/NEUIK_PlotData.c',
	'lib/NEUIK_ProgressBar.c',
	'lib/NEUIK_ProgressBarConfig.c',
	'lib/NEUIK_Stack.c',
	'lib/NEUIK_TextEdit.c',
	'lib/NEUIK_TextEdit_CaptureEvent.c',
	'lib/NEUIK_TextEditConfig.c',
	'lib/NEUIK_TextEntry.c',
	'lib/NEUIK_TextEntry_CaptureEvent.c',
	'lib/NEUIK_TextEntryConfig.c',
	'lib/NEUIK_ToggleButton.c',
	'lib/NEUIK_ToggleButtonConfig.c',
	'lib/NEUIK_Transformer.c',
	'lib/NEUIK_HGroup.c',
	'lib/NEUIK_VGroup.c',
	'lib/neuik_StockImage_app_crashed.c',
	'lib/neuik_StockImage_neuik_icon.c',
	'lib/neuik_StockImage_neuik_logo.c',
	'lib/MainMenu.c',
	'lib/Menu.c',
	'lib/MenuConfig.c',
	'lib/MenuItem.c',
]

SOURCES_OSX = [
	'lib/NEUIK_platform_darwin.c',
	'lib/NEUIK_FontSet_darwin.c',
]

SOURCES_LINUX = [
	'lib/NEUIK_platform_linux.c',
	'lib/NEUIK_FontSet_linux.c',
]

SOURCES_WINDOWS = [
	'lib/NEUIK_platform_windows.c',
	'lib/NEUIK_FontSet_windows.c',
]

libneuik_srcs += SOURCES_LINUX

libneuik = library('neuik', 
	libneuik_srcs, 
	dependencies : all_deps, 
	include_directories : include_dirs,
	install : true)

libneuik_dep = declare_dependency(
	include_directories : include_dirs,
	link_with : libneuik,
)

pkg = import('pkgconfig')
pkg.generate(libraries: [ libneuik ],
	version : meson.project_version(),
	name : 'neuik',
	description : 'GUI Toolkit')

