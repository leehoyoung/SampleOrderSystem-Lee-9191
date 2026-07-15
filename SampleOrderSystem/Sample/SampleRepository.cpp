#include "SampleRepository.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <stdexcept>

SampleRepository::SampleRepository(std::string filePath) : filePath_(std::move(filePath)) {}

void SampleRepository::load() {
    samples_.clear();

    std::ifstream in(filePath_, std::ios::binary);
    if (!in.is_open()) {
        return; // 최초 실행 시 정상적인 상황
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();

    if (content.find_first_not_of(" \t\r\n") == std::string::npos) {
        return; // 빈 파일
    }

    try {
        JsonValue root = JsonValue::parse(content);
        if (root.isArray()) {
            for (const auto& item : root.arrayValue) {
                samples_.push_back(Sample::fromJson(item));
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "[경고] samples.json을 읽는 중 오류가 발생하여 빈 목록으로 시작합니다: "
                  << ex.what() << "\n";
        samples_.clear();
    }
}

void SampleRepository::save() const {
    std::filesystem::path path(filePath_);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    JsonValue root = JsonValue::makeArray();
    for (const auto& s : samples_) {
        root.push_back(s.toJson());
    }

    std::filesystem::path tmpPath = path;
    tmpPath += ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            throw std::runtime_error("임시 파일에 쓸 수 없습니다: " + tmpPath.string());
        }
        out << root.dump(2);
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, path, ec);
    if (ec) {
        std::filesystem::remove(tmpPath);
        throw std::runtime_error("파일 교체(rename)에 실패했습니다: " + path.string() + " (" + ec.message() + ")");
    }
}

void SampleRepository::append(const Sample& sample) {
    samples_.push_back(sample);
    save();
}

std::string SampleRepository::nextId() const {
    long long maxNum = 0;
    const std::string prefix = "S-";
    for (const auto& s : samples_) {
        if (s.id.rfind(prefix, 0) != 0) continue;
        std::string numPart = s.id.substr(prefix.size());
        if (numPart.empty() || !std::all_of(numPart.begin(), numPart.end(),
                                             [](unsigned char c) { return std::isdigit(c); })) {
            continue;
        }
        maxNum = std::max(maxNum, std::stoll(numPart));
    }

    long long next = maxNum + 1;
    std::ostringstream oss;
    oss << prefix << std::setfill('0') << std::setw(3) << next;
    return oss.str();
}

void SampleRepository::applyStockDelta(const std::string& sampleId, int delta) {
    for (auto& sample : samples_) {
        if (sample.id == sampleId) {
            sample.stock += delta;
            save();
            return;
        }
    }
    throw std::out_of_range("해당 ID의 시료를 찾을 수 없습니다: " + sampleId);
}
