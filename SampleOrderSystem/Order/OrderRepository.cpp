#include "OrderRepository.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <stdexcept>

OrderRepository::OrderRepository(std::string filePath) : filePath_(std::move(filePath)) {}

void OrderRepository::load() {
    orders_.clear();

    std::ifstream in(filePath_, std::ios::binary);
    if (!in.is_open()) {
        return;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();

    if (content.find_first_not_of(" \t\r\n") == std::string::npos) {
        return;
    }

    try {
        JsonValue root = JsonValue::parse(content);
        if (root.isArray()) {
            for (const auto& item : root.arrayValue) {
                orders_.push_back(Order::fromJson(item));
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "[경고] orders.json을 읽는 중 오류가 발생하여 빈 목록으로 시작합니다: "
                  << ex.what() << "\n";
        orders_.clear();
    }
}

void OrderRepository::save() const {
    std::filesystem::path path(filePath_);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    JsonValue root = JsonValue::makeArray();
    for (const auto& o : orders_) {
        root.push_back(o.toJson());
    }

    std::filesystem::path tmpPath = path;
    tmpPath += ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            throw std::runtime_error("임시 파일에 쓸 수 없습니다: " + tmpPath.string());
        }
        out << root.dump(2);
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, path, ec);
    if (ec) {
        std::filesystem::remove(tmpPath);
        throw std::runtime_error("파일 교체(rename)에 실패했습니다: " + path.string() + " (" + ec.message() + ")");
    }
}

void OrderRepository::append(const Order& order) {
    orders_.push_back(order);
    save();
}

std::string OrderRepository::nextOrderNo(const std::string& yyyymmdd) const {
    long long maxNum = 0;
    const std::string prefix = "ORD-";
    for (const auto& o : orders_) {
        if (o.orderNo.rfind(prefix, 0) != 0) continue;
        size_t lastDash = o.orderNo.find_last_of('-');
        if (lastDash == std::string::npos || lastDash <= prefix.size()) continue;
        std::string numPart = o.orderNo.substr(lastDash + 1);
        if (numPart.empty() || !std::all_of(numPart.begin(), numPart.end(),
                                             [](unsigned char c) { return std::isdigit(c); })) {
            continue;
        }
        maxNum = std::max(maxNum, std::stoll(numPart));
    }

    long long next = maxNum + 1;
    std::ostringstream oss;
    oss << prefix << yyyymmdd << "-" << std::setfill('0') << std::setw(4) << next;
    return oss.str();
}

void OrderRepository::updateStatus(const std::string& orderNo, OrderStatus status) {
    for (auto& order : orders_) {
        if (order.orderNo == orderNo) {
            order.status = status;
            save();
            return;
        }
    }
    throw std::out_of_range("해당 주문번호를 찾을 수 없습니다: " + orderNo);
}
