#pragma once

#include <string>

#include "../Core/IView.h"

// 주문 관리 화면. 실제 화면 흐름은 PRD 7.3/7.4 기능 착수 시 채워진다.
class OrderView : public IView {
public:
    void showMessage(const std::string& message) const;
};
