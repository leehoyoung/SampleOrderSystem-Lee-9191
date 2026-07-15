#pragma once

// 마커 인터페이스. 각 기능 도메인(Sample, Order ...)의 Model은
// 서로 다른 데이터/CRUD 시그니처를 가지므로 공통 동작을 강제하지 않는다.
class IModel {
public:
    virtual ~IModel() = default;
};
