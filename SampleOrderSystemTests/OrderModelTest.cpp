// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include "../SampleOrderSystem/Order/OrderModel.h"
#include "../SampleOrderSystem/Order/OrderRepository.h"
#include "../SampleOrderSystem/Sample/SampleModel.h"
#include "../SampleOrderSystem/Sample/SampleRepository.h"

namespace {
const char* kTestFilePath = "test_data/order_model_test.json";
const char* kSampleTestFilePath = "test_data/order_model_test_samples.json";
}

class OrderModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kTestFilePath);
        std::filesystem::remove(kSampleTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kTestFilePath);
        std::filesystem::remove(kSampleTestFilePath);
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

TEST_F(OrderModelTest, CreateOrderDoesNotChangeSampleStock) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    Sample sample;
    sample.name = "시료1";
    sample.stock = 30;
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo);

    std::string orderNo = orderModel.createOrder(sampleId, "고객사", 10);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Reserved);
    EXPECT_EQ(all[0].orderNo, orderNo);

    std::vector<Sample> samplesAfter = sampleModel.getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].id, "S-001");
    EXPECT_EQ(samplesAfter[0].stock, 30);
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
