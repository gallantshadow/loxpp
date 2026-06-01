#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <print>

#include "ast_printer.h"
#include "lox.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"

namespace fs = std::filesystem;

bool Lox::hadError = false;

int Lox::runFile(const fs::path &path) {
  std::ifstream programFile{path};
  if (!programFile) {
    std::println(stderr, "Could not open the file: {}", path.string());
    return 66;
  }
  std::stringstream ss;
  ss << programFile.rdbuf();
  run(ss.view());

  if (hadError)
    return 65;
  
  return 0;
}

void Lox::runPrompt() {
  while (true) {
    std::cout << "|> ";
    std::string line;
    if (!std::getline(std::cin, line))
      break;
    run(line);
    hadError = false;
  }
}

void Lox::run(std::string_view source) {
  Scanner scanner{source};
  std::vector<Token> tokens = scanner.scanTokens();

  Parser parser(tokens);
  auto expr = parser.parse();
  
  std::println("{}", AstPrinter().print(*expr));
}

void Lox::report(int line, std::string_view where, std::string_view message) {
  std::println(stderr, "[line {}] Error{}: {}", line, where, message);
  hadError = true;
}

void Lox::error(int line, std::string_view message) {
  report(line, "", message);
}

void Lox::error(Token token, std::string_view message) {
  if (token.token == TokenType::LOX_EOF)
    report(token.line, " at end", message);
  else
    report(token.line, std::format(" at '{}'", token.lexeme), message);
}
