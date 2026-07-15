#pragma once
#include <string>
#include "../Json/JsonValue.h"

// PRD 6장 주문 상태 흐름
enum class OrderStatus { Reserved, Rejected, Producing, Confirmed, Released };

inline std::string toString(OrderStatus status) {
    switch (status) {
        case OrderStatus::Reserved: return "RESERVED";
        case OrderStatus::Rejected: return "REJECTED";
        case OrderStatus::Producing: return "PRODUCING";
        case OrderStatus::Confirmed: return "CONFIRMED";
        case OrderStatus::Released: return "RELEASED";
    }
    return "RESERVED";
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "REJECTED") return OrderStatus::Rejected;
    if (s == "PRODUCING") return OrderStatus::Producing;
    if (s == "CONFIRMED") return OrderStatus::Confirmed;
    if (s == "RELEASED") return OrderStatus::Released;
    return OrderStatus::Reserved;
}

// PRD 5.2 / 6장 주문(Order) 도메인 모델
struct Order {
    std::string orderNo;      // 예: "ORD-20260416-0001"
    std::string sampleId;     // 주문 대상 시료 ID
    std::string customerName;
    int quantity = 0;
    OrderStatus status = OrderStatus::Reserved;

    JsonValue toJson() const {
        JsonValue obj = JsonValue::makeObject();
        obj.set("orderNo", JsonValue::makeString(orderNo));
        obj.set("sampleId", JsonValue::makeString(sampleId));
        obj.set("customerName", JsonValue::makeString(customerName));
        obj.set("quantity", JsonValue::makeInt(quantity));
        obj.set("status", JsonValue::makeString(toString(status)));
        return obj;
    }

    static Order fromJson(const JsonValue& obj) {
        Order o;
        if (const JsonValue* v = obj.find("orderNo")) o.orderNo = v->asString();
        if (const JsonValue* v = obj.find("sampleId")) o.sampleId = v->asString();
        if (const JsonValue* v = obj.find("customerName")) o.customerName = v->asString();
        if (const JsonValue* v = obj.find("quantity")) o.quantity = static_cast<int>(v->asInt());
        if (const JsonValue* v = obj.find("status")) o.status = orderStatusFromString(v->asString());
        return o;
    }
};
