//
// Created by 94348 on 2022/5/20.
//
#include "func.h"

bool mkroot()
{
    Inode root{};
    DirItem itemList[32] = {0}; //一个block可以储存32个DirItem
    int inodeAddr = ialloc();
    cout << "inodeAddr: " << inodeAddr << endl;
    int blockAddr = balloc();
    cout << "blockAddr:" << blockAddr << endl;
    if (inodeAddr == -1 || blockAddr == -1)
    {
        cout << "Allocation error" << endl;
        return false;
    }
    strcpy(itemList[0].name, ".");
    strcpy(itemList[1].name, "..");
    itemList[0].inodeAddr = inodeAddr;
    itemList[1].inodeAddr = inodeAddr;
    f.seekp(blockAddr, ios::beg);
    f.write((char *)itemList, sizeof(itemList));

    root.isFile = false;
    root.id = 0;
    time(&root.ctime);
    time(&root.atime);
    time(&root.mtime);
    root.cnt = 2; //应该是1还是2？
    root.isFile = false;
    root.dirBlock[0] = blockAddr;
    root.indirBlock = -1;
    for (int i = 1; i < 10; ++i)
    {
        root.dirBlock[i] = -1;
    }
    root.size = BLOCK_SIZE;
    f.seekp(inodeAddr, ios::beg);
    f.write((char *)&root, sizeof(root));
    return true;
}

int ialloc()
{
    if (superBlock.freeInodeNum <= 0)
    {
        cout << "No free Inodes" << endl;
        return -1;
    }
    else
    {
        int i;
        for (i = 0; i < INODE_NUM; ++i)
        {
            if (inodeBitMap[i] == 0)
                break; //空闲inode
        }
        superBlock.freeInodeNum--;
        f.seekp(SUPER_BLOCK_ADD, ios::beg);
        f.write((char *)&superBlock, sizeof(superBlock));

        inodeBitMap[i] = (char)1;
        f.seekp(INODE_BITMAP_ADD, ios::beg);
        f.write((char *)inodeBitMap.data(), sizeof(char) * inodeBitMap.size());
        return INODE_BLOCK_ADD + i * INODE_SIZE;
    }
}

bool ifree(int addr)
{
    if ((addr - INODE_BLOCK_ADD) % INODE_SIZE != 0)
    {
        cout << "The address is not the beginning of an inode" << endl;
        return false;
    }
    if (superBlock.freeInodeNum == INODE_NUM)
    {
        cout << "No Inodes need to be free" << endl;
        return false;
    }
    int i = (addr - INODE_BLOCK_ADD) / INODE_SIZE;
    if (inodeBitMap[i] == 0)
    {
        cout << "The inode " << i << " is not used";
        return false;
    }

    Inode pad = {0};
    f.seekp(addr, ios::beg);
    f.write((char *)&pad, sizeof(pad));

    superBlock.freeInodeNum++;
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *)&superBlock, sizeof(superBlock));

    inodeBitMap[i] = (char)0;
    f.seekp(INODE_BITMAP_ADD, ios::beg);
    f.write((char *)inodeBitMap.data(), sizeof(char) * inodeBitMap.size());
    return true;
}

int balloc()
{
    if (superBlock.freeBlockNum <= 0)
    {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    int top = (superBlock.freeBlockNum - 1) % BLOCKS_PER_GROUP;
    int blockAddr;
    if (superBlock.free[top] == -1)
    {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    if (top == 0)
    {
        //栈底，更新freeAddr
        blockAddr = superBlock.free[top];
        superBlock.freeAddr = superBlock.free[top];
        f.seekg(superBlock.freeAddr, ios::beg);
        f.read(reinterpret_cast<char *>(superBlock.free), sizeof(superBlock.free));
    }
    else
    {
        blockAddr = superBlock.free[top];
    }
    superBlock.freeBlockNum--;
    //更新superblock
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *)&superBlock, sizeof(superBlock));
    //更新bitmap
    int i = (blockAddr - BLOCK_ADD) / BLOCK_SIZE;
    cout << "Block" << i << " is allocated" << endl;
    blockBitMap[i] = (char)1;
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *)blockBitMap.data(), blockBitMap.size() * sizeof(char));
    return blockAddr;
}

bool bfree(int addr)
{
    if ((addr - BLOCK_ADD) % BLOCK_SIZE != 0)
    {
        cout << "The address is not the beginning of a block" << endl;
        return false;
    }
    if (superBlock.freeBlockNum == BLOCK_NUM)
    {
        cout << "No blocks need to be free" << endl;
        return false;
    }
    int i = (addr - BLOCK_ADD) / BLOCK_SIZE;
    if ((int)blockBitMap[i] == 0)
    {
        cout << "The block is not used" << endl;
        return false;
    }

    vector<char> pad(BLOCK_SIZE, 0);
    f.seekp(addr, ios::beg);
    f.write((char *)pad.data(), pad.size() * sizeof(char));
    int top = (superBlock.freeBlockNum - 1) % BLOCKS_PER_GROUP;
    if (top == BLOCKS_PER_GROUP - 1)
    {
        //将空闲块堆栈内容写入新释放的块
        f.seekp(addr, ios::beg);
        f.write((char *)&superBlock.free, sizeof(superBlock.free));
        //更新空闲块堆栈
        for (int j = 1; j < BLOCKS_PER_GROUP; ++j)
        {
            superBlock.free[j] = -1;
        }
    }
    else
    {
        top++;
        superBlock.free[top] = addr;
    }
    superBlock.freeBlockNum++;
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *)&superBlock, sizeof(superBlock));

    blockBitMap[i] = (char)0;
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *)blockBitMap.data(), sizeof(char) * blockBitMap.size());
    return true;
}

void printInfo()
{
    cout << "Number of Free Inode: " << superBlock.freeInodeNum << "/" << INODE_NUM << endl;
    cout << "Number of Free Block: " << superBlock.freeBlockNum << "/" << BLOCK_NUM << endl;
    for (int i = 0; i < blockBitMap.size(); ++i)
    {
        if (i % BLOCKS_PER_GROUP == 0)
            cout << endl;
        if (blockBitMap[i] == 1)
            cout << "*";
        else
            cout << "-";
    }
}

bool init()
{
    superBlock.freeBlockNum = BLOCK_NUM;
    superBlock.freeInodeNum = INODE_NUM;
    superBlock.freeAddr = BLOCK_ADD;
    for (int i = (BLOCK_NUM / BLOCKS_PER_GROUP) - 1; i >= 0; --i)
    {
        cout << i << endl;
        if (i == (BLOCK_NUM / BLOCKS_PER_GROUP) - 1)
        {
            //最后一组
            superBlock.free[0] = -1;
        }
        else
        {
            superBlock.free[0] = BLOCK_ADD + (i + 1) * BLOCK_SIZE * BLOCKS_PER_GROUP;
        }
        for (int j = 1; j < BLOCKS_PER_GROUP; ++j)
        {
            superBlock.free[j] = BLOCK_ADD + ((i + 1) * BLOCKS_PER_GROUP - j) * BLOCK_SIZE;
        }
        f.seekp(BLOCK_ADD + i * BLOCK_SIZE * BLOCKS_PER_GROUP, ios::beg);
        f.write((char *)&superBlock.free, sizeof(superBlock.free));
        cout << "good?" << f.good() << endl;
    }
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *)&superBlock, sizeof(superBlock));

    f.seekp(INODE_BITMAP_ADD, ios::beg);
    f.write((char *)inodeBitMap.data(), inodeBitMap.size() * sizeof(char));
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *)blockBitMap.data(), blockBitMap.size() * sizeof(char));

    return true;
}

bool mkdir(int parent, const string &name)
{
    if (name.length() > 28)
    {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    if (repeat(parent, name, false))
    {
        return false;
    }
    int pos1 = -1, pos2 = -1;
    Inode parentInode{};
    f.seekg(parent, ios::beg);
    f.read((char *)&parentInode, sizeof(Inode));
    DirItem itemList[32];
    for (int i = 0; i < 10; ++i)
    {
        if ((parentInode.dirBlock[i] - BLOCK_ADD) % BLOCK_SIZE == 0)
        {
            f.seekg(parentInode.dirBlock[i], ios::beg);
            f.read((char *)itemList, sizeof(itemList));
            for (int j = 0; j < 32; ++j)
            {
                if (strcmp(itemList[j].name, "") == 0)
                {
                    pos1 = i;
                    pos2 = j;
                    i = 10;
                    break;
                }
            }
        }
        else
        {
            if (pos1 == -1)
            {
                pos1 = i;
                pos2 = 0;
            }
        }
    }
    //需要indirBlock
    int pos3 = -1; //indirect block的序号
    if (pos1 == -1)
    {
        int blocks[256] = {-1};
        if (parentInode.indirBlock == -1)
        {
            int indir = balloc();
            if (indir != -1)
            {
                parentInode.indirBlock = indir;
                int block = balloc();
                if (block == -1)
                {
                    cout << "No Free Blocks" << endl;
                    return false;
                }
                pos3 = 0;
                pos2 = 0;
            }
            else
            {
                cout << "Block allocation error" << endl;
                return false;
            }
        }
        else
        {
            f.seekg(parentInode.indirBlock, ios::beg);
            f.read((char *)blocks, sizeof(blocks));
            for (int i = 0; i < 256; ++i)
            {
                if ((blocks[i] - BLOCK_ADD) % BLOCK_SIZE == 0)
                {
                    f.seekg(blocks[i], ios::beg);
                    f.read((char *)itemList, sizeof(itemList));
                    for (int j = 0; j < 32; ++j)
                    {
                        if (strcmp(itemList[j].name, "") == 0)
                        {
                            pos3 = i;
                            pos2 = j;
                            i = 256;
                            break;
                        }
                    }
                }
                else
                {
                    if (pos3 == -1)
                    {
                        pos3 = i;
                        pos2 = 0;
                    }
                }
            }
        }
    }
    if (pos3 == -1 && pos1 == -1)
    {
        cout << "No free blocks" << endl;
        return false;
    }
    //创建inode
    Inode child{};
    DirItem items[32] = {0}; //一个block可以储存32个DirItem
    int inodeAddr = ialloc();
    cout << "inodeAddr: " << inodeAddr << endl;
    int blockAddr = balloc();
    cout << "blockAddr:" << blockAddr << endl;
    if (inodeAddr == -1 || blockAddr == -1)
    {
        cout << "Allocation error" << endl;
        return false;
    }
    strcpy(items[0].name, ".");
    strcpy(items[1].name, "..");
    items[0].inodeAddr = inodeAddr;
    items[1].inodeAddr = parent;

    child.id = (inodeAddr - INODE_BLOCK_ADD) / INODE_SIZE;
    time(&child.ctime);
    time(&child.atime);
    time(&child.mtime);
    child.cnt = 2;
    f.seekp(blockAddr, ios::beg);
    f.write((char *)items, sizeof(items));
    child.dirBlock[0] = blockAddr;
    for (int k = 1; k < 10; ++k)
    {
        child.dirBlock[k] = -1;
    }
    f.seekp(inodeAddr, ios::beg);
    f.write((char *)&child, sizeof(Inode));
    if (parentInode.dirBlock[pos1] != -1)
    {
        f.seekg(parentInode.dirBlock[pos1], ios::beg);
        f.read((char *)itemList, sizeof(itemList));
        strcpy(itemList[pos2].name, name.c_str());
        itemList[pos2].inodeAddr = inodeAddr;
        f.seekp(parentInode.dirBlock[pos1], ios::beg);
        f.write((char *)itemList, sizeof(itemList));
    }
    else if (pos1 != -1)
    {
        int tmpBlock = balloc();
        DirItem tmpList[32] = {0};
        if (tmpBlock == -1)
        {
            cout << "No Free Blocks" << endl;
            return false;
        }
        parentInode.dirBlock[pos1] = tmpBlock;
        tmpList[0].inodeAddr = inodeAddr;
        strcpy(tmpList[0].name, name.c_str());
        f.seekp(tmpBlock, ios::beg);
        f.write((char *)tmpList, sizeof(tmpList));
    }
    else if (pos3 != -1)
    {
        int blocks[256];
        DirItem tmpList[32] = {0};
        f.seekg(parentInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        if (blocks[pos3] == -1)
        {
            int tmp = balloc();
            if (tmp == -1)
            {
                cout << "No free blocks" << endl;
            }
            blocks[pos3] = tmp;
        }
        f.seekg(blocks[pos3], ios::beg);
        f.read((char *)tmpList, sizeof(tmpList));
        tmpList[pos2].inodeAddr = inodeAddr;
        strcpy(tmpList[pos2].name, name.c_str());
        f.seekp(blocks[pos3], ios::beg);
        f.write((char *)tmpList, sizeof(tmpList));
    }

    parentInode.cnt++;
    f.seekp(parent, ios::beg);
    f.write((char *)&parentInode, sizeof(parentInode));
    return true;
}

bool repeat(int parent, const string &name, bool isFile)
{
    Inode parentInode{};
    f.seekg(parent, ios::beg);
    f.read((char *)&parentInode, sizeof(Inode));
    DirItem itemList[32];
    for (int i : parentInode.dirBlock)
    {
        if (i == -1)
        {
            continue;
        }
        f.seekg(i, ios::beg);
        f.read((char *)itemList, sizeof(itemList));
        for (auto &j : itemList)
        {
            if (strcmp(j.name, name.c_str()) == 0)
            {
                Inode tmp{};
                f.seekg(j.inodeAddr, ios::beg);
                f.read((char *)&tmp, sizeof(Inode));
                if (tmp.isFile == isFile)
                {
                    cout << "The file or directory is existed" << endl;
                    return true;
                }
            }
        }
    }

    if (parentInode.indirBlock != -1)
    {
        int blocks[256];
        f.seekg(parentInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int block : blocks)
        {
            if ((block - BLOCK_ADD) % BLOCK_SIZE == 0)
            {
                f.seekg(block, ios::beg);
                f.read((char *)itemList, sizeof(itemList));
                for (auto &j : itemList)
                {
                    if (strcmp(j.name, name.c_str()) == 0)
                    {
                        Inode tmp{};
                        f.seekg(j.inodeAddr, ios::beg);
                        f.read((char *)&tmp, sizeof(Inode));
                        if (tmp.isFile == isFile)
                        {
                            cout << "The file or directory is existed" << endl;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool createDir(string name)
{
    return false;
}

// 进行一层目录位置的改变
// Q：有没有对不合法目录的处理？
void cd(int parent, const string &name)
{ //当前目录(parent)下的name目录
    Inode parentInode{};
    f.seekg(parent, ios::beg);
    f.read((char *)&parentInode, sizeof(Inode));
    DirItem itemList[32];
    // 查看dirBlock中的DirItem
    for (int i : parentInode.dirBlock)
    {
        if (i == -1)
        {
            continue;
        }
        f.seekg(i, ios::beg);
        f.read((char *)itemList, sizeof(itemList));
        for (auto &j : itemList)
        {
            // 读入对应名字的Item 找到非file的Inode
            if (strcmp(j.name, name.c_str()) == 0)
            {
                Inode tmp{};
                f.seekg(j.inodeAddr, ios::beg);
                f.read((char *)&tmp, sizeof(Inode));
                if (!tmp.isFile)
                {
                    if (strcmp(j.name, ".") == 0)
                    {
                        return;
                    }
                    else if (strcmp(j.name, "..") == 0)
                    {
                        int k;
                        for (k = strlen(curName.c_str()); k >= 0; k--)
                        {
                            if (curName[k] == '/')
                                break;
                        }
                        curName = curName.substr(0, k);
                        curAddr = j.inodeAddr;
                        return;
                    }
                    else
                    {
                        curAddr = j.inodeAddr;
                        curName.append("/");
                        curName.append(j.name);
                        return;
                    }
                }
            }
        }
    }

    if (parentInode.indirBlock != -1)
    {
        int blocks[256];
        f.seekg(parentInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int block : blocks)
        {
            if ((block - BLOCK_ADD) % BLOCK_SIZE == 0)
            {
                f.seekg(block, ios::beg);
                f.read((char *)itemList, sizeof(itemList));
                for (auto &j : itemList)
                {
                    if (strcmp(j.name, name.c_str()) == 0)
                    {
                        Inode tmp{};
                        f.seekg(j.inodeAddr, ios::beg);
                        f.read((char *)&tmp, sizeof(Inode));
                        if (!tmp.isFile)
                        {
                            if (strcmp(j.name, ".") == 0)
                            {
                                return;
                            }
                            else if (strcmp(j.name, "..") == 0)
                            {
                                int k;
                                for (k = strlen(curName.c_str()); k >= 0; k--)
                                {
                                    if (curName[k] == '/')
                                        break;
                                }
                                curName = curName.substr(0, k);
                                curAddr = j.inodeAddr;
                                return;
                            }
                            else
                            {
                                curAddr = j.inodeAddr;
                                curName.append("/");
                                curName.append(j.name);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // 可在此部分添加return false
}

void changeDir(string name)
{
    for (int i = 0; i < strlen(name.c_str()); ++i)
    {
        if (name[i] == '/')
        {
            string tmp = name.substr(0, i);
            cd(curAddr, tmp);
            name = name.substr(i + 1);
            i = 0;
        }
    }
    cd(curAddr, name);
}

bool toolcd(const string &name, int &l_curAddr, string &l_curName)
{ //当前目录(parent)下的name目录
    // 功能：进行一层cd操作
    // name: 一个不含'/'目录名
    // l_curAddr: 目录inode addr 引用
    // l_curName：目录字符串 引用
    // 备注:输出不保证目录字符串已经格式化，可能出现如../../..////等
    Inode parentInode{};
    f.seekg(l_curAddr, ios::beg);
    f.read((char *)&parentInode, sizeof(Inode));
    DirItem itemList[32];
    // 查看dirBlock中的DirItem
    for (int i : parentInode.dirBlock)
    {
        if (i == -1)
        {
            continue;
        }
        f.seekg(i, ios::beg);
        f.read((char *)itemList, sizeof(itemList));
        for (auto &j : itemList)
        {
            // 读入对应名字的Item 找到非file的Inode
            if (strcmp(j.name, name.c_str()) == 0)
            {
                Inode tmp{};
                f.seekg(j.inodeAddr, ios::beg);
                f.read((char *)&tmp, sizeof(Inode));
                if (!tmp.isFile)
                {
                    // 对path string进行更改
                    if (strcmp(j.name, ".") == 0)
                    {
                        return true;
                    }
                    else if (strcmp(j.name, "..") == 0)
                    {
                        int k;
                        for (k = strlen(l_curName.c_str()); k >= 0; k--)
                        {
                            if (l_curName[k] == '/')
                                break;
                        }
                        l_curName = l_curName.substr(0, k);
                        l_curAddr = j.inodeAddr;
                        return true;
                    }
                    else
                    {
                        l_curAddr = j.inodeAddr;
                        l_curName.append("/");
                        l_curName.append(j.name);
                        return true;
                    }
                }
            }
        }
    }

    if (parentInode.indirBlock != -1)
    {
        int blocks[256];
        f.seekg(parentInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int block : blocks)
        {
            if ((block - BLOCK_ADD) % BLOCK_SIZE == 0)
            {
                f.seekg(block, ios::beg);
                f.read((char *)itemList, sizeof(itemList));
                for (auto &j : itemList)
                {
                    if (strcmp(j.name, name.c_str()) == 0)
                    {
                        Inode tmp{};
                        f.seekg(j.inodeAddr, ios::beg);
                        f.read((char *)&tmp, sizeof(Inode));
                        if (!tmp.isFile)
                        {
                            if (strcmp(j.name, ".") == 0)
                            {
                                return true;
                            }
                            else if (strcmp(j.name, "..") == 0)
                            {
                                int k;
                                for (k = strlen(l_curName.c_str()); k >= 0; k--)
                                {
                                    if (l_curName[k] == '/')
                                        break;
                                }
                                l_curName = l_curName.substr(0, k);
                                l_curAddr = j.inodeAddr;
                                return true;
                            }
                            else
                            {
                                l_curAddr = j.inodeAddr;
                                l_curName.append("/");
                                l_curName.append(j.name);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    // 可在此部分添加return false
    // 没有提前return，目录变动不合法
    return false;
}

bool toolchangeDir(string name, int &l_curAddr, string &l_curName)
{
    // 功能：进行cd操作，支持绝对路径。当路径不合法则留在原路径。
    // name: 路径
    // l_curAddr: 目录inode addr 引用
    // l_curName：目录字符串 引用
    // 备注:输出不保证目录字符串已经格式化，可能出现如../../..////等

    // 保存副本，当目录不合法时用于复原
    int originAddr = l_curAddr;
    string originName = l_curName;

    if (strlen(name.c_str()) == 0)
        return true;
    if (name[0] == '/')
    { // name 为绝对路径,改变到根目录
        l_curAddr = INODE_BLOCK_ADD;
        l_curName = "/";
    }

    // 去除一次头部/
    int i = 0;
    for (; i < strlen(name.c_str()) && name[i] == '/'; ++i)
        ;
    if (i == strlen(name.c_str()))
        return true; // 全为/不改变路径
    // 至少有一些路径
    name = name.substr(i);
    i = 0;

    for (; i <= strlen(name.c_str()); ++i)
    { // 找到下一个/或到结束
        if (i == strlen(name.c_str()) || name[i] == '/')
        {
            string tmp = name.substr(0, i);
            if (toolcd(tmp, l_curAddr, l_curName) == false)
            { // 尝试一层寻路，失败则复原
                l_curAddr = originAddr;
                l_curName = originName;
                cout << "Invalid path" << endl;
                return false;
            }
            // 过滤掉可能的/符号, 之后i可能超出，则已完成
            for (; i < strlen(name.c_str()) && name[i] == '/'; ++i)
                ;
            if (i == strlen(name.c_str()))
                return true;
            name = name.substr(i);
            i = 0;
        }
    }
}

bool toolchangeDir(vector<string> elems, bool relative, int &l_curAddr, string &l_curName)
{
    //重载的版本，接收vector,默认为相对路径
    // 保存副本，当目录不合法时用于复原
    int originAddr = l_curAddr;
    string originName = l_curName;

    if (relative == false)
    { //绝对路径
        l_curAddr = INODE_BLOCK_ADD;
        l_curName = "/";
    }
    if (elems.empty())
        return true;
    for (const string &item : elems)
    {
        if (toolcd(item, l_curAddr, l_curName) == false)
        {
            l_curAddr = originAddr;
            l_curName = originName;
            cout << "Invalid path" << endl;
            return false;
        }
    }
    return true;
}

// 文件复制 复制file1到file2，如果file2存在则询问是否继续
bool copy(string path1, string path2)
{
    //目录复制要做吗?暂时不做
    //path可以是文件或者目录
    //path1:为文件,则要求path2:前部分为可达目录,最后一项默认为名字
    //达到path2倒数第二目录,尝试以最后项为文件名寻找该目录文件,如果已存在,发出询问
    //path1:为目录,则要求path2:为可达目录
    //达到path2,检查是否与path1为相同目录,是则应拒绝
    //理论方法：创建两个addr变量（inode地址）,调用cd或changeDir找对应inode addr
    //文件复制:类似于创建文件,只需要生成过程改为复制
    //目录复制

    // 有效性检查：分词，非空则pop出文件名，判断是否绝对路径，对前部分cd，有效则检查是否repeat
    // 以下仅复制文件
    // 检查path是否完全为空
    if (path1.empty() || path2.empty())
    {
        cout << "Invalid path/filename" << endl;
    }
    int addr1 = curAddr;
    int addr2 = curAddr;
    string pth1 = curName;
    string pth2 = curName;

    vector<string> tmppath;
    string fileName;
    bool relative = true;
    // path1
    // 分词
    tmppath = split(path1, '/');
    if (tmppath.empty())
    {
        cout << "Invalid path/filename" << endl;
        return false;
    }
    fileName = tmppath.back();
    if (fileName.length() > 28)
    {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    tmppath.pop_back();
    if (path1.at(0) == '/')
    { //绝对路径
        relative = false;
    }
    else
    {
        relative = true;
    }
    if (toolchangeDir(tmppath, relative, addr1, pth1) == false)
    {
        cout << "Invalid path/filename" << endl;
        return false;
    }
    addr1 = getFileAddr(addr1, fileName);
    if (addr1 == -1)
    {
        cout << "Invalid path/filename" << endl;
        return false;
    }

    // 到这里，确定了file1的addr
    // path2
    tmppath = split(path2, '/');
    if (tmppath.empty())
    {
        cout << "Invalid path/filename" << endl;
        return false;
    }
    fileName = tmppath.back();
    if (fileName.length() > 28)
    {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    tmppath.pop_back();
    if (path2.at(0) == '/')
    { //绝对路径
        relative = false;
    }
    else
    {
        relative = true;
    }
    if (toolchangeDir(tmppath, relative, addr2, pth2) == false)
    {
        cout << "Invalid path/filename" << endl;
        return false;
    }
    int parent2 = addr2;
    addr2 = getFileAddr(addr2, fileName);
    // 获取file1
    Inode fileInode1{};
    f.seekg(addr1, ios::beg);
    f.read((char *)&fileInode1, sizeof(Inode));
    // 进行文件复制
    // buffer
    char buffer[1024];
    if (addr2 != -1)
    { // 说明file已存在，询问是否覆盖
        cout << "target file already exist, proceed? [y/n]" << endl;
        string tmpstr;
        cin >> tmpstr;
        if (tmpstr == "y")
        {
            // 覆盖型复制，不需要调整parentInode
            // 根据size决定需要释放还是申请新空间
            // 成功后根据size逐个复制
            //todo 暂时returnfalse
            return false;
        }
        else
        {
            cout << "file copy canceled" << endl;
            return false;
        }
    }
    else // file不存在
    {
        addr2 = ialloc();
        // todo对parent inode进行更改:找到空闲的itemList中的item，分配name，addr2
        Inode fileInode2{};
        // // 设置inode的属性
        fileInode2.id = (addr2 -INODE_BLOCK_ADD) / INODE_SIZE;
        fileInode2.size = fileInode1.size;
        fileInode2.cnt = fileInode1.cnt;
        fileInode2.isFile = true;
        time(&fileInode2.ctime);
        time(&fileInode2.atime);
        time(&fileInode2.mtime);
        // 根据size逐个分配block和复制
        //不需要indir的部分
            int dirblocks[10] = {-1};
            for (int i = 0; i < fileInode2.size; i++)
            {
                int blockAddr = balloc();
                if (blockAddr == -1)
                {
                    return false;
                }
                
                dirblocks[i] = blockAddr;//inode dir信息
                f.seekg(fileInode1.dirBlock[i], ios::beg);
                f.read((char *)buffer, sizeof(buffer));
                f.seekp(blockAddr, ios::beg);
                f.write((char *)buffer, sizeof(buffer));
            }
            //保存inode dir
            memcpy(fileInode2.dirBlock, dirblocks, sizeof(dirblocks));
        if (fileInode2.size > 10)
        {
            //分配一个indir
            int indirBlockAddr = balloc();
            if (indirBlockAddr == -1){
                return false;
            }
            fileInode2.indirBlock = indirBlockAddr;//
            //把file1 indir中对应的addr位置取出
            int blocks1[256] = {-1};
            f.seekg(fileInode1.indirBlock, ios::beg);
            f.read((char *)blocks1, sizeof(blocks1));
            // 分配block和复制内容，block地址暂存到blocks2
            int blocks2[256] = {-1};
            for (int i=0; i<fileInode2.size-10; i++){
                int blockAddr = balloc();
                if (blockAddr == -1)
                {
                    return false;
                }
                blocks2[i] = blockAddr;
                f.seekg(blocks1[i], ios::beg);
                f.read((char *)buffer, sizeof(buffer));
                f.seekp(blocks2[i], ios::beg);
                f.write((char *)buffer, sizeof(buffer));
            }
            // 保存blocks2
            f.seekp(fileInode2.indirBlock, ios::beg);
            f.write((char *)blocks2, sizeof(blocks2));
        }
        else{
            fileInode2.indirBlock = -1;
        }
        //保存inode
        f.seekp(addr2, ios::beg);
        f.write((char *)&fileInode2, sizeof(fileInode2));
    }
}
// cat()(展示文件内容)
// 判断path是否可达而且为文件
// 通过inode展示文件内容
bool cat(const string &path)
{
}
//ls （展示所有文件）
//判断path是否可达而且为目录
//通过inode展示目录和文件列表 对文件:包括size
//win风格: 日期 时间 <DIR> size 名字
//可以按名字排序
bool ls(const string &path)
{
}
int getFileAddr(int parent, const string &name)
{
    // 进行一层查找，如果file在存在，返回addr，否则返回-1；
    Inode parentInode{};
    f.seekg(parent, ios::beg);
    f.read((char *)&parentInode, sizeof(Inode));
    DirItem itemList[32];
    for (int i : parentInode.dirBlock)
    {
        if (i == -1)
        {
            continue;
        }
        f.seekg(i, ios::beg);
        f.read((char *)itemList, sizeof(itemList));
        for (auto &j : itemList)
        {
            if (strcmp(j.name, name.c_str()) == 0)
            {
                Inode tmp{};
                f.seekg(j.inodeAddr, ios::beg);
                f.read((char *)&tmp, sizeof(Inode));
                if (tmp.isFile == true)
                {
                    return j.inodeAddr;
                }
            }
        }
    }

    if (parentInode.indirBlock != -1)
    {
        int blocks[256];
        f.seekg(parentInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int block : blocks)
        {
            if ((block - BLOCK_ADD) % BLOCK_SIZE == 0)
            {
                f.seekg(block, ios::beg);
                f.read((char *)itemList, sizeof(itemList));
                for (auto &j : itemList)
                {
                    if (strcmp(j.name, name.c_str()) == 0)
                    {
                        Inode tmp{};
                        f.seekg(j.inodeAddr, ios::beg);
                        f.read((char *)&tmp, sizeof(Inode));
                        if (tmp.isFile == true)
                        {
                            return j.inodeAddr;
                        }
                    }
                }
            }
        }
    }
    return -1;
}

vector<string> split(const string &strIn, char delim)
{
    char *str = const_cast<char *>(strIn.c_str());
    string s;
    s.append(1, delim);
    vector<string> elems;
    char *ptr = NULL;
    char *splitted = strtok_s(str, s.c_str(), &ptr);
    while (splitted != NULL)
    {
        elems.push_back(string(splitted));
        splitted = strtok_s(NULL, s.c_str(), &ptr);
    }
    return elems;
}