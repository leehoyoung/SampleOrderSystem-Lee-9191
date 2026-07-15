#pragma once

#include <string>
#include <vector>

#include "../Core/IModel.h"
#include "../Sample/SampleModel.h"
#include "Order.h"
#include "OrderRepository.h"

// 주문 데이터를 OrderRepository(JSON 파일)에 위임한다.
// PRD 7.4의 승인/거절/재고 분기 로직은 포함하지 않는다 — 해당 기능 착수 시
// spec-writer -> test-writer -> implementer가 TDD로 채운다.
//
// createOrder는 PRD 5.1(등록된 시료만 주문 가능)과 접수 단계 수량 검증(quantity <= 0
// 거부, 사용자 확정 사항)을 만족해야 한다 — 시료 조회를 위해 SampleModel을 참조한다.
// 검증 실패 시 std::invalid_argument를 던진다 (스텁 단계에서는 아직 미구현).
//
// 예외 계약: 이 클래스의 public 메서드는 실패 시 항상 std::invalid_argument만 던진다.
// OrderRepository::updateStatus / SampleRepository::applyStockDelta는 내부적으로
// std::out_of_range를 던질 수 있지만, OrderModel은 그 호출 전에 항상 자체 사전
// 검증(주문 존재/상태, 시료 존재 등)으로 우회하여 std::out_of_range가 호출자에게
// 노출되지 않도록 한다. 향후 새 public 메서드를 추가할 때도 이 계약을 유지해야 한다.
class OrderModel : public IModel {
public:
    OrderModel(OrderRepository& repository, SampleModel& sampleModel);

    // 상태는 항상 RESERVED로 생성된다 (PRD 7.3).
    // 등록되지 않은 sampleId, 혹은 quantity <= 0이면 std::invalid_argument를 던진다.
    std::string createOrder(const std::string& sampleId, const std::string& customerName, int quantity);
    std::vector<Order> getAll() const;
    void rejectOrder(const std::string& orderNo);

    // PRD 7.4: RESERVED 주문을 승인한다.
    // - 시료가 등록되어 있지 않거나, 동일 sampleId의 다른 주문이 이미 PRODUCING
    //   상태가 아니면서 재고가 quantity 이상이면 즉시 CONFIRMED로 전이하고 재고를
    //   그만큼 차감한다.
    // - 그렇지 않으면(재고 부족, 혹은 동일 sampleId에 이미 PRODUCING 주문 존재)
    //   PRODUCING으로 전이하고 재고는 변경하지 않는다.
    // - 대상 주문이 없거나 RESERVED 상태가 아니거나, sampleId가 더 이상 등록된
    //   시료 목록에 없으면 std::invalid_argument를 던진다 (스텁 단계에서는 아직 미구현).
    void approveOrder(const std::string& orderNo);

private:
    OrderRepository& repository_;
    SampleModel& sampleModel_;

    static std::string todayYyyymmdd();

    // rejectOrder/approveOrder가 공유하는 "RESERVED 주문 조회" 검증을 한 곳에 모은다.
    // 존재하지 않거나 RESERVED가 아니면 std::invalid_argument를 던진다.
    const Order& findReservedOrderOrThrow(const std::vector<Order>& orders, const std::string& orderNo,
        const std::string& wrongStatusMessage) const;
};
