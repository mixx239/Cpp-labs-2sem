#include <iostream>
#include <lib/api_manager.h>
#include "lib/argparser.h"
#include "lib/response_manager.h"


int main(int argc, char** argv) {
    Arguments parsed_args;
    if (!parsed_args.Parser(argc, argv)) {
        return 0;
    }
    ApiManager api_manager;
    if (!api_manager.ApiRequest(parsed_args.GetDate(), parsed_args.GetArrivalCityCode(), parsed_args.GetDepartureCityCode())) {
        return 0;
    }
    ResponseManager response_manager(parsed_args.GetDate(), parsed_args.GetArrivalCityCode(), parsed_args.GetDepartureCityCode());
    if (response_manager.IsResponseInCache()) {
        if (!response_manager.LoadResponseFromCache()) {
            if (!api_manager.ApiRequest(parsed_args.GetDate(), parsed_args.GetArrivalCityCode(), parsed_args.GetDepartureCityCode())) {
                return 0;
            }
            if (!response_manager.SaveResponseToCache(api_manager.GetJsonResponse())) {
                return 0;
            }
        }
    } else {
        if (!api_manager.ApiRequest(parsed_args.GetDate(), parsed_args.GetArrivalCityCode(), parsed_args.GetDepartureCityCode())) {
            return 0;
        }
        if (!response_manager.SaveResponseToCache(api_manager.GetJsonResponse())) {
            return 0;
        }
    }
}
