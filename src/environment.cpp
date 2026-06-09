#include <format>
#include <string>
#include <unordered_map>

#include "runtime_error.h"
#include "environment.h"

void Environment::define(std::string name, std::any value) {
  values.insert_or_assign(name, value);
}

std::any Environment::get(const Token &name) {
  if (values.contains(std::string(name.lexeme)))
    return values.at(std::string(name.lexeme));

  if(enclosing != nullptr) enclosing->get(name);
  
  throw RuntimeError{name, std::format("Undefined variable '{}'.", name.lexeme)};
}

void Environment::assign(const Token &name, std::any value) {
  if (values.contains(std::string(name.lexeme))){
    values.insert_or_assign(std::string(name.lexeme), value);
    return;
  }

  if (enclosing != nullptr) {
    enclosing->assign(name, value);
    return;
  }
  
  throw RuntimeError{name, std::format("Undefined variable '{}'.", name.lexeme)};
}
