#include <gtest/gtest.h>
#include <lib/api_manager.h>
#include "lib/response_manager.h"

TEST(ApiTest, ErrorRequests) {
    ApiManager api_manager;
    ASSERT_FALSE(api_manager.ApiRequest("239-239-239"));
    ASSERT_FALSE(api_manager.ApiRequest("2026-01-32"));
    ASSERT_FALSE(api_manager.ApiRequest("2026-01-01", "abc", "cde"));
    ASSERT_FALSE(api_manager.ApiRequest("2026-01-01", "c239", "c239"));
}
TEST(ApiTest, NormalRequests) {
    ApiManager api_manager;
    ASSERT_TRUE(api_manager.ApiRequest("2026-01-01"));
    ASSERT_TRUE(api_manager.ApiRequest("2026-01-02", "c25"));
    ASSERT_TRUE(api_manager.ApiRequest("2026-01-03", "c25", "c2"));
}


TEST(ResponseTest, ErrorRequest) {
    ApiManager api_manager;
    ASSERT_FALSE(api_manager.ApiRequest("1234-56-78"));
    ResponseManager response_manager("1234-56-78");
    ASSERT_FALSE(response_manager.IsResponseInCache());
    ASSERT_FALSE(response_manager.SaveResponseToCache(api_manager.GetJsonResponse()));
    ASSERT_FALSE(response_manager.IsResponseInCache());
    ASSERT_FALSE(response_manager.LoadResponseFromCache());
}
TEST(ResponseTest, NormalRequest) {
    ApiManager api_manager;
    ASSERT_TRUE(api_manager.ApiRequest("2025-03-10"));
    ResponseManager response_manager("2025-03-10");
    ASSERT_TRUE(response_manager.SaveResponseToCache(api_manager.GetJsonResponse()));
    ASSERT_TRUE(response_manager.IsResponseInCache());
    ASSERT_TRUE(response_manager.LoadResponseFromCache());
}
