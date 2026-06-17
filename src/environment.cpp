#include <print>
#include <format>
#include <string>
#include <unordered_map>

#include "runtime_error.h"
#include "environment.h"

void Environment::assign(const Token &name, std::any value) {
  if (values.contains(std::string(name.lexeme))){
    values.insert_or_assign(std::string(name.lexeme), value);
    return;
  }

  if (enclosing) {
    enclosing->assign(name, value);
    return;
  }
  
  throw RuntimeError{name, std::format("Undefined variable '{}'.", name.lexeme)};
}

void Environment::assignAt(int distance, const Token &name, std::any value) {
  ancestor(distance)->values.insert_or_assign(std::string(name.lexeme), value);
}

void Environment::define(std::string name, std::any value) {
  values.insert_or_assign(name, value);
}

std::any Environment::get(const Token &name) {
  if (values.contains(std::string(name.lexeme)))
    return values.at(std::string(name.lexeme));

  if (enclosing) {
    return enclosing->get(name);
  }
  
  throw RuntimeError{name, std::format("Undefined variable '{}'.", name.lexeme)};
}

std::any Environment::getAt(int distance, const std::string& name) {
  return ancestor(distance)->values.at(name);
}

std::shared_ptr<Environment> Environment::ancestor(int distance) {
  std::shared_ptr<Environment> env = shared_from_this();
  for (int i = 0; i < distance; i++)
    env = env->enclosing;
  return env;
}

void Environment::printEnv() {
  for (auto [a, b] : values) {
    std::print("'{}'\n", a);
  }
  if (!enclosing)
    return;
  std::print("{{\n");
  enclosing->printEnv();
  std::print("}}\n");
}
