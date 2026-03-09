#pragma once

#include <cstddef>
#include <limits>
#include <new>
#include <stdexcept>

#include "memory/MemoryManager.h"

namespace cc {

class HeapMemoryManager final : public MemoryManager {
public:
    explicit HeapMemoryManager(size_t limitBytes = std::numeric_limits<size_t>::max())
        : MemoryManager(limitBytes) {}

    [[nodiscard]] size_t allocatedBytes() const noexcept override {
        return allocatedBytes_;
    }

    [[nodiscard]] size_t maxBytes() const noexcept override {
        return limitBytes();
    }

    void* allocate(size_t bytes) override {
        if (allocatedBytes_ > limitBytes() - bytes) {
            throw std::bad_alloc();
        }

        auto* memory = new std::byte[bytes == 0U ? 1U : bytes];
        allocatedBytes_ += bytes;
        return memory;
    }

    void deallocate(void* ptr, size_t bytes) noexcept override {
        delete[] static_cast<std::byte*>(ptr);

        if (allocatedBytes_ >= bytes) {
            allocatedBytes_ -= bytes;
        } else {
            allocatedBytes_ = 0;
        }
    }

private:
    size_t allocatedBytes_ = 0;
};

} // namespace cc
