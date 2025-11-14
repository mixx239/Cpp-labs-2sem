#include <scheduler.h>

#include <gtest/gtest.h>

struct Math {
    int add(int a) const {
        return a + number_;
    }

    int mult(int a) const {
        return a * number_;
    }

    int number_;

    Math(int number) : number_(number) {} 
};

TEST(SchedulerTests, BasicTest) {
    TTaskScheduler scheduler;
    int a = 1;
    int b = 2;
    auto id1 = scheduler.add([](int x, int y) {return x + y;}, a, b);
    auto id2 = scheduler.add([](int x, int y) {return x + y;}, a, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add([](int x, int y) {return x * y;}, scheduler.getFutureResult<int>(id2), scheduler.getFutureResult<int>(id1));
    
    scheduler.executeAll();

    ASSERT_EQ(scheduler.getResult<int>(id1), 3);
    ASSERT_EQ(scheduler.getResult<int>(id2), 4);
    ASSERT_EQ(scheduler.getResult<int>(id3), 12);
}

TEST(SchedulerTests, DifferentTypesTest) {
    TTaskScheduler scheduler;
    int a = 1;
    int b = 2;
    float c = 2.5;
    auto id1 = scheduler.add([](int x, int y) {return x + y;}, a, b);
    auto id2 = scheduler.add([](float x, int y) {return x + y;}, c, scheduler.getFutureResult<int>(id1));
    
    scheduler.executeAll();

    ASSERT_EQ(scheduler.getResult<int>(id1), 3);
    ASSERT_EQ(scheduler.getResult<float>(id2), 5.5);
}

TEST(SchedulerTests, OneArgTest) {
    TTaskScheduler scheduler;
    int a = 1;
    float c = 2.5;
    auto id1 = scheduler.add([](int x) { return x + 10; }, a);
    auto id2 = scheduler.add([](float x) -> float { return x + 10.0; }, c);
    
    scheduler.executeAll();

    ASSERT_EQ(scheduler.getResult<int>(id1), 11);
    ASSERT_EQ(scheduler.getResult<float>(id2), 12.5);
}

TEST(SchedulerTests, NoArgTest) {
    TTaskScheduler scheduler;
    int c = 1;
    auto id1 = scheduler.add([&]() { return ++c; });
    
    scheduler.executeAll();

    ASSERT_EQ(scheduler.getResult<int>(id1), 2);
    ASSERT_EQ(c, 2);
}

TEST(SchedulerTests, RvalueTest) {
    TTaskScheduler scheduler;
    auto id1 = scheduler.add([](std::unique_ptr<int>&& x, int y) {return *x + y;}, std::make_unique<int>(1) , 2);
    
    scheduler.executeAll();

    ASSERT_EQ(scheduler.getResult<int>(id1), 3);
}

TEST(SchedulerTests, InvalidTypeTest) {
    TTaskScheduler scheduler;
    int a = 1;
    int b = 2;
    auto id1 = scheduler.add([](int x, int y) {return x + y;}, a, b);
   
    ASSERT_ANY_THROW(scheduler.getResult<float>(id1));
    ASSERT_EQ(scheduler.getResult<int>(id1), 3);
}

TEST(SchedulerTests, ClassMethodTest) {
    TTaskScheduler scheduler;
    Math math(2);
    auto id1 = scheduler.add(&Math::add, &math, 2);
    auto id2 = scheduler.add(&Math::add, math, scheduler.getFutureResult<int>(id1));
    auto id3 = scheduler.add(&Math::mult, &math, 5);
    auto id4 = scheduler.add(&Math::mult, math, scheduler.getFutureResult<int>(id1));

    ASSERT_EQ(scheduler.getResult<int>(id1), 4);
    ASSERT_EQ(scheduler.getResult<int>(id2), 6);
    ASSERT_EQ(scheduler.getResult<int>(id3), 10);
    ASSERT_EQ(scheduler.getResult<int>(id4), 8);
}

TEST(SchedulerTests, DoubleExecutionTest) {
    TTaskScheduler scheduler;
    int c = 0;
    auto id1 = scheduler.add([&] (int a) { return c += a; }, 2);

    ASSERT_EQ(c, 0);
    ASSERT_EQ(scheduler.getResult<int>(id1), 2);
    scheduler.executeAll();
    ASSERT_EQ(scheduler.getResult<int>(id1), 2);
}