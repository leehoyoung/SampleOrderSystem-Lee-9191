#include "OrderModel.h"

#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <vector>

OrderModel::OrderModel(OrderRepository& repository, SampleModel& sampleModel)
    : repository_(repository), sampleModel_(sampleModel) {}

std::string OrderModel::createOrder(const std::string& sampleId, const std::string& customerName, int quantity) {
    std::vector<Sample> samples = sampleModel_.getAll();
    bool sampleRegistered = std::any_of(samples.begin(), samples.end(),
        [&sampleId](const Sample& sample) { return sample.id == sampleId; });
    if (!sampleRegistered) {
        throw std::invalid_argument("등록되지 않은 시료 ID입니다: " + sampleId);
    }
    if (quantity <= 0) {
        throw std::invalid_argument("quantity는 0보다 커야 합니다.");
    }

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

void OrderModel::rejectOrder(const std::string& orderNo) {
    const std::vector<Order>& orders = repository_.listAll();
    findReservedOrderOrThrow(orders, orderNo, "RESERVED 상태의 주문만 거절할 수 있습니다: " + orderNo);

    repository_.updateStatus(orderNo, OrderStatus::Rejected);
}

void OrderModel::approveOrder(const std::string& orderNo) {
    const std::vector<Order>& orders = repository_.listAll();
    const Order& reservedOrder =
        findReservedOrderOrThrow(orders, orderNo, "RESERVED 상태의 주문만 승인할 수 있습니다: " + orderNo);

    const std::string sampleId = reservedOrder.sampleId;
    const int quantity = reservedOrder.quantity;

    std::vector<Sample> samples = sampleModel_.getAll();
    auto sampleIt = std::find_if(samples.begin(), samples.end(),
        [&sampleId](const Sample& sample) { return sample.id == sampleId; });
    if (sampleIt == samples.end()) {
        throw std::invalid_argument("등록되지 않은 시료 ID입니다: " + sampleId);
    }
    const int stock = sampleIt->stock;

    bool sameSampleProducing = std::any_of(orders.begin(), orders.end(),
        [&sampleId, &orderNo](const Order& order) {
            return order.sampleId == sampleId && order.orderNo != orderNo &&
                order.status == OrderStatus::Producing;
        });

    if (sameSampleProducing) {
        repository_.updateStatus(orderNo, OrderStatus::Producing);
        return;
    }

    if (stock >= quantity) {
        repository_.updateStatus(orderNo, OrderStatus::Confirmed);
        sampleModel_.decreaseStock(sampleId, quantity);
    } else {
        repository_.updateStatus(orderNo, OrderStatus::Producing);
    }
}

const Order& OrderModel::findReservedOrderOrThrow(const std::vector<Order>& orders, const std::string& orderNo,
        const std::string& wrongStatusMessage) const {
    auto it = std::find_if(orders.begin(), orders.end(),
        [&orderNo](const Order& order) { return order.orderNo == orderNo; });
    if (it == orders.end()) {
        throw std::invalid_argument("존재하지 않는 주문번호입니다: " + orderNo);
    }
    if (it->status != OrderStatus::Reserved) {
        throw std::invalid_argument(wrongStatusMessage);
    }
    return *it;
}

std::string OrderModel::todayYyyymmdd() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
    localtime_s(&tmBuf, &t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &tmBuf);
    return std::string(buf);
}
