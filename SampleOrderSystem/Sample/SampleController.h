#pragma once

#include "../Core/IController.h"
#include "SampleModel.h"
#include "SampleView.h"

// 시료 관리 기능(등록/목록/검색)의 흐름을 조율한다.
class SampleController : public IController {
public:
    SampleController(SampleModel& model, SampleView& view);
    void execute() override;

private:
    SampleModel& model_;
    SampleView& view_;

    void registerSample();
    void listSamples();
    void searchSample();
};
