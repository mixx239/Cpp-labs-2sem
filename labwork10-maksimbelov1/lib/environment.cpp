#include "environment.h"
#include "value.h"
#include "ast.h"
#include "std_lib.h"        

Environment::Environment() : parent_(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> parent)
    : parent_(std::move(parent)) {}


std::shared_ptr<Environment> Environment::create_global() {
    auto env = std::make_shared<Environment>();
    auto& table = get_stdlib_functions();
    for (auto &func : table) {
        env->declare(func.first, Value::make_stdlib_func(func.first));
    }
    return env;
}

std::shared_ptr<Environment> Environment::create_child(std::shared_ptr<Environment> parent) {
    return std::make_shared<Environment>(std::move(parent));
}


void Environment::declare(const std::string& name, const Value& value) {
    values_[name] = value;
}


void Environment::assign(const std::string& name, const Value& value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
    } else if (parent_) {
        parent_->assign(name, value);
    } else {
        throw std::runtime_error("undefined variable: " + name);
    }
}


Value Environment::get(const std::string& name) const {
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }
    if (parent_) {
        return parent_->get(name);
    }
    throw std::runtime_error("undefined variable: " + name);
}