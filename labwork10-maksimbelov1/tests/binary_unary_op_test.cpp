#include <lib/interpreter.h>
#include <gtest/gtest.h>



TEST(BinOpTests, PlusMinusTest) {
    std::string code = R"(
        x = -2
        y = +(-4)
        z = -(+(+(-6)))
        f = ---8
        println(-2)
        println(x)
        println(y)
        println(z)
        println(f)
    )";

    std::string expected = "-2\n-2\n-4\n6\n-8\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BinOpTests, PowTest) {
    std::string code = R"(
        x = 2^3
        y = 3^2
        z = 2^2^2
        println(x)
        println(y)
        println(z)
    )";

    std::string expected = "8\n9\n16\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BinOpTests, OrAndNotTest) {
    std::string code = R"(
        x = 0
        y = 1
        z = 1.0
        f = x or y
        t = y and z 
        k = not(y)
        println(f)
        println(t)
        println(k)
    )";

    std::string expected = "true\ntrue\nfalse\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BinOpTests, OrAndNotDifTypesTest) {
    std::string code = R"(
        x = [1, 2, 3]
        y = "not empty"
        f = x or y
        t = y and x
        k = not(y) or !x
        println(f)
        println(t)
        println(k)
    )";
 
    std::string expected = "true\ntrue\nfalse\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BinOpTests, BinOpAssignTest) {
    std::string code = R"(
        x = 1
        x += 1
        y = 2
        y *= 2
        z = 3
        z /= 3
        f = 4
        f -= 4
        k = 5
        k %= 4
        t = 6
        t ^= 2
        println(x)
        println(y)
        println(z)
        println(f)
        println(k)
        println(t)
    )";
 
    std::string expected = "2\n4\n1\n0\n1\n36\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(BinOpTests, DivBy0Test) {
    std::string code = R"(
        x = 1
        print(x / 0)
    )";
 
    std::string expected = "nil";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}









