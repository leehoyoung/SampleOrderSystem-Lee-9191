// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
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
