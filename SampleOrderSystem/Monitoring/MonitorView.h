#pragma once

#include <string>

#include "../Core/IView.h"

// 모니터링 화면. DataMonitor PoC의 테이블/대시보드 렌더링 스타일은
// PRD 7.5 기능 착수 시 이 클래스에 반영한다.
class MonitorView : public IView {
public:
    void showMessage(const std::string& message) const;
};
