#pragma once

#include <string>
#include <vector>

#include "../Core/IModel.h"
#include "Order.h"
#include "OrderRepository.h"

// 주문 데이터를 OrderRepository(JSON 파일)에 위임한다.
// PRD 7.4의 승인/거절/재고 분기 로직은 포함하지 않는다 — 해당 기능 착수 시
// spec-writer -> test-writer -> implementer가 TDD로 채운다.
class OrderModel : public IModel {
public:
    explicit OrderModel(OrderRepository& repository);

    // 상태는 항상 RESERVED로 생성된다 (PRD 7.3).
    std::string createOrder(const std::string& sampleId, const std::string& customerName, int quantity);
    std::vector<Order> getAll() const;

private:
    OrderRepository& repository_;

    static std::string todayYyyymmdd();
};
