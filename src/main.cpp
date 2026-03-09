#include <iostream>
#include <memory>

#include "list/LinkedByteList.h"
#include "memory/HeapMemoryManager.h"
#include "set/ByteHashSet.h"
#include "tree/NaryByteTree.h"

namespace {

void printList(cc::LinkedByteList& list) {
    std::unique_ptr<cc::Container::Iterator> it(list.newIterator());
    std::cout << "List: ";

    while (it != nullptr) {
        size_t size = 0;
        void* valuePtr = it->getElement(size);
        if (valuePtr == nullptr) {
            break;
        }

        std::cout << *static_cast<int*>(valuePtr) << ' ';

        if (!it->hasNext()) {
            break;
        }
        it->goToNext();
    }

    std::cout << '\n';
}

void printSet(cc::ByteHashSet& set) {
    std::unique_ptr<cc::Container::Iterator> it(set.newIterator());
    std::cout << "Set: ";

    while (it != nullptr) {
        size_t size = 0;
        void* valuePtr = it->getElement(size);
        if (valuePtr == nullptr) {
            break;
        }

        std::cout << *static_cast<int*>(valuePtr) << ' ';

        if (!it->hasNext()) {
            break;
        }
        it->goToNext();
    }

    std::cout << '\n';
}

void printTreePreorder(cc::NaryByteTree& tree) {
    std::unique_ptr<cc::Container::Iterator> it(tree.newIterator());
    std::cout << "Tree preorder: ";

    while (it != nullptr) {
        size_t size = 0;
        void* valuePtr = it->getElement(size);
        if (valuePtr == nullptr) {
            break;
        }

        std::cout << *static_cast<int*>(valuePtr) << ' ';

        if (!it->hasNext()) {
            break;
        }
        it->goToNext();
    }

    std::cout << '\n';
}

} // namespace

int main() {
    cc::HeapMemoryManager memory;

    cc::LinkedByteList list(memory);
    int one = 1;
    int two = 2;
    int three = 3;
    list.pushFront(&two, sizeof(two));
    list.pushFront(&one, sizeof(one));

    std::unique_ptr<cc::Container::Iterator> listIt(list.find(&two, sizeof(two)));
    list.insert(listIt.get(), &three, sizeof(three));
    printList(list);

    cc::ByteHashSet set(memory);
    set.insert(&one, sizeof(one));
    set.insert(&two, sizeof(two));
    set.insert(&two, sizeof(two)); // duplicate is ignored
    set.insert(&three, sizeof(three));
    printSet(set);

    cc::NaryByteTree tree(memory);
    int rootValue = 10;
    int leftValue = 20;
    int rightValue = 30;
    int leafValue = 40;

    tree.insertRoot(&rootValue, sizeof(rootValue));
    std::unique_ptr<cc::NaryByteTree::TreeIterator> root(tree.newTreeIterator());
    tree.insert(root.get(), 0, &leftValue, sizeof(leftValue));
    tree.insert(root.get(), 1, &rightValue, sizeof(rightValue));

    root->goToChild(0);
    tree.insert(root.get(), 0, &leafValue, sizeof(leafValue));
    printTreePreorder(tree);

    std::cout << "Allocated payload bytes: " << memory.allocatedBytes() << '\n';
    return 0;
}
