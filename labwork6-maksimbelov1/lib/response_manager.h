#include "api_manager.h"
#include <chrono>

class ResponseManager {
public:
    ResponseManager(const std::string date_, const std::string arrival_city_ = "c25", const std::string departure_city_ = "c2");

    bool SaveResponseToCache(const nlohmann::json& json_response);
    bool LoadResponseFromCache();
    bool PrintRoutes(const nlohmann::json& segment);
    bool IsResponseInCache();

private:
    std::string date_;
    std::string arrival_city_;
    std::string departure_city_;
    uint8_t maximum_cache_age_in_hours = 3;

};


