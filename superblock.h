#include "global.h"
using namespace std;
class SuperBlock
{
private:
    /* data */
public:
    const int s_INODE_NUM = INODE_NUM; // inode节点数
    const int s_BLOCK_NUM = BLOCK_NUM; //磁盘块块数

    int s_free_INODE_NUM; //空闲inode节点数
    int s_free_BLOCK_NUM; //空闲磁盘块数
    int s_free_addr;      //空闲块堆栈指针

    const int s_BLOCK_SIZE = BLOCK_SIZE;              //磁盘块大小
    const int s_INODE_SIZE = INODE_SIZE;              // inode大小
    const int s_SUPERBLOCK_SIZE = sizeof(SuperBlock); //超级块大小
    const int s_blocks_per_group = BLOCKS_PER_GROUP;  //每 blockgroup 的block数量

    //磁盘分布
    const int s_Superblock_StartAddr = SUPER_BLOCK_ADD;
    const int s_InodeBitmap_StartAddr = INODE_BITMAP_ADD;
    const int s_BlockBitmap_StartAddr = BLOCK_BITMAP_ADD;
    const int s_Inode_StartAddr = INODE_BLOCK_ADD;
    const int s_Block_StartAddr = BLOCK_ADD;
    int s_free[128]; //空闲块堆栈
    friend ofstream &operator<<(ofstream &, SuperBlock &);
    friend ifstream &operator>>(ifstream &, SuperBlock &);
    void printInfo();
    bool init();
    SuperBlock(/* args */);
    ~SuperBlock();
};

SuperBlock::SuperBlock(/* args */)
{
}

SuperBlock::~SuperBlock()
{
}

void SuperBlock::printInfo()
{
    cout << s_BLOCK_NUM << endl;
    cout << s_INODE_NUM << endl;
    cout << s_BLOCK_SIZE << endl;
    cout << s_INODE_SIZE << endl;
    cout << s_free_INODE_NUM << endl;
    cout << s_free_BLOCK_NUM << endl;
    cout << s_SUPERBLOCK_SIZE << endl;
    int used = (s_BLOCK_NUM - s_free_BLOCK_NUM) * s_BLOCK_SIZE + (s_INODE_NUM - s_free_INODE_NUM) * s_INODE_SIZE + s_SUPERBLOCK_SIZE + sizeof(inodeBitMap) + sizeof(blockBitMap);
    int total = s_BLOCK_NUM * s_BLOCK_SIZE + s_INODE_NUM * s_INODE_SIZE + s_SUPERBLOCK_SIZE + sizeof(inodeBitMap) + sizeof(blockBitMap);
    cout << "Disk Space Usage: " << used << "B / " << total << "B" << endl;
}

inline bool SuperBlock::init()
{
    cout << s_BLOCK_NUM << endl;
    cout << BLOCKS_PER_GROUP << endl;
    cout << (s_BLOCK_NUM / BLOCKS_PER_GROUP) << endl;
    s_free_addr = BLOCK_ADD;
    s_free_BLOCK_NUM = s_BLOCK_NUM;
    s_free_INODE_NUM = s_INODE_NUM;
    //都是2的幂，不需要考虑小数点的问题,初始化block并将其写入文件（磁盘）
    for (int i = (s_BLOCK_NUM / BLOCKS_PER_GROUP) - 1; i >= 0; i--)
    {
        cout << i << endl;
        if (i == (s_BLOCK_NUM / BLOCKS_PER_GROUP) - 1)
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
            s_free[j] = s_Block_StartAddr + (i * s_blocks_per_group + j) * s_BLOCK_SIZE;
        }
        fw.seekp(s_Block_StartAddr + i * s_BLOCK_SIZE * BLOCKS_PER_GROUP, ios::beg);
        fw.write((char *)&s_free, sizeof(s_free));
    }
    cout << "writing inode bitmap" << endl;
    fw.seekp(s_InodeBitmap_StartAddr, ios::beg);
    fw << inodeBitMap;

    cout << "writing block bitmap" << endl;
    fw.seekp(s_BlockBitmap_StartAddr, ios::beg);
    fw << blockBitMap;

    cout << s_free_addr << endl;
    return true;
}

ofstream &operator<<(ofstream &out, SuperBlock &superBlock)
{
    out.seekp(superBlock.s_Superblock_StartAddr, ios::beg);
    out << superBlock.s_free_addr << ' ' << superBlock.s_free_BLOCK_NUM << ' ' << superBlock.s_free_INODE_NUM << ' ';
    cout << superBlock.s_free_addr << ' ' << superBlock.s_free_BLOCK_NUM << ' ' << superBlock.s_free_INODE_NUM << ' ' << endl;
    out.seekp(superBlock.s_Block_StartAddr, ios::beg);
    out.write((char *)&superBlock.s_free, sizeof(superBlock.s_free));
    return out;
}

ifstream &operator>>(ifstream &in, SuperBlock &superBlock)
{
    in.seekg(superBlock.s_Superblock_StartAddr, ios::beg);
    in >> superBlock.s_free_addr >> superBlock.s_free_BLOCK_NUM >> superBlock.s_free_INODE_NUM;
    cout << superBlock.s_free_addr << ' ' << superBlock.s_free_BLOCK_NUM << ' ' << superBlock.s_free_INODE_NUM << ' ' << endl;
    in.seekg(superBlock.s_Block_StartAddr, ios::beg);
    in.read((char *)&superBlock.s_free, sizeof(superBlock.s_free));
    return in;
}