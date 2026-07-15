#pragma once

#include "../Core/IController.h"

// 모니터링 기능의 확장 지점. 실제 집계 로직은 PRD 7.5 착수 시 채운다.
class MonitorController : public IController {
public:
    void execute() override;
};
