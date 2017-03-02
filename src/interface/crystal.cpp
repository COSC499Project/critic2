#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

extern "C" void initialize();
extern "C" void call_crystal(const char *filename, int size);
extern "C" void get_positions(int *n,int **z,double **x);
//extern "C" void get_atomic_name(const char *atomName, int atomNum);
extern "C" void share_bond(int n_atom, int **connected_atom);

int main(int argc, char ** argv)
{
  char const *filename = "../../examples/data/pyridine.wfx";
  int *z; // atomic numbers
  double *x; // atomic positions
  int n; // number of atoms

  char const *atomName;

  initialize();
  call_crystal(filename, (int) strlen(filename));
  get_positions(&n,&z,&x);
  for (int i=0;i<n;i++)
    printf("%d %d %.10f %.10f %.10f\n",i,z[i],x[i*3+0],x[i*3+1],x[i*3+2]);

  //get_atomic_name(atomName, 21);
  int n_atom = 1;
  int *connected_atom;
  share_bond(n_atom, &connected_atom);
  printf("%d %d %d\n",n_atom, connected_atom[0], connected_atom[1]);

}
