// 사후 검증(characterization) 테스트 — 새 동작을 정의하는 TDD RED가 아니라,
// 이미 동작하는 포팅 코드의 현재 동작을 고정해 향후 회귀를 잡기 위해 작성됨.

#include <gtest/gtest.h>
#include <filesystem>
#include "../SampleOrderSystem/Sample/SampleRepository.h"

namespace {
const char* kTestFilePath = "test_data/sample_repository_test.json";
}

class SampleRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove(kTestFilePath);
    }

    void TearDown() override {
        std::filesystem::remove(kTestFilePath);
    }
};

TEST_F(SampleRepositoryTest, LoadWithNoExistingFileStartsWithEmptyList) {
    SampleRepository repo(kTestFilePath);
    repo.load();

    EXPECT_TRUE(repo.listAll().empty());
}

TEST_F(SampleRepositoryTest, AppendedDataPersistsAcrossRepositoryInstances) {
    SampleRepository repo(kTestFilePath);
    repo.load();

    Sample sample;
    sample.id = "S-001";
    sample.name = "테스트시료";
    sample.avgProductionTimeMin = 12.5;
    sample.yield = 0.9;
    sample.stock = 10;
    repo.append(sample);

    SampleRepository reloaded(kTestFilePath);
    reloaded.load();

    ASSERT_EQ(reloaded.listAll().size(), 1u);
    EXPECT_EQ(reloaded.listAll()[0].id, "S-001");
    EXPECT_EQ(reloaded.listAll()[0].name, "테스트시료");
    EXPECT_DOUBLE_EQ(reloaded.listAll()[0].avgProductionTimeMin, 12.5);
    EXPECT_DOUBLE_EQ(reloaded.listAll()[0].yield, 0.9);
    EXPECT_EQ(reloaded.listAll()[0].stock, 10);
}

TEST_F(SampleRepositoryTest, NextIdStartsAtS001WhenNoExistingData) {
    SampleRepository repo(kTestFilePath);
    repo.load();

    EXPECT_EQ(repo.nextId(), "S-001");
}

TEST_F(SampleRepositoryTest, NextIdContinuesFromExistingMaxNumber) {
    SampleRepository repo(kTestFilePath);
    repo.load();

    Sample sample;
    sample.id = "S-005";
    repo.append(sample);

    EXPECT_EQ(repo.nextId(), "S-006");
}

TEST_F(SampleRepositoryTest, NextIdIgnoresDataWithoutSPrefix) {
    SampleRepository repo(kTestFilePath);
    repo.load();

    Sample nonConforming;
    nonConforming.id = "X-999";
    repo.append(nonConforming);

    EXPECT_EQ(repo.nextId(), "S-001");
}
