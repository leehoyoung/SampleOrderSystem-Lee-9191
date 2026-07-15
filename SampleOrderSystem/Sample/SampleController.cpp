#include "SampleController.h"

SampleController::SampleController(SampleModel& model, SampleView& view)
    : model_(model), view_(view) {}

void SampleController::execute() {
    while (true) {
        view_.showMenu();
        const std::string choice = view_.promptMenuChoice();

        if (choice == "1") {
            registerSample();
        } else if (choice == "2") {
            listSamples();
        } else if (choice == "3") {
            searchSample();
        } else if (choice == "0") {
            return;
        } else {
            view_.showMessage("잘못된 선택입니다. 다시 입력해 주세요.");
        }
    }
}

void SampleController::registerSample() {
    const Sample input = view_.promptNewSample();
    const std::string id = model_.addSample(input);
    view_.showMessage("등록 완료. 발급된 시료 ID: " + id);
}

void SampleController::listSamples() {
    view_.showList(model_.getAll());
}

void SampleController::searchSample() {
    const std::string keyword = view_.promptSearchKeyword();
    view_.showList(model_.findByName(keyword));
}
