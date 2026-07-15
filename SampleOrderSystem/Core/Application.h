#pragma once

#include "Router.h"

// 프로그램의 메인 루프. 메인 메뉴를 출력하고 사용자의 선택을 Router로 위임한다.
class Application {
public:
    Application();
    void run();

private:
    Router router_;

    void showMainMenu() const;
};
