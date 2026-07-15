#pragma once

#include "../Core/IController.h"

// 주문 관리 기능의 확장 지점.
// 실제 로직(예약/승인/거절)은 PRD 7.3/7.4 착수 시 Sample 기능과 동일한
// Model/View/Controller 패턴으로 채워 넣는다.
class OrderController : public IController {
public:
    void execute() override;
};
