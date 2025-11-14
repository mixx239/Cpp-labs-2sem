#include <lib/interpreter.h>
#include <gtest/gtest.h>


TEST(FunctionTestSuite, SimpleFunctionTest0) {
    std::string code = "print(35)";

    std::string expected = "35";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, SimpleFunctionTest1) {
    std::string code = R"(
        x = 3
        print(x)
        y = x + 2
        print(y)
        z = 2 + 5
        print(z)
    )";

    std::string expected = "357";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, SimpleFunctionTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        x = incr(2)
        print(x)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FunctionAsArgTest) {
    std::string code = R"(
        incr = function(value)
            return value + 1
        end function

        printresult = function(value, func)
            result = func(value)
            print(result)
        end function

        printresult(2, incr)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, NestedFunctionTest) {
    std::string code = R"(
        // NB: inner and outer `value` are different symbols.
        // You are not required to implement closures (aka lambdas).

        incr_and_print = function(value)
            incr = function(value)
                return value + 1
            end function

            print(incr(value))
        end function

        incr_and_print(2)
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FunnySyntaxTest) {
    std::string code = R"(
        funcs = [
            function() return 1 end function,
            function() return 2 end function,
            function() return 3 end function,
        ]

        print(funcs[0]())
        print(funcs[1]())
        print(funcs[2]())
    )";

    std::string expected = "123";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}



TEST(FunctionTestSuite, CallAccessTest) {
    std::string code = R"(
        list_func = function()
            return [1, 2, 3, 9]
        end function

        print(list_func()[2])
    )";

    std::string expected = "3";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FibTest) {
    std::string code = R"(
        fib = function(n)
            if n == 0 then
                return 0
            end if

            a = 0
            b = 1

            for i in range(n - 1)
                c = a + b
                a = b
                b = c
            end for

            return b
        end function


        print(fib(10))
    )";

    std::string expected = "55";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, MaxTest) {
    std::string code = R"(
        max = function(arr)
            if len(arr) == 0 then
                return nil
            end if

            m = arr[0]

            for i in arr
                if i > m then m = i end if
            end for

            return m
        end function
        
        println(max([]))
        print(max([10, -1, 0, 2, 2025, 239]))

    )";

    std::string expected = "nil\n2025";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}


TEST(FunctionTestSuite, FizzBuzzTest) {
    std::string code = R"(
        fizzBuzz = function(n)
            for i in range(1, n)
                s = "Fizz" * (i % 3 == 0) + "Buzz" * (i % 5 == 0)
                if s == "" then
                    print(i)
                else
                    print(s)
                end if
            end for
        end function

        fizzBuzz(10)

    )";

    std::string expected = "12Fizz4BuzzFizz78Fizz";

    std::istringstream input(code);
    std::ostringstream output;

    ASSERT_TRUE(interpret(input, output));
    ASSERT_EQ(output.str(), expected);
}