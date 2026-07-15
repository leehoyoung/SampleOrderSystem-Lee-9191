#pragma once

#include <vector>

#include "../Core/IModel.h"
#include "Sample.h"
#include "SampleRepository.h"

// 시료 데이터를 SampleRepository(JSON 파일)에 위임하고, 조회용 필터링을 제공한다.
class SampleModel : public IModel {
public:
    explicit SampleModel(SampleRepository& repository);

    std::string addSample(const Sample& sample);
    void decreaseStock(const std::string& sampleId, int quantity);
    std::vector<Sample> getAll() const;
    std::vector<Sample> findByName(const std::string& keyword) const;

private:
    SampleRepository& repository_;
};
