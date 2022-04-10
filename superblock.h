#include "global.h"
using namespace std;
class SuperBlock
{
private:
    /* data */
public:
    const unsigned short s_INODE_NUM; // inode节点数
    const unsigned int s_BLOCK_NUM;   //磁盘块块数

    unsigned short s_free_INODE_NUM;           //空闲inode节点数
    unsigned int s_free_BLOCK_NUM;             //空闲磁盘块数
    int s_free_addr;                           //空闲块堆栈指针
    vector<int> s_free(s_blocks_per_group, 0); //空闲块堆栈

    const int s_BLOCK_SIZE;       //磁盘块大小
    const int s_INODE_SIZE;       // inode大小
    const int s_SUPERBLOCK_SIZE;  //超级块大小
    const int s_blocks_per_group; //每 blockgroup 的block数量

    //磁盘分布
    const int s_Superblock_StartAddr;
    const int s_InodeBitmap_StartAddr;
    const int s_BlockBitmap_StartAddr;
    const int s_Inode_StartAddr;
    const int s_Block_StartAddr;
    SuperBlock(/* args */);
    ~SuperBlock();
};

SuperBlock::SuperBlock(/* args */)
{
    s_INODE_NUM = INODE_NUM;
    s_BLOCK_NUM = BLOCK_NUM;

    s_free_BLOCK_NUM = s_BLOCK_NUM;
    s_free_INODE_NUM = s_INODE_NUM;

    s_BLOCK_SIZE = BLOCK_SIZE;
    s_INODE_SIZE = INODE_SIZE;
    s_SUPERBLOCK_SIZE = sizeof(SuperBlock);
    s_blocks_per_group = BLOCKS_PER_GROUP;

    s_Superblock_StartAddr = SUPER_BLOCK_ADD;
    s_InodeBitmap_StartAddr = INODE_BITMAP_ADD;
    s_BlockBitmap_StartAddr = BLOCK_BITMAP_ADD;
    s_Inode_StartAddr = INODE_BLOCK_ADD;
    s_Block_StartAddr = BLOCK_ADD;

    s_free_addr = BLOCK_ADD;
    //都是2的幂，不需要考虑小数点的问题,初始化block并将其写入文件（磁盘）
    for (size_t i = (s_INODE_NUM / BLOCKS_PER_GROUP) - 1; i >= 0; i--)
    {
        if (i == (s_INODE_NUM / BLOCKS_PER_GROUP) - 1)
        {
            //最后一组
            s_free[0] = -1;
        }
        else
        {
            //下一组的初始
            s_free[0] = s_Block_StartAddr + (i + 1) * s_BLOCK_SIZE * BLOCKS_PER_GROUP;
        }

        for (size_t j = 1; j < s_blocks_per_group; j++)
        {
            s_free[j] = s_free[j - 1] + s_BLOCK_SIZE;
        }
        fw.seekp(s_Block_StartAddr + i * s_BLOCK_SIZE * BLOCKS_PER_GROUP, ios::beg);
        fw.write((char *)&s_free, sizeof(s_free));
    }
    fw.seekp(s_InodeBitmap_StartAddr, ios::beg);
    fw.write((char *)&inodeBitMap, sizeof(inodeBitMap));

    fw.seekp(s_BlockBitmap_StartAddr, ios::beg);
    fw.write((char *)&blockBitMap, sizeof(blockBitMap));

    fw.seekp(s_Superblock_StartAddr, ios::beg);
    fw.write((char *)this, ios::beg);
}

SuperBlock::~SuperBlock()
{
}
