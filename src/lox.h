#ifndef LOX_H
#define LOX_H

#include <filesystem>
#include <string_view>

class Lox {
public:
  int runFile(const std::filesystem::path& path);
  void runPrompt();
  static void error(int line, std::string_view message);

private:
  void run(std::string_view source);
  static void report(int line, std::string_view where,
                     std::string_view message);
  static bool hadError;
};

#endif
