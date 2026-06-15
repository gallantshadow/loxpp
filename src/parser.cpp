#include "expr.h"
#include "lox.h"
#include "token.h"
#include "token_type.h"
#include "parser.h"
#include "stmt.h"

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements{};
  while (!isAtEnd()) {
    statements.push_back(declaration());
  }
  return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
  try {
    if (match(TokenType::VAR))
      return varDeclaration();
    return statement();
  } catch (ParserError error) {
    synchronize();
    return {};
  }
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
  Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
  std::unique_ptr<Expr> initializer = nullptr;
  if (match(TokenType::EQUAL)) {
    initializer = expression();
  }
  consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
  return make_unique<Var>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match(TokenType::PRINT))
    return printStatement();

  if (match(TokenType::LEFT_BRACE)) {
    return std::make_unique<Block>(block());
  }

  if (match(TokenType::IF)){
    return ifStatement();
  }
  
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
  std::unique_ptr<Expr> expr = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

  std::unique_ptr<Stmt> thenBranch = statement();
  std::unique_ptr<Stmt> elseBranch = nullptr;
  if (match(TokenType::ELSE)) {
    elseBranch = statement();
  }

  return make_unique<IfStmt>(std::move(expr), std::move(thenBranch),
                             std::move(elseBranch));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> statements{};
  while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    statements.push_back(declaration());
  }
  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

std::unique_ptr<Stmt> Parser::printStatement() {
  auto expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_unique<Print>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  auto expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return std::make_unique<Expression>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
  auto expr = logicOr();
  if (match(TokenType::EQUAL)) {
    Token equals = previous();
    auto value = assignment();

    if (auto *var = dynamic_cast<Variable *>(expr.get())) {
      return std::make_unique<Assign>(var->name, std::move(value));
    }
    
    error(equals, "Invalid assignment target.");
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicOr() {
  auto expr = logicAnd();
  while (match(TokenType::OR)) {
    Token op = previous();
    auto right = logicAnd();
    expr = make_unique<Logical>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::logicAnd() {
  auto expr = equality();
  while (match(TokenType::AND)) {
    Token op = previous();
    auto right = equality();
    expr = make_unique<Logical>(std::move(expr), op, std::move(right));
  }
  return expr;
}


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

  if (match(TokenType::IDENTIFIER)) {
    return std::make_unique<Variable>(previous());
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
  Lox::error(token, msg);
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
