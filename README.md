NEUIK
=====
The Nuclear Engineer's User Interface Kit, NEUIK, is a GUI toolkit library written in C on top of SDL2.

Currently it has been designed and tested to work under: Linux and OSX.

### QuickStart

This library and the associated examples are built using WAF.

A copy of the WAF build script is included along with the source files.

> **Prerequisites for building NEUIK:**

> You will need **SDL2**, **SDL2_ttf** and **SDL2_image** installed on the build system.

> **Buidling on Linux:**

> - It is best to obtain the requisite libraries using the standard package management for your distribution.
> - Make sure that the pkgconfig scripts for these SDL2 libraries are in a place pkgconfig can find them.  You may need to set the *PKG_CONFIG_PATH* environment variable if you having trouble with this.
> - For my LinuxMint 17.1 system I had to add the following line to my `$HOME/.bashrc` file:
> : export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig

> **Buidling on OSX:**

> - Download the latest **SDL2** from here: [http://www.libsdl.org/download-2.0.php]()
> - Download the latest **SDL2_ttf** from here: [http://www.libsdl.org/projects/SDL_ttf/]()
> - Download the latest **SDL2_image** from here: [http://www.libsdl.org/projects/SDL_image/]()
 
To build the NEUIK library on your system, run the following commands
: ./waf configure
: ./waf 

To build the examples, use the following command:
: ./waf --examples

### Features Desired for VERSION 1.0

## Display

-----------------------------------------
|  Feature                |  Status     |
|-------------------------|-------------|
|  Label                  |  [DONE]     |
|  StatusBar              |  [-]        |
|  ProgressBar            |  [DONE]     |
|  ScrollBar              |  [-]        |
|  Image                  |  [PARTIAL]  |
|  H&V Lines              |  [DONE]     |
|  H&V Separators         |  [-]        |
|  MultilineText          |  [-]        |
|                         |  [-]        |
-----------------------------------------

## Buttons

-----------------------------------------
|  Feature                |  Status     |
|-------------------------|-------------|
|  Button                 |  [DONE]     |
|  Check Button           |  [-]        |
|  Toggle Button          |  [DONE]     |
|  Link Button            |  [-]        |
|  Spin Button            |  [-]        |
|  ComboBox               |  [-]        |
|  ComboBox Text          |  [-]        |
|  Radio Button           |  [-]        |
|                         |  [-]        |
-----------------------------------------

## Entries

-----------------------------------------
|  Feature                |  Status     |
|-------------------------|-------------|
|  Text Entry             |  [PARTIAL]  |
|  Search Entry           |  [-]        |
|  TextEdit               |  [PARTIAL]  |
|  RichTextEdit           |  [-]        |
|                         |  [-]        |
-----------------------------------------

## Containers

-----------------------------------------
|  Feature                |  Status     |
|-------------------------|-------------|
|  ScrolledWindow         |  [-]        |
|  Stack                  |  [DONE]     |
|  StackSwitcher          |  [-]        |
|  TreeView               |  [-]        |
|  Notebook               |  [-]        |
|  Frame                  |  [DONE]     |
|  H&V Panes              |  [-]        |
|  Menu                   |  [BROKEN]   |
|  H&V Group              |  [DONE]     |
|  List Group             |  [DONE]     |
|  Flow Group             |  [DONE]     |
|  Gridlayout             |  [-]        |
|                         |  [-]        |
-----------------------------------------

## Windows

-----------------------------------------
|  Feature                |  Status     |
|-------------------------|-------------|
|  Window                 |  [DONE]     |
|  Message Dialog         |  [-]        |
|  About/Credits          |  [-]        |
|  File Chooser Dialog    |  [-]        |
|  Recent Chooser Dialog  |  [-]        |
|                         |  [-]        |
-----------------------------------------
