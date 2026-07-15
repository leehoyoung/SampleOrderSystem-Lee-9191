#pragma once

#include <string>
#include <vector>

#include "IOrderView.h"

// 주문 관리 화면(콘솔 입출력)을 담당한다. 실제 메뉴 문구/입력 처리는 PRD 7.3 착수
// 시점에 채워진다(showMenu 등은 아직 트리비얼 스텁 — implementer 영역).
class OrderView : public IOrderView {
public:
    void showMenu() const override;
    std::string promptMenuChoice() const override;
    void showMessage(const std::string& message) const override;

    std::string promptSampleId() const override;
    std::string promptCustomerName() const override;
    int promptQuantity() const override;

    void showOrderList(const std::vector<Order>& orders) const override;

    std::string promptOrderNo() const override;
};
