#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

TEST(FilterTest, FilterEven) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int x) { return x % 2 == 0; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(2, 4));
}

TEST(FilterTest, FilterUpperCase) {
    std::vector<std::string> input = {"hello", "world", "HELLO", "WORLD"};
    auto result =
        AsDataFlow(input)
            | Filter([](const std::string& x) { return std::all_of(x.begin(), x.end(), [](char c) { return std::isupper(c); }); })
            | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("HELLO", "WORLD"));
}

TEST(FilterTest, EmptyVector) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) 
                | Filter([](int x) { return x < 0; })
                | AsVector();
    ASSERT_TRUE(result.empty());
}

TEST(FilterTest, ConstVector) {
    const std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int x) { return x % 2 == 0; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAreArray(std::vector<int>{2, 4}));
}

TEST(FilterTest, 2Filter) {
    std::vector<int> input = {1, 2, 3, 4, 5, 6};
    auto result =
        AsDataFlow(input)
            | Filter([](int x) { return x % 2 == 0; })
            | Filter([](int x) { return x % 3 == 0; })
            | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(6));
}



