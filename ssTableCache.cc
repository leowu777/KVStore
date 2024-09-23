#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include "utils.h"
#include "ssTableCache.h"

std::vector<entryForMerge> ssTableCache::genSortedEntry(std::vector<uint64_t> &&keyArray, ssTable *table) {
    std::vector<entryForMerge> res;
    for (uint32_t i = 0; i < keyArray.size(); ++i) {
        res.emplace_back(keyArray[i], i, table);
    }
    return res;
}

void ssTableCache::mergesort(std::vector<entryForMerge> &array, uint64_t start, uint64_t end) {
    uint64_t mid = (start + end) >> 1;
    if (end <= start)  return;
    mergesort(array, start, mid);
    mergesort(array, mid + 1, end);
    uint64_t i = start, j = mid + 1, k = 0;
    std::vector<entryForMerge> tmpArray(end - start + 1);
    while (i <= mid && j <= end) {
        if (array[i].key > array[j].key)  tmpArray[k++] = array[j++];
        else  tmpArray[k++] = array[i++];
    }
    while (i <= mid)  tmpArray[k++] = array[i++];
    while (j <= end)  tmpArray[k++] = array[j++];
    for (i = start, k = 0; i <= end; ++i, ++k) {
        array[i] = tmpArray[k];
    }
}

void ssTableCache::compaction(uint32_t targetLevel, std::vector<ssTable *> &targetCells) {
    std::vector<entryForMerge> array;
    std::vector<entryForMerge> uniqueArray;
    for (auto &it: targetCells) {
        auto tmp = genSortedEntry(it->getKeySeq(), it);
        array.insert(array.cend(), tmp.cbegin(), tmp.cend());
    }
    mergesort(array, 0, array.size() - 1);
    uint32_t lastInd = 0;
    for (uint32_t i = 1; i < array.size(); ++i) {
        if (array[i].key != array[lastInd].key) {
            uniqueArray.emplace_back(array[lastInd]);
            lastInd = i;
        } else {
            if (array[i].table->header.timeStamp > array[lastInd].table->header.timeStamp) {
                lastInd = i;
            }
        }
    }
    uniqueArray.emplace_back(array[lastInd]);
    array.clear();
    std::vector<std::pair<uint64_t, std::string>> tmp;
    const uint32_t MAX_SIZE = 2 * 1024 * 1024 - sstableHeader::HEADER_SIZE - bloomFilter::FILTER_SIZE;
    uint64_t tmpTimeStamp = 0;
    uint32_t tmpSize = 0;
    bool needHandleDeleteFlag = targetLevel >= levelArray.size() - 1;
    for (auto &it: uniqueArray) 
    {
        std::string val = it.table->getValInd(it.ind);
        if (needHandleDeleteFlag && val == DELETE_FLAG) 
        {
            continue;
        }
        if (tmpSize + val.size() + sizeof (uint64_t) + sizeof (uint32_t) > MAX_SIZE) 
        {
            std::string targetDir = filePath + "/level-" + std::to_string(targetLevel);
            // 文件不存在
            if (!utils::dirExists(targetDir)) 
            {
                // 创建文件
                if (utils::mkdir(targetDir.c_str())) 
                {
                    std::cerr << "ERROR: Fail to create directory." << std::endl;
                    exit(1);
                }

                levelArray.emplace_back();
                nextLabel.push_back(0);
            }
            std::string fileName = targetDir + '/' + std::to_string(nextLabel[targetLevel]) + ".sst";
            levelArray[targetLevel].emplace_back(ssTable::genTable(fileName, tmpTimeStamp, tmp));
            ++nextLabel[targetLevel];
            tmp.clear();
            tmpTimeStamp = 0;
            tmpSize = 0;
        }
        tmp.emplace_back(it.key, val);
        tmpSize += val.size() + sizeof (uint64_t) + sizeof (uint32_t);
        if (it.table->header.timeStamp > tmpTimeStamp) 
        {
            tmpTimeStamp = it.table->header.timeStamp;
        }
    }
    if (!tmp.empty()) {
        std::string targetDir = filePath + "/level-" + std::to_string(targetLevel);
        if (!utils::dirExists(targetDir)) {
            if (utils::mkdir(targetDir.c_str())) {
                std::cerr << "ERROR: Fail to create directory." << std::endl;
                exit(1);
            }
            levelArray.emplace_back();
            nextLabel.push_back(0);
        }
        std::string fileName = targetDir + '/' + std::to_string(nextLabel[targetLevel]) + ".sst";
        levelArray[targetLevel].emplace_back(ssTable::genTable(fileName, tmpTimeStamp, tmp));
        ++nextLabel[targetLevel];
        tmp.clear();
    }
    for (auto &it: targetCells) {
        it->clearTable();
        delete it;
        it = nullptr;
    }
}

void ssTableCache::modify() {
    uint32_t level = 0;
    while (level < levelArray.size() && levelArray[level].size() > (0b10 << level)) {
        std::vector<ssTable *> targetCells;
        uint64_t minKey;
        uint64_t maxKey;
        if (level == 0) {
            minKey = levelArray[0][0]->header.minKey;
            maxKey = levelArray[0][0]->header.maxKey;
            for (auto &it: levelArray[0]) {
                targetCells.emplace_back(it);
                minKey = std::min(minKey, it->header.minKey);
                maxKey = std::max(maxKey, it->header.maxKey);
            }
            levelArray[0].clear();
        } else {
            std::deque<ssTable *> &tmpCells = levelArray[level];
            std::priority_queue<entryForCmp, std::vector<entryForCmp>, std::greater<>> q;
            for (uint32_t i = 0; i < tmpCells.size(); ++i) {
                q.emplace(tmpCells[i]->header.timeStamp, tmpCells[i]->header.minKey, tmpCells[i]->header.maxKey, i, tmpCells[i]);
            }
            minKey = q.top().minKey;
            maxKey = q.top().maxKey;
            std::vector<uint32_t> deleteList;
            uint32_t n = tmpCells.size() - (0b10 << level);
            while (n--) {
                targetCells.emplace_back(q.top().table);
                deleteList.emplace_back(q.top().index);
                minKey = std::min(minKey, q.top().minKey);
                maxKey = std::max(maxKey, q.top().maxKey);
                q.pop();
            }
            std::sort(deleteList.begin(), deleteList.end());
            auto iter = tmpCells.begin();
            uint32_t iterTimes = deleteList[0];
            while (iterTimes--)  ++iter;
            iter = tmpCells.erase(iter);
            for (uint32_t i = 1; i < deleteList.size(); ++i) {
                iterTimes = deleteList[i] - deleteList[i - 1] - 1;
                while (iterTimes--)  ++iter;
                iter = tmpCells.erase(iter);
            }
        }
        if (level + 1 < levelArray.size()) {
            std::deque<ssTable *> &tmpCells = levelArray[level + 1];
            auto iter = tmpCells.begin();
            while (iter != tmpCells.end()) {
                if (minKey <= (*iter)->header.maxKey && maxKey >= (*iter)->header.minKey) {
                    targetCells.emplace_back(*iter);
                    iter = tmpCells.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
        compaction(level + 1, targetCells);
        ++level;
    }
}

ssTableCache::ssTableCache(std::string f): filePath(std::move(f)){
    if (!utils::dirExists(filePath) && utils::mkdir(filePath.c_str())) {
        std::cerr << "ERROR: Fail to create directory." << std::endl;
        exit(1);
    }
    uint64_t level = 0;
    std::string subDir;
    nextTimeStamp = 1;
    while (utils::dirExists(subDir = filePath + "/level-" + std::to_string(level))) {
        uint64_t fileNum;
        uint64_t nextFileNum = 0;
        std::vector<std::string> ret;
        utils::scanDir(subDir, ret);
        levelArray.emplace_back();
        std::deque<ssTable *> &ssTableArray = levelArray[levelArray.size() - 1];
        for (const auto &it: ret) {
            std::string subFile = subDir;
            subFile += "/" + it;
            ssTableArray.emplace_back(new ssTable);
            ssTableArray[ssTableArray.size() - 1]->read(subFile);
            uint64_t tmp = ssTableArray[ssTableArray.size() - 1]->header.timeStamp;
            if (tmp >= nextTimeStamp)  nextTimeStamp = tmp + 1;
            fileNum = stoul(it.substr(0, it.find('.')));
            if (nextFileNum <= fileNum) {
                nextFileNum = fileNum + 1;
            }
        }
        nextLabel.push_back(nextFileNum);
        ++level;
    }
    if (levelArray.empty()) {
        if ( utils::mkdir((filePath + "/level-0").c_str()) ) {
            std::cerr << "ERROR: Fail to create directory." << std::endl;
            exit(1);
        }
        levelArray.emplace_back();
        nextLabel.push_back(0);
    }
}

ssTableCache::~ssTableCache() {
    for (auto & it : levelArray) {
        for (auto & item : it) {
            delete item;
            item = nullptr;
        }
    }
}

void ssTableCache::addTable(const std::vector<std::pair<uint64_t, std::string>> &kVarray) {
    std::string fileName = filePath + "/level-0/" + std::to_string(nextLabel[0]) + ".sst";
    levelArray[0].emplace_back(ssTable::genTable(fileName, nextTimeStamp, kVarray));
    ++nextTimeStamp;
    ++nextLabel[0];
    modify();
}

std::pair<bool, std::string> ssTableCache::getValKey(uint64_t key) const {
    std::pair<bool, std::string> res(false, "");
    uint64_t maxTimeStamp;
    if (!levelArray.empty()) {
        for (const auto &it: levelArray[0]) {
            std::pair<bool, std::string> test = it->getValKey(key);
            if (test.first) {
                if (res.first) {
                    if (maxTimeStamp < it->header.timeStamp) {
                        maxTimeStamp = it->header.timeStamp;
                        res.second = test.second;
                    }
                } else {
                    maxTimeStamp = it->header.timeStamp;
                    res.first = true;
                    res.second = test.second;
                }
            }
        }
        uint64_t i = 1;
        while (i < levelArray.size()) {
            for (const auto &it: levelArray[i]) {
                std::pair<bool, std::string> test = it->getValKey(key);
                if (test.first) {
                    if (res.first) {
                        if (maxTimeStamp < it->header.timeStamp) {
                            maxTimeStamp = it->header.timeStamp;
                            res.second = test.second;
                        }
                    } else {
                        maxTimeStamp = it->header.timeStamp;
                        res.first = true;
                        res.second = test.second;
                    }
                    break;
                }
            }
            ++i;
        }
    }

    return res;
}

void ssTableCache::reset() {
    for (auto & it : levelArray) {
        for (auto & item : it) {
            item->clearTable();
            delete item;
            item = nullptr;
        }
        it.clear();
    }
    for (uint64_t i = 0; i < levelArray.size(); ++i) {
        if( utils::rmdir((filePath + "/level-" + std::to_string(i)).c_str()) ) {
            std::cerr << "ERROR: Fail to delete directory." << std::endl;
            exit(1);
        }
    }
    levelArray.clear();
    nextLabel.clear();
    if ( utils::mkdir((filePath + "/level-0").c_str()) ) {
        std::cerr << "ERROR: Fail to create directory." << std::endl;
        exit(1);
    }
    levelArray.emplace_back();
    nextLabel.push_back(0);
    nextTimeStamp = 1;
}
