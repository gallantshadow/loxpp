#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>

#include "expr.h"
#include "stmt.h"

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  void interpret(const std::vector<std::unique_ptr<Stmt>> &statments);
  std::any visitPrintStmt(const Print &stmt) override;
  std::any visitExpressionStmt(const Expression &expr) override;
  std::any visitBinaryExpr(const Binary& expr) override;
  std::any visitGroupingExpr(const Grouping& expr) override;
  std::any visitLiteralExpr(const Literal& expr) override;
  std::any visitUnaryExpr(const Unary &expr) override;

private:
  std::any execute(const Stmt &stmt);
  std::any evaluate(const Expr &expr);
  bool isTruthy(std::any &object);
  bool isEqual(std::any &a, std::any &b);
  void checkNumberOperand(Token op, std::any &operand);
  void checkNumberOperands(Token op, std::any &left, std::any &right);
  std::string stringify(std::any obj);
};

#endif
