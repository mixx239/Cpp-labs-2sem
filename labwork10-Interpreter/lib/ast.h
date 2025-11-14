#pragma once
#include <memory>
#include <vector>
#include <string>
#include <limits>
#include "value.h"
#include "lexer.h"
#include "environment.h"


class Environment;

struct ExecutionArgs {
    std::shared_ptr<Environment> env_;
    std::ostream& output_;
    std::istream& input_;
    bool is_returning_;
    Value return_value_;
    bool is_breaking_;
    bool is_continuing_;

    ExecutionArgs(std::shared_ptr<Environment> env, std::ostream& out, std::istream& in)
        : env_(std::move(env)), output_(out), input_(in), is_returning_(false), is_breaking_(false), is_continuing_(false) {}
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual Value execute(ExecutionArgs& ex_args) = 0; 
};

using ASTPtr = std::unique_ptr<ASTNode>;

class NumberNode : public ASTNode {
    double value_;
public:
    NumberNode(double x);
    Value execute(ExecutionArgs& ex_args) override;
};

class NilNode : public ASTNode {
public:
    NilNode();
    Value execute(ExecutionArgs& ex_args) override;
};

class StringNode : public ASTNode {
    std::string value_;
public:   
    StringNode(std::string value);
    Value execute(ExecutionArgs& ex_args) override;
};

class AssignmentNode : public ASTNode {
    std::string name_;
    ASTPtr expr_;
public:
    AssignmentNode(std::string name, ASTPtr expr);
    Value execute(ExecutionArgs& ex_args) override;
};

class BinaryOpNode : public ASTNode {
    TokenType op_;
    ASTPtr left_;
    ASTPtr right_;
public:
    BinaryOpNode(TokenType op, ASTPtr l, ASTPtr r);
    Value execute(ExecutionArgs& ex_args) override;
};

class UnaryOpNode : public ASTNode {
    TokenType op_;
    ASTPtr obj_;
public:
    UnaryOpNode (TokenType op, ASTPtr obj);
    Value execute(ExecutionArgs& ex_args) override;
};

class VariableNode : public ASTNode {
    std::string name_;
public:
    VariableNode(std::string name);
    Value execute(ExecutionArgs& ex_args) override;
};

class IfNode : public ASTNode {
    ASTPtr condition_;
    ASTPtr then_block_;
    ASTPtr else_block_;
public:
    IfNode(ASTPtr cond, ASTPtr then, ASTPtr els);
    Value execute(ExecutionArgs& ex_args) override;
};

class FunctionNode : public ASTNode {
    std::vector<std::string> params_;
    ASTPtr body_;
public:
    FunctionNode(std::vector<std::string> params, ASTPtr body);
    Value execute(ExecutionArgs& ex_args) override;
};

class ReturnNode : public ASTNode {
    ASTPtr expr_;
public:
    ReturnNode(ASTPtr expr);
    Value execute(ExecutionArgs& ex_args) override;
};

class BlockNode : public ASTNode {
    std::vector<ASTPtr> commands_;
public:
    BlockNode(std::vector<ASTPtr> commands);
    Value execute(ExecutionArgs& ex_args) override;
};

class PrintNode : public ASTNode {
    ASTPtr expr_;
    bool is_ln_;
public:
    PrintNode(ASTPtr expr, bool is_ln);
    Value execute(ExecutionArgs& ex_args) override;
};

class CallNode : public ASTNode {
    ASTPtr function_;        
    std::vector<ASTPtr> arguments_;       
public:
    CallNode(ASTPtr func, std::vector<ASTPtr> args);
    Value execute(ExecutionArgs& ex_args) override;
};

class WhileNode : public ASTNode {
    ASTPtr condition_;
    ASTPtr body_;
public:
    WhileNode(ASTPtr cond, ASTPtr bod);
    Value execute(ExecutionArgs& ex_args) override;
};

class ContinueNode : public ASTNode {
public:
    Value execute(ExecutionArgs& ex_args) override;
};

class ForNode : public ASTNode {
    std::string var_name_;
    ASTPtr range_;
    ASTPtr body_;
public:
    ForNode(std::string var_name, ASTPtr range, ASTPtr body);
    Value execute(ExecutionArgs& ex_args) override;
};

class BreakNode : public ASTNode {
public:
    Value execute(ExecutionArgs& ex_args) override;
};

class ListNode: public ASTNode {
    std::vector<ASTPtr> elements_;
public:
    ListNode(std::vector<ASTPtr> elements);
    Value execute(ExecutionArgs& ex_args) override;
};

class IndexNode: public ASTNode {
    ASTPtr target_;
    ASTPtr idx_;
    ASTPtr end_idx_;
public:
    IndexNode(ASTPtr tgt, ASTPtr idx, ASTPtr end);
    Value execute(ExecutionArgs& ex_args) override;
};