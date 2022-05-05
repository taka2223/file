#include "superblock.h"
#include <iostream>
using namespace std;

int balloc(SuperBlock &superBlock)
{
    if (superBlock.s_free_BLOCK_NUM <= 0)
    {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    int top = (superBlock.s_free_BLOCK_NUM - 1) % superBlock.s_blocks_per_group;
    int blockAdd;
    if (superBlock.s_free[top] == -1)
    {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    if (top == 0)
    {
        //栈底
        //更新s_free
        blockAdd = superBlock.s_free[top];
        superBlock.s_free_addr = superBlock.s_free[top];
        fr.seekg(superBlock.s_free_addr, ios::beg);
        fr.read((char *)&superBlock.s_free, sizeof(superBlock.s_free));
    }
    else
    {
        blockAdd = superBlock.s_free[top];
        superBlock.s_free[top] = -1;
    }
    superBlock.s_free_BLOCK_NUM--;

    //更新superblock
    fw << superBlock;

    //更新bitmap
    int i = (blockAdd - superBlock.s_Block_StartAddr) / superBlock.s_BLOCK_SIZE;
    cout << "Block " << i << " is allocated" << endl;
    blockBitMap[i] = (char)1;
    fw.seekp(superBlock.s_BlockBitmap_StartAddr, ios::beg);
    fw << blockBitMap;

    return blockAdd;
}

bool bfree(int add, SuperBlock &superBlock)
{
    //是否为某个块的起始地址
    if ((add - superBlock.s_Block_StartAddr) % superBlock.s_BLOCK_SIZE != 0)
    {
        cout << "Invalid Block Address" << endl;
        return false;
    }
    if (superBlock.s_free_BLOCK_NUM == superBlock.s_BLOCK_NUM)
    {
        cout << "No blocks need to be free" << endl;
        return false;
    }
    int order = (add - superBlock.s_Block_StartAddr) / superBlock.s_BLOCK_SIZE;
    if ((int)blockBitMap[order] == 0)
    {
        cout << "The block is not used" << endl;
        return false;
    }

    //填充0
    vector<char> pad(superBlock.s_BLOCK_SIZE, 0);
    fw.seekp(add, ios::beg);
    fw.write((char *)pad.data(), pad.size() * sizeof(char));
    int top = (superBlock.s_free_BLOCK_NUM - 1) % superBlock.s_blocks_per_group;
    if (top == superBlock.s_blocks_per_group - 1)
    {
        //将空闲块堆栈内容写入新释放的块
        fw.seekp(add, ios::beg);
        fw.write((char *)&superBlock.s_free, sizeof(superBlock.s_free));
        //更行空闲块堆栈
        superBlock.s_free[0] = add;
        for (size_t i = 1; i < 128; i++)
        {
            superBlock.s_free[i] = -1;
        }
    }
    else
    {
        superBlock.s_free[++top] = add;
    }

    // 更新超级块和bitmap
    fw.seekp(superBlock.s_Superblock_StartAddr, ios::beg);
    fw << superBlock;

    fw.seekp(superBlock.s_BlockBitmap_StartAddr, ios::beg);
    fw << blockBitMap;

    return true;
}

void BlockInfo(const SuperBlock &superBlock)
{
    for (size_t i = 0; i < blockBitMap.size(); i++)
    {
        if (i % superBlock.s_blocks_per_group == 0)
            cout << endl;
        if (blockBitMap[i] == 1)
        {
            cout << "*";
        }
        else
        {
            cout << "-";
        }
    }
}