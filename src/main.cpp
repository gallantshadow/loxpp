#include "lox.h"
#include <iostream>

int main(int argc, char *argv[]) {
  Lox lox;
  if (argc > 2) {
    std::cerr << "Usage: lox [script]\n";
    return 64;
  } else if (argc == 2) {
    return lox.runFile(argv[1]);
  } else {
    lox.runPrompt();
  }
  return  0;
}
