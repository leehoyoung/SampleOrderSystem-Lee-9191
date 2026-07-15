#pragma once

#include "../Core/IModel.h"
#include "../Sample/SampleModel.h"
#include "../Order/OrderModel.h"

// PRD 7.5 모니터링 집계 지점. 실제 집계 로직(상태별 주문 수, 재고 상태 판정)은
// 해당 기능 착수 시 spec-writer -> test-writer -> implementer가 TDD로 채운다.
class MonitorModel : public IModel {
public:
    MonitorModel(const SampleModel& sampleModel, const OrderModel& orderModel);

private:
    const SampleModel& sampleModel_;
    const OrderModel& orderModel_;
};
