#pragma once

#include <cstddef>
#include <stdexcept>

#include "memory/MemoryManager.h"

namespace cc {

class Container {
public:
    class Iterator {
    public:
        virtual ~Iterator() = default;

        virtual void* getElement(size_t& size) = 0;
        virtual bool hasNext() const = 0;
        virtual void goToNext() = 0;
        virtual bool equals(const Iterator* other) const = 0;
    };

    class Error : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    explicit Container(MemoryManager& memory) : memory_(memory) {}
    virtual ~Container() = default;

    virtual int size() const = 0;
    virtual size_t maxBytes() const = 0;
    virtual Iterator* find(const void* elem, size_t size) = 0;
    virtual Iterator* newIterator() = 0;
    virtual void remove(Iterator* iter) = 0;
    virtual void clear() = 0;
    virtual bool empty() const = 0;

protected:
    MemoryManager& memory_;
};

} // namespace cc
