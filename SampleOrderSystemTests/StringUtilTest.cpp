// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include "../SampleOrderSystem/Core/StringUtil.h"

TEST(StringUtilTest, TrimRemovesLeadingAndTrailingSpaces) {
    EXPECT_EQ(StringUtil::trim("   hello   "), "hello");
}

TEST(StringUtilTest, TrimRemovesCarriageReturnAndNewline) {
    EXPECT_EQ(StringUtil::trim("hello\r\n"), "hello");
    EXPECT_EQ(StringUtil::trim("\r\n  mixed  \r\n"), "mixed");
}

TEST(StringUtilTest, TrimOfWhitespaceOnlyStringReturnsEmptyString) {
    EXPECT_EQ(StringUtil::trim("   \t\r\n  "), "");
}

TEST(StringUtilTest, TrimOfAlreadyTrimmedStringIsUnchanged) {
    EXPECT_EQ(StringUtil::trim("already-trimmed"), "already-trimmed");
}
