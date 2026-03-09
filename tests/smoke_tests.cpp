#include <cassert>
#include <memory>

#include "list/LinkedByteList.h"
#include "memory/HeapMemoryManager.h"
#include "set/ByteHashSet.h"
#include "tree/NaryByteTree.h"

int main() {
    cc::HeapMemoryManager memory;

    // List smoke test
    cc::LinkedByteList list(memory);
    int a = 1;
    int b = 2;
    int c = 3;

    assert(list.pushFront(&b, sizeof(b)) == 0);
    assert(list.pushFront(&a, sizeof(a)) == 0);

    std::unique_ptr<cc::Container::Iterator> listPos(list.find(&b, sizeof(b)));
    assert(listPos != nullptr);
    assert(list.insert(listPos.get(), &c, sizeof(c)) == 0);
    assert(list.size() == 3);

    // Set smoke test
    cc::ByteHashSet set(memory);
    assert(set.insert(&a, sizeof(a)) == 0);
    assert(set.insert(&a, sizeof(a)) == 1);
    assert(set.insert(&b, sizeof(b)) == 0);
    assert(set.size() == 2);

    std::unique_ptr<cc::Container::Iterator> setItem(set.find(&b, sizeof(b)));
    assert(setItem != nullptr);
    set.remove(setItem.get());
    assert(set.size() == 1);

    // Tree smoke test
    cc::NaryByteTree tree(memory);
    int root = 10;
    int child = 20;
    int leaf = 30;

    assert(tree.insertRoot(&root, sizeof(root)) == 0);
    std::unique_ptr<cc::NaryByteTree::TreeIterator> rootIt(tree.newTreeIterator());
    assert(rootIt != nullptr);
    assert(tree.insert(rootIt.get(), 0, &child, sizeof(child)) == 0);

    rootIt->goToChild(0);
    assert(tree.insert(rootIt.get(), 0, &leaf, sizeof(leaf)) == 0);
    assert(tree.size() == 3);

    std::unique_ptr<cc::Container::Iterator> found(tree.find(&leaf, sizeof(leaf)));
    assert(found != nullptr);

    return 0;
}
