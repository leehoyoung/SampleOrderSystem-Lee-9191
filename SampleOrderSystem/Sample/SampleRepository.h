#pragma once
#include <string>
#include <vector>
#include "Sample.h"

// samples.json 파일을 저장소로 사용하는 Sample 전용 Repository.
class SampleRepository {
public:
    explicit SampleRepository(std::string filePath);

    // 파일에서 데이터를 읽어 메모리에 적재. 파일이 없으면 빈 목록으로 시작.
    void load();

    const std::vector<Sample>& listAll() const { return samples_; }

    // 새 시료를 메모리 목록에 추가하고 즉시 파일에 반영한다.
    void append(const Sample& sample);

    // 기존 데이터의 "S-" 접두 뒤 숫자 중 최댓값 다음 번호로 새 ID를 만든다.
    std::string nextId() const;

    // 지정한 시료의 재고를 delta만큼 변경하고 즉시 파일에 반영한다.
    // 해당 ID의 시료가 없으면 std::out_of_range를 던진다.
    void applyStockDelta(const std::string& sampleId, int delta);

private:
    std::string filePath_;
    std::vector<Sample> samples_;

    void save() const;
};
