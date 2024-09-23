#include <iostream>
#include "kvstore.h"

KVStore::KVStore(const std::string &dir): KVStoreAPI(dir), ssTableSeq(dir) {}

KVStore::~KVStore() {
    if (!memTable.empty()) {
        std::vector<std::pair<uint64_t, std::string>> tmp = memTable.getKVSeq();
        ssTableSeq.addTable(tmp);
        memTable.clear();
    }
}

void KVStore::put(uint64_t key, const std::string &val) {
    if (!memTable.put(key, val)) {
        std::vector<std::pair<uint64_t, std::string>> tmp = memTable.getKVSeq();
        ssTableSeq.addTable(tmp);
        memTable.clear();
        memTable.put(key, val);
    }
}

std::string KVStore::get(uint64_t key)
{
    std::string *testGet = memTable.get(key);
    if (testGet) {
        if (*testGet == ssTableCache::DELETE_FLAG) {
            return "";
        }
        return *testGet;
    }
    std::pair<bool, std::string> tmp = ssTableSeq.getValKey(key);
    if (tmp.first && tmp.second != ssTableCache::DELETE_FLAG) {
        return tmp.second;
    }
	return "";
}

bool KVStore::del(uint64_t key) {
    std::pair<bool, std::string> tmp = memTable.remove(key);
    put(key, ssTableCache::DELETE_FLAG);
    if (tmp.first) {
        if (tmp.second == ssTableCache::DELETE_FLAG) {
            return false;
        }
        return true;
    }
    tmp = ssTableSeq.getValKey(key);
    if (tmp.first && tmp.second == ssTableCache::DELETE_FLAG) {
        return false;
    }

	return true;
}

void KVStore::reset() {
    memTable.clear();
    ssTableSeq.reset();
}
