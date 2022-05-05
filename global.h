#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
extern const std::string DISK_NAME;
extern const int BLOCK_SIZE;
extern const int BLOCK_NUM;
extern const int INODE_SIZE; //单个inode最大大小，不是固定的大小
extern const int INODE_NUM;  // inode最大数量，即文件最大数量

extern const int SUPER_BLOCK_ADD;
extern const int INODE_BITMAP_ADD; // indoe bitmap的开始地址，bitmap大小为1个block,可以监控1024个inode的状态
extern const int BLOCK_BITMAP_ADD; // block bitmap的开始地址，bitmap大小为20个block，可以监控20480个block的状态
extern const int INODE_BLOCK_ADD;  // inode的开始地址
extern const int BLOCK_ADD;        // block的开始

extern const int BLOCKS_PER_GROUP; //成组链接法中每组的block数量
extern std::vector<char> inodeBitMap;
extern std::vector<char> blockBitMap;
extern std::ifstream fr;
extern std::ofstream fw;
ifstream &operator>>(ifstream &in, vector<char> &bitmap);
ofstream &operator<<(ofstream &out, vector<char> &bitmap);