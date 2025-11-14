#pragma once
#include "lexer.h"
#include "ast.h"
#include <stdexcept>
#include <memory>

using ASTPtr = std::unique_ptr<ASTNode>;

class Parser {
public:
    Parser(Lexer& lexer);
    ASTPtr parse();

private:
    Lexer& lexer_;
    TokenType token_;
    std::string lexeme_;
    double number_;


    ASTPtr parse_unit();
    void next_token();
    void expect_token(TokenType expected);
    ASTPtr parse_block();
    ASTPtr parse_expression();
    ASTPtr parse_and();
    ASTPtr parse_add_sub();
    ASTPtr parse_mul_div();
    ASTPtr parse_pow();
    ASTPtr parse_unary();
    ASTPtr parse_variable();
    ASTPtr parse_if();
    ASTPtr parse_function();
    ASTPtr parse_call_from(const std::string& name);
    ASTPtr parse_print(bool is_ln);
    ASTPtr parse_comparison(); 
    ASTPtr parse_while();
    ASTPtr parse_for();
    ASTPtr parse_call_access();
};