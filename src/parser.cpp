#include "expr.h"
#include "token.h"
#include "token_type.h"
#include "parser.h"

std::unique_ptr<Expr> Parser::parse() {
  try {
    return expression();
  } catch (ParserError error) {
    return nullptr;
  }
}

std::unique_ptr<Expr> Parser::expression() { return equality(); }

std::unique_ptr<Expr> Parser::equality() {
  auto expr = comparison();
  peek();
  while (match(TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL)) {
    Token op = previous();
    auto right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  auto expr = term();
  while (match(TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS,
               TokenType::LESS_EQUAL)) {
    Token op = previous();
    auto right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = factor();
  while (match(TokenType::MINUS, TokenType::PLUS)) {
    Token op = previous();
    auto right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = unary();
  while (match(TokenType::STAR, TokenType::SLASH)) {
    Token op = previous();
    auto right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  while (match(TokenType::BANG, TokenType::MINUS)) {
    Token op = previous();
    auto right = primary();
    return std::make_unique<Unary>(op, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match(TokenType::FALSE))
    return std::make_unique<Literal>(false);
  
  if (match(TokenType::TRUE))
    return std::make_unique<Literal>(true);

  if (match(TokenType::NIL))
    return std::make_unique<Literal>(std::monostate());
  
  if (match(TokenType::NUMBER, TokenType::STRING))
    return std::make_unique<Literal>(previous().literal);

  if (match(TokenType::LEFT_PAREN)) {
    auto expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    return make_unique<Grouping>(std::move(expr));
  }
  throw error(peek(), "Expected expression.");
}

// helpers
template <typename... Args> bool Parser::match(Args ...types) {
  for (TokenType type : {types...}) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) {
  if (isAtEnd())
    return false;
  return peek().token == type;
}

bool Parser::isAtEnd() {
  return peek().token == TokenType::LOX_EOF;
}

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

Token Parser::peek() { return tokens.at(current); }

Token Parser::previous() { return tokens.at(current - 1); }

Token Parser::consume(TokenType type, std::string message) {
  if (check(type))
    return advance();
  throw error(peek(), message);
}

ParserError Parser::error(Token token, const std::string msg) {
    return ParserError(msg);
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
      if (previous().token == TokenType::SEMICOLON) return;

      switch (peek().token) {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
          return;
      }
      advance();
    }
  }
