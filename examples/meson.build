sdl2 = dependency('SDL2')

all_deps =[sdl2]

examples = [
	'00-helloWorld',
	'01-blank',
	# '02-mainMenu',
	'03-hasButton',
	'04-vGroup',
	'05-hGroup',
	'06-calculator',
	'07-textEntry',
	'08-wetbulbCalc',
	'09-progressCountdown',
	'10-timeSinceStarted',
	'11-hideAndShow',
	'12-windowConfigure',
	'13-eventHandler',
	'14-stack',
	'15-comboBox',
	# '16-popupVisual',
	'17-flowGroup',
	'18-listGroup',
	'19-errorReporter',
	'20-hasImage',
	'21-textEdit',
	'22-rotated',
	'23-celgroup',
	'24-canvas',
	'25-plot2d',
	# '99-debug',
]

ctr = 0
foreach example : examples
	example_name = example.split('-')[1]
	source_main = 'src/@0@/main-@1@.c'.format(example, example_name)

	executable(example, source_main,
		include_directories : include_dirs,
		dependencies : all_deps, 
		link_with : [ libneuik ],
		install : false)

	ctr+=1
endforeach