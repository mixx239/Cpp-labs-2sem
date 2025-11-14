#include "std_lib.h"


std::unordered_map<std::string, StdlibFunc>& get_stdlib_functions() {
    static bool seeded = ([](){ std::srand(static_cast<unsigned>(std::time(nullptr))); return true; })();

    static std::unordered_map<std::string, StdlibFunc> funcs = {
        {"abs", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            return Value(std::abs(x));
        }},
        {"ceil", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            return Value(std::ceil(x));
        }},
        {"floor", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            return Value(std::floor(x));
        }},
        {"round", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            return Value(std::round(x));
        }},
        {"sqrt", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            return x < 0 ? Value() : Value(std::sqrt(x));
        }},
        {"rnd", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            int n = static_cast<int>(std::get<double>(a[0].get_data()));
            if (n <= 0) return Value();
            return Value(static_cast<double>(std::rand() % n));
        }},
        {"parse_num", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::string) return Value();
            auto str = std::get<std::string>(a[0].get_data());
            char* endp = nullptr;
            double x = std::strtod(str.c_str(), &endp);
            if (endp == str.c_str() || *endp != '\0') return Value();
            return Value(x);
        }},
        {"to_string", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::number) return Value();
            double x = std::get<double>(a[0].get_data());
            long long int_x = static_cast<long long>(x);
            if (std::fabs(x - int_x) < std::numeric_limits<double>::epsilon())
                return Value(std::to_string(int_x));
            else
                return Value(std::to_string(x));
        }},

        {"len", [](auto& a) -> Value {
            if (a.size() != 1) return Value();
            if (a[0].type() == ValueType::string) {
                return Value(static_cast<double>(std::get<std::string>(a[0].get_data()).size()));
            }
            if (a[0].type() == ValueType::list) {
                return Value(static_cast<double>(std::get<List>(a[0].get_data())->size()));
            }
            return Value();
        }},
        {"lower", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::string) return Value();
            auto s = std::get<std::string>(a[0].get_data());
            for (char &c: s)
                c = std::tolower(c);
            return Value(s);
        }},
        {"upper", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::string) return Value();
            auto s = std::get<std::string>(a[0].get_data());
            for (char &c: s) c = std::toupper(c);
            return Value(s);
        }},
        {"split", [](auto& a) -> Value {
            if (a.size() != 2 || a[0].type() != ValueType::string || a[1].type() != ValueType::string)
                return Value();
            auto str = std::get<std::string>(a[0].get_data());
            auto del = std::get<std::string>(a[1].get_data());
            auto out = std::make_shared<std::vector<Value>>();
            size_t pos = 0;
            size_t found = 0;
            while ((found = str.find(del, pos)) != std::string::npos) {
                out->emplace_back(str.substr(pos, found - pos));
                pos = found + del.size();
            }
            out->emplace_back(str.substr(pos));
            return Value(out);
        }},
        {"join", [](auto& a) -> Value {
            if (a.size() != 2 || a[0].type() != ValueType::list || a[1].type() != ValueType::string)
                return Value();
            auto list = std::get<List>(a[0].get_data());
            auto del = std::get<std::string>(a[1].get_data());
            std::string res;
            for (size_t i = 0; i < list->size(); ++i) {
                res += (*list)[i].to_string();
                if (i + 1 < list->size()) res += del;
            }
            return Value(res);
        }},
        {"replace", [](auto& a) -> Value {
            if (a.size() != 3 || a[0].type() != ValueType::string || a[1].type() != ValueType::string || a[2].type() != ValueType::string)
                return Value();
            auto str = std::get<std::string>(a[0].get_data());
            const auto& old_str = std::get<std::string>(a[1].get_data());
            const auto& new_str = std::get<std::string>(a[2].get_data());
            size_t pos = 0;
            while ((pos = str.find(old_str, pos)) != std::string::npos) {
                str.replace(pos, old_str.size(), new_str);
                pos += new_str.size();
            }
            return Value(str);
        }},
        {"push", [](auto& a) -> Value {
            if (a.size() != 2 || a[0].type() != ValueType::list) return Value();
            auto list = std::get<List>(a[0].get_data());
            list->push_back(a[1]);
            return Value();
        }},
        {"pop", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::list) return Value();
            auto list = std::get<List>(a[0].get_data());
            if (list->empty()) return Value();
            Value back = list->back();
            list->pop_back();
            return back;
        }},
        {"insert", [](auto& a) -> Value {
            if (a.size() !=3 || a[0].type() != ValueType::list || a[1].type() != ValueType::number)
                return Value();
            auto list = std::get<List>(a[0].get_data());
            int idx = static_cast<int>(std::get<double>(a[1].get_data()));
            if (idx < 0) idx += list->size();
            if (idx < 0 || idx > static_cast<int>(list->size())) return Value();
            list->insert(list->begin() + idx, a[2]);
            return Value(list);
        }},
        {"remove", [](auto& a) -> Value {
            if (a.size() != 2 || a[0].type() != ValueType::list || a[1].type() != ValueType::number)
                return Value();
            auto list = std::get<List>(a[0].get_data());
            int idx = static_cast<int>(std::get<double>(a[1].get_data()));
            if (idx < 0) idx += list->size();
            if (idx < 0||idx >= static_cast<int>(list->size())) return Value();
            Value val = (*list)[idx];
            list->erase(list->begin() + idx);
            return val;
        }},
        {"sort", [](auto& a) -> Value {
            if (a.size() != 1 || a[0].type() != ValueType::list) return Value();
            auto list = std::get<List>(a[0].get_data());
            std::sort(list->begin(), list->end(), [](auto &l, auto &r){ return l.to_string() < r.to_string(); });
            return Value(list);
        }},
        {"range", [](auto& a) -> Value {
            int argc = a.size();
            if (argc < 1 || argc > 3) 
                throw std::runtime_error("range: wrong number of argmtans");

            double start, end, step;
            for (int i = 0; i < argc; ++i) {
                if (a[i].type() != ValueType::number)
                    throw std::runtime_error("range arguments must be numbers");
            }
            if (argc == 1) {
                start = 0;
                end = std::get<double>(a[0].get_data());
                step = 1;
            } else if (argc == 2) {
                start = std::get<double>(a[0].get_data());
                end = std::get<double>(a[1].get_data());
                step = 1;
            } else {
                start = std::get<double>(a[0].get_data());
                end = std::get<double>(a[1].get_data());
                step = std::get<double>(a[2].get_data());
            }
            if (step == 0) 
                throw std::runtime_error("step in the cycle of the form cannot be equal to 0");

            auto out = std::make_shared<std::vector<Value>>();
            if (step > 0) {
                for (double v = start; v < end; v += step) {
                    out->push_back(Value(v));
                }
            } else {
                for (double v = start; v > end; v += step) {
                    out->push_back(Value(v));
                }
            }
            return Value(out);
        }},

        {"read", [](auto& a) -> Value {
            std::string str;
            if (!std::getline(std::cin, str)) 
                return Value();
            return Value(str);
        }},
    };

    return funcs;
}