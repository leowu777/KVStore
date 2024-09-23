#pragma once
#include <vector>
#include <string>

struct Node{
    Node *right,*down;   //向右向下足矣
    uint64_t key;
    std::string val;
    Node(Node *right,Node *down, uint64_t key, std::string val): right(right), down(down), key(key), val(val){}
    Node(): right(nullptr), down(nullptr) {}
};

class Skiplist {
public:
    const static uint32_t MEM_SIZE = 2086880;
    uint32_t curSize;
	Node *head;
	Skiplist();
	~Skiplist();
    void clear();
    std::string* get(const uint64_t& key);
    bool empty() const;
    bool put(const uint64_t& key, const std::string& val);
    std::pair<bool, std::string> remove(const uint64_t& key);
    std::vector<std::pair<uint64_t, std::string>> getKVSeq();
};