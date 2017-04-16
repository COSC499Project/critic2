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
extern "C" void get_atom_bond(int n_atom, int nstarIdx, int *connected_atom, bool *neighCrystal);
extern "C" void auto_cp();
extern "C" void num_of_crit_points(int *n_critp);
extern "C" void get_cp_pos_type(int cpIdx, int *type, double *x, double *y, double *z);


int main(int argc, char ** argv)
{
  char const *filename = "../../examples/data/icecake.CHGCAR";

  int n; // number of atoms

  char const *atomName;

  initialize();
  init_struct();
  call_structure(filename, (int) strlen(filename), 0);
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

  for (int i = 0; i < n; i++) {
    //share_bond(i+1, connected_atom);
    int nstarN;

    num_of_bonds(i+1, &nstarN);

    for (int j = 0; j < nstarN; j++) {
      int connected_atom;
      bool neighCrystal = false;

      get_atom_bond(i+1, j+1, &connected_atom, &neighCrystal);
      printf("%d atom has %d bonds and one is %d and %d\n",i, nstarN, connected_atom-1, neighCrystal);
    }
  }

  // auto_cp();
  //
  // int numCP;
  // num_of_crit_points(&numCP);
  //
  // for (int i = n+1; i <= numCP; i++) {
  //   int cpType;
  //   double x;
  //   double y;
  //   double z;
  //
  //   get_cp_pos_type(i, &cpType, &x, &y, &z);
  //
  //   printf("Critical Points: %d %d %.10f %.10f %.10f\n",i,cpType, x, y, z);
  // }

}
