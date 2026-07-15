#pragma once

// Router가 메뉴 선택에 대응하는 Controller를 실행할 때 호출하는 진입점.
class IController {
public:
    virtual ~IController() = default;
    virtual void execute() = 0;
};
