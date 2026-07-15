#pragma once

#include <string>
#include <vector>

#include "../Core/IView.h"
#include "Order.h"

// 주문 관리 화면의 입출력 계약. OrderController가 실제 콘솔 구현(OrderView) 대신
// 테스트 더블(Fake/Mock)을 주입받아 검증할 수 있도록 인터페이스로 추출했다
// (docs/tasks/order-intake.md "확인 필요 사항" — Controller/View 테스트 전략 확정).
class IOrderView : public IView {
public:
    virtual void showMenu() const = 0;
    virtual std::string promptMenuChoice() const = 0;
    virtual void showMessage(const std::string& message) const = 0;

    virtual std::string promptSampleId() const = 0;
    virtual std::string promptCustomerName() const = 0;
    virtual int promptQuantity() const = 0;

    virtual void showOrderList(const std::vector<Order>& orders) const = 0;
};
