#include "lexer.h"
#include "parser.h"
#include "environment.h"
#include "ast.h"
#include <iostream>
#include <fstream>

bool interpret_file(const std::string& filename, std::ostream& output);
bool interpret(std::istream& input, std::ostream& output);