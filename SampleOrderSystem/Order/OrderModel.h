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
class OrderModel : public IModel {
public:
    OrderModel(OrderRepository& repository, SampleModel& sampleModel);

    // 상태는 항상 RESERVED로 생성된다 (PRD 7.3).
    // 등록되지 않은 sampleId, 혹은 quantity <= 0이면 std::invalid_argument를 던진다.
    std::string createOrder(const std::string& sampleId, const std::string& customerName, int quantity);
    std::vector<Order> getAll() const;
    void rejectOrder(const std::string& orderNo);

private:
    OrderRepository& repository_;
    SampleModel& sampleModel_;

    static std::string todayYyyymmdd();
};
