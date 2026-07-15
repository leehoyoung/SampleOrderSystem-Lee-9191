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

std::string OrderModel::todayYyyymmdd() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
    localtime_s(&tmBuf, &t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d", &tmBuf);
    return std::string(buf);
}
