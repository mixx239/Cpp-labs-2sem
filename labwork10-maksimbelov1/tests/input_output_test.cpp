#include <lib/interpreter.h>
#include <gtest/gtest.h>


TEST(InputOutputTests, ReadPrintTest) {
    std::string code = R"(
        s = read()
        print(s)
    )";

    std::string expected = "ITMO";

    std::istringstream input(code);
    std::ostringstream output;

    std::istringstream cin_input("ITMO\n");
    auto* old_buf = std::cin.rdbuf(cin_input.rdbuf());

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(InputOutputTests, ReadPrintlnTest) {
    std::string code = R"(
        println(read())
    )";

    std::string expected = "ITMO\n";

    std::istringstream input(code);
    std::ostringstream output;

    std::istringstream cin_input("ITMO\n");
    auto* old_buf = std::cin.rdbuf(cin_input.rdbuf());

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}