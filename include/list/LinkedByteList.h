#pragma once

#include <cstddef>
#include <cstring>
#include <stdexcept>

#include "common/ByteUtils.h"
#include "core/Container.h"

namespace cc {

class LinkedByteList final : public Container {
private:
    struct Node {
        void* data = nullptr;
        size_t size = 0;
        Node* next = nullptr;
        Node* prev = nullptr;
    };

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    int count_ = 0;

    [[nodiscard]] Node* createNode(const void* elem, size_t elemSize) {
        auto* node = new Node();
        node->size = elemSize;
        node->data = nullptr;

        if (elemSize > 0U) {
            node->data = memory_.allocate(elemSize);
            std::memcpy(node->data, elem, elemSize);
        }

        return node;
    }

    void destroyNode(Node* node) {
        if (node == nullptr) {
            return;
        }

        if (node->data != nullptr) {
            memory_.deallocate(node->data, node->size);
        }
        delete node;
    }

public:
    explicit LinkedByteList(MemoryManager& memory) : Container(memory) {}

    ~LinkedByteList() override {
        clear();
    }

    class ListIterator final : public Container::Iterator {
    public:
        explicit ListIterator(Node* current) : current_(current) {}

        void* getElement(size_t& size) override {
            if (current_ == nullptr) {
                size = 0;
                return nullptr;
            }

            size = current_->size;
            return current_->data;
        }

        bool hasNext() const override {
            return current_ != nullptr && current_->next != nullptr;
        }

        void goToNext() override {
            if (current_ != nullptr) {
                current_ = current_->next;
            }
        }

        bool equals(const Iterator* other) const override {
            const auto* right = dynamic_cast<const ListIterator*>(other);
            return right != nullptr && right->current_ == current_;
        }

    private:
        Node* current_ = nullptr;

        friend class LinkedByteList;
    };

    int pushFront(const void* elem, size_t elemSize) {
        auto* node = createNode(elem, elemSize);
        node->next = head_;

        if (head_ != nullptr) {
            head_->prev = node;
        }

        head_ = node;

        if (tail_ == nullptr) {
            tail_ = node;
        }

        ++count_;
        return 0;
    }

    void popFront() {
        if (head_ == nullptr) {
            throw Error("list is empty");
        }

        auto* next = head_->next;
        destroyNode(head_);
        head_ = next;

        if (head_ != nullptr) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }

        --count_;
    }

    void* front(size_t& size) {
        if (head_ == nullptr) {
            size = 0;
            return nullptr;
        }

        size = head_->size;
        return head_->data;
    }

    int insert(Iterator* iter, const void* elem, size_t elemSize) {
        if (iter == nullptr) {
            return pushFront(elem, elemSize);
        }

        auto* listIter = dynamic_cast<ListIterator*>(iter);
        if (listIter == nullptr) {
            throw Error("iterator does not belong to LinkedByteList");
        }

        if (listIter->current_ == nullptr) {
            auto* node = createNode(elem, elemSize);

            if (tail_ == nullptr) {
                head_ = tail_ = node;
            } else {
                tail_->next = node;
                node->prev = tail_;
                tail_ = node;
            }

            ++count_;
            return 0;
        }

        auto* current = listIter->current_;
        auto* node = createNode(elem, elemSize);
        node->next = current;
        node->prev = current->prev;

        if (current->prev != nullptr) {
            current->prev->next = node;
        } else {
            head_ = node;
        }

        current->prev = node;
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
        for (Node* current = head_; current != nullptr; current = current->next) {
            if (common::byteEquals(current->data, current->size, elem, size)) {
                return new ListIterator(current);
            }
        }

        return nullptr;
    }

    Iterator* newIterator() override {
        if (head_ == nullptr) {
            return nullptr;
        }

        return new ListIterator(head_);
    }

    void remove(Iterator* iter) override {
        auto* listIter = dynamic_cast<ListIterator*>(iter);
        if (listIter == nullptr || listIter->current_ == nullptr) {
            throw Error("invalid iterator for remove");
        }

        auto* node = listIter->current_;
        auto* next = node->next;

        if (node->prev != nullptr) {
            node->prev->next = node->next;
        } else {
            head_ = node->next;
        }

        if (node->next != nullptr) {
            node->next->prev = node->prev;
        } else {
            tail_ = node->prev;
        }

        destroyNode(node);
        listIter->current_ = next;
        --count_;
    }

    void clear() override {
        auto* current = head_;
        while (current != nullptr) {
            auto* next = current->next;
            destroyNode(current);
            current = next;
        }

        head_ = nullptr;
        tail_ = nullptr;
        count_ = 0;
    }

    [[nodiscard]] bool empty() const override {
        return count_ == 0;
    }
};

} // namespace cc
