#pragma once

#include <map>
#include <stdexcept>
#include <vector>

namespace pg {

template <typename T>
class Channel;

template <typename T>
class Equalizer {
private:
    std::map<int, std::vector<T*>> unq_val_to_iter;
    const int quantize_;
    const size_t size_;
    T min_val_;
    T max_val_;

    T MinInVec(const std::vector<T*>& vec) const {
        T min = max_val_;
        for (auto ptr : vec) {
            if (*ptr < min) {
                min = *ptr;
            }
        }
        return min;
    }

    T MaxInVec(const std::vector<T*>& vec) const {
        T max = min_val_;
        for (auto ptr : vec) {
            if (*ptr > max) {
                max = *ptr;
            }
        }
        return max;
    }

public:
    Equalizer(const Channel<T>& other, int quantize = 1000) :
        quantize_(quantize), size_(other.size()) {
        auto min_max = MinMaxValues(other);
        min_val_ = min_max[0];
        max_val_ = min_max[1];
        for (auto iter = other.begin(); iter != other.end(); ++iter) {
            unq_val_to_iter[int(*iter / max_val_ * quantize_)].push_back(iter);
        }
    }

    T FindLowerPercentile(float bound = 0.0f) const {
        if (bound < 0.0f || bound > 1.0f) {
            throw std::runtime_error("Lower Bound must be between 0.0 and 1.0");
        }
        float cur_ratio = 0.0f;
        float prev_ratio = 0.0f;
        for (auto map_iter = unq_val_to_iter.begin(); map_iter != unq_val_to_iter.end();
             ++map_iter) {
            cur_ratio += float(map_iter->second.size()) / size_;
            if (cur_ratio > bound) {
                if (map_iter == unq_val_to_iter.begin()) {
                    return min_val_;
                } else {
                    if ((bound - prev_ratio) < (cur_ratio - bound)) {
                        return MaxInVec(std::prev(map_iter)->second);
                    } else {
                        return MinInVec(map_iter->second);
                    }
                }
            }
            prev_ratio = cur_ratio;
        }
        return max_val_;
    }

    T FindUpperPercentile(float bound = 1.0f) const {
        if (bound < 0.0f || bound > 1.0f) {
            throw std::runtime_error("Upper Bound must be between 0.0 and 1.0");
        }
        bound = 1.0f - bound;
        float cur_ratio = 0.0f;
        float prev_ratio = 0.0f;
        for (auto map_iter = unq_val_to_iter.rbegin(); map_iter != unq_val_to_iter.rend();
             ++map_iter) {
            cur_ratio += float(map_iter->second.size()) / size_;
            if (cur_ratio > bound) {
                if (map_iter == unq_val_to_iter.rbegin()) {
                    return max_val_;
                } else {
                    if ((bound - prev_ratio) < (cur_ratio - bound)) {
                        return MinInVec(std::prev(map_iter)->second);
                    } else {
                        return MaxInVec(map_iter->second);
                    }
                }
            }
            prev_ratio = cur_ratio;
        }
        return min_val_;
    }

    void ExportEqualized(Channel<T>& dst, float out_min, float out_max) const {
        if (size_ != dst.size()) {
            throw std::runtime_error("EQ: Sizes of input and output channels must be equal");
        }
        int cum_sum = 0;
        int num_of_darkest = int(unq_val_to_iter.begin()->second.size());
        for (const auto& [unique_value, iterators] : unq_val_to_iter) {
            cum_sum += int(iterators.size());
            for (auto iter : iterators) {
                *iter = float(cum_sum - num_of_darkest) / size_ * (out_max - out_min) + out_min;
            }
        }
    }
};

}    // namespace pg
