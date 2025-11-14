#include "lexer.h"
#include <iostream>

Lexer::Lexer(std::istream& input) : input_(input), line_(1), number_(0), current_char_(' ') {
    next_char();
}


void Lexer::next_char() {
    current_char_ = input_.get();
    if (current_char_ == EOF) {
        current_char_ = '\0';
    }
}


double Lexer::get_number() {
    return number_;
}


const std::string& Lexer::get_lexeme() {
    return lexeme_;
}


size_t Lexer::get_line() {
    return line_;
}


bool Lexer::is_next_equal(char expected) {
    if (input_.peek() == expected) {
        next_char();
        return true;
    }
    return false;
}


void Lexer::skip_whitespace_and_comments() {
    while (true) {
        if (current_char_ == ' ' || current_char_ == '\t' || current_char_ == '\r') {
            next_char();
        } else if (current_char_ == '\n') {
            ++line_;
            next_char();
        } else if (current_char_ == '/') {
            if (input_.peek() == '/') {
                next_char();
                while (current_char_ != '\n' && current_char_ != '\0') {
                    next_char();
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}


TokenType Lexer::construct_number() {
    std::string str_number = "";
    while (std::isdigit(current_char_)) {
        str_number += current_char_;
        next_char();
    }

    if (current_char_ == '.') {
        str_number += '.';
        next_char();
        while (std::isdigit(current_char_)) {
            str_number += current_char_;
            next_char();
        }
    }

    if (current_char_ == 'e' || current_char_ == 'E') {
        str_number += current_char_;
        next_char();
        if (current_char_ == '+' || current_char_ == '-') {
            str_number += current_char_;
            next_char();
        }
        while (std::isdigit(current_char_)) {
            str_number += current_char_;
            next_char();
        }
    }
    lexeme_ = str_number;
    number_ = std::strtod(str_number.c_str(), nullptr);
    return TokenType::number_;
}


TokenType Lexer::construct_identifier() {
    std::string ident;

    if (std::isalpha(current_char_) || current_char_ == '_') {
        ident += current_char_;
        next_char();
        while (std::isalnum(current_char_) || current_char_ == '_') {
            ident += current_char_;
            next_char();
        }
    }

    lexeme_ = ident;

    if (ident == "end") {
        skip_whitespace_and_comments();
        std::string second_word = "";
        while (std::isalpha(current_char_)) {
            second_word += current_char_;
            next_char();
        }
        lexeme_ += " " + second_word;
        if (second_word == "if") return TokenType::end_if_;
        if (second_word == "for") return TokenType::end_for_;
        if (second_word == "while") return TokenType::end_while_;
        if (second_word == "function") return TokenType::end_function_;

        return unknown_;
    }

    if (ident == "if") return TokenType::if_;
    if (ident == "then") return TokenType::then_;
    if (ident == "else") return TokenType::else_;
    if (ident == "while") return TokenType::while_;
    if (ident == "for") return TokenType::for_;
    if (ident == "in") return TokenType::in_;
    if (ident == "function") return TokenType::function_;
    if (ident == "return") return TokenType::return_;
    if (ident == "and") return TokenType::and_;
    if (ident == "or") return TokenType::or_;
    if (ident == "not") return TokenType::not_;
    if (ident == "nil") return TokenType::nil_;
    if (ident == "print") return TokenType::print_;
    if (ident == "println") return TokenType::println_;
    if (ident == "true") return TokenType::true_;
    if (ident == "false") return TokenType::false_;
    if (ident == "break") return TokenType::break_;
    if (ident == "continue") return TokenType::continue_;

    return identifier_;
}


TokenType Lexer::construct_str() {
    std::string str;
    next_char();

    while (current_char_ != '"' && current_char_ != '\0') {
        if (current_char_ == '\n') ++line_;
        str += current_char_;
        next_char();
    }

    lexeme_ = str;
    if (current_char_ == '"') {
        next_char(); 
        return TokenType::string_;
    }
    return TokenType::unknown_;
}


TokenType Lexer::next_token() {
    skip_whitespace_and_comments();

    if(current_char_ == '\0') {
        lexeme_ = "\0";
        return TokenType::eof_;
    }
    if (std::isdigit(current_char_)) return construct_number();
    if (std::isalpha(current_char_) || current_char_ == '_') return construct_identifier();
    if (current_char_ == '"') return construct_str();

    switch (current_char_) {
        case '!': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "!="; return TokenType::not_equal_; } lexeme_ = "!"; return TokenType::not_;
        case '=': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "=="; return TokenType::equal_; } lexeme_ = "="; return TokenType::assign_;
        case '+': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "+="; return TokenType::plus_assign_; } lexeme_ = "+"; return TokenType::plus_;
        case '-': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "-="; return TokenType::minus_assign_; } lexeme_ = "-"; return TokenType::minus_;
        case '*': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "*="; return TokenType::mul_assign_; } lexeme_ = "*"; return TokenType::mul_;
        case '/': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "/="; return TokenType::div_assign_; } lexeme_ = "/"; return TokenType::div_;
        case '%': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "%="; return TokenType::percent_assign_; } lexeme_ = "%"; return TokenType::percent_;
        case '^': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "^="; return TokenType::pow_assign_; } lexeme_ = "^"; return TokenType::pow_;
        case '>': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = ">="; return TokenType::greater_equal_; } lexeme_ = ">"; return TokenType::greater_;
        case '<': next_char(); if (current_char_ == '=') { next_char(); lexeme_ = "<="; return TokenType::less_equal_; } lexeme_ = "<"; return TokenType::less_;
        case '(': next_char(); lexeme_ = "("; return TokenType::l_paren_;
        case ')': next_char(); lexeme_ = ")"; return TokenType::r_paren_;
        case '[': next_char(); lexeme_ = "["; return TokenType::l_bracket_;
        case ']': next_char(); lexeme_ = "]"; return TokenType::r_bracket_;
        case ',': next_char(); lexeme_ = ","; return TokenType::comma_;
        case ':': next_char(); lexeme_ = ":"; return TokenType::colon_;
        default:
            lexeme_ = current_char_;
            next_char();
            return unknown_;
    }
}
