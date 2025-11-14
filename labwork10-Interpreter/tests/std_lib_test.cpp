#include <lib/interpreter.h>
#include <gtest/gtest.h>



TEST(StdlibTests, NumericFunctions) {
    std::string code = R"(
        println(abs(-5))
        println(ceil(1.2))
        println(floor(1.8))
        println(round(1.4))
        println(round(1.6))
        println(sqrt(9))
        println(parse_num("123"))
        println(parse_num("invalid"))
        println(to_string(3.14))
    )";

    std::string expected =
        "5\n" 
        "2\n"
        "1\n"
        "1\n"
        "2\n"
        "3\n"
        "123\n"
        "nil\n"
        "3.140000\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StdlibTests, StringFunctions) {
    std::string code = R"(
        println(len("ITMo"))
        println(lower("ITMo"))
        println(upper("ITMo"))
        l = split("I,T,M,O", ",")
        println(join(l, "-"))
        println(replace("239_ITMO", "_", "->"))
    )";

    std::string expected =
        "4\n"
        "itmo\n"
        "ITMO\n"
        "I-T-M-O\n"
        "239->ITMO\n";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(StdlibTests, ListFunctions) {
    std::string code = R"(
        l = [1, 3]
        push(l, 2)
        println(l[1])
        println(l[2])
        println(pop(l))

        l2 = [1, 3]
        insert(l2, 1, 2)
        println(l2[1])
        println(remove(l2, 1))

        l3 = [3, 2, 9]
        sort(l3)
        println(l3[0])
        println(l3[1])
        println(l3[2])

        l4 = range(5)
        for i in range(5)
            print(l4[i])
        end for
    )";

    std::string expected =
        "3\n"
        "2\n"
        "2\n"
        "2\n"
        "2\n"
        "2\n"
        "3\n"
        "9\n"
        "01234";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}