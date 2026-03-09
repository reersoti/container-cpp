#pragma once

#include <cstddef>
#include <cstring>

namespace cc::common {

inline bool byteEquals(const void* left, size_t leftSize, const void* right, size_t rightSize) {
    if (leftSize != rightSize) {
        return false;
    }

    if (leftSize == 0U) {
        return true;
    }

    return std::memcmp(left, right, leftSize) == 0;
}

inline size_t fnv1aHash(const void* data, size_t size) {
    constexpr size_t kOffsetBasis = sizeof(size_t) == 8U
        ? 14695981039346656037ull
        : 2166136261u;
    constexpr size_t kPrime = sizeof(size_t) == 8U
        ? 1099511628211ull
        : 16777619u;

    const auto* bytes = static_cast<const unsigned char*>(data);
    size_t hash = kOffsetBasis;

    for (size_t i = 0; i < size; ++i) {
        hash ^= static_cast<size_t>(bytes[i]);
        hash *= kPrime;
    }

    return hash;
}

} // namespace cc::common
