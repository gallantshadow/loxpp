#ifndef SCANNER_H

#include <string_view>
#include <unordered_map>
#include <vector>

#include "token.h"
#include "token_type.h"

class Scanner {
public:
  Scanner(std::string_view source) : source(source) {};
  std::vector<Token> scanTokens();

private:
  void scanToken();
  bool isAtEnd() const;
  char advance();
  void addToken(TokenType token);
  void addToken(TokenType token, Literal literal);
  bool match(char expected);
  char peek() const;
  void stringLiteral();
  bool isDigit(char c) const;
  void number();
  char peekNext() const;
  bool isAlpha(char c) const;
  bool isAlphaNumberic(char c) const;
  void identifier();

  const std::string_view source;
  static const std::unordered_map<std::string_view, TokenType> keywords;
  std::vector<Token> tokens{};
  std::size_t start{0};
  std::size_t current{0};
  std::size_t line{1};
};

#endif
