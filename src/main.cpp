#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

#include "Scanner.h"

int main(int argc, const char *argv[]) {
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  Scanner scanner;

  auto file = fopen(input, "r");
  assert(file);
  
  scanner.parse(file);

  cout << "AST: " << scanner.ast.ToString() << endl;
  return 0;
}