#pragma once

#include <string>
#include <vector>

#include "../Core/IView.h"
#include "Sample.h"

// 시료 관리 화면(콘솔 입출력)을 담당한다.
class SampleView : public IView {
public:
    void showMenu() const;
    void showMessage(const std::string& message) const;
    void showList(const std::vector<Sample>& samples) const;

    Sample promptNewSample() const;
    std::string promptSearchKeyword() const;
    std::string promptMenuChoice() const;
};
