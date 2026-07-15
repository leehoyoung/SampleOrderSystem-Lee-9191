#include "SampleModel.h"

#include <algorithm>
#include <cctype>

namespace {
std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}
}  // namespace

SampleModel::SampleModel(SampleRepository& repository) : repository_(repository) {}

std::string SampleModel::addSample(const Sample& sample) {
    Sample stored = sample;
    stored.id = repository_.nextId();
    repository_.append(stored);
    return stored.id;
}

std::vector<Sample> SampleModel::getAll() const {
    return repository_.listAll();
}

std::vector<Sample> SampleModel::findByName(const std::string& keyword) const {
    std::vector<Sample> result;
    const std::string lowerKeyword = toLower(keyword);

    for (const auto& sample : repository_.listAll()) {
        if (toLower(sample.name).find(lowerKeyword) != std::string::npos) {
            result.push_back(sample);
        }
    }
    return result;
}
