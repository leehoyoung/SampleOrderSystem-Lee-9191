// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include "../SampleOrderSystem/Order/OrderRepository.h"

namespace {
const char* kTestFilePath = "test_data/order_repository_test.json";
}

class OrderRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kTestFilePath);
    }
};

TEST_F(OrderRepositoryTest, LoadWithNoExistingFileStartsWithEmptyList) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    EXPECT_TRUE(repo.listAll().empty());
}

TEST_F(OrderRepositoryTest, AppendedDataPersistsAcrossRepositoryInstances) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    Order order;
    order.orderNo = "ORD-20260416-0001";
    order.sampleId = "S-001";
    order.customerName = "고객사";
    order.quantity = 5;
    order.status = OrderStatus::Reserved;
    repo.append(order);

    OrderRepository reloaded(kTestFilePath);
    reloaded.load();

    ASSERT_EQ(reloaded.listAll().size(), 1u);
    EXPECT_EQ(reloaded.listAll()[0].orderNo, "ORD-20260416-0001");
    EXPECT_EQ(reloaded.listAll()[0].sampleId, "S-001");
    EXPECT_EQ(reloaded.listAll()[0].customerName, "고객사");
    EXPECT_EQ(reloaded.listAll()[0].quantity, 5);
    EXPECT_EQ(reloaded.listAll()[0].status, OrderStatus::Reserved);
}

TEST_F(OrderRepositoryTest, NextOrderNoFormatsAsOrdYyyymmddDash0001WhenEmpty) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    EXPECT_EQ(repo.nextOrderNo("20260416"), "ORD-20260416-0001");
}

TEST_F(OrderRepositoryTest, NextOrderNoContinuesGloballyRegardlessOfDate) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    Order existing;
    existing.orderNo = "ORD-20260101-0007";
    repo.append(existing);

    EXPECT_EQ(repo.nextOrderNo("20260416"), "ORD-20260416-0008");
}
