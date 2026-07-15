#pragma once

#include "../Core/IController.h"
#include "IOrderView.h"
#include "OrderModel.h"

// 주문 관리 기능(주문 접수/목록 조회/뒤로가기)의 메뉴 흐름을 조율한다.
// 실제 분기 로직은 PRD 7.3 착수 시 채워진다 — 지금은 트리비얼 스텁.
class OrderController : public IController {
public:
    OrderController(IOrderView& view, OrderModel& model);
    void execute() override;

private:
    IOrderView& view_;
    OrderModel& model_;

    void receiveOrder();
    void listOrders();
    void approveOrderFlow();
    void rejectOrderFlow();
};
