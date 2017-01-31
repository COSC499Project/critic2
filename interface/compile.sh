gfortran -c interface.f90 -I../src
g++ interface.o -I../src crystal.cpp -o crystal ../src/criticlib.a -lgfortran ../src/qhull/libqhull_critic.a ../src/cubpack/libcubpack.a ../src/ciftbx/libciftbx.a ../src/oldlibs/libmisc.a -lgomp
