// docs/tasks/order-intake.md 태스크 5~11 — OrderController의 메뉴 흐름을
// IOrderView 테스트 더블(FakeOrderView)로 검증하는 RED 테스트.

#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "../SampleOrderSystem/Order/OrderController.h"
#include "../SampleOrderSystem/Order/IOrderView.h"
#include "../SampleOrderSystem/Order/OrderModel.h"
#include "../SampleOrderSystem/Order/OrderRepository.h"
#include "../SampleOrderSystem/Sample/Sample.h"
#include "../SampleOrderSystem/Sample/SampleModel.h"
#include "../SampleOrderSystem/Sample/SampleRepository.h"

namespace {

const char* kOrderTestFilePath = "test_data/order_controller_test_orders.json";
const char* kSampleTestFilePath = "test_data/order_controller_test_samples.json";

// 테스트 전용 페이크 View — 프로덕션 코드에는 추가하지 않는다 (RouterTest.cpp의
// SpyController와 동일한 관례).
class FakeOrderView : public IOrderView {
public:
    void showMenu() const override { ++showMenuCallCount; }

    std::string promptMenuChoice() const override {
        if (menuChoiceQueueIndex < menuChoiceQueue.size()) {
            return menuChoiceQueue[menuChoiceQueueIndex++];
        }
        return "0";
    }

    void showMessage(const std::string& message) const override {
        messages.push_back(message);
    }

    std::string promptSampleId() const override { return sampleIdToReturn; }
    std::string promptCustomerName() const override { return customerNameToReturn; }
    int promptQuantity() const override { return quantityToReturn; }

    void showOrderList(const std::vector<Order>& orders) const override {
        ++showOrderListCallCount;
        shownOrders = orders;
    }

    std::string promptOrderNo() const override { return orderNoToReturn; }

    // 호출 기록/큐 — 테스트에서 자유롭게 설정/검증한다.
    mutable int showMenuCallCount = 0;
    std::vector<std::string> menuChoiceQueue;
    mutable size_t menuChoiceQueueIndex = 0;
    mutable std::vector<std::string> messages;

    std::string sampleIdToReturn;
    std::string customerNameToReturn;
    int quantityToReturn = 0;
    std::string orderNoToReturn;

    mutable int showOrderListCallCount = 0;
    mutable std::vector<Order> shownOrders;
};

}  // namespace

class OrderControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kOrderTestFilePath);
        std::filesystem::remove(kSampleTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kOrderTestFilePath);
        std::filesystem::remove(kSampleTestFilePath);
    }
};

// 태스크 5: 주문 관리 메뉴 진입 시 하위 메뉴(주문 접수/목록 조회/뒤로가기)가 표시된다.
TEST_F(OrderControllerTest, EnteringMenuShowsSubMenu) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"0"};

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_GE(view.showMenuCallCount, 1);
}

// 태스크 6: 주문 접수 메뉴에서 시료ID/고객명/수량을 입력하면 주문이 생성되고
// 발급된 주문번호가 화면에 표시된다.
TEST_F(OrderControllerTest, ReceivingOrderCreatesOrderAndShowsIssuedOrderNo) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"1", "0"};
    view.sampleIdToReturn = "S-001";
    view.customerNameToReturn = "ACME";
    view.quantityToReturn = 10;

    OrderController controller(view, orderModel);
    controller.execute();

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    const std::string& issuedOrderNo = all[0].orderNo;

    bool orderNoShown = std::any_of(view.messages.begin(), view.messages.end(),
        [&issuedOrderNo](const std::string& message) { return message.find(issuedOrderNo) != std::string::npos; });
    EXPECT_TRUE(orderNoShown);
}

// 태스크 7 [경계]: 주문 접수 메뉴에서 존재하지 않는 시료 ID를 입력하면 주문이
// 생성되지 않고 실패 메시지가 표시된다.
TEST_F(OrderControllerTest, ReceivingOrderWithUnknownSampleIdShowsFailureMessageAndCreatesNoOrder) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"1", "0"};
    view.sampleIdToReturn = "S-999";
    view.customerNameToReturn = "ACME";
    view.quantityToReturn = 10;

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_EQ(orderModel.getAll().size(), 0u);
    EXPECT_FALSE(view.messages.empty());
}

// 태스크 17 [확정]: 접수된 주문 목록 조회 메뉴는 RESERVED 상태 주문만 표시한다
// (기존 태스크 8 "전체 표시" 요구사항은 이번 기능으로 좁혀 대체되었다).
TEST_F(OrderControllerTest, ListingOrdersShowsOnlyReservedOrders) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 100;
    std::string sampleId = sampleModel.addSample(sample);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    orderModel.createOrder(sampleId, "고객사A", 10);  // RESERVED
    orderModel.createOrder(sampleId, "고객사B", 20);  // RESERVED

    const std::string confirmedOrderNo = orderModel.createOrder(sampleId, "고객사C", 5);
    orderModel.approveOrder(confirmedOrderNo);  // 재고 충분 -> CONFIRMED

    const std::string rejectedOrderNo = orderModel.createOrder(sampleId, "고객사D", 1);
    orderModel.rejectOrder(rejectedOrderNo);  // REJECTED

    FakeOrderView view;
    view.menuChoiceQueue = {"2", "0"};

    OrderController controller(view, orderModel);
    controller.execute();

    ASSERT_EQ(view.shownOrders.size(), 2u);
    for (const auto& order : view.shownOrders) {
        EXPECT_EQ(order.status, OrderStatus::Reserved);
    }
}

// 태스크 18 [경계]: RESERVED 주문이 하나도 없을 때(전부 CONFIRMED/REJECTED) 목록
// 조회는 크래시/예외 없이 빈 상태를 표시한다.
TEST_F(OrderControllerTest, ListingOrdersWithNoReservedOrdersShowsEmptyListWithoutCrash) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 100;
    std::string sampleId = sampleModel.addSample(sample);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    const std::string confirmedOrderNo = orderModel.createOrder(sampleId, "고객사A", 5);
    orderModel.approveOrder(confirmedOrderNo);  // CONFIRMED

    const std::string rejectedOrderNo = orderModel.createOrder(sampleId, "고객사B", 1);
    orderModel.rejectOrder(rejectedOrderNo);  // REJECTED

    FakeOrderView view;
    view.menuChoiceQueue = {"2", "0"};

    OrderController controller(view, orderModel);

    EXPECT_NO_THROW(controller.execute());
    EXPECT_GE(view.showOrderListCallCount, 1);
    EXPECT_TRUE(view.shownOrders.empty());
}

// 태스크 10: 주문 관리 메뉴에서 "뒤로가기(0)" 선택 시 메인 메뉴로 복귀한다
// (execute()가 반환되어, 큐에 남은 이후 입력은 소비되지 않는다).
TEST_F(OrderControllerTest, SelectingBackReturnsWithoutConsumingFurtherInput) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    // "0" 선택 직후 execute()가 반환되어야 하므로, 뒤에 남은 "1"은 소비되지 않아야 한다.
    view.menuChoiceQueue = {"0", "1"};

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_EQ(view.menuChoiceQueueIndex, 1u);
}

// 태스크 11 [경계]: 정의되지 않은 메뉴 번호/문자를 입력하면 오류 메시지를 표시하고
// 같은 메뉴를 재표시한다 (프로그램이 종료되지 않는다).
TEST_F(OrderControllerTest, SelectingUndefinedChoiceShowsErrorAndRedisplaysMenu) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"9", "0"};

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_FALSE(view.messages.empty());
    EXPECT_GE(view.showMenuCallCount, 2);
    EXPECT_EQ(view.menuChoiceQueueIndex, 2u);
}

// 태스크 19: "주문 승인" 메뉴에서 주문번호를 입력하면 재고가 충분한 경우 즉시
// CONFIRMED로 전이되고, 화면에 CONFIRMED임을 알 수 있는 완료 메시지가 표시된다.
TEST_F(OrderControllerTest, ApprovingOrderWithSufficientStockShowsConfirmedMessage) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 30;
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);
    const std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    FakeOrderView view;
    view.menuChoiceQueue = {"3", "0"};
    view.orderNoToReturn = orderNo;

    OrderController controller(view, orderModel);
    controller.execute();

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);

    bool confirmedMessageShown = std::any_of(view.messages.begin(), view.messages.end(),
        [](const std::string& message) { return message.find("CONFIRMED") != std::string::npos; });
    EXPECT_TRUE(confirmedMessageShown);
}

// 태스크 20: 재고 부족으로 PRODUCING으로 전이된 경우, 화면에 PRODUCING임을 알 수
// 있는 메시지가 표시된다 (CONFIRMED 메시지와 구분되어야 한다).
TEST_F(OrderControllerTest, ApprovingOrderWithInsufficientStockShowsProducingMessage) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 5;
    std::string sampleId = sampleModel.addSample(sample);
    ASSERT_EQ(sampleId, "S-001");

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);
    const std::string orderNo = orderModel.createOrder(sampleId, "ACME", 20);

    FakeOrderView view;
    view.menuChoiceQueue = {"3", "0"};
    view.orderNoToReturn = orderNo;

    OrderController controller(view, orderModel);
    controller.execute();

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Producing);

    bool producingMessageShown = std::any_of(view.messages.begin(), view.messages.end(),
        [](const std::string& message) { return message.find("PRODUCING") != std::string::npos; });
    EXPECT_TRUE(producingMessageShown);

    bool confirmedMessageShown = std::any_of(view.messages.begin(), view.messages.end(),
        [](const std::string& message) { return message.find("CONFIRMED") != std::string::npos; });
    EXPECT_FALSE(confirmedMessageShown);
}

// 태스크 21: "주문 거절" 메뉴에서 주문번호를 입력하면 거절 처리 후 완료 메시지가
// 표시된다.
TEST_F(OrderControllerTest, RejectingOrderShowsRejectedMessage) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 30;
    std::string sampleId = sampleModel.addSample(sample);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);
    const std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);

    FakeOrderView view;
    view.menuChoiceQueue = {"4", "0"};
    view.orderNoToReturn = orderNo;

    OrderController controller(view, orderModel);
    controller.execute();

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Rejected);

    bool rejectedMessageShown = std::any_of(view.messages.begin(), view.messages.end(),
        [](const std::string& message) { return message.find("REJECTED") != std::string::npos; });
    EXPECT_TRUE(rejectedMessageShown);
}

// 태스크 22 [경계]: "주문 승인" 메뉴에서 존재하지 않는 주문번호를 입력하면 실패
// 메시지가 표시되고 메뉴로 복귀한다 (프로그램이 종료되지 않는다).
TEST_F(OrderControllerTest, ApprovingUnknownOrderNoShowsFailureMessageAndRedisplaysMenu) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"3", "0"};
    view.orderNoToReturn = "ORD-NOPE";

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_FALSE(view.messages.empty());
    EXPECT_GE(view.showMenuCallCount, 2);
}

// 태스크 22 회귀: "주문 거절" 메뉴에서도 존재하지 않는 주문번호 입력 시 동일하게
// 실패 메시지가 표시된다.
TEST_F(OrderControllerTest, RejectingUnknownOrderNoShowsFailureMessage) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);

    FakeOrderView view;
    view.menuChoiceQueue = {"4", "0"};
    view.orderNoToReturn = "ORD-NOPE";

    OrderController controller(view, orderModel);
    controller.execute();

    EXPECT_FALSE(view.messages.empty());
}

// 태스크 23 [경계]: 이미 CONFIRMED 처리된 주문번호를 "주문 거절" 메뉴에 입력하면
// 실패 메시지가 표시되고 해당 주문 상태는 CONFIRMED로 유지된다.
TEST_F(OrderControllerTest, RejectingAlreadyConfirmedOrderShowsFailureMessageAndKeepsStatus) {
    SampleRepository sampleRepo(kSampleTestFilePath);
    sampleRepo.load();
    SampleModel sampleModel(sampleRepo);
    Sample sample;
    sample.name = "시료1";
    sample.stock = 30;
    std::string sampleId = sampleModel.addSample(sample);

    OrderRepository orderRepo(kOrderTestFilePath);
    orderRepo.load();
    OrderModel orderModel(orderRepo, sampleModel);
    const std::string orderNo = orderModel.createOrder(sampleId, "ACME", 10);
    orderModel.approveOrder(orderNo);  // 재고 충분 -> CONFIRMED

    FakeOrderView view;
    view.menuChoiceQueue = {"4", "0"};
    view.orderNoToReturn = orderNo;

    OrderController controller(view, orderModel);
    controller.execute();

    std::vector<Order> all = orderModel.getAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].status, OrderStatus::Confirmed);
    EXPECT_FALSE(view.messages.empty());
}
