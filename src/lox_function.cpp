#include <format>
#include <memory>

#include "environment.h"
#include "lox_function.h"
#include "interpreter.h"
#include "return.h"


LoxFunction::LoxFunction(const Function& declaration, std::shared_ptr<Environment> closure) : declaration(declaration), closure(std::move(closure)) {
}

int LoxFunction::arity() { return declaration.params.size(); };

std::any LoxFunction::call(Interpreter &interpreter,
                           std::vector<std::any> &arguments) {
  std::shared_ptr<Environment> env = std::make_shared<Environment>(closure);
  for (int i = 0; i < declaration.params.size(); i++) {
    env->define(std::string(declaration.params.at(i).lexeme), arguments.at(i));
  }
  try {
    interpreter.executeBlock(declaration.body, env);
  } catch (ReturnValue returnValue) {
    return returnValue.value;
  }
  return nullptr;
}

std::string LoxFunction::toString() {
  return std::format("<fn {}>", declaration.name.lexeme);
}

