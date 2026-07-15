#include "Application.h"

#include <iostream>

#include "../Sample/SampleController.h"
#include "../Order/OrderController.h"
#include "../Monitoring/MonitorController.h"
#include "StringUtil.h"

Application::Application()
    : sampleRepository_("data/samples.json"), sampleModel_(sampleRepository_),
      orderRepository_("data/orders.json"), orderModel_(orderRepository_, sampleModel_),
      monitorModel_(sampleModel_, orderModel_) {
    sampleRepository_.load();
    orderRepository_.load();
    router_.registerController("1", std::make_shared<SampleController>(sampleModel_, sampleView_));
    router_.registerController("2", std::make_shared<OrderController>(orderView_, orderModel_));
    router_.registerController("3", std::make_shared<MonitorController>());
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
              << "[1] 시료 관리\n"
              << "[2] 주문 관리\n"
              << "[3] 모니터링\n"
              << "[0] 종료\n\n"
              << "선택 > ";
}
