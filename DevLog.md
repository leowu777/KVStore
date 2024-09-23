# DevLog for KVStore

## June.10
- 正在进行: 不带缓存和compaction的sstable
- todo: 
    - sstable完整版(索引区等)
    - compaction
    - 删去一些不必要的边界判断(读文件相关)

## June.9 ##
- 已完成: 给skiplist添加deleted特性,并添加size信息
    - 具体: 修改put和remove的返回值,添加put和remove中关于size的内容
- bloomfilter完成, makefile修改完毕
- todo:
    - sstable的索引区
    - compaction

## June.8 ##
- 已实现sstableHeader类的读写
    - 遇到过的问题: 
    - 二进制读写的写法
    - makefile的修改
    - 初始化列表的位置(是写在头文件还是实现文件里?)
- 仅实现skiplist部分
- todo:
    - 完整的memtable(?)
    - sstable的header
    - sstable的bloomfilter
    - sstable的索引区
    - compaction
