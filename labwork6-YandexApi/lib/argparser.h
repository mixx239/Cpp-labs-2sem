#pragma once
#include <string>
#include <cctype>
#include <cstring>
#include <iostream>


class Arguments {
public:
    Arguments();
    bool Parser(int argc, char** argv);
    std::string GetDate();
    std::string GetArrivalCityCode();
    std::string GetDepartureCityCode();
    
private:
    std::string Pskov_city_code = "c25";
    std::string SPB_city_code = "c2";
    std::string date;
    std::string arrival_city_code = Pskov_city_code;
    std::string departure_city_code = SPB_city_code;
};

