#include <gtest/gtest.h>
#include "../CrossCutLib/Logger.h"

Logger logger; // An "extern Logger logger" may be declared in some of ChkdraftLib, referencing Main.cpp in Chkdraft, instantiate it here as well

TEST(ConstantsTest, Basic)
{
    EXPECT_TRUE(true);
}
