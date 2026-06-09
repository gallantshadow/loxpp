#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>

#include "token.h"
#include "expr.h"
#include "stmt.h"


class ParserError : public std::runtime_error {
public:
  ParserError(const std::string &msg) : std::runtime_error(msg) {}
};

/**
   program        → declaration* EOF;
   declaration    → varDecl | statement;
   statement      → exprStmt | printStmt | block;
   block          → "{" declaration* "}" ;
   varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
   exprStmt       → expression ";" ;
   printStmt      → "print" expression ";" ;
   expression     → assignment ;
   assignement    → IDENTIFIER "=" assignment | equality ;
   equality       → comparison ( ( "!=" | "==" ) comparison )* ;
   comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
   term           → factor ( ( "-" | "+" ) factor )* ;
   factor         → unary ( ( "/" | "*" ) unary )* ;
   unary          → ( "!" | "-" ) unary | primary ;
   primary        → NUMBER | STRING | "true" | "false" | "nil"
                  | "(" expression ")" | INDENTIFIER ;
*/

class Parser {
public:
  Parser(std::vector<Token> tokens) : tokens(tokens) {}
  std::vector<std::unique_ptr<Stmt>> parse();

private:
  std::unique_ptr<Stmt> declaration();
  std::unique_ptr<Stmt> varDeclaration();
  std::unique_ptr<Stmt> statement();
  std::vector<std::unique_ptr<Stmt>> block();
  std::unique_ptr<Stmt> printStatement();
  std::unique_ptr<Stmt> expressionStatement();
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> assignment();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();

  // helpers
  template <typename... Args> bool match(Args... types);
  bool check(TokenType type);
  bool isAtEnd();
  Token advance();
  Token previous();
  Token peek();
  Token consume(TokenType type, std::string message);
  ParserError error(Token token, std::string message);
  void synchronize();
  
  std::size_t current = 0;
  const std::vector<Token> tokens;
};

#endif
