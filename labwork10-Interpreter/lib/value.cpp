#include "environment.h"
#include "value.h"
#include "ast.h"
#include "std_lib.h"


Value::Value() : type_(ValueType::nil), data_(false) {}
Value::Value(double x) : type_(ValueType::number), data_(x) {}
Value::Value(const std::string& s) : type_(ValueType::string), data_(s) {}
Value::Value(bool b) : type_(ValueType::boolean), data_(b) {}
Value::Value(const List& list) : type_(ValueType::list), data_(list) {}
Value::Value(std::shared_ptr<FunctionObject> fn) : type_(ValueType::function), data_(fn) {}


bool Value::is_nil() const {
    return type_ == ValueType::nil;
}


Value Value::make_stdlib_func(const std::string& name) {
    Value func;
    func.type_ = ValueType::stdlib_function;
    func.data_ = name;
    return func;
}


std::string Value::to_string() const {
    switch (type_) {
        case ValueType::number: {
            double d = std::get<double>(data_);
            if (std::floor(d) == d) {
                return std::to_string(static_cast<long long>(d));
            } else {
                return std::to_string(d);
            }
        }
        case ValueType::string:
            return std::get<std::string>(data_);
        case ValueType::boolean:
            return std::get<bool>(data_) ? "true" : "false";
        case ValueType::list: {
            std::string res = "[";
            const auto& list = *std::get<List>(data_);
            for (size_t i = 0; i < list.size(); ++i) {
                res += list[i].to_string();
                if (i + 1 != list.size()) res += ", ";
            }
            return res + "]";
        }
        case ValueType::function:
            return "<function>";
        case ValueType::nil:
            return "nil";
        case ValueType::stdlib_function:
            return "<stdlib>";
    }
    return "nil";
}


bool Value::to_bool() const {
    switch (type_) {
        case ValueType::nil:
            return false;
        case ValueType::boolean:
            return std::get<bool>(data_);
        case ValueType::number:
            return std::get<double>(data_) != 0.0;
        case ValueType::string:
            return !std::get<std::string>(data_).empty();
        case ValueType::list:
            return !std::get<List>(data_)->empty();
        case ValueType::function:
        case ValueType::stdlib_function:
            return true;
    }
    return false;
}


Value Value::operator+(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) + std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string)
        return Value(std::get<std::string>(data_) + std::get<std::string>(other.data_));
    if (type_ == ValueType::list && other.type_ == ValueType::list) {
        auto result = std::make_shared<std::vector<Value>>(*std::get<List>(data_));
        const auto& other_list = *std::get<List>(other.data_);
        result->insert(result->end(), other_list.begin(), other_list.end());
        return Value(result);
    }
    throw std::runtime_error("invalid types (operator '+')");
}


Value Value::operator-(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) - std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string) {
        std::string str = std::get<std::string>(data_);
        std::string suffix = std::get<std::string>(other.data_);
        if (str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
            return Value(str.substr(0, str.size() - suffix.size()));
        }
        return Value(str);
    }
    throw std::runtime_error("invalid types (operator '-')");
}


Value Value::operator*(const Value& other) const {
    if (other.type_ == ValueType::number || other.type_ == ValueType::boolean) {
        double factor;
        if (other.type_ == ValueType::number) {
            factor = std::get<double>(other.data_);
        } else if (other.type_ == ValueType::boolean) {
            factor = std::get<bool>(other.data_);
        }
        if (type_ == ValueType::number)
            return Value(std::get<double>(data_) * factor);
        else if (type_ == ValueType::string) {
            std::string str = "";
            for (size_t i = 0; i < factor; ++i) {
                str += std::get<std::string>(data_);
            }
            return Value(str);
        }
    }
    throw std::runtime_error("invalid types (operator '*')");
}


Value Value::operator/(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number) {
        if (std::fabs(std::get<double>(other.data_) - 0.0) < std::numeric_limits<double>::epsilon()) 
            return Value();
        return Value(std::get<double>(data_) / std::get<double>(other.data_));
    }
    throw std::runtime_error("invalid types (operator '/')");
}


Value Value::operator%(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::fmod(std::get<double>(data_), std::get<double>(other.data_)));
    throw std::runtime_error("invalid types (operator '%')");
}


Value Value::pow(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::pow(std::get<double>(data_), std::get<double>(other.data_)));
    throw std::runtime_error("invalid types (operator '^')");
}


bool Value::operator==(const Value& other) const {
    if (type_ != other.type_) return false;
    switch (type_) {
        case ValueType::number:
            return std::get<double>(data_) == std::get<double>(other.data_);
        case ValueType::string:
            return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        case ValueType::boolean:
            return std::get<bool>(data_) == std::get<bool>(other.data_);
        case ValueType::list:
            return *std::get<List>(data_) == *std::get<List>(other.data_);
        case ValueType::function:
            return std::get<std::shared_ptr<FunctionObject>>(data_) == std::get<std::shared_ptr<FunctionObject>>(other.data_);
        case ValueType::stdlib_function:
            return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        case ValueType::nil:
            return true;
    }
    return false;
}


bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}


Value Value::equal(const Value& other) const {
    return Value(*this == other);
}


Value Value::not_equal(const Value& other) const {
    return Value(*this != other);
}


Value Value::operator<(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) < std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string)
        return Value(std::get<std::string>(data_) < std::get<std::string>(other.data_));
    throw std::runtime_error("invalid types (operator '<')");
}


Value Value::operator<=(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) <= std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string)
        return Value(std::get<std::string>(data_) <= std::get<std::string>(other.data_));
    throw std::runtime_error("invalid types (operator '<=')");
}


Value Value::operator>(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) > std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string)
        return Value(std::get<std::string>(data_) > std::get<std::string>(other.data_));
    throw std::runtime_error("invalid types (operator '>')");
}


Value Value::operator>=(const Value& other) const {
    if (type_ == ValueType::number && other.type_ == ValueType::number)
        return Value(std::get<double>(data_) >= std::get<double>(other.data_));
    if (type_ == ValueType::string && other.type_ == ValueType::string)
        return Value(std::get<std::string>(data_) >= std::get<std::string>(other.data_));
    throw std::runtime_error("invalid types (operator '>=')");
}


Value Value::logic_and(const Value& other) const {
    return Value(to_bool() && other.to_bool());
}

Value Value::logic_or(const Value& other) const {
    return Value(to_bool() || other.to_bool());
}


Value Value::logic_not() const {
    return Value(!to_bool());
}


Value Value::index(int idx) const {
    if (type_ == ValueType::string) {
        const auto& s = std::get<std::string>(data_);
        if (idx < 0) idx += static_cast<int>(s.size());
        if (idx < 0 || idx >= static_cast<int>(s.size()))
            throw std::runtime_error("index out of range");
        return Value(std::string(1, s[idx]));
    }
    if (type_ == ValueType::list) {
        const auto& l = *std::get<List>(data_);
        if (idx < 0) idx += static_cast<int>(l.size());
        if (idx < 0 || idx >= static_cast<int>(l.size()))
            throw std::runtime_error("index out of range");
        return l[idx];
    }
    throw std::runtime_error("index can only be applied to str and lists");
}


Value Value::slice(int start, int end) const {
    if (type_ == ValueType::string) {
        const auto& s = std::get<std::string>(data_);
        int len = static_cast<int>(s.size());
        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::max(0, std::min(start, len));
        end   = std::max(0, std::min(end, len));
        if (start > end) start = end;
        return Value(s.substr(start, end - start));
    }
    if (type_ == ValueType::list) {
        const auto& l = *std::get<List>(data_);
        int len = static_cast<int>(l.size());
        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::max(0, std::min(start, len));
        end   = std::max(0, std::min(end, len));
        if (start > end) start = end;
        auto sub = std::make_shared<std::vector<Value>>(l.begin() + start, l.begin() + end);
        return Value(sub);
    }
    throw std::runtime_error("slice can only be applied to str and lists");
}


Value Value::call(const std::vector<Value>& args, ExecutionArgs& ex_args) const {
    if (type_ == ValueType::stdlib_function) {
        const auto& name = std::get<std::string>(data_);
        auto& std_functions = get_stdlib_functions();
        auto it = std_functions.find(name);
        if (it == std_functions.end())
            throw std::runtime_error("undefined name: " + name);
        return it->second(args);
    }
    if (type_ != ValueType::function)
        throw std::runtime_error("call non-function");
    const auto& func = std::get<std::shared_ptr<FunctionObject>>(data_);
    if (args.size() != func->params_.size())
        throw std::runtime_error("incorrect number of arguments");
    ExecutionArgs local(func->env_, ex_args.output_, ex_args.input_);
    for (size_t i = 0; i < args.size(); ++i)
        local.env_->declare(func->params_[i], args[i]);
    Value result = func->body_->execute(local);
    return local.is_returning_ ? local.return_value_ : result;
}