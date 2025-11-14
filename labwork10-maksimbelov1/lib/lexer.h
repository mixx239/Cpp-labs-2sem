#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <istream>

enum TokenType {
    number_,
    string_,    
    identifier_,
    nil_,
    true_,
    false_,
    
    if_,
    else_,
    while_,
    for_,
    in_,
    function_,
    then_,
    end_if_,
    end_for_,
    end_while_,
    end_function_,
    return_,
    print_,
    println_,
    break_,
    continue_,

    assign_,
    equal_,
    not_equal_,
    plus_,
    minus_,
    mul_,
    div_,
    percent_,
    pow_,
    greater_,
    greater_equal_,
    less_,
    less_equal_,

    plus_assign_,
    minus_assign_,
    mul_assign_,
    div_assign_,
    percent_assign_,
    pow_assign_,

    not_,
    and_, 
    or_,

    l_paren_,
    r_paren_,
    l_bracket_,
    r_bracket_,
    comma_,
    colon_,

    eof_,
    eol_,

    unknown_
};


class Lexer {
public:
    Lexer(std::istream& input);

    TokenType next_token();
    double get_number();
    const std::string& get_lexeme();
    size_t get_line();

private:
    std::istream& input_;
    size_t line_;
    size_t pos_;
    double number_;
    std::string lexeme_;
    char current_char_;
    
    void next_char();
    void skip_whitespace_and_comments();
    bool is_next_equal(char expected);
    TokenType construct_number();
    TokenType construct_str();
    TokenType construct_identifier();
};