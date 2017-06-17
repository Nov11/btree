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

    void BTreeNode::remove(BTreeNode::Key key) {
        if(exists(key)){
            if(isLeaf){
                keys_.erase(remove(keys_.begin(), keys_.end(), key));
                return;
            }else{
                bool exists;
                int idx = index(key, exists);
                assert(exists);
                auto& prev = links_[idx];
                size_t prevKeyCnt = prev->numOfKeys();
                if(prevKeyCnt >= btree_->minKeys() + 1){
                    //swap keys
                    using std::swap;
                    swap(*prev->keys_.rbegin(), keys_[idx]);
                    prev->remove(key);
                    return;
                }else{
                    //exam right child
                    auto& post = links_[idx + 1];
                    size_t postKeyCnt = post->numOfKeys();
                    if(postKeyCnt >= btree_->minKeys() + 1){
                        using std::swap;
                        swap(*post->keys_.begin(), keys_[idx]);
                        post->remove(key);
                        return;
                    }else{
                        assert((post->isLeaf && prev->isLeaf) || (!post->isLeaf && !prev->isLeaf));
//                        if(post->isLeaf && prev->isLeaf){
//                            std::copy(post->keys_.begin(), post->keys_.end(), std::back_inserter(prev->keys_));
//                        }else{
                            prev->keys_.push_back(key);
                            std::copy(post->keys_.begin(), post->keys_.end(), std::back_inserter(prev->keys_));
                            std::copy(post->links_.begin(), post->links_.end(), std::back_inserter(prev->links_));
                            keys_.erase(remove(keys_.begin(), keys_.end(), key));
                            links_.erase(links_.begin() + idx + 1);
                            prev->remove(key);
                            return;
//                        }
                    }
                }
            }
        }else{
            if(isLeaf){
                //if this is leaf node and the node does not contain key, key is not in this tree.
                return;
            }
            //key does not exist in this node
            //find the right sub node that key is possibly in
            bool exists;
            int idx = index(key, exists);
            assert(!exists);
            auto& node = links_[idx];
            if(node->numOfKeys() >= btree_->minKeys() + 1){
                //node has at least minKeys() + 1 keys
                node->remove(key);
                return;
            }else{
                //if one key can be moved from node's previous sibling
                if(idx - 1 >= 0){
                    //node has previous node
                    auto& prev = links_[idx - 1];
                    if(prev->numOfKeys() >= btree_->minKeys() + 1){
                        //move one key from previous node to this node, and move keys_[idx] to 'node'
                        using std::swap;
                        node->keys_.insert(node->keys_.begin(), keys_[idx - 1]);
                        if(!prev->isLeaf){node->links_.insert(node->links_.begin(), *prev->links_.rbegin());}
                        swap(*prev->keys_.rbegin(), keys_[idx - 1]);
                        prev->keys_.pop_back();
                        if(!prev->isLeaf){
                            prev->links_.pop_back();
                        }
                        node->remove(key);
                        return;
                    }
                }
                //node has previous sibling but sibling has minKeys() key or node has no previous sibling
                if(idx + 1 < links_.size()){
                    //it has post sibling
                    auto& post = links_[idx + 1];
                    if(post->numOfKeys() >= btree_->minKeys() + 1){
                        //move one key from previous node to this node, and move keys_[idx] to 'node'
                        using std::swap;
                        node->keys_.push_back(keys_[idx]);
                        if(!node->isLeaf){node->links_.push_back(*post->links_.begin());}
                        swap(*post->keys_.begin(), keys_[idx]);
                        post->keys_.erase(post->keys_.begin());
                        if(!node->isLeaf){
                            post->links_.erase(post->links_.begin());
                        }
                        node->remove(key);
                        return;
                    }
                }

                //siblings has no more than minKeys key in them(or that)
                if(idx + 1 < links_.size()){
                    auto& post = links_[idx + 1];
                    node->keys_.push_back(keys_[idx]);
                    node->keys_.insert(node->keys_.begin(), post->keys_.begin(), post->keys_.end());
                    if(!node->isLeaf){
                        node->links_.insert(node->links_.end(), post->links_.begin(), post->links_.end());
                    }
                    keys_.erase(remove(keys_.begin(), keys_.end(), keys_[idx]));
                    links_.erase(links_.begin() + idx + 1);
                    if(btree_->getRoot() == this && numOfKeys() == 0){
                        btree_->setRoot(node);
                    }
                    node->remove(key);
                    return;
                }
                if(idx + 1 == links_.size()){
                    //this is last link of this node
                    assert(idx - 1 >= 0);//even root node has two links
                    auto& prev = links_[idx - 1];
                    prev->keys_.push_back(keys_[idx - 1]);
                    prev->keys_.insert(prev->keys_.begin(), node->keys_.begin(), node->keys_.end());
                    if(!node->isLeaf){
                        prev->links_.insert(prev->links_.begin(), node->links_.begin(), node->links_.end());
                    }
                    keys_.pop_back();
                    links_.pop_back();
                    if(btree_->getRoot() == this && numOfKeys() == 0){
                        btree_->setRoot(prev);
                    }
                    prev->remove(key);
                    return;
                }
            }
        }
    }

    bool BTreeNode::exists(BTreeNode::Key key) {
        size_t b = 0;
        size_t e = keys_.size();
        size_t mid = 0;
        while(b < e){
            mid = b + (e - b) / 2;
            if(keys_[mid] == key){
                return true;
            }else if(keys_[mid] < key){
                b = mid + 1;
            }else{
                e = mid;
            }
        }
        return false;
    }

    int BTreeNode::index(BTreeNode::Key key, bool& exists) {
        exists = false;
        size_t b = 0;
        size_t e = keys_.size();
        size_t mid = 0;
        while(b < e){
            mid = b + (e - b) / 2;
            if(keys_[mid] == key){
                exists = true;
                return static_cast<int>(mid);
            }else if(keys_[mid] < key){
                b = mid + 1;
            }else{
                e = mid;
            }
        }
        return static_cast<int>(b);
    }
}

