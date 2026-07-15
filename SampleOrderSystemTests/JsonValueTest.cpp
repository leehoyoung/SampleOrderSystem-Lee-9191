// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include "../SampleOrderSystem/Json/JsonValue.h"

TEST(JsonValueTest, ParseDumpRoundTripPreservesNestedStructure) {
    JsonValue root = JsonValue::makeObject();
    root.set("name", JsonValue::makeString("sample"));
    root.set("count", JsonValue::makeInt(3));
    root.set("ratio", JsonValue::makeDouble(1.5));

    JsonValue boolValue;
    boolValue.type = JsonValue::Type::Bool;
    boolValue.boolValue = true;
    root.set("active", boolValue);

    root.set("nothing", JsonValue());

    JsonValue arr = JsonValue::makeArray();
    arr.push_back(JsonValue::makeInt(1));
    arr.push_back(JsonValue::makeString("two"));
    JsonValue nestedObj = JsonValue::makeObject();
    nestedObj.set("inner", JsonValue::makeString("value"));
    arr.push_back(nestedObj);
    root.set("items", arr);

    std::string dumped = root.dump(2);
    JsonValue parsed = JsonValue::parse(dumped);

    ASSERT_TRUE(parsed.isObject());
    EXPECT_EQ(parsed.find("name")->asString(), "sample");
    EXPECT_EQ(parsed.find("count")->asInt(), 3);
    EXPECT_DOUBLE_EQ(parsed.find("ratio")->asDouble(), 1.5);
    EXPECT_TRUE(parsed.find("active")->boolValue);
    EXPECT_TRUE(parsed.find("nothing")->isNull());

    const JsonValue* items = parsed.find("items");
    ASSERT_NE(items, nullptr);
    ASSERT_TRUE(items->isArray());
    ASSERT_EQ(items->arrayValue.size(), 3u);
    EXPECT_EQ(items->arrayValue[0].asInt(), 1);
    EXPECT_EQ(items->arrayValue[1].asString(), "two");
    EXPECT_EQ(items->arrayValue[2].find("inner")->asString(), "value");
}

TEST(JsonValueTest, FindReturnsNullptrForMissingKey) {
    JsonValue obj = JsonValue::makeObject();
    obj.set("existing", JsonValue::makeString("value"));

    EXPECT_NE(obj.find("existing"), nullptr);
    EXPECT_EQ(obj.find("missing"), nullptr);
}

TEST(JsonValueTest, AsAccessorsReturnDefaultWhenTypeMismatched) {
    JsonValue stringValue = JsonValue::makeString("text");
    EXPECT_EQ(stringValue.asInt(-1), -1);
    EXPECT_EQ(stringValue.asDouble(-1.0), -1.0);

    JsonValue intValue = JsonValue::makeInt(5);
    EXPECT_EQ(intValue.asString("default"), "default");
}

TEST(JsonValueTest, StringEscapeRoundTripsQuotesBackslashesAndNewlines) {
    const std::string original = "line1\nline2\\with\"quote\"";
    JsonValue obj = JsonValue::makeObject();
    obj.set("text", JsonValue::makeString(original));

    JsonValue parsed = JsonValue::parse(obj.dump(2));
    EXPECT_EQ(parsed.find("text")->asString(), original);
}

TEST(JsonValueTest, ParseThrowsRuntimeErrorOnUnclosedObject) {
    EXPECT_THROW(JsonValue::parse("{\"key\": \"value\""), std::runtime_error);
}
