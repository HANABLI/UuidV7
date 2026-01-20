#include <UuidV7/UuidV7.hpp>
#include <gtest/gtest.h>

TEST(UuidV7Test, GenerateAndParseRoundtrip) {
    std::string s = UuidV7::UuidV7::Generate().ToString();
    EXPECT_EQ(UuidV7::UuidV7::FromString(s).ToString(), s);
}

TEST(UuidV7Test, FromStringInvalidInputs) {
    // Wrong length
    EXPECT_THROW(UuidV7::UuidV7::FromString("short"), std::invalid_argument);
    // Non-hex characters
    EXPECT_THROW(UuidV7::UuidV7::FromString("zzzzzzzz-zzzz-zzzz-zzzz-zzzzzzzzzzzz"),
                 std::invalid_argument);
    // Wrong hyphen positions
    EXPECT_THROW(UuidV7::UuidV7::FromString("0000000-00000-0000-0000-000000000000"),
                 std::invalid_argument);
}

TEST(UuidV7Test, FromStringUppercaseAcceptedAndNormalized) {
    std::string s_in = "01234567-89AB-47CD-8EF0-1234567890AB";
    std::string expected = "01234567-89ab-47cd-8ef0-1234567890ab";
    EXPECT_EQ(UuidV7::UuidV7::FromString(s_in).ToString(), expected);
}

TEST(UuidV7Test, NilUuidStringFormat) {
    EXPECT_EQ(UuidV7::UuidV7::Nil().ToString(), "00000000-0000-0000-0000-000000000000");
}

TEST(UuidV7Test, ToUint16ValidAndOutOfRange) {
    UuidV7::UuidV7::bytes_type b{};
    b[2] = 0x12;
    b[3] = 0x34;
    UuidV7::UuidV7 u(b);

    EXPECT_EQ(u.ToUint16(2), 0x1234u);

    // offset 14 is valid (reads b[14], b[15]); offset 15 should throw
    EXPECT_NO_THROW(u.ToUint16(14));
    EXPECT_THROW(u.ToUint16(15), std::out_of_range);
}

TEST(UuidV7Test, ComparisonOperators) {
    UuidV7::UuidV7::bytes_type a{};
    UuidV7::UuidV7::bytes_type b{};
    a[15] = 1;
    b[15] = 2;

    UuidV7::UuidV7 ua(a);
    UuidV7::UuidV7 ub(b);

    EXPECT_TRUE(ua != ub);
    EXPECT_FALSE(ua == ub);
    EXPECT_TRUE(ua < ub);
}

TEST(UuidV7Test, ToStringHexFormattingAndGrouping) {
    UuidV7::UuidV7::bytes_type b{};
    b[0] = 0x01;
    b[1] = 0x23;
    b[2] = 0x45;
    b[3] = 0x67;
    b[4] = 0x89;
    b[5] = 0xAB;
    b[6] = 0xCD;
    b[7] = 0xEF;
    b[8] = 0xF0;
    b[9] = 0x0F;
    b[10] = 0x12;
    b[11] = 0x34;
    b[12] = 0x56;
    b[13] = 0x78;
    b[14] = 0x9A;
    b[15] = 0xBC;
    EXPECT_EQ(UuidV7::UuidV7(b).ToString(), "01234567-89ab-cdef-f00f-123456789abc");
}

TEST(UuidV7Test, GeneratedUuidHasVersion7AndRFC4122Variant) {
    auto bytes = UuidV7::UuidV7::Generate().GetBytes();
    // version 7 => high nibble of byte 6 == 0x7
    EXPECT_EQ((bytes[6] & 0xF0), 0x70);
    // RFC 4122 variant => bits 7..6 == 10 (i.e. mask 0xC0 equals 0x80)
    EXPECT_EQ((bytes[8] & 0xC0), 0x80);
}