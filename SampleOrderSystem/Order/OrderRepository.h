#pragma once
#include <string>
#include <vector>
#include "Order.h"

// orders.json 파일을 저장소로 사용하는 Order 전용 Repository.
class OrderRepository {
public:
    explicit OrderRepository(std::string filePath);

    void load();

    const std::vector<Order>& listAll() const { return orders_; }

    // 새 주문을 메모리 목록에 추가하고 즉시 파일에 반영한다.
    void append(const Order& order);

    // "ORD-YYYYMMDD-NNNN" 형식에서 NNNN 부분을 기존 최댓값 다음 번호로 이어서 부여한다.
    // NNNN은 날짜와 무관하게 파일 전체에서 전역으로 증가한다.
    std::string nextOrderNo(const std::string& yyyymmdd) const;

private:
    std::string filePath_;
    std::vector<Order> orders_;

    void save() const;
};
