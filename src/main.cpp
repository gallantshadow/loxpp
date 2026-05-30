#include "lox.h"
#include "ast_printer.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "=== AST printer test ===" << std::endl;

  auto expression = std::make_unique<Binary>(
      std::make_unique<Unary>(Token(TokenType::MINUS, "-", std::monostate{}, 1),
                              std::make_unique<Literal>(123.0)),
      Token(TokenType::STAR, "*", std::monostate{}, 1),
      std::make_unique<Grouping>(std::make_unique<Literal>(45.67)));

  std::cout << AstPrinter().print(*expression) << '\n';
  
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
