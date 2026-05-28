
#include <vector>

#include "lox.h"
#include "scanner.h"
#include "token_type.h"

const std::unordered_map<std::string_view, TokenType> Scanner::keywords = {
    {"and",    TokenType::AND},
    {"class",  TokenType::CLASS},
    {"else",   TokenType::ELSE},
    {"false",  TokenType::FALSE},
    {"for",    TokenType::FOR},
    {"fun",    TokenType::FUN},
    {"if",     TokenType::IF},
    {"nil",    TokenType::NIL},
    {"or",     TokenType::OR},
    {"print",  TokenType::PRINT},
    {"return", TokenType::RETURN},
    {"super",  TokenType::SUPER},
    {"this",   TokenType::THIS},
    {"true",   TokenType::TRUE},
    {"var",    TokenType::VAR},
    {"while",  TokenType::WHILE},
};

std::vector<Token> Scanner::scanTokens() {
  while (!isAtEnd()) {
    start = current;
    scanToken();
  }

  tokens.emplace_back(TokenType::LOX_EOF, "", std::monostate(), line);
  
  return tokens;
}

void Scanner::scanToken() {
  char c = advance();
  switch (c) {
  case '(':
    addToken(TokenType::LEFT_PAREN);
    break;
  case ')':
    addToken(TokenType::RIGHT_PAREN);
    break;
  case '{':
    addToken(TokenType::LEFT_BRACE);
    break;
  case '}':
    addToken(TokenType::RIGHT_BRACE);
    break;
  case ',':
    addToken(TokenType::COMMA);
    break;
  case '.':
    addToken(TokenType::DOT);
    break;
  case '-':
    addToken(TokenType::MINUS);
    break;
  case '+':
    addToken(TokenType::PLUS);
    break;
  case ';':
    addToken(TokenType::SEMICOLON);
    break;
  case '*':
    addToken(TokenType::STAR);
    break;
  case '!':
    addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
    break;
  case '=':
    addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
    break;
  case '>':
    addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
    break;
  case '<':
    addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
    break;
  case '/':
    if (match('/')) {
      while(!isAtEnd() && peek() != '\n') advance();
    } else {
      addToken(TokenType::SLASH);
    }
    break;
  case ' ':
  case '\t':
  case '\r':
    break;
  case '\n':
    line++;
    break;
  case '"':
    stringLiteral();
    break;
  default:
    if (isDigit(c)) {
      number();
    } else if (isAlpha(c)) {
      identifier();
    }
    else {
      Lox::error(line, "Unexpected character.");
    }
    break;
  }
}

bool Scanner::isAtEnd() const { return current >= source.size(); }

char Scanner::advance() { return source.at(current++); }

void Scanner::addToken(TokenType token) {
  addToken(token, std::monostate());
}

void Scanner::addToken(TokenType token, LiteralType literal) {
  std::string_view text = source.substr(start, current - start);
  tokens.emplace_back(token, text, literal, line);
}

bool Scanner::match(char expected) {
  if(isAtEnd()) return false;
  if (expected != source.at(current)) return false;
  current++;
  return true;
}

char Scanner::peek() const {
  if(isAtEnd()) return '\0';
  return source.at(current);
}

void Scanner::stringLiteral() {
  while (peek() != '"' && !isAtEnd()) {
    if(peek() == '\n') line++;
    advance();
  }
  if (isAtEnd()) {
    Lox::error(line, "Unterminated string.");
    return;
  }
  advance();
  std::string_view value = source.substr(start + 1, current - start - 2);
  addToken(TokenType::STRING, value);
}

bool Scanner::isDigit(char c) const { return c >= '0' && c <= '9'; }

void Scanner::number() {
  while (isDigit(peek()))
    advance();

  if (peek() == '.' && isDigit(peekNext())) {
    advance();
    while (isDigit(peek()))
      advance();
  }
  addToken(TokenType::NUMBER, stod(std::string(source.substr(start, current - start))));
}

char Scanner::peekNext() const {
  if (current + 1 >= source.size())
    return '\0';
  return source.at(current+1);
}

bool Scanner::isAlpha(char c) const {
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c == '_';
}

bool Scanner::isAlphaNumberic(char c) const { return isAlpha(c) || isDigit(c); }

void Scanner::identifier() {
  while (isAlphaNumberic(peek()))
    advance();

  std::string_view ident = source.substr(start, current - start);

  TokenType token = TokenType::IDENTIFIER;
  
  if (keywords.contains(ident)) {
    token = keywords.at(ident);
  }
  
  addToken(token);
}

