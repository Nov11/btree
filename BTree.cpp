//
// Created by c6s on 2017/6/16.
//

#include "BTree.h"

namespace BTreeNS {
    bool BTree::search(BTreeNode::Key key, BTreeNode *result, int &index) {
        return root->search(key, result, index);
    }

    void BTree::remove(BTreeNode::Key key) {
        root->remove(key);
    }

    void BTree::insert(BTreeNode::Key key, BTreeNode::Value value) {
        root->insert(key, value);
    }

    bool BTreeNode::search(BTreeNode::Key key, BTreeNode *result, int &index) {
        size_t i = 0;
        for (; i < keys_.size(); i++) {
            if (keys_[i] == key) {
                result = this;
                index = static_cast<int>(i);
                return true;
            } else if (key < keys_[i]) {
                break;
            }
        }
        if(isLeaf){return false;}
        //search node of index
        return links_[i]->search(key, result, index);
    }

    void BTreeNode::insert(BTreeNode::Key key, BTreeNode::Value value) {
        //this is root node of btree
        if(keys_.size() == btree_->maxKeys()){
            std::shared_ptr<BTreeNode> newNode(new BTreeNode);
            newNode->isLeaf = false;
            newNode->btree_ = btree_;
            newNode->links_.push_back(shared_from_this());
            newNode->split(0);
            btree_->setRoot(newNode);
            newNode->insertNotFull(key, value);
        }else{
            insertNotFull(key, value);
        }
    }

    void BTreeNode::split(int childIdx) {
        assert(childIdx >= 0 && childIdx < links_.size());
        assert(this->keys_.size() < static_cast<size_t>(this->btree_->getDim() * 2 - 1));
        auto node = links_[childIdx];
        assert(node->keys_.size() == static_cast<size_t>(this->btree_->getDim() * 2 - 1));
        //create a new node
        std::shared_ptr<BTreeNode> newNode(new BTreeNode);
        newNode->isLeaf = node->isLeaf;
        newNode->btree_ = node->btree_;
        //copy the last half keys and links to new node
        int dim = node->btree_->getDim();
        //sizeof key_ is 2 * dim_ - 1 now. let t = dim_.
        //move index from t to 2t - 1 that is t - 1 keys to new node
        auto& v = newNode->keys_;
        v.insert(v.begin(), &node->keys_[dim], &node->keys_[2 * dim - 1]);
        //move t links to new node
        auto& l = newNode->links_;
        l.insert(l.begin(), &node->links_[dim], &node->links_[2 * dim]);
        //trim keys_ and links_ of node
        Key pushUp = node->keys_[dim - 1];
        node->keys_.resize(static_cast<size_t>(dim - 1));
        node->links_.resize(static_cast<size_t>(dim));

        //insert pushUp to this node's key_
        keys_.insert(keys_.begin() + childIdx, pushUp);
        links_.insert(links_.begin() + childIdx + 1, newNode);
    }

    void BTreeNode::insertNotFull(BTreeNode::Key key, BTreeNode::Value value) {
        assert(keys_.size() < btree_->maxKeys() && keys_.size() >= btree_->minKeys());
//        if(isLeaf){
//            size_t b = 0;
//            size_t e = keys_.size();
//            size_t mid = 0;
//            while(b < e){
//                mid = b + (e - b) / 2;
//                if(keys_[mid] == key){
//                    keys_[mid].v = value;
//                    break;
//                }else if(keys_[mid] < key){
//                    b = mid + 1;
//                }else{
//                    e = mid;
//                }
//            }
//            if(b == e){
//                keys_.insert(keys_.begin() + b, key);
//            }
//            return;
//        }

        //not leaf node
        size_t b = 0;
        size_t e = keys_.size();
        size_t mid = 0;
        while(b < e){
            mid = b + (e - b) / 2;
            if(keys_[mid] == key){
                keys_[mid].v = value;
                break;
            }else if(keys_[mid] < key){
                b = mid + 1;
            }else{
                e = mid;
            }
        }
        if(b == e){
            if(isLeaf){
                keys_.insert(keys_.begin() + b, key);
            }else {
                //insert into link_[b]
                auto &node = links_[b];
                if (node->isNodeFullOfKeys()) {
                    split(b);
                    if (keys_[b] < key) {
                        node = links_[b + 1];
                    }
                }
                node->insertNotFull(key, value);
            }
        }
    }
}

