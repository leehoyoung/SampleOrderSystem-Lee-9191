// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include "../SampleOrderSystem/Order/OrderModel.h"
#include "../SampleOrderSystem/Order/OrderRepository.h"

namespace {
const char* kTestFilePath = "test_data/order_model_test.json";
}

class OrderModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kTestFilePath);
    }
};

TEST_F(OrderModelTest, CreateOrderAlwaysStartsAsReservedWithNextOrderNoFormat) {
    OrderRepository repo(kTestFilePath);
    repo.load();
    OrderModel model(repo);

    std::string orderNo = model.createOrder("S-001", "고객사", 10);

    std::vector<Order> all = model.getAll();
    ASSERT_EQ(all.size(), 1u);
    const Order& created = all[0];
    EXPECT_EQ(created.orderNo, orderNo);
    EXPECT_EQ(created.status, OrderStatus::Reserved);
    EXPECT_EQ(orderNo.rfind("ORD-", 0), 0u);
    EXPECT_NE(orderNo.find("-0001"), std::string::npos);
}

TEST_F(OrderModelTest, GetAllReturnsAllCreatedOrders) {
    OrderRepository repo(kTestFilePath);
    repo.load();
    OrderModel model(repo);

    model.createOrder("S-001", "고객사A", 10);
    model.createOrder("S-002", "고객사B", 20);
    model.createOrder("S-003", "고객사C", 30);

    EXPECT_EQ(model.getAll().size(), 3u);
}
