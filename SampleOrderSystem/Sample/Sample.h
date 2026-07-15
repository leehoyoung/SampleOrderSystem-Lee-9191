#pragma once
#include <string>
#include "../Json/JsonValue.h"

// PRD 5.1 시료(Sample) 도메인 모델
struct Sample {
    std::string id;                    // 예: "S-001"
    std::string name;
    double avgProductionTimeMin = 0.0; // 평균 생산시간 (min/ea)
    double yield = 0.0;                // 수율 (0~1)
    int stock = 0;                     // 현재 재고 수량

    JsonValue toJson() const {
        JsonValue obj = JsonValue::makeObject();
        obj.set("id", JsonValue::makeString(id));
        obj.set("name", JsonValue::makeString(name));
        obj.set("avgProductionTimeMin", JsonValue::makeDouble(avgProductionTimeMin));
        obj.set("yield", JsonValue::makeDouble(yield));
        obj.set("stock", JsonValue::makeInt(stock));
        return obj;
    }

    static Sample fromJson(const JsonValue& obj) {
        Sample s;
        if (const JsonValue* v = obj.find("id")) s.id = v->asString();
        if (const JsonValue* v = obj.find("name")) s.name = v->asString();
        if (const JsonValue* v = obj.find("avgProductionTimeMin")) s.avgProductionTimeMin = v->asDouble();
        if (const JsonValue* v = obj.find("yield")) s.yield = v->asDouble();
        if (const JsonValue* v = obj.find("stock")) s.stock = static_cast<int>(v->asInt());
        return s;
    }
};
