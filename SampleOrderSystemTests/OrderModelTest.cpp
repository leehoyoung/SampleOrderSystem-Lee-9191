// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.
// 단, CreateOrderRejects* 테스트들은 PRD 5.1/사용자 확정 수량 규칙을 검증하는
// 신규 RED 테스트다 (docs/tasks/order-intake.md 태스크 2, 3, 3-1).

#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>
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
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository repo(kTestFilePath);
    repo.load();
    OrderModel model(repo, sampleModel);

    std::string orderNo = model.createOrder(sampleId, "고객사", 10);

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
    OrderModel orderModel(orderRepo, sampleModel);

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
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료";
    std::string sampleId1 = sampleModel.addSample(sample);
    std::string sampleId2 = sampleModel.addSample(sample);
    std::string sampleId3 = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId1, "S-001");
    ASSERT_EQ(sampleId2, "S-002");
    ASSERT_EQ(sampleId3, "S-003");

    OrderRepository repo(kTestFilePath);
    repo.load();
    OrderModel model(repo, sampleModel);

    model.createOrder(sampleId1, "고객사A", 10);
    model.createOrder(sampleId2, "고객사B", 20);
    model.createOrder(sampleId3, "고객사C", 30);

    EXPECT_EQ(model.getAll().size(), 3u);
}

// 태스크 4: 등록된 시료 ID로 주문을 생성하면 RESERVED 상태 주문이 만들어진다.
// (태스크 2·3·3-1의 검증 로직이 정상 케이스를 막지 않는지 확인하는 회귀 테스트)
TEST_F(OrderModelTest, CreateOrderSucceedsForRegisteredSample) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    std::string orderNo = orderModel.createOrder(sampleId, "ACME 연구소", 15);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    const Order& created = all[0];
    EXPECT_EQ(created.status, OrderStatus::Reserved);
    EXPECT_EQ(created.orderNo, orderNo);
    EXPECT_EQ(orderNo.rfind("ORD-", 0), 0u);
    EXPECT_EQ(created.sampleId, sampleId);
    EXPECT_EQ(created.customerName, "ACME 연구소");
    EXPECT_EQ(created.quantity, 15);
}

// 태스크 2 [경계]: 등록되지 않은 시료 ID로는 주문이 생성되지 않는다. (PRD 5.1)
TEST_F(OrderModelTest, CreateOrderRejectsUnregisteredSampleId) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    EXPECT_THROW(orderModel.createOrder("S-999", "ACME", 10), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3 [경계]: 등록된 시료가 하나도 없으면 어떤 sampleId로도 주문이 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsWhenNoSampleRegistered) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    ASSERT_EQ(sampleModel.getAll().size(), 0u);

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    EXPECT_THROW(orderModel.createOrder("S-001", "ACME", 10), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3-1 [경계, 확정]: quantity == 0인 주문은 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsZeroQuantity) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    EXPECT_THROW(orderModel.createOrder(sampleId, "ACME", 0), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3-1 [경계, 확정]: quantity < 0인 주문은 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsNegativeQuantity) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    EXPECT_THROW(orderModel.createOrder(sampleId, "ACME", -5), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// order-approval 태스크 5: RESERVED 주문을 거절하면 REJECTED로 전이되고 재고는 그대로다.
TEST_F(OrderModelTest, RejectOrderTransitionsReservedToRejectedWithoutChangingStock) {
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
    OrderModel orderModel(orderRepo, sampleModel);

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    orderModel.rejectOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Rejected);

    std::vector<Sample> samplesAfter = sampleModel.getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

// order-approval 태스크 6 [경계]: 존재하지 않는 주문번호로 거절을 시도하면 실패한다.
TEST_F(OrderModelTest, RejectOrderThrowsWhenOrderNoNotFound) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    EXPECT_THROW(orderModel.rejectOrder("ORD-NOPE"), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// order-approval 태스크 7 [경계]: 이미 CONFIRMED 상태인 주문은 거절할 수 없다(이중 처리 방지).
TEST_F(OrderModelTest, RejectOrderThrowsWhenOrderNotReserved) {
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
    OrderModel orderModel(orderRepo, sampleModel);

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);
    orderRepo.updateStatus(orderNo, OrderStatus::Confirmed);

    EXPECT_THROW(orderModel.rejectOrder(orderNo), std::invalid_argument);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);
}
