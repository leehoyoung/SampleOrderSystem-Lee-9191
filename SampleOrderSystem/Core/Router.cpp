#include "Router.h"

#include <iostream>

void Router::registerController(const std::string& key, std::shared_ptr<IController> controller) {
    controllers_[key] = std::move(controller);
}

void Router::dispatch(const std::string& key) const {
    auto it = controllers_.find(key);
    if (it == controllers_.end()) {
        std::cout << "잘못된 선택입니다. 다시 입력해 주세요.\n";
        return;
    }
    it->second->execute();
}
