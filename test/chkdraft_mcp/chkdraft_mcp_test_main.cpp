#include <gtest/gtest.h>
#include <iostream>
#include <cross_cut/logger.h>

Logger logger(std::cerr, LogLevel::Warn); // MappingCore declares an "extern Logger logger" that the executable has to define

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
