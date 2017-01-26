#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

extern "C" void call_crystal(const char *filename, int* size);

int main(int argc, char ** argv)
{
  char const *filename = "/Users/Goreng/dev/critic2/examples/data/thymine_rho.cube";
  int len = 57;

  call_crystal(filename, &len);
}
