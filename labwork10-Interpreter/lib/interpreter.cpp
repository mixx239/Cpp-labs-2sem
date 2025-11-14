#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "environment.h"
#include "ast.h"
#include "std_lib.h"

bool interpret_file(const std::string& filename, std::ostream& output) {
    std::ifstream input_file(filename);
    if (!input_file) {
        output << "cannot open input file: " << filename << "\n";
        return false;
    }
    return interpret(input_file, output);
}


bool interpret(std::istream& input, std::ostream& output) {
    try {
        Lexer lexer(input);
        Parser parser(lexer);
        ASTPtr program = parser.parse();
        auto global_env = Environment::create_global();
        ExecutionArgs execution_args(global_env, output, input);

        Value result = program->execute(execution_args);
        
        return true;
    } catch (const std::exception& e) {
        output << "Error: " << e.what() << '\n';
        return false;
    }
}