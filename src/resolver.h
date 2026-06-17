#ifndef RESOVLER_H
#define RESOLVER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "expr.h"
#include "stmt.h"

class Interpreter;

class Resolver : public ExprVisitor, public StmtVisitor {
public:
  explicit Resolver(Interpreter &interpreter)
      : interpreter(interpreter) {};
  void resolve(const std::vector<std::unique_ptr<Stmt>> &statements);

private:
   enum class FunctionType {
    NONE,
    FUNCTION
  };
  // StmtVisitor
  std::any visitBlockStmt(const Block &stmt) override;
  std::any visitExpressionStmt(const Expression &expr) override;
  std::any visitFunctionStmt(const Function &stmt) override;
  std::any visitIfStmtStmt(const IfStmt &stmt) override;
  std::any visitPrintStmt(const Print &stmt) override;
  std::any visitReturnStmt(const Return &stmt) override;
  std::any visitVarStmt(const Var &stmt) override;
  std::any visitWhileStmt(const While &stmt) override;

  // ExprVisitor
  std::any visitAssignExpr(const Assign &expr) override;
  std::any visitBinaryExpr(const Binary &expr) override;
  std::any visitCallExpr(const Call &expr) override;
  std::any visitGroupingExpr(const Grouping& expr) override;
  std::any visitLiteralExpr(const Literal &expr) override;
  std::any visitLogicalExpr(const Logical &expr) override;
  std::any visitUnaryExpr(const Unary &expr) override;
  std::any visitVariableExpr(const Variable &expr) override;

  // helpers
  void resolve(const Stmt &stmt);
  void resolve(const Expr &expr);
  void resolveFunction(const Function& stmt, FunctionType type);
  void beginScope();
  void endScope();
  void declare(const Token &name);
  void define(const Token &name);
  void resolveLocal(const Expr &expr, const Token &name);


  Interpreter &interpreter;
  std::vector<std::map<std::string, bool>> scopes;
  FunctionType currentFunction = FunctionType::NONE;
};

#endif
