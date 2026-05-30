#ifndef ASTPRINTER_H
#define ASTPRINTER_H

#include <any>
#include "expr.h"

class AstPrinter : public Visitor {
public:
  std::string print(Expr &expr);

  std::any visitBinaryExpr(const Binary &expr) override;
  std::any visitGroupingExpr(const Grouping &expr) override;
  std::any visitLiteralExpr(const Literal &expr) override;
  std::any visitUnaryExpr(const Unary &expr) override;

private:
  template <typename... Args>
    std::string paranthesize(std::string_view name, Args &...expr);
};

#endif
