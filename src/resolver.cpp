#include "lox.h"
#include "resolver.h"

std::any Resolver::visitBlockStmt(const Block &stmt) {
  beginScope();
  resolve(stmt.statements);
  endScope();
  return {};
}

std::any Resolver::visitExpressionStmt(const Expression &stmt) {
  resolve(*stmt.expression);
  return {};
}

std::any Resolver::visitFunctionStmt(const Function &stmt) {
  declare(stmt.name);
  define(stmt.name);
  resolveFunction(stmt, FunctionType::FUNCTION);
  return {};
}

std::any Resolver::visitIfStmtStmt(const IfStmt &stmt) {
  resolve(*stmt.condition);
  resolve(*stmt.thenBranch);
  if (stmt.elseBranch)
    resolve(*stmt.elseBranch);
  return {};
}

std::any Resolver::visitPrintStmt(const Print &stmt) {
  resolve(*stmt.expression);
  return{};
}

std::any Resolver::visitReturnStmt(const Return &stmt) {
  if (currentFunction == FunctionType::NONE) {
    Lox::error(stmt.keyword, "Can't return from top-level code.");
  }
  
  if(stmt.value)
    resolve(*stmt.value);
  return {};
}

std::any Resolver::visitVarStmt(const Var &stmt) {
  declare(stmt.name);
  if (stmt.initializer) {
    resolve(*stmt.initializer);
  }
  define(stmt.name);
  return {};
}

std::any Resolver::visitWhileStmt(const While &stmt) {
  resolve(*stmt.condition);
  resolve(*stmt.body);
  return {};
}

std::any Resolver::visitAssignExpr(const Assign &expr) {
  resolve(*expr.value);
  resolveLocal(expr, expr.name);
  return {};
}

std::any Resolver::visitBinaryExpr(const Binary &expr) {
  resolve(*expr.left);
  resolve(*expr.right);
  return {};
}

std::any Resolver::visitCallExpr(const Call &expr) {
  resolve(*expr.callee);
  for (auto &argument : expr.arguments) {
    resolve(*argument);
  }
  return {};
}

std::any Resolver::visitGroupingExpr(const Grouping &expr) {
  resolve(*expr.expression);
  return {};
}

std::any Resolver::visitLiteralExpr(const Literal &expr) { return {}; }

std::any Resolver::visitLogicalExpr(const Logical &expr) {
  resolve(*expr.left);
  resolve(*expr.right);
  return {};
}

std::any Resolver::visitUnaryExpr(const Unary &expr) {
  resolve(*expr.right);
  return {};
}

std::any Resolver::visitVariableExpr(const Variable& expr) {
  if (!scopes.empty()) {
    auto& scope = scopes.back();
    auto it = scope.find(std::string(expr.name.lexeme));
    if (it != scope.end() && it->second == false) 
      Lox::error(expr.name, "Can't read local variable in its own initializer.");
  }
  resolveLocal(expr, expr.name);
  return {};
}

void Resolver::resolve(const std::vector<std::unique_ptr<Stmt>> &statements) {
  for (auto &stmt : statements) {
    resolve(*stmt);
  }
}

void Resolver::resolve(const Stmt &stmt) { stmt.accept(*this); }

void Resolver::resolve(const Expr &expr) { expr.accept(*this); }

void Resolver::beginScope() { scopes.emplace_back(); }

void Resolver::endScope() { scopes.pop_back(); }

void Resolver::declare(const Token &name) {
  if (scopes.empty())
    return;
  if (scopes.back().contains(std::string(name.lexeme)))
    Lox::error(name, "Already a variable with this name in this scope.");
  scopes.back().insert_or_assign(std::string(name.lexeme), false);
}

void Resolver::define(const Token &name) {
  if (scopes.empty())
    return;
  scopes.back().insert_or_assign(std::string(name.lexeme), true);
}

void Resolver::resolveLocal(const Expr &expr, const Token &name) {
  for (int i = scopes.size() - 1; i >= 0; i--) {
    if (scopes.at(i).contains(std::string(name.lexeme))) {
      interpreter.resolve(expr, scopes.size() - 1 - i);
      return;
    } 
  }
}

void Resolver::resolveFunction(const Function &function, FunctionType type) {
  FunctionType enclosingFunction = currentFunction;
  currentFunction = type;
  
  beginScope();
  for (auto &token : function.params) {
    declare(token);
    define(token);
  }
  resolve(function.body);
  endScope();

  currentFunction = enclosingFunction;
}


