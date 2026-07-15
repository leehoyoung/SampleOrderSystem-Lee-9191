#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "IController.h"

// 명령어(메뉴 키) -> Controller 매핑을 관리하고 실행을 위임한다.
class Router {
public:
    void registerController(const std::string& key, std::shared_ptr<IController> controller);
    void dispatch(const std::string& key) const;

private:
    std::unordered_map<std::string, std::shared_ptr<IController>> controllers_;
};
