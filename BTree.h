//
// Created by c6s on 2017/6/16.
//

#ifndef BTREE_BTREE_H
#define BTREE_BTREE_H

#include <vector>
#include <memory>
#include <assert.h>
#include <map>

namespace BTreeNS {
    class Noncopyable {
    protected:
        Noncopyable() = default;

        ~Noncopyable() = default;

    private:
        Noncopyable(const Noncopyable &) = delete;

        Noncopyable &operator=(const Noncopyable &) = delete;

    };
    struct P{
        int k;
//        int v;
        using second_type = int;
        bool operator<(const P& param){
            return k < param.k;
        }
        bool operator==(const P& param){
            return k == param.k;
        }
    };
    class BTree;
    struct BTreeNode : public Noncopyable, public std::enable_shared_from_this<BTreeNode> {
        bool isLeaf = true;
        using Key = P;
//        using Value = Key::second_type;
        std::vector<std::shared_ptr<BTreeNode>> links_;
        std::vector<Key> keys_;
        BTree *btree_ = nullptr;

        bool search(Key, BTreeNode *, int &index);

        void insert(Key k);

        void insertNotFull(Key k);

        void removeKey(Key k);

        void split(int childIndex);

        void setTree(BTree *ptr) { btree_ = ptr; }

        size_t numOfKeys() const {return keys_.size();}

        size_t numOfChildren() const {return links_.size();}

        bool isNodeFullOfKeys() const;

        bool exists(Key k);

        int index(BTreeNode::Key k, bool& exists);
    };

    class BTree : public Noncopyable {
    public:
        using Key = P;

        BTree(int dim)
                : dim_(dim) , root(new BTreeNode){
            assert(dim >= 2);
            root->setTree(this);
        }

        void insert(Key k);

        void removeKey(Key k);

        bool search(Key k, BTreeNode *, int &index);

        int getDim() const { return dim_; }

        size_t maxKeys() const { return static_cast<size_t>(dim_ * 2 - 1); }
        size_t minKeys() const { return static_cast<size_t>(dim_ - 1); }
        void setRoot(std::shared_ptr<BTreeNode> &ptr) { root = ptr; }
        std::shared_ptr<BTreeNode> getRoot() const {return root;}
    private:
        //for nodes except root, there is at most 2 * dim_ out links and at least dim_ out links.
        int dim_;
        std::shared_ptr<BTreeNode> root;
    };
}

#endif //BTREE_BTREE_H
