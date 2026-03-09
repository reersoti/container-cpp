#pragma once

#include <cstddef>
#include <limits>

namespace cc {

class MemoryManager {
public:
    explicit MemoryManager(size_t limitBytes = std::numeric_limits<size_t>::max())
        : limitBytes_(limitBytes) {}

    virtual ~MemoryManager() = default;

    [[nodiscard]] size_t limitBytes() const noexcept {
        return limitBytes_;
    }

    virtual size_t allocatedBytes() const noexcept = 0;
    virtual size_t maxBytes() const noexcept = 0;
    virtual void* allocate(size_t bytes) = 0;
    virtual void deallocate(void* ptr, size_t bytes) noexcept = 0;

private:
    size_t limitBytes_;
};

} // namespace cc
