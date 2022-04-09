#include <vector>
#include <time.h>
#include <string>
#include <iostream>
#include "global.h"
class file
{
private:
    std::string fileName;
    Inode metadata;
    int balloc();
    bool balloc();

public:
    file(std::string name, int size);
    ~file();
};

file::file(std::string name, int size, SuperBlock &superBlock, std::vector<bool> blockBitMap) : fileName(name)
{
    time(&metadata.createT);
    time(&metadata.modifyT);
    if (superBlock.freeBlockCount < size)
    {
        std::cerr << "No enough free blocks" << std::endl;
        metadata.id = -1;
    }
    else
        while (size > 0)
        {
            int curr = 0;
            for (curr; curr < blockBitMap.size(); curr++)
            {
                if (!blockBitMap[curr])
                {
                    /* code */
                }
            }
        }
}

file::~file()
{
}
