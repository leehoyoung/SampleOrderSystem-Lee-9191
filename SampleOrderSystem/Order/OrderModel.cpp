#include "OrderModel.h"

#include <ctime>

OrderModel::OrderModel(OrderRepository& repository) : repository_(repository) {}

std::string OrderModel::createOrder(const std::string& sampleId, const std::string& customerName, int quantity) {
    Order order;
    order.orderNo = repository_.nextOrderNo(todayYyyymmdd());
    order.sampleId = sampleId;
    order.customerName = customerName;
    order.quantity = quantity;
    order.status = OrderStatus::Reserved;

    repository_.append(order);
    return order.orderNo;
}

std::vector<Order> OrderModel::getAll() const {
    return repository_.listAll();
}

std::string OrderModel::todayYyyymmdd() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
    localtime_s(&tmBuf, &t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &tmBuf);
    return std::string(buf);
}
