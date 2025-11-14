#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <iostream>
#include "value.h"
#include "ast.h"


using StdlibFunc = std::function<Value(const std::vector<Value>&)>;

std::unordered_map<std::string, StdlibFunc>& get_stdlib_functions();