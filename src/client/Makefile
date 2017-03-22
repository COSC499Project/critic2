#
# Cross Platform Makefile
# Compatible with Ubuntu 14.04.1 and Mac OS X
#
#
# You will need GLFW (http://www.glfw.org)
#
#   apt-get install libglfw-dev  # Linux
#   brew install glfw            # Mac OS X
#

CXX = g++ 

EXE = critic2
OBJS = compat.o main.o imgui_impl_glfw_gl3.o
OBJS += imgui.o imgui_demo.o imgui_draw.o
OBJS += libs/gl3w/GL/gl3w.o

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), )
	@echo no name found switching to alternet method
	UNAME_S := $(uname)
endif

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS = -lGL `pkg-config --static --libs glfw3`
	LIBS += ../criticlib.a -lgfortran ../qhull/libqhull_critic.a ../cubpack/libcubpack.a ../ciftbx/libciftbx.a ../oldlibs/libmisc.a ../spglib/libspglib.a -lgomp

	CXXFLAGS = -I./ -Ilibs/gl3w `pkg-config --cflags glfw3`
	CXXFLAGS += -Wall -Wformat -DLINUX -D_GLIBCXX_USE_CXX11_ABI=0 -std=c++11 -w -g
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -lglfw3
	LIBS += ../criticlib.a -lgfortran ../qhull/libqhull_critic.a ../cubpack/libcubpack.a ../ciftbx/libciftbx.a ../oldlibs/libmisc.a -lgomp
	#LIBS += -L/usr/local/lib -lglfw

	CXXFLAGS = -I./ -Ilibs/gl3w -I/usr/local/include
	CXXFLAGS += -Wall -Wformat
#	CXXFLAGS += -D__APPLE__
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), CYGWIN_NT-10.0)
   ECHO_MESSAGE = "Windows"
   %.o: %.mod
   LIBS = ./libopengl32.a ./GLFW/libglfw3dll.a ./GLFW/libglfw3.a ./GLFW/glfw3.dll
   LIBS += -lGL -lGLU -lXrandr -lXext -lX11 -lgdi32 -limm32 -lXi -lXinerama -lXcursor -lrt -lm -pthread
   LIBS += ../criticlib.a -lgfortran ../qhull/libqhull_critic.a ../cubpack/libcubpack.a ../ciftbx/libciftbx.a ../oldlibs/libmisc.a -lgomp
   
   CXXFLAGS = -I./ -L./GLFW/libglfw3 -I./GLFW/ -I./libs/gl3w/
   CXXFLAGS += -Wall -Wformat
   CXXFLAGS += -DWIN32
   CFLAGS = $(CXXFLAGS)
endif


.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< 

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) ../interface.o tinyfiledialogs.c $(CXXFLAGS) $(LIBS)

clean:
	rm $(EXE) $(OBJS)
