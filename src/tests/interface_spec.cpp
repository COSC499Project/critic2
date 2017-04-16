#define CATCH_CONFIG_MAIN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include "catch.hh"

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


TEST_CASE( "Gets correct number of atoms for Molecule", "[get_num_atoms]" ) {

  initialize();
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  int n; // number of atoms

  call_structure(filename, (int) strlen(filename), 1);

  get_num_atoms(&n);

  REQUIRE( n == 12 );
}

TEST_CASE( "Gets correct number of atoms for Crystal", "[get_num_atoms]" ) {

  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  int n; // number of atoms

  call_structure(filename, (int) strlen(filename), 0);

  get_num_atoms(&n);

  REQUIRE( n == 120 );
}
