#include <sstream>

#include "ast_printer.h"

std::string AstPrinter::print(Expr &expr) {
  return std::any_cast<std::string>(expr.accept(*this));
}

template <typename... Args>
std::string AstPrinter::paranthesize(std::string_view name,
                                     Args &...exprs) {
  std::ostringstream os;
  os << "(" << name;
  ((os <<  " " << std::any_cast<std::string>(exprs.accept(*this))), ...);
  os << ")";
  return os.str();
}


std::any AstPrinter::visitBinaryExpr(const Binary &expr) {
  return paranthesize(expr.op.lexeme, *expr.left, *expr.right);
}

std::any AstPrinter::visitGroupingExpr(const Grouping &expr) {
  return paranthesize("group", *expr.expression);
}

std::any AstPrinter::visitLiteralExpr(const Literal &expr) {
  return literalTypeToString(expr.value);
}

std::any AstPrinter::visitUnaryExpr(const Unary &expr) {
  return paranthesize(expr.op.lexeme, *expr.right);
}
