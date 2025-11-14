#include "api_manager.h"

ApiManager::ApiManager() {
    std::fstream key_file("../../yandex_key.json");
    if (!key_file) {
        std::cerr << "connot open yandex_key.json\n";
        return;
    } 
    nlohmann::json yandex_key;
    key_file >> yandex_key;
    api_key = yandex_key["yandex_api_key"];
    key_file.close();
}


bool ApiManager::ApiRequest(const std::string date, const std::string arrival_city, const std::string departure_city) {
    std::string url = "https://api.rasp.yandex.net/v3.0/search/?apikey=" + 
                      api_key +
                      "&lang=ru_RU&format=json&transfers=true&from=" +
                      departure_city +
                      "&to=" + 
                      arrival_city +
                      "&date=" + 
                      date;

    cpr::Response response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "API request failed\n" << response.error.message;
        return false;
    }
    try {
        json_response = nlohmann::json::parse(response.text);
    } catch (nlohmann::json::parse_error error) {
        std::cerr << "Response parsing error: " << error.what() << "\n";
    }
    return true;
}


nlohmann::json ApiManager::GetJsonResponse() {
    return json_response;
}