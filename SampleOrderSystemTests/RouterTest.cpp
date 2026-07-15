// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <memory>
#include "../SampleOrderSystem/Core/Router.h"
#include "../SampleOrderSystem/Core/IController.h"

namespace {

// 테스트 전용 스파이 컨트롤러 — 프로덕션 코드에는 추가하지 않는다.
class SpyController : public IController {
public:
    void execute() override { executed = true; }
    bool executed = false;
};

}  // namespace

TEST(RouterTest, DispatchInvokesRegisteredControllerForMatchingKey) {
    Router router;
    auto spy = std::make_shared<SpyController>();
    router.registerController("1", spy);

    router.dispatch("1");

    EXPECT_TRUE(spy->executed);
}

TEST(RouterTest, DispatchOnUnknownKeyDoesNotCrash) {
    Router router;
    auto spy = std::make_shared<SpyController>();
    router.registerController("1", spy);

    EXPECT_NO_THROW(router.dispatch("unknown-key"));
    EXPECT_FALSE(spy->executed);
}
