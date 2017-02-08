#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

extern "C" void initialize();
extern "C" void call_crystal(const char *filename, int size);
extern "C" void get_positions(int *n,int **z,double **x);

int main(int argc, char ** argv)
{
  char const *filename = "../../examples/data/thymine_rho.cube";
  int *z; // atomic numbers
  double *x; // atomic positions
  int n; // number of atoms

  initialize();
  call_crystal(filename, (int) strlen(filename));
  get_positions(&n,&z,&x);
  for (int i=0;i<n;i++)
    printf("%d %d %.10f %.10f %.10f\n",i,z[i],x[i*3+0],x[i*3+1],x[i*3+2]);
}
