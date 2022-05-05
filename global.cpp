#include "global.h"
const std::string DISK_NAME = "filesystem.txt";
const int BLOCK_SIZE = 1024;
const int BLOCK_NUM = 16384;
const int INODE_SIZE = 128; //单个inode最大大小，不是固定的大小
const int INODE_NUM = 1024; // inode最大数量，即文件最大数量

const int SUPER_BLOCK_ADD = 0;
const int INODE_BITMAP_ADD = SUPER_BLOCK_ADD + 1 * BLOCK_SIZE;                              // indoe bitmap的开始地址，bitmap大小为1个block,可以监控1024个inode的状态
const int BLOCK_BITMAP_ADD = INODE_BITMAP_ADD + 1 * BLOCK_SIZE;                             // block bitmap的开始地址，bitmap大小为20个block，可以监控20480个block的状态
const int INODE_BLOCK_ADD = BLOCK_BITMAP_ADD + 20 * BLOCK_SIZE;                             // inode的开始地址
const int BLOCK_ADD = INODE_BLOCK_ADD + INODE_NUM / (BLOCK_SIZE / INODE_SIZE) * BLOCK_SIZE; // block的开始地址
const int BLOCKS_PER_GROUP = 128;
std::vector<char> inodeBitMap(INODE_NUM, 0);
std::vector<char> blockBitMap(BLOCK_NUM, 0);
std::ifstream fr;
std::ofstream fw;
ifstream &operator>>(ifstream &in, vector<char> &bitmap)
{
    in.read((char *)bitmap.data(), bitmap.size() * sizeof(char));
    return in;
}

ofstream &operator<<(ofstream &out, vector<char> &bitmap)
{
    out.write((char *)bitmap.data(), bitmap.size() * sizeof(char));
    return out;
}
