#include "environment.h"
#include "value.h"
#include "ast.h"  


NumberNode::NumberNode(double x) : value_(x) {}

Value NumberNode::execute(ExecutionArgs& ex_args) {
    return Value(value_);
}


NilNode::NilNode() {}

Value NilNode::execute(ExecutionArgs& ex_args) {
    return Value();
}


StringNode::StringNode(std::string value) : value_(std::move(value)) {}

Value StringNode::execute(ExecutionArgs& ex_args) {
    return Value(value_);
}


AssignmentNode::AssignmentNode(std::string name, ASTPtr expr)
    : name_(std::move(name)), expr_(std::move(expr)) {}

Value AssignmentNode::execute(ExecutionArgs& ex_args) {
    Value value = expr_->execute(ex_args);
    try {
        ex_args.env_->assign(name_, value);
    } catch (std::runtime_error&) {
        ex_args.env_->declare(name_, value);
    }
    return value;
}


BinaryOpNode::BinaryOpNode(TokenType op, ASTPtr left, ASTPtr right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {}

Value BinaryOpNode::execute(ExecutionArgs& ex_args) {
    Value lhs = left_->execute(ex_args);
    Value rhs = right_->execute(ex_args);
    switch (op_) {
        case TokenType::plus_: return lhs + rhs;
        case TokenType::minus_: return lhs - rhs;
        case TokenType::mul_: return lhs * rhs;
        case TokenType::div_: return lhs / rhs;
        case TokenType::percent_: return lhs % rhs;
        case TokenType::pow_: return lhs.pow(rhs);
        case TokenType::equal_: return lhs.equal(rhs);
        case TokenType::not_equal_: return lhs.not_equal(rhs);
        case TokenType::less_: return lhs < rhs;
        case TokenType::less_equal_: return lhs <= rhs;
        case TokenType::greater_: return lhs > rhs;
        case TokenType::greater_equal_: return lhs >= rhs;
        case TokenType::and_: return lhs.logic_and(rhs);
        case TokenType::or_: return lhs.logic_or(rhs);
        default:
            throw std::runtime_error("unsupported binary operator " + op_);
    }
}


UnaryOpNode::UnaryOpNode(TokenType op, ASTPtr obj)
    : op_(op), obj_(std::move(obj)) {}

Value UnaryOpNode::execute(ExecutionArgs& ex_args) {
    Value value = obj_->execute(ex_args);
    switch (op_) {
        case TokenType::plus_: return value;
        case TokenType::minus_: {
            double x = std::get<double>(value.get_data());
            return Value(-x);
        }
        case TokenType::not_: return value.logic_not();
        default:
            throw std::runtime_error("unsupported binary operator " + op_);
    }

}


VariableNode::VariableNode(std::string name)
    : name_(std::move(name)) {}

Value VariableNode::execute(ExecutionArgs& ex_args) {
    return ex_args.env_->get(name_);
}


IfNode::IfNode(ASTPtr cond, ASTPtr then_b, ASTPtr els_b)
    : condition_(std::move(cond)), then_block_(std::move(then_b)), else_block_(std::move(els_b)) {}

Value IfNode::execute(ExecutionArgs& ex_args) {
    Value cond = condition_->execute(ex_args);
    bool is_cond_true = false;
    if (cond.type() == ValueType::boolean) {
        is_cond_true = std::get<bool>(cond.get_data()) ;
    } else if (cond.type() == ValueType::number) {
        is_cond_true = (std::get<double>(cond.get_data()) != 0.0);
    }
    if (is_cond_true) {
        return then_block_->execute(ex_args);
    }
    if (else_block_) {
        return else_block_->execute(ex_args);
    }
    return Value();
}


FunctionNode::FunctionNode(std::vector<std::string> params, ASTPtr body)
    : params_(std::move(params)), body_(std::move(body)) {}

Value FunctionNode::execute(ExecutionArgs& ex_args) {
    auto func_env = Environment::create_child(ex_args.env_);
    return Value(std::make_shared<FunctionObject>(params_, std::move(body_), func_env));
}


ReturnNode::ReturnNode(ASTPtr expr) : expr_(std::move(expr)) {}

Value ReturnNode::execute(ExecutionArgs& ex_args) {
    Value value = expr_->execute(ex_args);
    ex_args.is_returning_ = true;
    ex_args.return_value_ = value;
    return value;
}


BlockNode::BlockNode(std::vector<ASTPtr> commands) : commands_(std::move(commands)) {}

Value BlockNode::execute(ExecutionArgs& ex_args) {
    Value last;
    for (auto& com : commands_) {
        last = com->execute(ex_args);
        if(ex_args.is_returning_) {
            return ex_args.return_value_;
        }
        if (ex_args.is_breaking_ || ex_args.is_continuing_) {
            return last;
        }
    }
    return last;
}


PrintNode::PrintNode(ASTPtr expr, bool is_ln = false) : expr_(std::move(expr)), is_ln_(is_ln) {}

Value PrintNode::execute(ExecutionArgs& ex_args) {
    Value result = expr_->execute(ex_args); 
    if (is_ln_)
        ex_args.output_ << result.to_string() << "\n"; 
    else 
        ex_args.output_ << result.to_string();
    return result;
}


CallNode::CallNode(ASTPtr func, std::vector<ASTPtr> args) : function_(std::move(func)), arguments_(std::move(args)) {}

Value CallNode::execute(ExecutionArgs& ex_args) {
    Value func_val = function_->execute(ex_args);

    std::vector<Value> realArgs;
    for (auto& arg : arguments_) {
        realArgs.push_back(arg->execute(ex_args));
    }

    return func_val.call(realArgs, ex_args);
}


WhileNode::WhileNode(ASTPtr cond, ASTPtr body) : condition_(std::move(cond)), body_(std::move(body)) {}

Value WhileNode::execute(ExecutionArgs& ex_args) {
    while (std::get<bool>(condition_->execute(ex_args).get_data())) {
        ex_args.is_continuing_ = false;
        ex_args.is_breaking_ = false;
        body_->execute(ex_args);
        if (ex_args.is_breaking_) {
            ex_args.is_breaking_ = false;
            break;
        }
        if (ex_args.is_continuing_) {
            ex_args.is_continuing_ = false;
            continue;
        }
    }
    return Value();
}


ForNode::ForNode(std::string var_name, ASTPtr range, ASTPtr body)
    : var_name_(std::move(var_name)), range_(std::move(range)), body_(std::move(body)) {}

Value ForNode::execute(ExecutionArgs& ex_args) {
    auto range = range_->execute(ex_args);
    auto range_list = std::get<List>(range.get_data());
    for (const Value& i : *range_list) {
        try {
            ex_args.env_->assign(var_name_, Value(i));
        } catch (...) {
            ex_args.env_->declare(var_name_, Value(i));
        }
        ex_args.is_continuing_ = false;
        ex_args.is_breaking_ = false;     
        body_->execute(ex_args);
        if (ex_args.is_breaking_) {
            ex_args.is_breaking_ = false;
            break;
        }
        if (ex_args.is_continuing_) {
            ex_args.is_continuing_ = false;
            continue;
        }
    }
    return Value();
}


Value BreakNode::execute(ExecutionArgs& ex_args) {
    ex_args.is_breaking_ = true;
    return Value();
}


Value ContinueNode::execute(ExecutionArgs& ex_args) {
    ex_args.is_continuing_ = true;
    return Value();
}
    

ListNode::ListNode(std::vector<ASTPtr> elems) : elements_(std::move(elems)) {}

Value ListNode::execute(ExecutionArgs& ex_args) {
    auto result = std::make_shared<std::vector<Value>>();
    for (auto& element : elements_) {
        result->push_back(element->execute(ex_args));
    }
    return Value(result);
}


IndexNode::IndexNode(ASTPtr tgt, ASTPtr idx, ASTPtr end = nullptr)
    : target_(std::move(tgt)), idx_(std::move(idx)), end_idx_(std::move(end)) {}

Value IndexNode::execute(ExecutionArgs& ex_args) {
    Value target = target_->execute(ex_args);
    if (!idx_ && !end_idx_) {
        return target.slice(0, std::numeric_limits<int>::max());
    } else if (!idx_) {
        int j = static_cast<int>(std::get<double>(end_idx_->execute(ex_args).get_data()));
        return target.slice(0, j);
    } else if (!end_idx_) {
        int i = static_cast<int>(std::get<double>(idx_->execute(ex_args).get_data()));
        return target.index(i);
    } else {
        int i = static_cast<int>(std::get<double>(idx_->execute(ex_args).get_data()));
        int j = static_cast<int>(std::get<double>(end_idx_->execute(ex_args).get_data()));
        return target.slice(i, j);
    }
}