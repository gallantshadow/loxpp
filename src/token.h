#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <string_view>
#include <variant>

#include "token_type.h"

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
  using Ts::operator()...;
};

using LiteralType = std::variant<std::monostate, double, std::string_view, bool>;
std::string literalTypeToString(const LiteralType& literal);

class Token {
public:
  const TokenType token;
  const std::string_view lexeme;
  const LiteralType literal;
  const std::size_t line;
  
  Token(TokenType type, std::string_view lexeme, LiteralType literal, std::size_t line)
      : token(type), lexeme(lexeme), literal(literal), line(line) {}

  std::string toString() const;
};

#endif
