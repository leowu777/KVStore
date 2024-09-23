#include "skiplist.h"
#include <random>
#include <chrono>
Skiplist::Skiplist() {
	curSize = 0;
	head = new Node;
}

Skiplist::~Skiplist() {
	Node *headPtr = head;
    while(headPtr) {
        Node *ptr = headPtr;
        headPtr = headPtr->down;
        while(ptr) {
            Node *tmp = ptr;
            ptr = ptr->right;
            delete tmp;
        }
    }
}

void Skiplist::clear() {
	Node *headPtr = head;
    while(headPtr) {
        Node *ptr = headPtr;
        headPtr = headPtr->down;
        while(ptr) {
            Node *tmp = ptr;
            ptr = ptr->right;
            delete tmp;
        }
    }
    curSize = 0;
    head = new Node;
}

std::string* Skiplist::get(const uint64_t& key) {
	Node *p = head;
    while(p) {
        while(p->right && p->right->key < key) {
            p = p->right;
        }
        if(p->right && p->right->key == key) {
            p = p->right;
            std::string* res = &(p->val);
            return res;
        } else {
            p = p->down;
        }
    }
    return nullptr;
}

bool Skiplist::put(const uint64_t& key, const std::string& s) {
    std::vector<Node*> leftPath;    //从上至下记录搜索路径
    Node *p = head;
    while(p){
        while(p->right && p->right->key < key){ 
            p = p->right;
        }
        leftPath.push_back(p);
        p = p->down;
    }
    if (leftPath.back()->right && leftPath.back()->right->key == key) {
        uint64_t oldSize = curSize;
        curSize = curSize + s.length() * sizeof (char) - leftPath.back()->val.length() * sizeof (char);
        if (curSize > MEM_SIZE) {
            curSize = oldSize;
            return false;
        }
        // replace old val
        while (!leftPath.empty()) {
            Node *p = leftPath.back()->right;
            if (!p || p->key != key) {
                break;
            }
            p->val = s;
            leftPath.pop_back();
        }

    } else {
        // try add size
        uint64_t oldSize = curSize;
        curSize = curSize + s.length() * sizeof (char) + sizeof (uint64_t) + sizeof (uint32_t);
        if (curSize > MEM_SIZE) {
            curSize = oldSize;
            return false;
        }

        // add new val
        Node *lastNode = nullptr;
        static std::default_random_engine e(std::chrono::steady_clock::now().time_since_epoch().count());
        static std::uniform_int_distribution<uint8_t> u(0, 1);
        bool needAdd = true;

        while (!leftPath.empty() && needAdd) {
            Node *p = leftPath.back();
            p->right = new Node (p->right, lastNode, key, s);

            lastNode = p->right;
            needAdd = u(e);
            leftPath.pop_back();
        }
        if (leftPath.empty() && needAdd) {
            head = new Node (new Node (nullptr, lastNode, key, s), head, 0, std::string());
        }
    }

    return true;
}

std::pair<bool, std::string> Skiplist::remove(const uint64_t& key) {
    std::vector<Node*> pathList;    //从上至下记录搜索路径
    Node *p = head;
    while(p){
        while(p->right && p->right->key < key){ 
            p = p->right;
        }
        pathList.push_back(p);
        p = p->down;
    }
    std::pair<bool, std::string> res = std::pair<bool, std::string>(0, "");
    if(pathList.back()->right && pathList.back()->right->key == key) {
        curSize -= (pathList.back()->right->val.length() * sizeof(char) + 12);
        res.first = 1;
        res.second = pathList.back()->right->val;
        while(!pathList.empty()) {
            Node *p = pathList.back();
            Node *tobedelete = p->right;
            if(!tobedelete || tobedelete->key != key) {
                break;
            }
            p->right = tobedelete->right;
            delete tobedelete;
            pathList.pop_back();
        }
    }
    return res;
}

bool Skiplist::empty() const {
    return curSize == 0;
}

std::vector<std::pair<uint64_t, std::string>> Skiplist::getKVSeq() {
    std::vector<std::pair<uint64_t, std::string>> res;
    if(curSize == 0) return res;
    Node *p = head;
    while(p->down) p = p->down;
    while(p->right) {
        p = p->right;
        res.emplace_back(p->key, p->val);
    }
    return res;
}
