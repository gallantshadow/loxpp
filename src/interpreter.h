#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <memory>
#include <vector>

#include "environment.h"
#include "expr.h"
#include "lox_function.h"
#include "stmt.h"

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  Interpreter();
  void interpret(const std::vector<std::unique_ptr<Stmt>> &statments);
  void executeBlock(const std::vector<std::unique_ptr<Stmt>> &statements,
                    std::shared_ptr<Environment> newEnvironment);

  // StmtVisitor
  std::any visitIfStmtStmt(const IfStmt &stmt) override;
  std::any visitPrintStmt(const Print &stmt) override;
  std::any visitReturnStmt(const Return &stmt) override;
  std::any visitWhileStmt(const While &stmt) override;
  std::any visitExpressionStmt(const Expression &expr) override;
  std::any visitFunctionStmt(const Function &stmt) override;
  std::any visitVarStmt(const Var &stmt) override;
  std::any visitBlockStmt(const Block & stmt) override;

  //ExprVisitor
  std::any visitBinaryExpr(const Binary& expr) override;
  std::any visitGroupingExpr(const Grouping& expr) override;
  std::any visitLiteralExpr(const Literal &expr) override;
  std::any visitLogicalExpr(const Logical &expr) override;
  std::any visitUnaryExpr(const Unary &expr) override;
  std::any visitCallExpr(const Call &expr) override;
  std::any visitVariableExpr(const Variable &expr) override;
  std::any visitAssignExpr(const Assign &expr) override;

private:
  void execute(const Stmt &stmt);
  std::any evaluate(const Expr &expr);
  bool isTruthy(std::any &object);
  bool isEqual(std::any &a, std::any &b);
  void checkNumberOperand(Token op, std::any &operand);
  void checkNumberOperands(Token op, std::any &left, std::any &right);
  std::string stringify(std::any obj);

  std::shared_ptr<Environment> environment{};
  std::shared_ptr<Environment> globals{std::make_shared<Environment>()};

  friend class LoxFunction;
};

#endif
