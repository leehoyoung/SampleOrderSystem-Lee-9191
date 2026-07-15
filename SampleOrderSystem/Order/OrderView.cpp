#include "OrderView.h"

#include <iostream>

#include "../Core/StringUtil.h"

void OrderView::showMenu() const {
    std::cout << "\n----------------------------------------\n"
              << "[1] 주문 접수  [2] 접수된 주문 목록  [3] 주문 승인  [4] 주문 거절  [0] 뒤로가기\n"
              << "선택 > ";
}

std::string OrderView::promptMenuChoice() const {
    std::string choice;
    std::getline(std::cin, choice);
    return StringUtil::trim(choice);
}

void OrderView::showMessage(const std::string& message) const {
    std::cout << message << "\n";
}

std::string OrderView::promptSampleId() const {
    std::cout << "시료 ID > ";
    std::string sampleId;
    std::getline(std::cin, sampleId);
    return StringUtil::trim(sampleId);
}

std::string OrderView::promptCustomerName() const {
    std::cout << "고객명 > ";
    std::string customerName;
    std::getline(std::cin, customerName);
    return StringUtil::trim(customerName);
}

int OrderView::promptQuantity() const {
    std::cout << "수량 > ";
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(StringUtil::trim(line));
    } catch (const std::exception&) {
        return 0;
    }
}

std::string OrderView::promptOrderNo() const {
    std::cout << "주문번호 > ";
    std::string orderNo;
    std::getline(std::cin, orderNo);
    return StringUtil::trim(orderNo);
}

void OrderView::showOrderList(const std::vector<Order>& orders) const {
    if (orders.empty()) {
        std::cout << "접수된 주문이 없습니다.\n";
        return;
    }

    std::cout << "\n주문번호\t시료ID\t고객명\t수량\t상태\n";
    for (const auto& order : orders) {
        std::cout << order.orderNo << '\t' << order.sampleId << '\t' << order.customerName
                   << '\t' << order.quantity << "\t" << toString(order.status) << '\n';
    }
}
