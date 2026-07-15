#include "OrderController.h"

#include <algorithm>
#include <stdexcept>

OrderController::OrderController(IOrderView& view, OrderModel& model)
    : view_(view), model_(model) {}

void OrderController::execute() {
    while (true) {
        view_.showMenu();
        const std::string choice = view_.promptMenuChoice();

        if (choice == "1") {
            receiveOrder();
        } else if (choice == "2") {
            listOrders();
        } else if (choice == "3") {
            approveOrderFlow();
        } else if (choice == "4") {
            rejectOrderFlow();
        } else if (choice == "0") {
            return;
        } else {
            view_.showMessage("잘못된 선택입니다. 다시 입력해 주세요.");
        }
    }
}

void OrderController::receiveOrder() {
    const std::string sampleId = view_.promptSampleId();
    const std::string customerName = view_.promptCustomerName();
    const int quantity = view_.promptQuantity();

    try {
        const std::string orderNo = model_.createOrder(sampleId, customerName, quantity);
        view_.showMessage("주문이 접수되었습니다. 주문번호: " + orderNo);
    } catch (const std::invalid_argument& e) {
        view_.showMessage(std::string("주문 접수에 실패했습니다: ") + e.what());
    }
}

void OrderController::listOrders() {
    const std::vector<Order> all = model_.getAll();
    std::vector<Order> reservedOnly;
    for (const auto& order : all) {
        if (order.status == OrderStatus::Reserved) {
            reservedOnly.push_back(order);
        }
    }
    view_.showOrderList(reservedOnly);
}

void OrderController::approveOrderFlow() {
    const std::string orderNo = view_.promptOrderNo();

    try {
        model_.approveOrder(orderNo);

        const std::vector<Order> all = model_.getAll();
        auto it = std::find_if(all.begin(), all.end(),
            [&orderNo](const Order& order) { return order.orderNo == orderNo; });
        if (it != all.end() && it->status == OrderStatus::Producing) {
            view_.showMessage("재고 부족으로 PRODUCING 상태(생산 라인 등록)가 되었습니다.");
        } else {
            view_.showMessage("주문이 승인되어 CONFIRMED 상태가 되었습니다.");
        }
    } catch (const std::invalid_argument& e) {
        view_.showMessage(std::string("주문 승인에 실패했습니다: ") + e.what());
    }
}

void OrderController::rejectOrderFlow() {
    const std::string orderNo = view_.promptOrderNo();

    try {
        model_.rejectOrder(orderNo);
        view_.showMessage("주문이 거절되어 REJECTED 상태가 되었습니다.");
    } catch (const std::invalid_argument& e) {
        view_.showMessage(std::string("주문 거절에 실패했습니다: ") + e.what());
    }
}
