// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>
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

// 태스크 3 [기반]: 기존 주문의 상태를 갱신하는 메서드가 상태를 바꾸고 영속화한다.
TEST_F(OrderRepositoryTest, UpdateStatusChangesStatusOfExistingOrder) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    Order order;
    order.orderNo = "ORD-20260101-0001";
    order.sampleId = "S-001";
    order.customerName = "고객사";
    order.quantity = 5;
    order.status = OrderStatus::Reserved;
    repo.append(order);

    repo.updateStatus("ORD-20260101-0001", OrderStatus::Rejected);

    ASSERT_EQ(repo.listAll().size(), 1u);
    EXPECT_EQ(repo.listAll()[0].status, OrderStatus::Rejected);
    EXPECT_EQ(repo.listAll()[0].sampleId, "S-001");
    EXPECT_EQ(repo.listAll()[0].customerName, "고객사");
    EXPECT_EQ(repo.listAll()[0].quantity, 5);
}

// 태스크 4 [기반, 경계]: 존재하지 않는 주문번호로 상태 갱신을 시도하면 실패하고,
// 목록에 아무 변화가 없다.
TEST_F(OrderRepositoryTest, UpdateStatusThrowsWhenOrderNoNotFound) {
    OrderRepository repo(kTestFilePath);
    repo.load();

    EXPECT_THROW(repo.updateStatus("ORD-NOPE", OrderStatus::Rejected), std::out_of_range);

    EXPECT_TRUE(repo.listAll().empty());
}
