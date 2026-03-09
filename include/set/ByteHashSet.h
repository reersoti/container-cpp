#pragma once

#include <cstddef>
#include <cstring>
#include <vector>

#include "common/ByteUtils.h"
#include "core/Container.h"

namespace cc {

class ByteHashSet final : public Container {
private:
    struct Entry {
        void* data = nullptr;
        size_t size = 0;
        Entry* next = nullptr;
    };

    std::vector<Entry*> buckets_;
    int count_ = 0;

    [[nodiscard]] size_t bucketIndex(const void* elem, size_t size) const {
        return common::fnv1aHash(elem, size) % buckets_.size();
    }

    [[nodiscard]] Entry* createEntry(const void* elem, size_t size) {
        auto* entry = new Entry();
        entry->size = size;
        entry->data = nullptr;

        if (size > 0U) {
            entry->data = memory_.allocate(size);
            std::memcpy(entry->data, elem, size);
        }

        return entry;
    }

    void destroyEntry(Entry* entry) {
        if (entry == nullptr) {
            return;
        }

        if (entry->data != nullptr) {
            memory_.deallocate(entry->data, entry->size);
        }
        delete entry;
    }

    [[nodiscard]] Entry* findEntry(const void* elem, size_t size) const {
        auto* current = buckets_[bucketIndex(elem, size)];

        while (current != nullptr) {
            if (common::byteEquals(current->data, current->size, elem, size)) {
                return current;
            }
            current = current->next;
        }

        return nullptr;
    }

    [[nodiscard]] Entry* firstEntry(size_t& bucket) const {
        for (bucket = 0; bucket < buckets_.size(); ++bucket) {
            if (buckets_[bucket] != nullptr) {
                return buckets_[bucket];
            }
        }

        return nullptr;
    }

    [[nodiscard]] Entry* nextEntry(size_t& bucket, Entry* current) const {
        if (current != nullptr && current->next != nullptr) {
            return current->next;
        }

        for (size_t nextBucket = bucket + 1; nextBucket < buckets_.size(); ++nextBucket) {
            if (buckets_[nextBucket] != nullptr) {
                bucket = nextBucket;
                return buckets_[nextBucket];
            }
        }

        bucket = buckets_.size();
        return nullptr;
    }

public:
    explicit ByteHashSet(MemoryManager& memory, size_t bucketCount = 257)
        : Container(memory), buckets_(bucketCount, nullptr) {}

    ~ByteHashSet() override {
        clear();
    }

    class SetIterator final : public Container::Iterator {
    public:
        SetIterator(ByteHashSet* owner, size_t bucket, Entry* current)
            : owner_(owner), bucket_(bucket), current_(current) {}

        void* getElement(size_t& size) override {
            if (current_ == nullptr) {
                size = 0;
                return nullptr;
            }

            size = current_->size;
            return current_->data;
        }

        bool hasNext() const override {
            if (owner_ == nullptr || current_ == nullptr) {
                return false;
            }

            size_t probeBucket = bucket_;
            return owner_->nextEntry(probeBucket, current_) != nullptr;
        }

        void goToNext() override {
            if (owner_ == nullptr || current_ == nullptr) {
                return;
            }

            current_ = owner_->nextEntry(bucket_, current_);
        }

        bool equals(const Iterator* other) const override {
            const auto* right = dynamic_cast<const SetIterator*>(other);
            return right != nullptr && right->owner_ == owner_ && right->current_ == current_;
        }

    private:
        ByteHashSet* owner_ = nullptr;
        size_t bucket_ = 0;
        Entry* current_ = nullptr;

        friend class ByteHashSet;
    };

    int insert(const void* elem, size_t size) {
        if (findEntry(elem, size) != nullptr) {
            return 1;
        }

        const size_t bucket = bucketIndex(elem, size);
        auto* entry = createEntry(elem, size);
        entry->next = buckets_[bucket];
        buckets_[bucket] = entry;
        ++count_;
        return 0;
    }

    [[nodiscard]] int size() const override {
        return count_;
    }

    [[nodiscard]] size_t maxBytes() const override {
        return memory_.maxBytes();
    }

    Iterator* find(const void* elem, size_t size) override {
        const size_t bucket = bucketIndex(elem, size);
        auto* current = buckets_[bucket];

        while (current != nullptr) {
            if (common::byteEquals(current->data, current->size, elem, size)) {
                return new SetIterator(this, bucket, current);
            }
            current = current->next;
        }

        return nullptr;
    }

    Iterator* newIterator() override {
        size_t bucket = 0;
        auto* entry = firstEntry(bucket);
        if (entry == nullptr) {
            return nullptr;
        }

        return new SetIterator(this, bucket, entry);
    }

    void remove(Iterator* iter) override {
        auto* setIter = dynamic_cast<SetIterator*>(iter);
        if (setIter == nullptr || setIter->current_ == nullptr) {
            throw Error("invalid iterator for remove");
        }

        const size_t currentBucket = setIter->bucket_;
        auto* current = buckets_[currentBucket];
        Entry* previous = nullptr;

        while (current != nullptr && current != setIter->current_) {
            previous = current;
            current = current->next;
        }

        if (current == nullptr) {
            throw Error("iterator does not point to an element in this set");
        }

        size_t nextBucket = currentBucket;
        auto* next = nextEntry(nextBucket, current);

        if (previous == nullptr) {
            buckets_[currentBucket] = current->next;
        } else {
            previous->next = current->next;
        }

        destroyEntry(current);
        setIter->bucket_ = nextBucket;
        setIter->current_ = next;
        --count_;
    }

    void clear() override {
        for (auto*& bucket : buckets_) {
            auto* current = bucket;
            while (current != nullptr) {
                auto* next = current->next;
                destroyEntry(current);
                current = next;
            }
            bucket = nullptr;
        }

        count_ = 0;
    }

    [[nodiscard]] bool empty() const override {
        return count_ == 0;
    }
};

} // namespace cc
