// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include "../SampleOrderSystem/Order/Order.h"

TEST(OrderTest, OrderStatusToStringFromStringRoundTripsForAllValues) {
    const OrderStatus statuses[] = {
        OrderStatus::Reserved, OrderStatus::Rejected, OrderStatus::Producing,
        OrderStatus::Confirmed, OrderStatus::Released,
    };

    for (OrderStatus status : statuses) {
        EXPECT_EQ(orderStatusFromString(toString(status)), status);
    }
}

TEST(OrderTest, OrderStatusFromStringFallsBackToReservedForUnknownValue) {
    EXPECT_EQ(orderStatusFromString("NOT_A_REAL_STATUS"), OrderStatus::Reserved);
}

TEST(OrderTest, ToJsonFromJsonRoundTripPreservesAllFields) {
    Order order;
    order.orderNo = "ORD-20260416-0001";
    order.sampleId = "S-001";
    order.customerName = "고객사";
    order.quantity = 42;
    order.status = OrderStatus::Confirmed;

    Order restored = Order::fromJson(order.toJson());

    EXPECT_EQ(restored.orderNo, order.orderNo);
    EXPECT_EQ(restored.sampleId, order.sampleId);
    EXPECT_EQ(restored.customerName, order.customerName);
    EXPECT_EQ(restored.quantity, order.quantity);
    EXPECT_EQ(restored.status, order.status);
}
