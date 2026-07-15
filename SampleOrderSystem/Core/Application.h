#pragma once

#include "Router.h"
#include "../Sample/SampleRepository.h"
#include "../Sample/SampleModel.h"
#include "../Sample/SampleView.h"

// 프로그램의 메인 루프. 메인 메뉴를 출력하고 사용자의 선택을 Router로 위임한다.
class Application {
public:
    Application();
    void run();

private:
    Router router_;

    // Controller는 이 Model/View를 참조로 사용하므로, Controller보다 먼저 생성/소멸되어야 한다.
    SampleRepository sampleRepository_;
    SampleModel sampleModel_;
    SampleView sampleView_;

    void showMainMenu() const;
};
