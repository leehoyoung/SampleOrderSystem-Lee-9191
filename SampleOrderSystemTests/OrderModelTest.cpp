// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.
// 단, CreateOrderRejects* 테스트들은 PRD 5.1/사용자 확정 수량 규칙을 검증하는
// 신규 RED 테스트다 (docs/tasks/order-intake.md 태스크 2, 3, 3-1).

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
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

    // SampleRepository/SampleModel을 준비한다. 반환된 참조는 픽스처가 살아있는 동안
    // (테스트 함수 종료까지) 유효하다.
    SampleModel& makeSampleModel() {
        sampleRepo_ = std::make_unique<SampleRepository>(kSampleTestFilePath);
        sampleRepo_->load();
        sampleModel_ = std::make_unique<SampleModel>(*sampleRepo_);
        return *sampleModel_;
    }

    // OrderRepository/OrderModel을 준비한다. orderRepo_는 테스트에서 updateStatus/append
    // 등 리포지토리 직접 조작이 필요할 때 그대로 사용한다.
    OrderModel& makeOrderModel(SampleModel& sampleModel) {
        orderRepo_ = std::make_unique<OrderRepository>(kTestFilePath);
        orderRepo_->load();
        orderModel_ = std::make_unique<OrderModel>(*orderRepo_, sampleModel);
        return *orderModel_;
    }

    // 가장 흔한 셋업: 시료 하나를 등록한 SampleModel + OrderModel을 함께 준비한다.
    OrderModel& makeOrderModelWithSample(int stock, std::string* outSampleId = nullptr) {
        SampleModel& sampleModel = makeSampleModel();
        Sample sample;
        sample.name = "시료1";
        sample.stock = stock;
        std::string sampleId = sampleModel.addSample(sample);
        if (outSampleId) {
            *outSampleId = sampleId;
        }
        return makeOrderModel(sampleModel);
    }

    std::unique_ptr<SampleRepository> sampleRepo_;
    std::unique_ptr<SampleModel> sampleModel_;
    std::unique_ptr<OrderRepository> orderRepo_;
    std::unique_ptr<OrderModel> orderModel_;
};

TEST_F(OrderModelTest, CreateOrderAlwaysStartsAsReservedWithNextOrderNoFormat) {
    std::string sampleId;
    OrderModel& model = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

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
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "고객사", 10);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Reserved);
    EXPECT_EQ(all[0].orderNo, orderNo);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].id, "S-001");
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

TEST_F(OrderModelTest, GetAllReturnsAllCreatedOrders) {
    SampleModel& sampleModel = makeSampleModel();
    Sample sample;
    sample.name = "시료";
    std::string sampleId1 = sampleModel.addSample(sample);
    std::string sampleId2 = sampleModel.addSample(sample);
    std::string sampleId3 = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId1, "S-001");
    ASSERT_EQ(sampleId2, "S-002");
    ASSERT_EQ(sampleId3, "S-003");

    OrderModel& model = makeOrderModel(sampleModel);

    model.createOrder(sampleId1, "고객사A", 10);
    model.createOrder(sampleId2, "고객사B", 20);
    model.createOrder(sampleId3, "고객사C", 30);

    EXPECT_EQ(model.getAll().size(), 3u);
}

// 태스크 4: 등록된 시료 ID로 주문을 생성하면 RESERVED 상태 주문이 만들어진다.
// (태스크 2·3·3-1의 검증 로직이 정상 케이스를 막지 않는지 확인하는 회귀 테스트)
TEST_F(OrderModelTest, CreateOrderSucceedsForRegisteredSample) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

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
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    EXPECT_THROW(orderModel.createOrder("S-999", "ACME", 10), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3 [경계]: 등록된 시료가 하나도 없으면 어떤 sampleId로도 주문이 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsWhenNoSampleRegistered) {
    SampleModel& sampleModel = makeSampleModel();
    ASSERT_EQ(sampleModel.getAll().size(), 0u);

    OrderModel& orderModel = makeOrderModel(sampleModel);

    EXPECT_THROW(orderModel.createOrder("S-001", "ACME", 10), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3-1 [경계, 확정]: quantity == 0인 주문은 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsZeroQuantity) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    EXPECT_THROW(orderModel.createOrder(sampleId, "ACME", 0), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// 태스크 3-1 [경계, 확정]: quantity < 0인 주문은 생성되지 않는다.
TEST_F(OrderModelTest, CreateOrderRejectsNegativeQuantity) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    EXPECT_THROW(orderModel.createOrder(sampleId, "ACME", -5), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// order-approval 태스크 5: RESERVED 주문을 거절하면 REJECTED로 전이되고 재고는 그대로다.
TEST_F(OrderModelTest, RejectOrderTransitionsReservedToRejectedWithoutChangingStock) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    orderModel.rejectOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Rejected);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

// order-approval 태스크 6 [경계]: 존재하지 않는 주문번호로 거절을 시도하면 실패한다.
TEST_F(OrderModelTest, RejectOrderThrowsWhenOrderNoNotFound) {
    SampleModel& sampleModel = makeSampleModel();
    OrderModel& orderModel = makeOrderModel(sampleModel);

    EXPECT_THROW(orderModel.rejectOrder("ORD-NOPE"), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// order-approval 태스크 7 [경계]: 이미 CONFIRMED 상태인 주문은 거절할 수 없다(이중 처리 방지).
TEST_F(OrderModelTest, RejectOrderThrowsWhenOrderNotReserved) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);
    orderRepo_->updateStatus(orderNo, OrderStatus::Confirmed);

    EXPECT_THROW(orderModel.rejectOrder(orderNo), std::invalid_argument);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);
}

// order-approval 태스크 8: 재고가 주문 수량보다 충분히 많으면 즉시 CONFIRMED로
// 전이하고 그만큼 재고를 차감한다.
TEST_F(OrderModelTest, ApproveOrderConfirmsAndDecreasesStockWhenStockSufficient) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    orderModel.approveOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 20);
}

// order-approval 태스크 9 [확정]: 재고와 주문 수량이 정확히 일치해도 "충분"으로
// 처리되어 즉시 CONFIRMED + 재고 0으로 차감된다.
TEST_F(OrderModelTest, ApproveOrderConfirmsAndZeroesStockWhenStockExactlyMatchesQuantity) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(10, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    orderModel.approveOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 0);
}

// order-approval 태스크 10: 재고가 주문 수량보다 부족하면 PRODUCING으로 전이하고
// 재고를 변경하지 않는다.
TEST_F(OrderModelTest, ApproveOrderMovesToProducingWithoutChangingStockWhenStockInsufficient) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 50);

    orderModel.approveOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Producing);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

// order-approval 태스크 11 [경계]: 재고 0인 시료에 대한 승인도 PRODUCING으로
// 처리되며 재고는 0 그대로 유지된다.
TEST_F(OrderModelTest, ApproveOrderMovesToProducingWhenStockIsZero) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(0, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 5);

    orderModel.approveOrder(orderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Producing);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 0);
}

// order-approval 태스크 12 [핵심 도메인 규칙]: 동일 시료에 이미 PRODUCING 상태인
// 주문이 존재하면, 재고 필드 값이 신규 주문 수량 이상이어도 신규 승인은 무조건
// PRODUCING으로 분류되고 재고는 변경되지 않는다.
TEST_F(OrderModelTest, ApproveOrderStaysProducingWhenSameSampleHasExistingProducingOrder) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string firstOrderNo = orderModel.createOrder(sampleId, "ACME", 50);
    orderModel.approveOrder(firstOrderNo);
    ASSERT_EQ(orderModel.getAll()[0].status, OrderStatus::Producing);

    std::string secondOrderNo = orderModel.createOrder(sampleId, "ACME2", 10);
    orderModel.approveOrder(secondOrderNo);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 2u);
    auto it = std::find_if(all.begin(), all.end(),
        [&secondOrderNo](const Order& order) { return order.orderNo == secondOrderNo; });
    ASSERT_NE(it, all.end());
    EXPECT_EQ(it->status, OrderStatus::Producing);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

// order-approval 태스크 13 [경계]: 동일 시료에 PRODUCING 주문이 있어도 다른
// 시료의 승인 판정에는 영향을 주지 않는다.
TEST_F(OrderModelTest, ApproveOrderConfirmsUnaffectedByProducingOrderOfDifferentSample) {
    SampleModel& sampleModel = makeSampleModel();
    Sample sample1;
    sample1.name = "시료1";
    sample1.stock = 30;
    std::string sampleId1 = sampleModel.addSample(sample1);
    ASSERT_EQ(sampleId1, "S-001");
    Sample sample2;
    sample2.name = "시료2";
    sample2.stock = 20;
    std::string sampleId2 = sampleModel.addSample(sample2);
    ASSERT_EQ(sampleId2, "S-002");

    OrderModel& orderModel = makeOrderModel(sampleModel);

    std::string producingOrderNo = orderModel.createOrder(sampleId1, "ACME", 50);
    orderModel.approveOrder(producingOrderNo);
    ASSERT_EQ(orderModel.getAll()[0].status, OrderStatus::Producing);

    std::string targetOrderNo = orderModel.createOrder(sampleId2, "ACME2", 5);
    orderModel.approveOrder(targetOrderNo);

    std::vector<Order> all = orderModel.getAll();
    auto it = std::find_if(all.begin(), all.end(),
        [&targetOrderNo](const Order& order) { return order.orderNo == targetOrderNo; });
    ASSERT_NE(it, all.end());
    EXPECT_EQ(it->status, OrderStatus::Confirmed);

    std::vector<Sample> samplesAfter = sampleModel.getAll();
    ASSERT_EQ(samplesAfter.size(), 2u);
    auto sampleIt = std::find_if(samplesAfter.begin(), samplesAfter.end(),
        [&sampleId2](const Sample& sample) { return sample.id == sampleId2; });
    ASSERT_NE(sampleIt, samplesAfter.end());
    EXPECT_EQ(sampleIt->stock, 15);

    auto sample1It = std::find_if(samplesAfter.begin(), samplesAfter.end(),
        [&sampleId1](const Sample& sample) { return sample.id == sampleId1; });
    ASSERT_NE(sample1It, samplesAfter.end());
    EXPECT_EQ(sample1It->stock, 30);
}

// order-approval 태스크 14 [경계]: 존재하지 않는 주문번호로 승인을 시도하면 실패한다.
TEST_F(OrderModelTest, ApproveOrderThrowsWhenOrderNoNotFound) {
    SampleModel& sampleModel = makeSampleModel();
    OrderModel& orderModel = makeOrderModel(sampleModel);

    EXPECT_THROW(orderModel.approveOrder("ORD-NOPE"), std::invalid_argument);
    EXPECT_EQ(orderModel.getAll().size(), 0u);
}

// order-approval 태스크 15 [경계]: RESERVED가 아닌 주문(이미 처리됨)을 승인하려
// 하면 실패한다.
TEST_F(OrderModelTest, ApproveOrderThrowsWhenOrderNotReserved) {
    std::string sampleId;
    OrderModel& orderModel = makeOrderModelWithSample(30, &sampleId);
    ASSERT_EQ(sampleId, "S-001");

    std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);
    orderRepo_->updateStatus(orderNo, OrderStatus::Rejected);

    EXPECT_THROW(orderModel.approveOrder(orderNo), std::invalid_argument);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Rejected);

    std::vector<Sample> samplesAfter = sampleModel_->getAll();
    ASSERT_EQ(samplesAfter.size(), 1u);
    EXPECT_EQ(samplesAfter[0].stock, 30);
}

// order-approval 태스크 16 [경계, 방어적]: 승인 대상 주문의 sampleId가 더 이상
// 등록된 시료 목록에 없으면 실패한다.
TEST_F(OrderModelTest, ApproveOrderThrowsWhenSampleNoLongerRegistered) {
    SampleModel& sampleModel = makeSampleModel();
    ASSERT_EQ(sampleModel.getAll().size(), 0u);

    OrderModel& orderModel = makeOrderModel(sampleModel);

    // createOrder를 거치지 않고 리포지토리에 직접 "주문은 있지만 시료는 없는"
    // 상태를 만든다 (S-999는 sampleModel에 등록된 적이 없음).
    Order orphanOrder;
    orphanOrder.orderNo = "ORD-20260715-0001";
    orphanOrder.sampleId = "S-999";
    orphanOrder.customerName = "ACME";
    orphanOrder.quantity = 10;
    orphanOrder.status = OrderStatus::Reserved;
    orderRepo_->append(orphanOrder);

    EXPECT_THROW(orderModel.approveOrder(orphanOrder.orderNo), std::invalid_argument);

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Reserved);
}
