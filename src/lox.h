#ifndef LOX_H
#define LOX_H

#include <filesystem>
#include <string_view>

#include "runtime_error.h"
#include "token.h"
#include "interpreter.h"

class Lox {
public:
  int runFile(const std::filesystem::path& path);
  void runPrompt();
  static void error(int line, std::string_view message);
  static void error(Token line, std::string_view message);
  static void runtimeError(RuntimeError& error);

private:
  void run(std::string_view source);
  static void report(int line, std::string_view where,
                     std::string_view message);
  static bool hadError;
  static bool hadRuntimeError;
  static Interpreter interpreter;
};

#endif
