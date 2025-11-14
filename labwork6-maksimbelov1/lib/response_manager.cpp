#include "response_manager.h"

ResponseManager::ResponseManager(const std::string date, const std::string arrival_city, const std::string departure_city):
    date_(date),
    arrival_city_(arrival_city),
    departure_city_(departure_city) {}


bool ResponseManager::SaveResponseToCache(const nlohmann::json& json_response) {
    if(json_response.empty()) {
        std::cerr << "Server response is empty\n";
        return false;
    } 

    std::filesystem::create_directories("cache");
    std::ofstream cache_file("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json");
    
    if (cache_file) {
        nlohmann::json filtered_response;
        filtered_response["segments"] = nlohmann::json::array();
        if (json_response["segments"].size() == 0) {
            std:: cout << "Маршруты на данную дату не найдены\n";
        }
        for (const nlohmann::json& segment : json_response["segments"]) {
            nlohmann::json reduced_segment;
            if (!segment["has_transfers"]) {
                reduced_segment["has_transfers"] = false;
                reduced_segment["departure"] = segment["departure"];
                reduced_segment["arrival"] = segment["arrival"];
                reduced_segment["from"] = segment["from"]["title"];
                reduced_segment["to"] = segment["to"]["title"];
                reduced_segment["transport_type"] = segment["thread"]["transport_type"];
                reduced_segment["number"] = segment["thread"]["number"];
                reduced_segment["title"] = segment["thread"]["title"];

                filtered_response["segments"].push_back(reduced_segment);
                if (!PrintRoutes(reduced_segment)) {
                    std::cerr << "Routes print error\n";
                    return false;
                }
            } else if (segment.contains("transfers") && segment["transfers"].size() == 1) {
                reduced_segment["has_transfers"] = true;
                reduced_segment["departure"] = segment["departure"];
                reduced_segment["arrival"] = segment["arrival"];
                reduced_segment["from"] = segment["departure_from"]["title"];
                reduced_segment["to"] = segment["arrival_to"]["title"];

                reduced_segment["transport_type_1"] = segment["details"][0]["thread"]["transport_type"];
                reduced_segment["number_1"] = segment["details"][0]["thread"]["number"];
                reduced_segment["title_1"] = segment["details"][0]["thread"]["title"];
                reduced_segment["transfer_arrival"] = segment["details"][0]["arrival"];
                reduced_segment["transfer_to"] = segment["details"][0]["to"]["title"];

                reduced_segment["transfer_time"] = segment["details"][1]["duration"];
                reduced_segment["transport_type_2"] = segment["details"][2]["thread"]["transport_type"];
                reduced_segment["number_2"] = segment["details"][2]["thread"]["number"];
                reduced_segment["title_2"] = segment["details"][2]["thread"]["title"];
                reduced_segment["transfer_departure"] = segment["details"][2]["departure"];

                filtered_response["segments"].push_back(reduced_segment);
                if (!PrintRoutes(reduced_segment)) {
                    std::cerr << "Routes print error\n";
                    return false;
                }
            }
        }
        cache_file << filtered_response.dump(4);
        cache_file.close();
        return true;
    }
    return false;
}

bool ResponseManager::LoadResponseFromCache() {
    std::fstream response_file("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json");
    if (!response_file) {
        return false;
    } 
    nlohmann::json json_response;
    response_file >> json_response;
    if (!json_response.contains("segments")) {
        std::cerr << "Invalid response format: element 'segments' not found\n";
        response_file.close();
        return false; 
    }
    if (json_response["segments"].size() == 0) {
        std:: cout << "Маршруты на данную дату не найдены\n";
        response_file.close();
        return true;
    }
    for (const nlohmann::json& segment : json_response["segments"]) {
        if (!PrintRoutes(segment)) {
            response_file.close();
            return false;
        }
    }
    return true;
}

bool ResponseManager::IsResponseInCache() {
    if (!std::filesystem::exists("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json")) {
        return false;
    }
    if (std::filesystem::is_empty("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json")) {
        std::filesystem::remove("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json");
        return false;
    }
    std::filesystem::file_time_type write_time = std::filesystem::last_write_time("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json");
    std::filesystem::file_time_type time_now = std::filesystem::file_time_type::clock::now();
    std::chrono::hours file_age = std::chrono::duration_cast<std::chrono::hours> (time_now - write_time);
    if (file_age.count() > maximum_cache_age_in_hours) {
        std::filesystem::remove("cache/" + date_ + "_" + departure_city_ + "_" + arrival_city_ + ".json");
        return false;
    }
    return true;
}



bool ResponseManager::PrintRoutes(const nlohmann::json& segment) {
    if (!segment["has_transfers"]) {
        std::cout << "\nМаршрут без пересадок:\n"
                  << segment["from"] << " - " << segment["to"] << "\n"
                  << "Отправление: " << segment["departure"] << "\n"
                  << "Прибытие: " << segment["arrival"] << "\n"
                  << "Тип транспорта: " << segment["transport_type"] << "\n"
                  << "Маршрут: " << segment["title"] << "\n";
        if (segment["number"] != "") {
            std::cout << "Номер: " << segment["number"] << "\n";
        }
        std::cout << "\n-------------------------------------------\n";
    } else {
        std::cout << "\nМаршрут с 1 пересадкой:\n"
        << segment["from"] << " - " << segment["transfer_to"] << "\n"
        << "Отправление: " << segment["departure"] << "\n"
        << "Прибытие к месту пересадки: " << segment["transfer_arrival"] << "\n"
        << "Тип транспорта: " << segment["transport_type_1"] << "\n"
        << "Маршрут: " << segment["title_1"] << "\n";
        if (segment["number_1"] != "") {
            std::cout << "Номер: " << segment["number_1"] << "\n";
        }
        std::cout << "------ Время пересадки: " << segment["transfer_time"] << " сек ------\n";
    
        std::cout << segment["transfer_to"] << " - " << segment["to"] << "\n"
        << "Отправление из места пересадки: " << segment["transfer_departure"] << "\n"
        << "Прибытие: " << segment["arrival"] << "\n"
        << "Тип транспорта: " << segment["transport_type_2"] << "\n"
        << "Маршрут: " << segment["title_2"] << "\n";
        if (segment["number_2"] != "") {
            std::cout << "Номер: " << segment["number_2"] << "\n";
        }
        std::cout << "\n-------------------------------------------\n";
    }
    return true;
}




