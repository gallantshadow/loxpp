#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <string_view>

enum class TokenType {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character token.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals.
  IDENTIFIER,
  NUMBER,
  STRING,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  LOX_EOF
};

inline constexpr std::string_view tokenToString(TokenType token) {
  constexpr std::string_view names[] = {
      "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE", "COMMA", "DOT",
      "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR",

      // One or two character token.
      "BANG", "BANG_EQUAL", "EQUAL", "EQUAL_EQUAL", "GREATER", "GREATER_EQUAL",
      "LESS", "LESS_EQUAL",

      // Literals.
      "IDENTIFIER", "NUMBER", "STRING",

      // Keywords.
      "AND", "CLASS", "ELSE", "FALSE", "FUN", "FOR", "IF", "NIL", "OR", "PRINT",
      "RETURN", "SUPER", "THIS", "TRUE", "VAR", "WHILE",

      "LOX_EOF"};

  static_assert(
      std::size(names) ==
      static_cast<std::underlying_type_t<TokenType>>(TokenType::LOX_EOF) + 1);
  
  return names[static_cast<std::underlying_type_t<TokenType>>(token)];
}

#endif
