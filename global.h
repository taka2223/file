//
// Created by 94348 on 2022/5/17.
//

#ifndef FILESTREAM_GLOBAL_H
#define FILESTREAM_GLOBAL_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>

using namespace std;
extern const std::string DISK_NAME;
extern const int BLOCK_SIZE;
extern const int BLOCK_NUM;
extern const int INODE_SIZE; //单个inode最大大小，不是固定的大小
extern const int INODE_NUM;  // inode最大数量，即文件最大数量
extern const int MAX_FILE_SIZE;//单个文件最大空间，以KB为单位
extern const int SUPER_BLOCK_ADD;
extern const int INODE_BITMAP_ADD; // indoe bitmap的开始地址，bitmap大小为1个block,可以监控1024个inode的状态
extern const int BLOCK_BITMAP_ADD; // block bitmap的开始地址，bitmap大小为20个block，可以监控20480个block的状态
extern const int INODE_BLOCK_ADD;  // inode的开始地址
extern const int BLOCK_ADD;        // block的开始

extern const int BLOCKS_PER_GROUP; //成组链接法中每组的block数量
extern std::vector<char> inodeBitMap;
extern std::vector<char> blockBitMap;
extern std::fstream f;
struct SuperBlock {
    int freeInodeNum = 0;
    int freeBlockNum = 0;
    int freeAddr = 0;
    int free[128]{};
};
struct Inode {
    int id;
    int size;//文件大小
    int dirBlock[10];//直接块，10个
    int indirBlock;//间接块，1个（1个块可以存1024/4个块的地址，因此1个间接块可以存大小为1024/4 * 1024B的数据）
    int cnt;//若为目录，该目录下有cnt个子目录或文件
    bool isFile;
    time_t ctime;//inode上次改动的时间（创建）
    time_t mtime;//文件内容上一次修改的时间（略？）
    time_t atime;//上次打开的时间（略？）
};
struct DirItem{
    //28+4 B
    char name[28]="";
    int inodeAddr=-1;
    // 用于排序的比较函数
    bool operator<(const DirItem& item) const;
    bool operator>(const DirItem& item) const;
    bool operator==(const DirItem& item) const;
};
extern SuperBlock superBlock;
extern int curAddr;//当前目录地址
extern string curName;//当前目录名
#endif //FILESTREAM_GLOBAL_H
