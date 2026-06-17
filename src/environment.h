#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <any>
#include <string>
#include <memory>
#include <unordered_map>

#include "token.h"

class Environment : public std::enable_shared_from_this<Environment> {
public:
  Environment() : enclosing(nullptr) {};
  explicit Environment(std::shared_ptr<Environment> enclosing)
      : enclosing(enclosing) {};

  void assign(const Token &name, std::any value);
  void assignAt(int distance, const Token &name, std::any value);
  void define(std::string name, std::any value);
  std::any get(const Token &name);
  std::any getAt(int distance, const std::string &name);
  std::shared_ptr<Environment> ancestor(int distance);
  void printEnv();

private:
  std::unordered_map<std::string, std::any> values{};
  std::shared_ptr<Environment> enclosing;
};

#endif
