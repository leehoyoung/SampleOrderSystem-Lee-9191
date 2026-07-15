#pragma once

// 마커 인터페이스. 각 기능 도메인의 View는 서로 다른 화면 구성을 가지므로
// 공통 출력 메서드를 강제하지 않는다.
class IView {
public:
    virtual ~IView() = default;
};
