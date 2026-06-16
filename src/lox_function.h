#ifndef LOXFUNCTION_H
#define LOXFUNCTION_H

#include <any>

#include "environment.h"
#include "lox_callable.h"
#include "stmt.h"

class LoxFunction : public LoxCallable {
public:
  LoxFunction(const Function &declaration, std::shared_ptr<Environment> closure);
  int arity() override;
  std::any call(Interpreter &interpreter, std::vector<std::any> &arguments) override;
  std::string toString() override;
private:
  const Function &declaration;
  std::shared_ptr<Environment> closure;
};

#endif
