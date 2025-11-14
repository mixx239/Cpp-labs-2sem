#pragma once
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>


class ApiManager {
public:
    ApiManager();
    bool ApiRequest(const std::string date, const std::string arrival_city = "c25", const std::string departure_city = "c2");
    nlohmann::json GetJsonResponse();

private:
    std::string api_key;
    nlohmann::json json_response;
    nlohmann::json json_city_codes;
};