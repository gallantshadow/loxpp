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
    if (match(TokenType::FUN))
      return function("function");
    
    if (match(TokenType::VAR))
      return varDeclaration();
    
    return statement();
  } catch (ParserError error) {
    synchronize();
    return {};
  }
}

std::unique_ptr<Stmt> Parser::function(std::string kind) {
  Token name =
      consume(TokenType::IDENTIFIER, std::format("Expect {} name", kind));
  consume(TokenType::LEFT_PAREN, std::format("Expect '(' after {} name", kind));

  //parse parameter
  std::vector<Token> parameters{};
  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      if (parameters.size() > 255) {
	error(peek(), "Can't have more than 255 parameters.");
      }

      parameters.push_back(
          consume(TokenType::IDENTIFIER, "Expect parameter name."));
    } while (match(TokenType::COMMA));
  }
  consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
  // parse body
  consume(TokenType::LEFT_BRACE, std::format("Expect '{{' before {} body", kind));
  auto body = block();
  return make_unique<Function>(name, std::move(parameters), std::move(body));
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
  if (match(TokenType::FOR)) {
    return forStatement();
  }
  if (match(TokenType::IF)){
    return ifStatement();
  }
  if (match(TokenType::LEFT_BRACE)) {
    return std::make_unique<Block>(block());
  }
  if (match(TokenType::PRINT)) {
    return printStatement();
  }
  if (match(TokenType::RETURN)) {
    return returnStatement();
  }
  if (match(TokenType::WHILE)) {
    return whileStatement();
  }

  
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  auto expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after expression.");
  return std::make_unique<Expression>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::forStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
  std::unique_ptr<Stmt> initializer = nullptr;
  if (match(TokenType::SEMICOLON)) {
    initializer = nullptr;
  } else if (match(TokenType::VAR)) {
    initializer = varDeclaration();
  } else {
    initializer = expressionStatement();
  }

  std::unique_ptr<Expr> condition = nullptr;
  if (!check(TokenType::SEMICOLON)) {
    condition = expression();
  }
  consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

  std::unique_ptr<Expr> increment = nullptr;
  if (!check(TokenType::RIGHT_PAREN)) {
    increment = expression();
  }

  consume(TokenType::RIGHT_PAREN, "Expect ')' after for clause.");

  std::unique_ptr<Stmt> body = statement();
  
  if (increment) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(std::move(body));
    stmts.push_back(std::make_unique<Expression>(std::move(increment)));
    body = std::make_unique<Block>(std::move(stmts));
  }

  if (!condition) {
    condition = std::make_unique<Literal>(true) ;
  }

  body = std::make_unique<While>(std::move(condition), std::move(body));

  if (initializer) {
    std::vector<std::unique_ptr<Stmt>> stmts;
    stmts.push_back(std::move(initializer));
    stmts.push_back(std::move(body));
    body = std::make_unique<Block>(std::move(stmts));
  }

  return body;
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

std::unique_ptr<Stmt> Parser::printStatement() {
  auto expr = expression();
  consume(TokenType::SEMICOLON, "Expect ';' after value.");
  return std::make_unique<Print>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
  Token keyword = previous();
  
  std::unique_ptr<Expr> value{};
  if (!check(TokenType::SEMICOLON)) {
    value = expression();
  }
  consume(TokenType::SEMICOLON, "Expect ';' after return value.");  
  return std::make_unique<Return>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
  consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
  std::unique_ptr<Expr> condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

  std::unique_ptr<Stmt> body = statement();
  return make_unique<While>(std::move(condition), std::move(body));
}


std::vector<std::unique_ptr<Stmt>> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> statements{};
  while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
    statements.push_back(declaration());
  }
  consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
  return statements;
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
  return call();
}


std::unique_ptr<Expr> Parser::call() {
  auto expr = primary();

  while (true) {
    if (match(TokenType::LEFT_PAREN)) {
      expr = finishCall(std::move(expr));
    } else {
      break;
    }
  }
   
  return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
  std::vector<std::unique_ptr<Expr>> arguments;
  if (!check(TokenType::RIGHT_PAREN)) {
    do {
      if (arguments.size() >= 255) {
        error(peek(), "Can't have more than 255 arguments");
      }
      arguments.push_back(expression());
    } while (match(TokenType::COMMA));
  }

  Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

  return std::make_unique<Call>(std::move(callee), paren, std::move(arguments));
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
