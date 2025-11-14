#include "parser.h"
#include <iostream>

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    next_token();
}


void Parser::next_token() {
    token_ = lexer_.next_token();
    lexeme_ = lexer_.get_lexeme();
    if (token_ == TokenType::number_) {
        number_ = lexer_.get_number();
    }
}


void Parser::expect_token(TokenType expected) {
    if (token_ != expected) {
        throw std::runtime_error("line: " + std::to_string(lexer_.get_line()) + "   unexpected token: " + lexeme_ + " but expected " + std::to_string(expected));
    }
    next_token();
}


ASTPtr Parser::parse() {
    return parse_block();
}


ASTPtr Parser::parse_block() {
    std::vector<ASTPtr> commands;

    while (true) {
        if (token_ == TokenType::eof_ ||
            token_ == TokenType::end_if_ ||
            token_ == TokenType::end_function_ ||
            token_ == TokenType::else_ ||
            token_ == TokenType::end_while_ ||
            token_ == TokenType::end_for_) {
            break;
        }
        switch (token_) {
        case TokenType::if_: commands.push_back(parse_if()); break;
        case TokenType::function_: commands.push_back(parse_function()); break;
        case TokenType::print_: commands.push_back(parse_print(false)); break;
        case TokenType::println_: commands.push_back(parse_print(true)); break;
        case TokenType::while_: commands.push_back(parse_while()); break;
        case TokenType::for_: commands.push_back(parse_for()); break;
        case TokenType::identifier_: {
            std::string name = lexeme_;
            next_token();
            if (token_ == TokenType::assign_ ||
                token_ == TokenType::plus_assign_ ||
                token_ == TokenType::minus_assign_ ||
                token_ == TokenType::mul_assign_ ||
                token_ == TokenType::div_assign_ ||
                token_ == TokenType::percent_assign_||
                token_ == TokenType::pow_assign_) {
                TokenType op = token_;
                next_token();
                ASTPtr rhs = parse_expression();
                if (op == TokenType::assign_) {
                    commands.push_back(std::make_unique<AssignmentNode>(name, std::move(rhs)));
                } else {
                    TokenType binop;
                    switch (op) {
                        case TokenType::plus_assign_: binop = TokenType::plus_; break;
                        case TokenType::minus_assign_: binop = TokenType::minus_; break;
                        case TokenType::mul_assign_: binop = TokenType::mul_; break;
                        case TokenType::div_assign_: binop = TokenType::div_; break;
                        case TokenType::percent_assign_: binop = TokenType::percent_; break;
                        case TokenType::pow_assign_: binop = TokenType::pow_; break;
                    }
                    ASTPtr lhs = std::make_unique<VariableNode>(name);
                    ASTPtr expr = std::make_unique<BinaryOpNode>(binop, std::move(lhs), std::move(rhs));
                    commands.push_back(std::make_unique<AssignmentNode>(name, std::move(expr)));
                }
            } else {
                ASTPtr expr = parse_call_from(name);
                commands.push_back(std::move(expr));
            }
            break;
        }
        case TokenType::return_: {
            next_token();                           
            ASTPtr expr = parse_expression();
            commands.push_back(std::make_unique<ReturnNode>(std::move(expr)));
            break;
        }
        case TokenType::break_: {
            next_token();
            commands.push_back(std::make_unique<BreakNode>());
            break;
        }
        case TokenType::continue_: {
            next_token();
            commands.push_back(std::make_unique<ContinueNode>());
            break;
        }
        default:
            commands.push_back(parse_expression());
            break;
        }
    }
    return std::make_unique<BlockNode>(std::move(commands));
}


ASTPtr Parser::parse_expression() {
    ASTPtr lhs = parse_and();
    while (token_ == TokenType::or_) {
        next_token();
        ASTPtr rhs = parse_and();
        lhs = std::make_unique<BinaryOpNode>(TokenType::or_, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser::parse_and() {
    ASTPtr lhs = parse_comparison();
    while (token_ == TokenType::and_) {
        next_token();
        ASTPtr rhs = parse_comparison();
        lhs = std::make_unique<BinaryOpNode>(TokenType::and_, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser::parse_comparison() {
    ASTPtr lhs = parse_add_sub(); 
    while (token_ == TokenType::equal_ || token_ == TokenType::not_equal_ ||
           token_ == TokenType::less_ || token_ == TokenType::less_equal_ ||
           token_ == TokenType::greater_ || token_ == TokenType::greater_equal_ ) {
        TokenType op = token_;
        next_token();
        ASTPtr rhs = parse_add_sub();
        lhs = std::make_unique<BinaryOpNode>(op, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser:: parse_add_sub() {
    ASTPtr lhs = parse_mul_div();
    while (token_ == TokenType::plus_ || token_ == TokenType::minus_) {
        TokenType op = token_;
        next_token();
        ASTPtr rhs = parse_mul_div();
        lhs = std::make_unique<BinaryOpNode>(op, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser::parse_mul_div() {
    ASTPtr lhs = parse_pow();
    while (token_ == TokenType::mul_ || token_ == TokenType::div_ || token_ == TokenType::percent_) {
        TokenType op = token_;
        next_token();
        ASTPtr rhs = parse_pow();
        lhs = std::make_unique<BinaryOpNode>(op, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser::parse_pow() {
    ASTPtr lhs = parse_unary();
    if (token_ == TokenType::pow_) {
        next_token();
        ASTPtr rhs = parse_pow();
        lhs = std::make_unique<BinaryOpNode>(TokenType::pow_, std::move(lhs), std::move(rhs));
    }
    return lhs;
}


ASTPtr Parser::parse_unary() {
    if (token_ == TokenType::plus_ ||
        token_ == TokenType::minus_ ||
        token_ == TokenType::not_) {
        TokenType op = token_;
        next_token();
        ASTPtr operand = parse_unary();
        return std::make_unique<UnaryOpNode>(op, std::move(operand));
    }
    return parse_call_access();
}


ASTPtr Parser::parse_call_access() {
    ASTPtr expr = parse_unit();
    while (true) {
        if (token_ == TokenType::l_paren_) {
            next_token();  
            std::vector<ASTPtr> args;
            if (token_ != TokenType::r_paren_) {
                args.push_back(parse_expression());
                while (token_ == TokenType::comma_) {
                    next_token();
                    args.push_back(parse_expression());
                }
            }
            expect_token(TokenType::r_paren_);
            expr = std::make_unique<CallNode>(std::move(expr), std::move(args));
        } else if (token_ == TokenType::l_bracket_) {
            next_token();
            ASTPtr idx = nullptr;
            ASTPtr end_idx = nullptr;
            if (token_ != TokenType::colon_) 
                idx = parse_expression();

            if (token_ == TokenType::colon_) {
                next_token();
                if (token_ != TokenType::r_bracket_)
                    end_idx = parse_expression();
            }

            expect_token(TokenType::r_bracket_);
            expr = std::make_unique<IndexNode>(std::move(expr), std::move(idx), std::move(end_idx));
        } else {
            break;
        }
    }
    return expr;
}


ASTPtr Parser::parse_unit() {
    switch (token_) {
        case TokenType::true_: {
            next_token();
            return std::make_unique<NumberNode>(1.0);
        }
        case TokenType::false_: {
            next_token();
            return std::make_unique<NumberNode>(0.0);
        }
        case TokenType::nil_: {
            next_token();
            return std::make_unique<NilNode>();
        }        
        case TokenType::number_: {
            double val = number_;
            next_token();
            return std::make_unique<NumberNode>(val);
        }
        case TokenType::string_: {
            std::string val = lexeme_;
            next_token();
            return std::make_unique<StringNode>(val);
        }
        case TokenType::identifier_: {
            return parse_variable();
        }
        case TokenType::l_paren_: {
            next_token();
            ASTPtr expr = parse_expression();
            expect_token(TokenType::r_paren_);
            return expr;
        }
        case TokenType::function_: {
            return parse_function();
        }
        case TokenType::l_bracket_: {
            next_token();
            std::vector<ASTPtr> elems;
            if (token_ != TokenType::r_bracket_) {
                elems.push_back(parse_expression());
                while (token_ == TokenType::comma_) {
                    next_token();
                    if (token_ != TokenType::r_bracket_)
                        elems.push_back(parse_expression());
                }
            }
            expect_token(TokenType::r_bracket_);
            return std::make_unique<ListNode>(std::move(elems));
        }
        default:
            throw std::runtime_error("line: " + std::to_string(lexer_.get_line()) + "   value was expected, got " + lexeme_);
    }
}


ASTPtr Parser::parse_call_from(const std::string& name) {
    ASTPtr expr = std::make_unique<VariableNode>(name);
    while (token_ == TokenType::l_paren_) {
        next_token();
        std::vector<ASTPtr> args;
        if (token_ != TokenType::r_paren_) {
            do {
                args.push_back(parse_expression());
            } while (token_ == TokenType::comma_ && (next_token(), true));
        }
        expect_token(TokenType::r_paren_);
        expr = std::make_unique<CallNode>(std::move(expr), std::move(args));
    }
    return expr;
}


ASTPtr Parser::parse_print(bool is_ln) {
    next_token();
    expect_token(TokenType::l_paren_);
    ASTPtr expr = parse_expression();
    expect_token(TokenType::r_paren_);
    return(std::make_unique<PrintNode>(std::move(expr), is_ln));
}


ASTPtr Parser::parse_variable() {
    std::string name = lexeme_;
    next_token();
    return std::make_unique<VariableNode>(name);
}


ASTPtr Parser::parse_if() {
    expect_token(TokenType::if_);
    ASTPtr cond = parse_expression(); 
    expect_token(TokenType::then_);
    
    ASTPtr then_block = parse_block();

    ASTPtr else_block = nullptr;
    if (token_ == TokenType::else_) {
        next_token();
        else_block = parse_block();
        next_token();
    } else {
        expect_token(TokenType::end_if_);
    }
    return std::make_unique<IfNode>(std::move(cond), std::move(then_block), std::move(else_block));
}


ASTPtr Parser::parse_function() {
    expect_token(TokenType::function_);
    expect_token(TokenType::l_paren_);
    
    std::vector<std::string> params;
    if (token_ != TokenType::r_paren_) {
        do {
            params.push_back(lexeme_);
            expect_token(TokenType::identifier_);
        } while (token_ == TokenType::comma_ && (next_token(), true));
    }
    
    expect_token(TokenType::r_paren_);
    ASTPtr body = parse_block();
    expect_token(TokenType::end_function_);
    
    return std::make_unique<FunctionNode>(std::move(params), std::move(body));
}


ASTPtr Parser::parse_while() {
    expect_token(TokenType::while_);
    ASTPtr cond = parse_expression(); 
    ASTPtr body = parse_block();
    expect_token(TokenType::end_while_);
    return std::make_unique<WhileNode>(std::move(cond), std::move(body));
}


ASTPtr Parser::parse_for() {
    expect_token(TokenType::for_);
    std::string var_name = lexeme_;
    expect_token(TokenType::identifier_);
    expect_token(TokenType::in_);
    ASTPtr range = parse_expression();
    ASTPtr body = parse_block();
    expect_token(TokenType::end_for_);
    return std::make_unique<ForNode>(var_name, std::move(range), std::move(body));
}