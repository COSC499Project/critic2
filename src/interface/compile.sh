gfortran -c interface.f90 -I../../src
g++ ../interface.o crystal.cpp -o crystal ../criticlib.a -lgfortran ../qhull/libqhull_critic.a ../cubpack/libcubpack.a ../ciftbx/libciftbx.a ../oldlibs/libmisc.a -lgomp
