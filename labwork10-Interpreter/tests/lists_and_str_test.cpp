#include <lib/interpreter.h>
#include <gtest/gtest.h>

TEST(ListTests, IndexTest) {
    std::string code = R"(
        l = [1, 2, 3, 4, 5, 6, 7, 8, 9]

        print(l[1])
        print(l[2])
        print(l[8])
    )";

    std::string expected = "239";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(ListTests, SliceTest) {
    std::string code = R"(
        l = [1, 2, 3, 4, 5, 6, 7, 8, 9]

        print(l[1:3])
        print(l[8:9])
    )";

    std::string expected = "[2, 3][9]";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StrTests, IndexTest) {
    std::string code = R"(
        s = "ITMO239ITMO"

        print(s[0])
        print(s[1])
        print(s[4])
        print(s[5])
        print(s[6])
    )";

    std::string expected = "IT239";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StrTests, SliceTest) {
    std::string code = R"(
        s = "ITMO239ITMO239"

        print(s[0:4])
        println(s[11:14])
        println(s[13:])
        println(s[:1])
    )";

    std::string expected = "ITMO239\n9\nI\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StrTests, MultTest) {
    std::string code = R"(
        s = "ITMO"
        t = s * -10
        s = s * 2
        k = s * 0
        println(s)
        println(k)
        print(t)
    )";

    std::string expected = "ITMOITMO\n\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StrTests, PlusMinusStrTest) {
    std::string code = R"(
        s = "ITMO"
        t = s + "239"
        println(t)
        t = t - "239"
        print(t)
    )";

    std::string expected = "ITMO239\nITMO";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StrTests, CompStrTest) {
    std::string code = R"(
        s = "ITMO"
        t = "NOTITMO"
        r = "ITMO"
        println(s < t)
        println(s <= t)
        println(s > t)
        println(s >= t)
        println(s == r)
        println(s != r)
    )";

    std::string expected = "true\ntrue\nfalse\nfalse\ntrue\nfalse\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(ListTests, 2listsTest) {
    std::string code = R"(
        l = [1, 2, 3]
        l2 = l
        push(l, 9)
        push(l2, 100)
        println(l[3])
        println(l2[3])
        println(l[4])
        println(l2[4])
    )";

    std::string expected = "9\n9\n100\n100\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(ListTests, listSortTest) {
    std::string code = R"(
        l = [1, 2, "itmo", 3, "hi"]
        l = sort(l)
        for i in l 
            println(i)
        end for
    )";

    std::string expected = "1\n2\n3\nhi\nitmo\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}