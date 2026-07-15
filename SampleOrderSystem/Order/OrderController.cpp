#include "OrderController.h"

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
    view_.showOrderList(model_.getAll());
}
