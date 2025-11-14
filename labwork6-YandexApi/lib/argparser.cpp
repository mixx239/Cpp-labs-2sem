#include "argparser.h"

Arguments::Arguments() {}

bool Arguments::Parser(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (strlen(argv[i]) == strlen("2025-01-01") && isdigit(argv[i][0]) ) {
            date = argv[i];
        } else if (strcmp(argv[i], "1") == 0) {
            arrival_city_code = Pskov_city_code;
            departure_city_code = SPB_city_code;
        } else if (strcmp(argv[i], "0") == 0) {
            arrival_city_code = SPB_city_code;
            departure_city_code = Pskov_city_code;
        } else {
            std::cerr << "Input data error\n" << argv[i];
            return false;
        }
    }
    if (date == "") {
        std::cerr << "Input data error\n";
        return false;
    }
    return true;
}

std::string Arguments::GetDate() {
    return date;
}
std::string Arguments::GetArrivalCityCode() {
    return arrival_city_code;
}
std::string Arguments::GetDepartureCityCode() {
    return departure_city_code;
}

