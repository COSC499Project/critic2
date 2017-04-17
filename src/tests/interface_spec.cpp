#define CATCH_CONFIG_MAIN
#define EPSILON (1.0E-8)

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

TEST_CASE( "Gets correct position for atom of a Molecule", "[get_atom_position]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  double x;
  double y;
  double z;
  int atomicN;

  call_structure(filename, (int) strlen(filename), 1);

  get_atom_position(1, &atomicN, &x, &y, &z);

  REQUIRE( x == 0.0 );
  REQUIRE( fabs(y - 1.393557) < EPSILON );
  REQUIRE( z == 0.0 );
}

TEST_CASE( "Gets correct atomic number for atom of a Molecule", "[get_atom_position]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  double x;
  double y;
  double z;
  int atomicN;

  call_structure(filename, (int) strlen(filename), 1);

  get_atom_position(1, &atomicN, &x, &y, &z);

  REQUIRE( atomicN == 6 );
}

TEST_CASE( "Gets correct position for atom of a Crystal", "[get_atom_position]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  double x;
  double y;
  double z;
  int atomicN;

  call_structure(filename, (int) strlen(filename), 0);

  get_atom_position(1, &atomicN, &x, &y, &z);

  REQUIRE( fabs(x - 5.3443124893) < EPSILON );
  REQUIRE( fabs(y - 12.66830192) < EPSILON );
  REQUIRE( fabs(z - 9.2873686243) < EPSILON );
}

TEST_CASE( "Gets correct number of bonds for Molecule", "[num_of_bonds]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  call_structure(filename, (int) strlen(filename), 1);

  int nStarN;

  num_of_bonds(1, &nStarN);

  REQUIRE( nStarN == 3 );
}

TEST_CASE( "Gets correct number of bonds for Crystal", "[num_of_bonds]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  call_structure(filename, (int) strlen(filename), 0);

  int nStarN;

  num_of_bonds(1, &nStarN);

  REQUIRE( nStarN == 1 );
}

TEST_CASE( "Gets correct bond between atoms of a Molecule", "[get_atom_bond]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  call_structure(filename, (int) strlen(filename), 1);

  int connected_atom;
  bool neighCrystal = false;

  get_atom_bond(1, 1, &connected_atom, &neighCrystal);

  REQUIRE( connected_atom == 2 );
}

TEST_CASE( "Gets correct bond between atoms of a Crystal - not connected to neighbor crystal cell", "[get_atom_bond]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  call_structure(filename, (int) strlen(filename), 0);

  int connected_atom;
  bool neighCrystal = false;

  get_atom_bond(1, 1, &connected_atom, &neighCrystal);

  REQUIRE( connected_atom == 103 );
  REQUIRE( neighCrystal == false );
}

TEST_CASE( "Gets correct bond between atoms of a Crystal - where it is connected to a neighbor crystal cell, ", "[get_atom_bond]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  call_structure(filename, (int) strlen(filename), 0);

  int connected_atom;
  bool neighCrystal = false;

  get_atom_bond(3, 1, &connected_atom, &neighCrystal);

  REQUIRE( connected_atom == 101 );
  REQUIRE( neighCrystal == true );
}

TEST_CASE( "Gets correct number of critical points for Molecule", "[num_of_crit_points]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  call_structure(filename, (int) strlen(filename), 1);
  auto_cp();

  int numCP;
  num_of_crit_points(&numCP);

  REQUIRE( numCP == 25 );
}

TEST_CASE( "Gets correct number of critical points for Crystal", "[num_of_crit_points]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  call_structure(filename, (int) strlen(filename), 0);
  auto_cp();

  int numCP;
  num_of_crit_points(&numCP);

  REQUIRE( numCP == 280 );
}

TEST_CASE( "Gets correct details of critical point for Molecule", "[get_cp_pos_type]" ) {
  init_struct();

  char const *filename = "../../examples/data/benzene.wfx";

  call_structure(filename, (int) strlen(filename), 1);
  auto_cp();

  int cpType;
  double x;
  double y;
  double z;

  get_cp_pos_type(13, &cpType, &x, &y, &z);

  REQUIRE( cpType == -1 );
  REQUIRE( fabs(x - 0.6017982559) < EPSILON );
  REQUIRE( fabs(y - 1.0423451551) < EPSILON );
  REQUIRE( z == 0.0 );
}

TEST_CASE( "Gets correct details of critical point for Crystal", "[get_cp_pos_type]" ) {
  init_struct();

  char const *filename = "../../examples/data/icecake.CHGCAR";

  call_structure(filename, (int) strlen(filename), 0);
  auto_cp();

  int cpType;
  double x;
  double y;
  double z;

  get_cp_pos_type(280, &cpType, &x, &y, &z);

  REQUIRE( cpType == 3 );
  REQUIRE( fabs(x - 5.4759051759) < EPSILON );
  REQUIRE( fabs(y - 11.7470526404) < EPSILON );
  REQUIRE( fabs(z - 19.9102535566) < EPSILON );
}
