#include "Application.h"

#include <iostream>

#include "StringUtil.h"

Application::Application() {
}

void Application::run() {
    while (true) {
        showMainMenu();

        std::string choice;
        std::getline(std::cin, choice);
        choice = StringUtil::trim(choice);

        if (choice == "0") {
            std::cout << "프로그램을 종료합니다.\n";
            return;
        }

        router_.dispatch(choice);
    }
}

void Application::showMainMenu() const {
    std::cout << "\n==========================================\n"
              << " 반도체 시료 생산주문관리 시스템\n"
              << "==========================================\n"
              << "[0] 종료\n\n"
              << "선택 > ";
}
