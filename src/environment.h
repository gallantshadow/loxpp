#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <any>
#include <string>
#include <memory>
#include <unordered_map>

#include "token.h"

class Environment {
public:
  Environment() : enclosing(nullptr) {};
  explicit Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {};
  void define(std::string name, std::any value);
  std::any get(const Token &name);
  void assign(const Token &name, std::any value);
  void printEnv();

private:
  std::unordered_map<std::string, std::any> values{};
  std::shared_ptr<Environment> enclosing;
};

#endif
