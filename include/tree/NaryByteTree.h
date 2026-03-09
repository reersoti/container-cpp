#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "common/ByteUtils.h"
#include "core/Container.h"

namespace cc {

class NaryByteTree final : public Container {
private:
    struct Node {
        void* data = nullptr;
        size_t size = 0;
        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;
    };

    std::unique_ptr<Node> root_;
    int count_ = 0;

    [[nodiscard]] Node* createDetachedNode(const void* elem, size_t size) {
        auto node = std::make_unique<Node>();
        node->size = size;
        node->data = nullptr;

        if (size > 0U) {
            node->data = memory_.allocate(size);
            std::memcpy(node->data, elem, size);
        }

        auto* raw = node.get();
        detachedNodes_.push_back(std::move(node));
        return raw;
    }

    std::unique_ptr<Node> takeDetachedNode(Node* node) {
        for (auto it = detachedNodes_.begin(); it != detachedNodes_.end(); ++it) {
            if (it->get() == node) {
                auto result = std::move(*it);
                detachedNodes_.erase(it);
                return result;
            }
        }

        throw Error("internal tree state is corrupted");
    }

    void releaseNode(Node* node) {
        if (node == nullptr) {
            return;
        }

        for (auto& child : node->children) {
            releaseNode(child.get());
        }

        if (node->data != nullptr) {
            memory_.deallocate(node->data, node->size);
        }
    }

    [[nodiscard]] Node* findNodeRecursive(Node* node, const void* elem, size_t size) const {
        if (node == nullptr) {
            return nullptr;
        }

        if (common::byteEquals(node->data, node->size, elem, size)) {
            return node;
        }

        for (const auto& child : node->children) {
            if (auto* found = findNodeRecursive(child.get(), elem, size); found != nullptr) {
                return found;
            }
        }

        return nullptr;
    }

    [[nodiscard]] static int subtreeSize(const Node* node) {
        if (node == nullptr) {
            return 0;
        }

        int total = 1;
        for (const auto& child : node->children) {
            total += subtreeSize(child.get());
        }
        return total;
    }

    [[nodiscard]] static Node* firstChild(Node* node, int index) {
        if (node == nullptr || index < 0 || static_cast<size_t>(index) >= node->children.size()) {
            return nullptr;
        }

        return node->children[static_cast<size_t>(index)].get();
    }

    [[nodiscard]] static Node* preorderSuccessor(Node* node) {
        if (node == nullptr) {
            return nullptr;
        }

        if (!node->children.empty()) {
            return node->children.front().get();
        }

        while (node->parent != nullptr) {
            auto& siblings = node->parent->children;
            for (size_t i = 0; i < siblings.size(); ++i) {
                if (siblings[i].get() == node) {
                    if (i + 1U < siblings.size()) {
                        return siblings[i + 1U].get();
                    }
                    break;
                }
            }
            node = node->parent;
        }

        return nullptr;
    }

    [[nodiscard]] static Node* nextAfterSubtree(Node* node) {
        if (node == nullptr) {
            return nullptr;
        }

        while (node->parent != nullptr) {
            auto& siblings = node->parent->children;
            for (size_t i = 0; i < siblings.size(); ++i) {
                if (siblings[i].get() == node) {
                    if (i + 1U < siblings.size()) {
                        return siblings[i + 1U].get();
                    }
                    break;
                }
            }
            node = node->parent;
        }

        return nullptr;
    }

public:
    explicit NaryByteTree(MemoryManager& memory) : Container(memory) {}

    ~NaryByteTree() override {
        clear();
    }

    class TreeIterator final : public Container::Iterator {
    public:
        TreeIterator(NaryByteTree* owner, Node* current) : owner_(owner), current_(current) {}

        void* getElement(size_t& size) override {
            if (current_ == nullptr) {
                size = 0;
                return nullptr;
            }

            size = current_->size;
            return current_->data;
        }

        bool hasNext() const override {
            return current_ != nullptr && preorderSuccessor(current_) != nullptr;
        }

        void goToNext() override {
            if (current_ != nullptr) {
                current_ = preorderSuccessor(current_);
            }
        }

        bool equals(const Iterator* other) const override {
            const auto* right = dynamic_cast<const TreeIterator*>(other);
            return right != nullptr && right->owner_ == owner_ && right->current_ == current_;
        }

        bool goToParent() {
            if (current_ == nullptr || current_->parent == nullptr) {
                return false;
            }

            current_ = current_->parent;
            return true;
        }

        bool goToChild(int childIndex) {
            auto* child = firstChild(current_, childIndex);
            if (child == nullptr) {
                return false;
            }

            current_ = child;
            return true;
        }

    private:
        NaryByteTree* owner_ = nullptr;
        Node* current_ = nullptr;

        friend class NaryByteTree;
    };

    int insertRoot(const void* elem, size_t size) {
        if (root_ != nullptr) {
            return 1;
        }

        Node* detached = createDetachedNode(elem, size);
        root_ = takeDetachedNode(detached);
        ++count_;
        return 0;
    }

    int insert(TreeIterator* parentIter, int childIndex, const void* elem, size_t size) {
        if (root_ == nullptr) {
            return insertRoot(elem, size);
        }

        if (parentIter == nullptr || parentIter->current_ == nullptr) {
            throw Error("parent iterator is required for a non-empty tree");
        }

        auto* parent = parentIter->current_;
        if (childIndex < 0 || static_cast<size_t>(childIndex) > parent->children.size()) {
            throw Error("child index is out of range");
        }

        Node* detached = createDetachedNode(elem, size);
        detached->parent = parent;

        auto ownedNode = takeDetachedNode(detached);
        parent->children.insert(parent->children.begin() + childIndex, std::move(ownedNode));
        ++count_;
        return 0;
    }

    bool remove(TreeIterator* iter, bool leafOnly) {
        if (iter == nullptr || iter->current_ == nullptr) {
            return false;
        }

        auto* node = iter->current_;
        if (leafOnly && !node->children.empty()) {
            return false;
        }

        const int removedNodes = subtreeSize(node);
        Node* next = nextAfterSubtree(node);

        if (node == root_.get()) {
            releaseNode(root_.get());
            root_.reset();
        } else {
            auto& siblings = node->parent->children;
            for (auto it = siblings.begin(); it != siblings.end(); ++it) {
                if (it->get() == node) {
                    releaseNode(it->get());
                    siblings.erase(it);
                    break;
                }
            }
        }

        count_ -= removedNodes;
        iter->current_ = next;
        return true;
    }

    [[nodiscard]] int size() const override {
        return count_;
    }

    [[nodiscard]] size_t maxBytes() const override {
        return memory_.maxBytes();
    }

    Iterator* find(const void* elem, size_t size) override {
        auto* found = findNodeRecursive(root_.get(), elem, size);
        if (found == nullptr) {
            return nullptr;
        }

        return new TreeIterator(this, found);
    }

    Iterator* newIterator() override {
        if (root_ == nullptr) {
            return nullptr;
        }

        return new TreeIterator(this, root_.get());
    }

    TreeIterator* newTreeIterator() {
        if (root_ == nullptr) {
            return nullptr;
        }

        return new TreeIterator(this, root_.get());
    }

    void remove(Iterator* iter) override {
        auto* treeIter = dynamic_cast<TreeIterator*>(iter);
        if (treeIter == nullptr) {
            throw Error("iterator does not belong to NaryByteTree");
        }

        if (!remove(treeIter, false)) {
            throw Error("failed to remove tree node");
        }
    }

    void clear() override {
        if (root_ != nullptr) {
            releaseNode(root_.get());
            root_.reset();
        }

        detachedNodes_.clear();
        count_ = 0;
    }

    [[nodiscard]] bool empty() const override {
        return count_ == 0;
    }

private:
    std::vector<std::unique_ptr<Node>> detachedNodes_;
};

} // namespace cc
