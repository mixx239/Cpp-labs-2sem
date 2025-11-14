#include <processing.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>


TEST(DirTets, DirOpenTest) { 
    std::stringstream file_emulator;
    Dir("C:/Users/MB/c++github/labwork8-maksimbelov1/for_tests", false) 
      | Filter([](const std::filesystem::path& p){ return p.extension() == ".txt"; })
      | OpenFiles()
      | Split(" ")
      | Out(file_emulator);

    ASSERT_EQ(file_emulator.str(), "1 2 3 4 5 ");
}

TEST(DirTets, DirRecurseOpenTest) { 
    std::stringstream file_emulator;
    Dir("C:/Users/MB/c++github/labwork8-maksimbelov1/for_tests", true) 
      | Filter([](const std::filesystem::path& p){ return p.extension() == ".txt"; })
      | OpenFiles()
      | Split(" ")
      | Out(file_emulator);

    ASSERT_EQ(file_emulator.str(), "1 2 3 4 5 ");
}