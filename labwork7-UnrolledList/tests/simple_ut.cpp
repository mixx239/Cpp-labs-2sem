#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <list>

/*
    В данном файле представлен ряд тестов, где используются (вместе, раздельно и по-очереди):
        - push_back
        - push_front
        - insert
    Методы применяются на unrolled_list и на std::list.
    Ожидается, что в итоге порядок элементов в контейнерах будут идентичен
*/

TEST(UnrolledLinkedList, pushBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushFront) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_front(i);
        unrolled_list.push_front(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, pushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, insertAndPushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it, i);
            unrolled_list.insert(unrolled_it, i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(UnrolledLinkedList, popFrontBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    for (int i = 0; i < 500; ++i) {
        if (i % 2 == 0) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            std_list.pop_front();
            unrolled_list.pop_front();
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    for (int i = 0; i < 500; ++i) {
        std_list.pop_back();
        unrolled_list.pop_back();
    }

    ASSERT_TRUE(unrolled_list.empty());
}


TEST(UnrolledLinkedList, insertN) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 100; ++i) {
        std_list.insert(std_list.begin(), 100, i);
        unrolled_list.insert(unrolled_list.begin(), 100, i);
    }
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    auto std_it = std_list.begin();
    auto unrolled_it = unrolled_list.begin();

    for (int i = 0; i < 100; ++i) {
        std_it = std_list.insert(std_it, 100, i);
        unrolled_it = unrolled_list.insert(unrolled_it, 100, i);
        ++unrolled_it;
        ++std_it;
    }
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}



TEST(UnrolledLinkedList, reverseIterator) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;


    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    ASSERT_EQ(std_list.front(), unrolled_list.front());
    ASSERT_EQ(std_list.back(), unrolled_list.back());

    auto std_it = std_list.rbegin();
    auto unrolled_it = unrolled_list.rbegin();
    for (int i = 0; i < 1000; ++i) {
        ASSERT_EQ(*std_it, *unrolled_it);
        ++std_it;
        ++unrolled_it;
    }
}


TEST(UnrolledLinkedList, erase) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    auto std_it = std_list.begin();
    auto unrolled_it = unrolled_list.begin();
    
    for (int i = 0; i < 100; ++i) {
        std_it = std_list.erase(std_it);
        unrolled_it = unrolled_list.erase(unrolled_it);
    }
    
    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    auto std_it2 = std_list.begin();
    auto unrolled_it2 = unrolled_list.begin();

    for (int i = 0; i < 200; ++i) {
        ++std_it2;
        ++unrolled_it2;
    }
    
    std_it = std_list.erase(std_it, std_it2);
    unrolled_it = unrolled_list.erase(unrolled_it, unrolled_it2);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    std_it = std_list.erase(std_it, std_list.end());
    unrolled_it = unrolled_list.erase(unrolled_it, unrolled_list.end());

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    std_it = std_list.erase(std_list.begin(), std_list.end());
    unrolled_it = unrolled_list.erase(unrolled_list.begin(), unrolled_list.end());

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    ASSERT_TRUE(unrolled_list.empty());
    ASSERT_TRUE(std_list.empty());
}



