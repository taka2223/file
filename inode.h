#include <time.h>
#include <vector>
#include "global.h"
#include <iostream>
using namespace std;
class Inode
{
private:
    int id;                           // 4B
    std::vector<int> blockAdd(10, 0); // 30B
    int indirectAdd;                  //根据要求地址长24bits, 3B
    time_t createT;                   // 8B
    time_t modifyT;                   // 8B
    int currentSize;

public:
    Inode(/* args */);
    ~Inode();
    time_t getCreateT() const { return createT; }
    time_t getModifyT() const { return modifyT; }
    int getCurrentSize() const { return currentSize; }
    int getIndirectAdd() const { return indirectAdd; }
    std::vector<int> getBlockAdd() const { return blockAdd; }
    int getId() const { return id; }
    int getAdd() const { return INODE_BLOCK_ADD + id * INODE_SIZE; }
};

Inode::Inode(/* args */)
{
    if (superBlock.freeInodeCount <= 0)
    {
        cout << "No free inodes available" << endl;
    }
    else
    {
        int i;
        for (i = 0; i < superBlock.inodeCount; i++)
        {
            if (!inodeBitMap[i])
            {
                /* code */
            }
        }
    }
}

Inode::~Inode()
{
}
