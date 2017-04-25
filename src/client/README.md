# Critic2 GUI

## Overview

This gui helps visualize the critical point information of a critic2 file.

Common errors
- OS detection failure || fix manually set your system to selected through the makefile in the client folder
- Win compilation error || remove -D_WIN32 from the makefile
- missing library || check your package manager for all required libraries for your OS (specified in the Makefile)

### Global:
Left click: move molecule
Right click: rotate molecile
Click tree: select 

## Installation Guide

### Linux
The only additional library that the GUI requires is glfw:
```
sudo apt-get install libglfw3-dev
```
Then in `critic2` folder, run:
```
autoreconf -i
./configure
make
```
In `critic2/src` folder, run:
```
ar cr criticlib.a *.o
```
Finally in `critic2/src/client`, run:
```
make
```
Run the program in `critic2/src/client` with:

./critic2
### OSX (Mac)
Ensure you have gcc and gfortran compilers on your OSX distro. This can be done in many different ways. One of the easier ways is to use Homebrew and use the command:
```
brew install gcc
```
This install package contains gcc, g++, gfortran, and others.

Then ensure that you have OpenGL and GLUT libraries by installing Xcode developer tools, which requires you to install Xcode and then its additional development tools.

Then in `critic2` folder, run:
```
autoreconf -i
./configure
make
make install
```
In `critic2/src` folder, run:
```
ar cr criticlib.a *.o
```
Finally in `critic2/src/client`, run:
```
make
```
Run the program in `critic2/src/client` with:

./critic2
### Windows
Get https://cygwin.com/install.html installation Manager.
Basic setup:
select these for install (paste into search bar of cygwin setup)

autoconf
automake
fortran-gcc
libgfortran3
xinit
libtool
gcc-g++
libx11-devel
libXrandr-devel
libxinerama-devel
libGLU-devel
libXcursor-devel
libxi-devel

download http://www.glfw.org/download.html
64-bit binaries and copy lib-mingw-w64/glfw3.dll to the src/client folder


make sure folder name is critic2
open Cygwin64 Terminal in admin mode
navigate to the critic2 folder
```
autoreconf -i
./configure
make
make install
```
test install of critic2.exe, then:
```
cd src
ar cr criticlib.a *.o
gfortran -c interface.f90 -I../src
cd client
```
Add -D_WIN32 to the makefile, then
```
make all
```
Then remove -D_WIN32 from the makefile
```
make all
```
## Developing / Extending Our Work
### Imgui Development
The UI overlay used for this was ImGui (https://github.com/ocornut/imgui), a useful library that is very simple. The "immediate mode" architecture allows you to place calls to ImGui functions anywhere in your code without worrying about callbacks.

The library is well documented in the comments of `imgui.cpp` and `imgui.h`, but we will give you some examples to get started with anyway

To create a new window, just call, ImGui::Begin to create it and ImGui::End to end it.
```
ImGui::Begin("Title", &open);
ImGui::End();
```
open is a pointer to a boolean, which indicates whether the window is closed or open. Generally you want to declare this as a static variable, outside the main loop.

Any UI elements we wish to add to this window just goes between the ImGui::Begin call and the ImGui::End call. ImGui has a vast amount of built-in widgets, so a good way to find what you want is to browse the demo window provided by the developers of ImGui.  You can do this by toggling the boolean value in critic2/src/client/main.cpp at line 22, from false to true.
```
bool ShowDemoWindow = true;
```
Then make, and run the program to see the demo window.

After finding something you wish to add, you can grep through the imgui_demo.cpp file to find the code that generates it. Usually it will be a single line.

For instance if we want to add a button, a checkbox, some text, and a draggable number bar, we could do
```
ImGui::Begin("Title", &open);
ImGui::Checkbox(“checkbox”, &check);
if (ImGui::Button(“Do Something”)) {
	DoSomething();
	}
	ImGui::Text(“Here is some text”);
	ImGui::Text(“print formatting also works, see: %d”, 1000);
	ImGui::DragFloat(“drag float”, &fl, 0.05f);
ImGui::End();
```
Some comments about this. Both Checkbox and Button return a boolean value, so they can be put in if statements, which is one of the ways ImGui avoids having to use callbacks.
### Fortran / C Interface and Loading Molecular Info
The interface between Fortran and C++ consists of the file critic2/src/interface.f90
Subroutines are written here and bound to a C function name. Which are then matched with a C function in critic2/src/client/main.cpp, called by:
```
extern "C" void function name();
```
As you can see, the interface consists of functions which are called when the user loads a molecule/crystal file, or when they call the generate/load critical point functions, from the top menu bar.

When molecules/crystals are loaded, or their critical points generated, the information goes into arrays of structs. It is from these arrays (ie: bonds, loadedCriticalPoints, and loadedAtoms) that information is accessed by the 3D modelling code.





