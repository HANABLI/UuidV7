#include <UuidV7/UuidV7.hpp>
#include <gtest/gtest.h>

TEST(UuidV7Test, Uuid_generation_test) {
    UuidV7::UuidV7 id = UuidV7::UuidV7::Generate();
    std::string s = id.ToString();

    UuidV7::UuidV7 id2 = UuidV7::UuidV7::FromStroring(s);

    ASSERT_TRUE(id2 == id);
}