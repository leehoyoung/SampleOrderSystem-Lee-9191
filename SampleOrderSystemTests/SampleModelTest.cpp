// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include <stdexcept>
#include "../SampleOrderSystem/Sample/SampleModel.h"
#include "../SampleOrderSystem/Sample/SampleRepository.h"

namespace {
const char* kTestFilePath = "test_data/sample_model_test.json";
}

class SampleModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kTestFilePath);
    }
};

TEST_F(SampleModelTest, AddSampleReturnsIdFollowingRepositoryNextIdRule) {
    SampleRepository repo(kTestFilePath);
    repo.load();
    SampleModel model(repo);

    Sample sample;
    sample.name = "첫번째시료";
    std::string firstId = model.addSample(sample);
    EXPECT_EQ(firstId, "S-001");

    Sample second;
    second.name = "두번째시료";
    std::string secondId = model.addSample(second);
    EXPECT_EQ(secondId, "S-002");
}

TEST_F(SampleModelTest, FindByNameMatchesCaseInsensitiveSubstring) {
    SampleRepository repo(kTestFilePath);
    repo.load();
    SampleModel model(repo);

    Sample sample;
    sample.name = "Wafer Sample ABC";
    model.addSample(sample);

    EXPECT_EQ(model.findByName("wafer").size(), 1u);
    EXPECT_EQ(model.findByName("SAMPLE").size(), 1u);
    EXPECT_TRUE(model.findByName("no-match").empty());
}

// 태스크 1 [기반]: 재고 차감 메서드가 시료의 재고를 정확히 줄이고 영속화한다.
TEST_F(SampleModelTest, DecreaseStockReducesStockBySpecifiedQuantity) {
    SampleRepository repo(kTestFilePath);
    repo.load();
    SampleModel model(repo);

    Sample sample;
    sample.id = "S-001";
    sample.name = "테스트시료";
    sample.stock = 30;
    repo.append(sample);

    model.decreaseStock("S-001", 10);

    ASSERT_EQ(model.getAll().size(), 1u);
    EXPECT_EQ(model.getAll()[0].stock, 20);
}

// 태스크 2 [기반, 경계]: 존재하지 않는 시료 ID로 재고 차감을 시도하면 실패하고,
// 다른 시료의 재고도 변경되지 않는다.
TEST_F(SampleModelTest, DecreaseStockThrowsWhenSampleIdNotFound) {
    SampleRepository repo(kTestFilePath);
    repo.load();
    SampleModel model(repo);

    Sample sample;
    sample.id = "S-001";
    sample.name = "테스트시료";
    sample.stock = 30;
    repo.append(sample);

    EXPECT_THROW(model.decreaseStock("S-999", 10), std::out_of_range);

    ASSERT_EQ(model.getAll().size(), 1u);
    EXPECT_EQ(model.getAll()[0].stock, 30);
}
