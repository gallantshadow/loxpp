#include <print>
#include <vector>
#include <chrono>
#include <format>

#include "interpreter.h"
#include "expr.h"
#include "token.h"
#include "runtime_error.h"
#include "lox.h"
#include "lox_callable.h"
#include "lox_function.h"
#include "return.h"

Interpreter::Interpreter() {
  class ClockCallable : public LoxCallable {
    int arity() override { return 0; }
    std::any call(Interpreter &interpreter, std::vector<std::any> &arguments) override {
      using namespace std::chrono;
      return duration<double>(
			      high_resolution_clock::now().time_since_epoch()).count();
    }
    std::string toString() override {
      return std::format("<native fn>");
    }
  };
  std::shared_ptr<LoxCallable> clock = std::make_shared<ClockCallable>();
  globals->define("clock", clock);
  environment = globals;
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>> &statements) {
  try {
    for (auto &stmt : statements) {
      execute(*stmt);
    }
  } catch (RuntimeError error) {
    Lox::runtimeError(error);
  }
}

std::any Interpreter::visitIfStmtStmt(const IfStmt &stmt) {
  auto expr = evaluate(*stmt.condition);
  if (isTruthy(expr)) {
    execute(*stmt.thenBranch);
  } else if (stmt.elseBranch) {
    execute(*stmt.elseBranch);
  }
  return nullptr;
}

std::any Interpreter::visitPrintStmt(const Print &stmt) {
  auto value = evaluate(*stmt.expression);
  std::print("{}\n", stringify(value));
  return nullptr;
}

std::any Interpreter::visitReturnStmt(const Return &stmt) {
  std::any value = nullptr;
  if (stmt.value)
    value = evaluate(*stmt.value);
  throw ReturnValue{value};
}

std::any Interpreter::visitWhileStmt(const While &stmt) {
  auto value = evaluate(*stmt.condition);
  while (isTruthy(value)) {
    execute(*stmt.body);
    value = evaluate(*stmt.condition);
  }
  return nullptr;
}

std::any Interpreter::visitExpressionStmt(const Expression &stmt) {
  evaluate(*stmt.expression);
  return nullptr;
}

std::any Interpreter::visitFunctionStmt(const Function &stmt) {
  std::shared_ptr<LoxCallable> function = std::make_shared<LoxFunction>(stmt, environment);
  environment->define(std::string(stmt.name.lexeme), function);
  return nullptr;
}

std::any Interpreter::visitVarStmt(const Var &stmt) {
  std::any value = {};
  if (stmt.initializer != nullptr) {
    value = evaluate(*stmt.initializer);
  }
  environment->define(std::string(stmt.name.lexeme), value);
  return nullptr;
}

std::any Interpreter::visitBlockStmt(const Block &stmt) {
  executeBlock(stmt.statements, std::make_shared<Environment>(environment));
  return nullptr;
}

std::any Interpreter::visitLiteralExpr(const Literal &expr) {
  return std::visit(
      overloads{
          [](std::monostate) -> std::any { return {}; },
          [](std::string_view s) -> std::any { return std::string(s); },
          [](auto v) -> std::any { return v; },
      },
      expr.value);
}

std::any Interpreter::visitLogicalExpr(const Logical &expr) {
  std::any value = evaluate(*expr.left);

  if(expr.op.token == TokenType::OR) {
    if (isTruthy(value))
      return value;
  } else {
    if (!isTruthy(value))
      return value;
  }   
  return evaluate(*expr.right);
}

std::any Interpreter::visitGroupingExpr(const Grouping &expr) {
  return evaluate(*expr.expression);
}

std::any Interpreter::visitUnaryExpr(const Unary &expr) {
  std::any right = evaluate(*expr.right);

  switch (expr.op.token) {
  case TokenType::BANG:
    return !isTruthy(right);
  case TokenType::MINUS:
    checkNumberOperand(expr.op, right);
    return -std::any_cast<double>(right);
  default:
    // should not be possible this would be parsing error
    break;
  }
  return nullptr;
}

std::any Interpreter::visitBinaryExpr(const Binary &expr) {
  std::any left = evaluate(*expr.left);
  std::any right = evaluate(*expr.right);

  switch (expr.op.token) {
  case TokenType::MINUS:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) - std::any_cast<double>(right);
  case TokenType::STAR:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) * std::any_cast<double>(right);
  case TokenType::SLASH:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) / std::any_cast<double>(right);
  case TokenType::PLUS:
    if (left.type() == typeid(double) && right.type() == typeid(double))
      return std::any_cast<double>(left) + std::any_cast<double>(right);

    if (left.type() == typeid(std::string) &&
        right.type() == typeid(std::string))
      return std::any_cast<std::string>(left) +
             std::any_cast<std::string>(right);
    throw RuntimeError(expr.op, "Operands must be two numbers or two strings.");
  case TokenType::GREATER:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) > std::any_cast<double>(right);
  case TokenType::GREATER_EQUAL:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) >= std::any_cast<double>(right);
  case TokenType::LESS:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) < std::any_cast<double>(right);
  case TokenType::LESS_EQUAL:
    checkNumberOperands(expr.op, left, right);
    return std::any_cast<double>(left) <= std::any_cast<double>(right);
  case TokenType::BANG_EQUAL:
    return !isEqual(left, right);
  case TokenType::EQUAL_EQUAL :
    return isEqual(left, right);
  default:
    // should not be possible this would be parsing error
    break;
  }
  return nullptr;
}

std::any Interpreter::visitCallExpr(const Call &expr) {
  std::any callee = evaluate(*expr.callee);

  std::vector<std::any> arguments{};
  for (auto &argument : expr.arguments) {
    arguments.push_back(evaluate(*argument));
  }

  if (callee.type() != typeid(std::shared_ptr<LoxCallable>))
    throw RuntimeError(expr.paren, "Can only call functions and classes.");


  auto function = std::any_cast<std::shared_ptr<LoxCallable>>(callee);

  if (arguments.size() != function->arity())
    throw RuntimeError(expr.paren, "Expected " +
                                       std::to_string(function->arity()) +
                                       " arguments but got " +
                                       std::to_string(arguments.size()) + ".");

  return function->call(*this, arguments);
}

std::any Interpreter::visitVariableExpr(const Variable &expr) {
  return environment->get(expr.name);
}

std::any Interpreter::visitAssignExpr(const Assign &expr) {
  std::any value = evaluate(*expr.value);
  environment->assign(expr.name, value);
  return value;
}

void Interpreter::execute(const Stmt &stmt) { stmt.accept(*this); }

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>> &statements,
			       std::shared_ptr<Environment> newEnvironment) {
  struct Restore {
    std::shared_ptr<Environment> &env;
    std::shared_ptr<Environment> saved;
    ~Restore() {
      env = std::move(saved);
    }
  } guard{environment, environment};

  environment = std::move(newEnvironment);

  for (auto &stmt : statements) {
    execute(*stmt);
  }
}

std::any Interpreter::evaluate(const Expr &expr) {
  return expr.accept(*this);
}

bool Interpreter::isTruthy(std::any& value) {
  if (!value.has_value())
    return false;
  if (value.type() == typeid(bool))
    return std::any_cast<bool>(value);
  return true;
}

bool Interpreter::isEqual(std::any &a, std::any &b) {
  if (!a.has_value() && !b.has_value())
    return true;
  if (!a.has_value() || !b.has_value())
    return true;
  if (a.type() != b.type()) return false; 

  if (a.type() == typeid(double))
    return std::any_cast<double>(a) == std::any_cast<double>(b);
  if (a.type() == typeid(bool))
    return std::any_cast<bool>(a) == std::any_cast<bool>(b);
  if (a.type() == typeid(std::string))
    return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
  return false;
}

void Interpreter::checkNumberOperand(Token op, std::any &operand) {
  if (operand.type() == typeid(double))
    return;
  throw RuntimeError(op, "Operand must be number.");
}

void Interpreter::checkNumberOperands(Token op, std::any &left,
                                     std::any &right) {
  if (left.type() == typeid(double) && right.type() == typeid(double))
    return;
  throw RuntimeError(op, "Operand must be number.");
}

std::string Interpreter::stringify(std::any obj) {
  if (!obj.has_value())
    return "nil";
  if (obj.type() == typeid(double)) {
    std::string text = std::format("{}", std::any_cast<double>(obj));
    if (text.ends_with(".0"))
      text = text.substr(0, text.size() - 2);
    return text;
  }
  if (obj.type() == typeid(bool))
    return std::format("{}", std::any_cast<bool>(obj));

  if (obj.type() == typeid(std::string))
    return std::any_cast<std::string>(obj);

  if (obj.type() == typeid(std::shared_ptr<LoxCallable>))
    return std::any_cast<std::shared_ptr<LoxCallable>>(obj)->toString();

  return "";
}
