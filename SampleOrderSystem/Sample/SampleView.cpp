#include "SampleView.h"

#include <iostream>
#include <limits>

#include "../Core/StringUtil.h"

void SampleView::showMenu() const {
    std::cout << "\n[1] 시료 관리\n"
              << "----------------------------------------\n"
              << "[1] 시료 등록  [2] 시료 목록  [3] 시료 검색  [0] 뒤로\n"
              << "선택 > ";
}

void SampleView::showMessage(const std::string& message) const {
    std::cout << message << "\n";
}

void SampleView::showList(const std::vector<Sample>& samples) const {
    if (samples.empty()) {
        std::cout << "등록된 시료가 없습니다.\n";
        return;
    }

    std::cout << "\nID\t이름\t\t평균생산시간\t수율\t재고\n";
    for (const auto& sample : samples) {
        std::cout << sample.id << '\t' << sample.name << "\t\t" << sample.avgProductionTimeMin
                   << " min/ea\t" << sample.yield << '\t' << sample.stock << " ea\n";
    }
}

Sample SampleView::promptNewSample() const {
    Sample sample;

    std::cout << "시료명 > ";
    std::getline(std::cin, sample.name);
    sample.name = StringUtil::trim(sample.name);

    std::cout << "평균 생산시간(min/ea) > ";
    std::cin >> sample.avgProductionTimeMin;

    std::cout << "수율(0.0 ~ 1.0) > ";
    std::cin >> sample.yield;

    std::cout << "초기 재고 수량 > ";
    std::cin >> sample.stock;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return sample;
}

std::string SampleView::promptSearchKeyword() const {
    std::cout << "검색어(이름) > ";
    std::string keyword;
    std::getline(std::cin, keyword);
    return StringUtil::trim(keyword);
}

std::string SampleView::promptMenuChoice() const {
    std::string choice;
    std::getline(std::cin, choice);
    return StringUtil::trim(choice);
}
