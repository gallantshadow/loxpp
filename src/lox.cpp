#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <print>

#include "interpreter.h"
#include "lox.h"
#include "parser.h"
#include "resolver.h"
#include "scanner.h"
#include "token.h"

namespace fs = std::filesystem;

bool Lox::hadError = false;
bool Lox::hadRuntimeError = false;
Interpreter Lox::interpreter{};

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
  if (hadRuntimeError)
    return 70;
  
  return 0;
}

void Lox::runPrompt() {
  std::string line;
  while (true) {
    std::cout << "|> ";
    if (!std::getline(std::cin, line))
      break;
    sessionSources.push_back(line);
    run(sessionSources.back());
    hadError = false;
  }
}

void Lox::run(std::string_view source) {
  Scanner scanner{source};
  std::vector<Token> tokens = scanner.scanTokens();
  Parser parser(tokens);
  std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
  if (hadError)
    return;
  Resolver resolver{interpreter};
  resolver.resolve(statements);
  if (hadError)
    return;
  interpreter.interpret(statements);
  sessionAsts.push_back(std::move(statements));
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

void Lox::runtimeError(RuntimeError &error) {
  std::println(stderr, "{}\n[line {}]", error.what(), error.token.line);
  hadRuntimeError = true;
}
