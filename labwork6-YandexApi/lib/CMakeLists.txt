add_library(lib
    argparser.h
    argparser.cpp
    api_manager.h
    api_manager.cpp
    response_manager.h
    response_manager.cpp)


target_link_libraries(lib
    PUBLIC nlohmann_json::nlohmann_json
    PUBLIC cpr::cpr
)

