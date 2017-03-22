#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

extern "C" void initialize();
extern "C" void init_struct();
extern "C" void call_structure(const char *filename, int size, int isMolecule);
extern "C" void get_num_atoms(int *n);
extern "C" void get_atom_position(int n, int *atomicN, double *x, double *y, double *z);
extern "C" void num_of_bonds(int n, int *nstarN);
//extern "C" void get_3positions(int *n,int **anum,double **x, double **y, double **z);
//extern "C" void get_atomic_name(const char *atomName, int atomNum);
//extern "C" void share_bond(int n_atom, int *connected_atom);


int main(int argc, char ** argv)
{
  char const *filename = "../../examples/data/pyridine.wfx";

  int n; // number of atoms

  char const *atomName;

  initialize();
  init_struct();
  call_structure(filename, (int) strlen(filename), 1);
  get_num_atoms(&n);
  printf("num of atoms %d\n", n);

  for (int i=1; i <= n; i++) {
    int atomicN;
    double x;
    double y;
    double z;

    get_atom_position(i, &atomicN, &x, &y, &z);
    printf("Atoms: %d %d %.10f %.10f %.10f\n",i,atomicN, x, y, z);

  }
  // get_3positions(&n, &anum, &x, &y, &z);
  // for (int i=0;i<n;i++) {
  //   printf("%d %d %.10f %.10f %.10f\n",i,anum[i],x[i*3+0],y[i*3+0],z[i*3+0]);
  // }



  //get_atomic_name(atomName, 21);
  int *connected_atom;
  for (int i = 0; i < n; i++) {
    //share_bond(i+1, connected_atom);
    int nstarN;

    num_of_bonds(i+1, &nstarN);

    printf("%d atom has %d bonds\n",i, nstarN);
  }


}
