#pragma once
#include <stdexcept>
#include <cmath>
#include <variant>
#include <string>
#include <vector>
#include <memory>


using List = std::shared_ptr<std::vector<class Value>>;

class Environment;
class ASTNode;
struct ExecutionArgs;
using ASTPtr = std::unique_ptr<ASTNode>;

struct FunctionObject {
    std::vector<std::string> params_;
    ASTPtr body_;
    std::shared_ptr<Environment> env_;

    FunctionObject(std::vector<std::string> params, ASTPtr body, std::shared_ptr<Environment> env)
        : params_(std::move(params)), body_(std::move(body)), env_(std::move(env)) {}
};

enum class ValueType {
    number,
    string,
    boolean,
    list,
    function,
    stdlib_function,
    nil
};

class Value {
public:
    Value();
    explicit Value(double x);
    explicit Value(const std::string& s);
    explicit Value(bool b);
    explicit Value(const List& list);
    explicit Value(std::shared_ptr<FunctionObject> fn);
    static Value make_stdlib_func(const std::string& name);

    ValueType type() const { return type_; }
    auto get_data() const { return data_; }
    bool is_nil() const;
    std::string to_string() const;
    bool to_bool() const;

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value pow(const Value& other) const;
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    Value equal(const Value& other) const;
    Value not_equal(const Value& other) const;
    Value operator<(const Value& other) const;
    Value operator<=(const Value& other) const;
    Value operator>(const Value& other) const;
    Value operator>=(const Value& other) const;

    Value logic_and(const Value& other) const;
    Value logic_or(const Value& other) const;
    Value logic_not() const;

    Value index(int idx) const;
    Value slice(int start, int end) const;

    Value call(const std::vector<Value>& args, ExecutionArgs& ex_args) const;

private:
    ValueType type_;
    std::variant<double, std::string, bool, List, std::shared_ptr<FunctionObject>> data_;
};