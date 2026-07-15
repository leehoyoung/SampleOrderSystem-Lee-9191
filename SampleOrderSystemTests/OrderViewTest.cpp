// cpp-code-reviewer 지적(order-approval PR) — OrderView::showMenu()가 실제 콘솔에
// 표시하는 문구에 승인/거절 옵션이 빠져 있어도 FakeOrderView 기반 Controller 테스트로는
// 잡히지 않는다. 이 파일은 OrderView(실제 콘솔 구현) 단위 테스트로 화면 문구 자체를
// 검증한다 (Controller 동작 검증이 아니라 View 문구 검증이므로 표준출력 캡처를 사용).

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include "../SampleOrderSystem/Order/OrderView.h"

TEST(OrderViewTest, ShowMenuIncludesApproveAndRejectOptions) {
    OrderView view;

    std::ostringstream captured;
    std::streambuf* originalCoutBuffer = std::cout.rdbuf(captured.rdbuf());
    view.showMenu();
    std::cout.rdbuf(originalCoutBuffer);

    std::string output = captured.str();
    EXPECT_NE(output.find("주문 승인"), std::string::npos);
    EXPECT_NE(output.find("주문 거절"), std::string::npos);
}
