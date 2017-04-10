# Critic2 GUI

## Overview

This gui helps visualize the critical point information of a critic2 file.

## Files

MAKEFILE:

## Installation Prerequisites: 
- A custom glfw installation may be required http://www.glfw.org/ (place in the GLFW folder in /client)
- A custom gl3w installation may be required follow instructions in https://github.com/skaslev/gl3w and move the library files (libglfw3.a, libglfw3dll.a)
- TODO: Package Requirments

## Installation Instructions:

Critic2 must be made and installed first.
Navigate to the Critic2/src folder and use the command to compile critic2 into a library:
```
ar cr criticlib.a *.o
```
Then compile the interface file using (in the same directory:
```
gfortran -c interface.f90 -I../src
```
Navigate to the /client folder and use the command
```
Make all
```
Common errors
- OS detection failure || fix manually set your system to selected through the makefile in the client folder
- Win compilation error || remove -D_WIN32 from the makefile
- missing library || check your package manager for all required libraries for your OS (specified in the Makefile)

## Usage Instructions
### Linux:

### Win:
run critic2.exe (from client folder), Load molecule menu may appear in the command line.
### Mac:

### Global:
Left click: move molecule
Right click: rotate molecile
Click tree: select 

## Additional software used

ImGUI: https://github.com/ocornut/imgui 

## Copyright notice





