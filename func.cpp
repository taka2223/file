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
        if (parentInode.indirBlock == -1) //无indirblock
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
                blocks[0] = block;
                f.seekp(indir, ios::beg);
                f.write((char *)blocks, sizeof(blocks));
                pos3 = 0;
                pos2 = 0;
            }
            else
            {
                cout << "Block allocation error" << endl;
                return false;
            }
        }
        else //已有indir块
        {
            f.seekg(parentInode.indirBlock, ios::beg);
            f.read((char *)blocks, sizeof(blocks));
            for (int i = 0; i < 256; ++i)
            {
                if ((blocks[i] - BLOCK_ADD) % BLOCK_SIZE == 0)
                { //现有块
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
                { //新块 需分配
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
    DirItem items[32]; //一个block可以储存32个DirItem
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
    //mk
    child.indirBlock = -1;
    /*
    cout<<"When create directory"<<endl;
    cout<<"Inode address "<<inodeAddr<<endl;
    cout<<"direct blocks: ";
    for(int i=0;i<10;i++){
        cout<<child.dirBlock[i]<<"  ";
    }
    cout<<endl;
    cout<<"indirect block "<<child.indirBlock<<endl;
    cout<<"cnt: "<<child.cnt<<endl;
    */
    f.seekp(inodeAddr, ios::beg);
    f.write((char *)&child, sizeof(Inode));
    if (pos1 != -1 && parentInode.dirBlock[pos1] != -1)
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
    // 理论上不会到这里
    return false;
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
        return false;
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
            // tips:理论上应该先完成所有的空间分配计算并尝试分配，确认可行再调整
            // 复制行为：先获取两个blockaddr列表，最后只需要进行对应复制
            vector<int> blocksof1;
            vector<int> blocksof2;
            int newindirof2 = -1;
            // 获取file1的对应addr
            // dir
            if (fileInode1.size > 0)
            {
                for (int i = 0; i < fileInode1.size && i < 10; i++)
                {
                    blocksof1.push_back(fileInode1.dirBlock[i]);
                }
            }
            // indir
            if (fileInode1.size > 10)
            {
                int blocks[256] = {-1};
                f.seekg(fileInode1.indirBlock, ios::beg);
                f.read((char *)blocks, sizeof(blocks));
                for (int i = 0; i < fileInode1.size - 10; i++)
                {
                    blocksof1.push_back(blocks[i]);
                }
            }
            // 尝试分配所有所需的blocks，如果size》10，也尝试分配一个indir
            if (fileInode1.size > 10)
            {
                int blockaddr = balloc();
                if (blockaddr == -1)
                {
                    return false; // 此时没有负面效果，可直接返回
                }
                newindirof2 = blockaddr;
            }
            // 分配block,如果中途失败，需要释放之前所有分配的空间，包括可能的indir
            for (int i = 0; i < fileInode1.size; i++)
            {
                int blockaddr = balloc();
                if (blockaddr == -1)
                {
                    // 复原操作 恢复indir 和 block
                    bfree(newindirof2);
                    for (int item : blocksof2)
                    {
                        bfree(item);
                    }
                    return false;
                }
                blocksof2.push_back(blockaddr);
            }
            // 文件内容复制
            if (blocksof1.size() != blocksof2.size())
            {   
                // debug info
                cout << "1: "<<fileInode1.size << endl;
                cout << "1: "<<blocksof1.size() <<"\n2: " << blocksof2.size(); 
                cout << "unexpected error during copy" << endl;
                return false;
            }
            char buffer[1024];
            for (int i = 0; i < blocksof1.size(); i++)
            {
                f.seekg(blocksof1.at(i), ios::beg);
                f.read((char *)buffer, sizeof(buffer));
                f.seekp(blocksof2.at(i), ios::beg);
                f.write((char *)buffer, sizeof(buffer));
            }
            // 释放inode所有原有块 如果有，也包括indir
            Inode fileInode2{};
            f.seekg(addr2, ios::beg);
            f.read((char *)&fileInode2, sizeof(fileInode2));
            // dir
            if (fileInode2.size > 0)
            {
                for (int i = 0; i < 10 && i < fileInode2.size; i++)
                {
                    if (!bfree(fileInode2.dirBlock[i]))
                    {
                        cout << "unexpected error during copy" << endl;
                        return false;
                    }
                    fileInode2.dirBlock[i] = -1;
                }
            }
            // indir
            if (fileInode2.size > 10)
            {
                int blocks[256] = {-1};
                f.seekg(fileInode2.indirBlock, ios::beg);
                f.read((char *)blocks, sizeof(blocks));
                // blocks
                for (int i = 0; i < fileInode2.size - 10; i++)
                {
                    if (!bfree(blocks[i]))
                    {
                        cout << "unexpected error during copy" << endl;
                        return false;
                    }
                    // blocks[i] = -1; indir释放了不用写回
                }
                // indir 本身
                if (!bfree(fileInode2.indirBlock))
                {
                    cout << "unexpected error during copy" << endl;
                    return false;
                }
                fileInode2.indirBlock = -1;
            }
            // 释放完毕相对于有一个新的inode
            // 按序为dir和indir，indir->blocks赋地址
            // dir
            for (int i = 0; i < 10; i++)
            {
                if (i < fileInode1.size)
                {
                    fileInode2.dirBlock[i] = blocksof2[i];
                }
                else
                { //初始化未使用部分
                    fileInode2.dirBlock[i] = -1;
                }
            }
            // indir,先赋indir本身 -1则充当初始化
            fileInode2.indirBlock = newindirof2;
            if (newindirof2 != -1) // ->blocks部分
            {
                // ->blocks部分直接保存
                int blocks[256] = {-1};
                for (int i = 0; i < fileInode1.size - 10; i++)
                {
                    blocks[i] = blocksof2.at(i + 10);
                }
                f.seekp(fileInode2.indirBlock, ios::beg);
                f.write((char *)blocks, sizeof(blocks));
            }
            // // 设置inode的其他属性
            fileInode2.size = fileInode1.size; //必须最后
            fileInode2.cnt = fileInode1.cnt;
            fileInode2.isFile = true;
            time(&fileInode2.atime);
            time(&fileInode2.mtime);
            // 保存inode2
            f.seekp(addr2, ios::beg);
            f.write((char *)&fileInode2, sizeof(fileInode2));
            return true;
        }
        else
        {
            cout << "file copy canceled" << endl;
            return false;
        }
    }
    else // file不存在
    // 修改为上述同样的方式，只需要更改复制的部分
    {
        addr2 = ialloc();
        // 对parent inode进行更改:找到空闲的itemList中的item，分配name，addr2
        Inode parentInode2{};
        f.seekg(parent2, ios::beg);
        f.read((char *)&parentInode2, sizeof(parentInode2));
        int pos1 = -1; //dirblock idx
        int pos2 = -1; //diritem idx
        DirItem itemList[32];
        for (int i = 0; i < 10; ++i)
        {
            if ((parentInode2.dirBlock[i] - BLOCK_ADD) % BLOCK_SIZE == 0)
            {
                f.seekg(parentInode2.dirBlock[i], ios::beg);
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
            else //dirblock未用完
            {
                if (pos1 == -1)
                {
                    pos1 = i;
                    pos2 = 0;
                }
            }
        }
        // 需要indir
        int pos3 = -1; //blocks idx
        if (pos1 == -1)
        {
            int blocks[256] = {-1};
            if (parentInode2.indirBlock == -1)
            {
                int indir = balloc();
                if (indir != -1)
                {
                    parentInode2.indirBlock = indir;
                    int block = balloc();
                    if (block == -1)
                    {
                        cout << "No Free Blocks" << endl;
                        return false;
                    }
                    blocks[0] = block;
                    f.seekp(indir, ios::beg);
                    f.write((char *)blocks, sizeof(blocks));
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
                f.seekg(parentInode2.indirBlock, ios::beg);
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
        // 添加diritem到parentinode 或其indirblock
        // case:在dirblock现有块pos1 != -1
        // case 在dirblock新块pos1 != -1
        // case 在indir现有快pos1 == -1, pos3 !=-1
        // case 在indir新块
        // case 在indir，新indir新块
        if (pos1 != -1 && parentInode2.dirBlock[pos1] != -1)
        {
            f.seekg(parentInode2.dirBlock[pos1], ios::beg);
            f.read((char *)itemList, sizeof(itemList));
            strcpy(itemList[pos2].name, fileName.c_str());
            itemList[pos2].inodeAddr = addr2;
            f.seekp(parentInode2.dirBlock[pos1], ios::beg);
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
            parentInode2.dirBlock[pos1] = tmpBlock;
            tmpList[0].inodeAddr = addr2;
            strcpy(tmpList[0].name, fileName.c_str());
            f.seekp(tmpBlock, ios::beg);
            f.write((char *)tmpList, sizeof(tmpList));
        }
        else if (pos3 != -1)
        {
            int blocks[256];
            DirItem tmpList[32] = {0};
            f.seekg(parentInode2.indirBlock, ios::beg);
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
            //todo Q假设了新分配的block是全0
            f.seekg(blocks[pos3], ios::beg);
            f.read((char *)tmpList, sizeof(tmpList));
            tmpList[pos2].inodeAddr = addr2;
            strcpy(tmpList[pos2].name, fileName.c_str());
            f.seekp(blocks[pos3], ios::beg);
            f.write((char *)tmpList, sizeof(tmpList));
        }
        // 其他inode信息
        time(&parentInode2.atime);
        time(&parentInode2.mtime);
        parentInode2.cnt++;
        // 保存parentinode
        f.seekp(parent2, ios::beg);
        f.write((char *)&parentInode2, sizeof(parentInode2));
        Inode fileInode2{}; // 新建的不需要读取
        // mark 复制开始
        vector<int> blocksof1;
        vector<int> blocksof2;
        int newindirof2 = -1;
        // 获取file1的对应addr
        // dir
        if (fileInode1.size > 0)
        {
            for (int i = 0; i < fileInode1.size && i < 10; i++)
            {
                blocksof1.push_back(fileInode1.dirBlock[i]);
            }
        }
        // indir
        if (fileInode1.size > 10)
        {
            int blocks[256] = {-1};
            f.seekg(fileInode1.indirBlock, ios::beg);
            f.read((char *)blocks, sizeof(blocks));
            for (int i = 0; i < fileInode1.size - 10; i++)
            {
                blocksof1.push_back(blocks[i]);
            }
        }
        // 尝试分配所有所需的blocks，如果size》10，也尝试分配一个indir
        if (fileInode1.size > 10)
        {
            int blockaddr = balloc();
            if (blockaddr == -1)
            {
                return false; // 此时没有负面效果，可直接返回
            }
            newindirof2 = blockaddr;
        }
        // 分配block,如果中途失败，需要释放之前所有分配的空间，包括可能的indir
        for (int i = 0; i < fileInode1.size; i++)
        {
            int blockaddr = balloc();
            if (blockaddr == -1)
            {
                // 复原操作 恢复indir 和 block
                bfree(newindirof2);
                for (int item : blocksof2)
                {
                    bfree(item);
                }
                return false;
            }
            blocksof2.push_back(blockaddr);
        }
        // 文件内容复制
        if (blocksof1.size() != blocksof2.size())
        {
            cout << "unexpected error during copy" << endl;
            return false;
        }
        char buffer[1024];
        for (int i = 0; i < blocksof1.size(); i++)
        {
            f.seekg(blocksof1.at(i), ios::beg);
            f.read((char *)buffer, sizeof(buffer));
            f.seekp(blocksof2.at(i), ios::beg);
            f.write((char *)buffer, sizeof(buffer));
        }
        // inode2没有原有block需要释放
        // 按序为dir和indir，indir->blocks赋地址
        // dir
        for (int i = 0; i < 10; i++)
        {
            if (i < fileInode1.size)
            {
                fileInode2.dirBlock[i] = blocksof2[i];
            }
            else
            { //初始化未使用部分
                fileInode2.dirBlock[i] = -1;
            }
        }

        // indir,先赋indir本身 -1则充当初始化
        fileInode2.indirBlock = newindirof2;
        if (newindirof2 != -1) // ->blocks部分
        {
            // ->blocks部分直接保存
            int blocks[256] = {-1};
            for (int i = 0; i < fileInode1.size - 10; i++)
            {
                blocks[i] = blocksof2.at(i + 10);
            }
            f.seekp(fileInode2.indirBlock, ios::beg);
            f.write((char *)blocks, sizeof(blocks));
        }
        // // 设置inode的属性
        fileInode2.id = (addr2 - INODE_BLOCK_ADD) / INODE_SIZE;
        fileInode2.size = fileInode1.size;
        fileInode2.cnt = fileInode1.cnt;
        fileInode2.isFile = true;
        time(&fileInode2.ctime);
        time(&fileInode2.atime);
        time(&fileInode2.mtime);
        // 保存inode2
        f.seekp(addr2, ios::beg);
        f.write((char *)&fileInode2, sizeof(fileInode2));
        return true;
    }
    return true;
}
// cat()(展示文件内容)
// 判断path是否可达而且为文件
// 通过inode展示文件内容
bool cat(const string &path)
{
    if (path.empty())
    {
        cout << "Invalid path/filename1" << endl;
        return false;
    }
    int addrBackup = curAddr;
    string nameBackup = curName;
    // 分词确定路径
    vector<string> tmppath;
    string fileName;
    bool relative = true;
    tmppath = split(path, '/');
    if (tmppath.empty())
    {
        cout << "Invalid path/fileName2" << endl;
        return false;
    }
    fileName = tmppath.back();
    if (fileName.length() > 28)
    {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    tmppath.pop_back();
    if (path.at(0) == '/')
    {
        relative = false;
    }
    else
    {
        relative = true;
    }
    if (toolchangeDir(tmppath, relative) == false)
    {
        cout << "Invalid path/filename3" << endl;
        return false;
    }
    int fileAddr = -1;
    fileAddr = getFileAddr(curAddr, fileName);
    if (fileAddr == -1)
    {
        cout << "Invalid path/fileName4" << endl;
        // 应尝试返回原目录
        curAddr = addrBackup;
        curName = nameBackup;
        return false;
    }
    // 读取inode 展示内容
    Inode fileInode{};
    f.seekg(fileAddr, ios::beg);
    f.read((char *)&fileInode, sizeof(fileInode));
    char buffer[1024];
    // dir部分
    if (fileInode.size > 0)
    {
        for (int i = 0; i < fileInode.size && i < 10; i++)
        {
            f.seekg(fileInode.dirBlock[i], ios::beg);
            f.read((char *)buffer, sizeof(buffer));
            cout << buffer;
        }
    }
    // indir部分
    if (fileInode.size > 10)
    {
        int blocks[256] = {-1};
        // 读取地址
        f.seekg(fileInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int i = 0; i < fileInode.size - 10; i++)
        {
            f.seekg(blocks[i], ios::beg);
            f.read((char *)buffer, sizeof(buffer));
            cout << buffer;
        }
    }
    cout << endl;
    return true;
}
//ls （展示所有文件）
//判断path是否可达而且为目录
//通过inode展示目录和文件列表 对文件:包括size
//win风格: 日期 时间 <DIR> size 名字
//or: size y/m/d time name /
//可以按名字排序
bool ls(const string &path)
{
    int addr = curAddr;
    string name = curName;
    // if (path.empty()){
    //     cout << "Invalid path" << endl;
    //     return false;
    // }
    // path 允许为空
    if (!toolchangeDir(path, addr, name))
    {
        // cout << "Invalid path" << endl;
        return false;
    }
    // 显示文件名
    if (name != curName)
    {
        cout << "showing directory: " << name << endl;
        // todo 路径格式化函数
    }
    // get inode
    Inode dirInode{};
    f.seekg(addr, ios::beg);
    f.read((char *)&dirInode, sizeof(dirInode));
    // 显示目录内容 size y/m/d time name/
    DirItem itemsbuf[32];
    vector<DirItem> allItems = {};
    // dir部分
    for (int blockAddr : dirInode.dirBlock)
    {
        // cout << blockAddr;
        // system("pause");
        if (blockAddr == -1)
        {
            continue;
        }
        f.seekg(blockAddr, ios::beg);
        f.read((char *)itemsbuf, sizeof(itemsbuf));
        for (DirItem item : itemsbuf)
        {
            if (strcmp(item.name, "") != 0)
            {
                allItems.push_back(item);
            }
        }
    }
    // indir部分
    if (dirInode.indirBlock != -1)
    {
        // cout << "indirAddr: " << dirInode.indirBlock << " " << (dirInode.indirBlock - BLOCK_ADD)%BLOCK_SIZE;
        // system("pause");
        // 获取地址
        int blocks[256] = {-1};
        f.seekg(dirInode.indirBlock, ios::beg);
        f.read((char *)blocks, sizeof(blocks));
        for (int blockAddr : blocks)
        {
            // cout << blockAddr << (blockAddr - BLOCK_ADD)%BLOCK_SIZE;
            // system("pause");
            if (blockAddr == -1)
            {
                continue;
            }
            f.seekg(blockAddr, ios::beg);
            f.read((char *)itemsbuf, sizeof(itemsbuf));
            for (DirItem item : itemsbuf)
            {
                if (strcmp(item.name, "") == 0)
                {
                    allItems.push_back(item);
                }
            }
        }
    }
    // 排序
    sort(allItems.begin(), allItems.end());
    // 输出
    Inode tmpInode;
    struct tm *info;
    char tbuffer[32];
    // 表头 (略)
    // cout << "created time"
    for (DirItem &item : allItems)
    {
        // 获取indoe信息
        f.seekg(item.inodeAddr, ios::beg);
        f.read((char *)&tmpInode, sizeof(tmpInode));
        // 获取格式化时间
        info = localtime(&tmpInode.ctime);
        strftime(tbuffer, 80, "%Y-%m-%d %H:%M:%S", info);
        cout << tbuffer << '\t';
        // size
        if (tmpInode.isFile)
        {
            cout << tmpInode.size;
        }
        cout << '\t';
        // name, / for dir
        cout << item.name;
        if (!tmpInode.isFile)
        {
            cout << '/';
        }
        cout << "\t\n";
    }
    return true;
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

bool createFile(int parent,const string &name, const int &size){
    if (name.length() > 28) {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    // todo error:createFile name dir
    if(size>MAX_FILE_SIZE){
        cout << "Exceed the maximum size of a file " << endl;
        return false;
    }
    if (repeat(parent,name, true)){
        cout<< "File existed" << endl;
        return false;
    }
    Inode parentInode{};
    f.seekg(parent,ios::beg);
    f.read((char*)&parentInode,sizeof(Inode));
    int pos1=-1,pos2=-1;
    DirItem itemList[32];
    for (int i = 0; i <10; ++i) {
        if ((parentInode.dirBlock[i]-BLOCK_ADD)%BLOCK_SIZE==0){
            f.seekg(parentInode.dirBlock[i],ios::beg);
            f.read((char*)itemList,sizeof(itemList));
            for (int j = 0; j <32 ; ++j) {
                if (strcmp(itemList[j].name,"")==0){
                    pos1 = i;
                    pos2 = j;
                    i=10;
                    break;
                }
            }
        } else{
            if (pos1==-1){
                pos1 = i;
                pos2 = 0;
            }
        }
    }
    //需要indirBlock
    int pos3=-1;//indirect block的序号
    if (pos1==-1){
        int blocks[256]={-1};
        if (parentInode.indirBlock==-1){
            int indir = balloc();
            if (indir!=-1){
                parentInode.indirBlock = indir;
                int block = balloc();
                if (block==-1){
                    cout<<"No Free Blocks"<<endl;
                    return false;
                }
                pos3 = 0;
                pos2 = 0;
            } else{
                cout<<"Block allocation error"<<endl;
                return false;
            }
        } else{
            f.seekg(parentInode.indirBlock,ios::beg);
            f.read((char*)blocks,sizeof(blocks));
            for (int i = 0; i <256 ; ++i) {
                if ((blocks[i]-BLOCK_ADD)%BLOCK_SIZE==0){
                    f.seekg(blocks[i],ios::beg);
                    f.read((char*)itemList,sizeof(itemList));
                    for (int j = 0; j <32 ; ++j) {
                        if (strcmp(itemList[j].name,"")==0){
                            pos3 = i;
                            pos2 = j;
                            i=256;
                            break;
                        }
                    }
                } else{
                    if (pos3==-1){
                        pos3 = i;
                        pos2 = 0;
                    }
                }
            }
        }
    }
    if (pos3==-1&&pos1==-1){
        cout<<"No free blocks"<<endl;
        return false;
    }
    //创建inode
    Inode  child{};
    int inodeAddr = ialloc();
    cout << "inodeAddr: " << inodeAddr << endl;
    int blockadd[size];
    for(int i=0;i<size;++i){   
        int blockAddr = balloc();
        blockadd[i]=blockAddr;
        cout << "blockAddr:" << blockAddr << endl;
        if (inodeAddr == -1 || blockAddr == -1) {
        cout << "Allocation error" << endl;
        return false;
        }
    }
    for(int i=0;i<size;++i){
        f.seekp(blockadd[i],ios::beg);
        srand(time(0));
        char temp[1024];
        for(int j=0;j<1024;++j){
             temp[j]=char('a'+rand()%26);
        }   
        f.write((char*)temp,sizeof(temp));
    }
    if(size<=10){
        for(int i=0;i<size;i++){
            child.dirBlock[i]=blockadd[i];
        }
        for(int i=size;i<10;i++){
            child.dirBlock[i]=-1;
        }
        child.indirBlock=-1;
    }//只需要用到直接块
    else{
        for(int i=0;i<10;i++){
            child.dirBlock[i]=blockadd[i];
        }
        int indirectblock=balloc();
        int blocks[256];
        for(int i=10;i<size;i++){
            blocks[i-10]=blockadd[i];
        }
        for(int i=size-10;i<256;i++){
            blocks[i]=-1;
        }
        child.indirBlock=indirectblock;
        f.seekp(indirectblock,ios::beg);
        f.write((char*)blocks,sizeof(blocks));
    }
    child.id = (inodeAddr-INODE_BLOCK_ADD)/INODE_SIZE;
    time(&child.ctime);
    time(&child.atime);
    time(&child.mtime);
    child.isFile=true;
    child.size=size;
    f.seekp(inodeAddr,ios::beg);
    f.write((char*)&child,sizeof(Inode));
    //写入文件信息

    if (parentInode.dirBlock[pos1]!=-1){
        f.seekg(parentInode.dirBlock[pos1],ios::beg);
        f.read((char*)itemList,sizeof(itemList));
        strcpy(itemList[pos2].name,name.c_str());
        itemList[pos2].inodeAddr = inodeAddr;
        f.seekp(parentInode.dirBlock[pos1],ios::beg);
        f.write((char*)itemList,sizeof(itemList));
    } else if (pos1!=-1){
        int tmpBlock = balloc();
        DirItem tmpList[32]={0};
        if (tmpBlock==-1){
            cout<<"No Free Blocks"<<endl;
            return false;
        }
        parentInode.dirBlock[pos1]=tmpBlock;
        tmpList[0].inodeAddr=inodeAddr;
        strcpy(tmpList[0].name,name.c_str());
        f.seekp(tmpBlock,ios::beg);
        f.write((char*)tmpList,sizeof(tmpList));
    } else if (pos3!=-1){
        int blocks[256];
        DirItem tmpList[32]={0};
        f.seekg(parentInode.indirBlock,ios::beg);
        f.read((char*)blocks,sizeof(blocks));
        if (blocks[pos3]==-1){
            int tmp = balloc();
            if (tmp==-1){
                cout<<"No free blocks"<<endl;
            }
            blocks[pos3]=tmp;
        }
        f.seekg(blocks[pos3],ios::beg);
        f.read((char*)tmpList,sizeof(tmpList));
        tmpList[pos2].inodeAddr=inodeAddr;
        strcpy(tmpList[pos2].name,name.c_str());
        f.seekp(blocks[pos3],ios::beg);
        f.write((char*)tmpList,sizeof(tmpList));

     //更新当前目录信息
    parentInode.cnt++;
    f.seekp(parent,ios::beg);
    f.write((char*)&parentInode,sizeof(parentInode));
    return true;
}
}

bool deleteFile(int parent, const string &name)
{
    if (name.length() > 28)
    {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    if (!repeat(parent, name, true))
    {
        cout << "Could not find the file" << endl;
    }
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
                if (tmp.isFile)
                {
                    //找到文件所在位置
                    deleteF(j.inodeAddr);
                    cout << "The file whose inode address is " << j.inodeAddr << " has been deleted" << endl;
                    strcpy(j.name, "");
                    j.inodeAddr = 0;
                    break;
                }
            }
        }
        f.seekp(i, ios::beg);
        f.write((char *)itemList, sizeof(itemList));
        return true;
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
                        if (tmp.isFile)
                        {
                            deleteF(j.inodeAddr);
                            cout << "The file whose inode address is " << j.inodeAddr << " has been deleted" << endl;
                            strcpy(j.name, "");
                            j.inodeAddr = 0;
                            break;
                        }
                    }
                }
                f.seekp(block, ios::beg);
                f.write((char *)itemList, sizeof(itemList));
            }
        }
    }
}
void deleteF(int inodeAddr){
    Inode file{};
    f.seekg(inodeAddr,ios::beg);
    f.read((char*)&file,sizeof(Inode));
    DirItem itemList[32];
    vector<int>blockAddrs;
    for(int i:file.dirBlock){
        if(i==-1){
            break;
        }
        blockAddrs.push_back(i);
    }
    if(file.indirBlock!=-1){
        int blocks[256];
        f.seekg(file.indirBlock,ios::beg);
        f.read((char*)blocks,sizeof(blocks));
        for(int block:blocks){
            if ((block-BLOCK_ADD)%BLOCK_SIZE==0){
                blockAddrs.push_back(block);
            }
        }
    }
    ifree(inodeAddr);
    cout<<"Inode at "<< inodeAddr <<" has been released" << endl;
    for(int block:blockAddrs){
        bfree(block);
        cout<<"Block at "<< block <<" has been released" << endl;
    }
}
bool deleteDir(int parent,const string& name){
    if (name.length() > 28) {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    if (!repeat(parent,name,false)){
        cout<<"Could not find the directory"<< endl;
        return false;       
    }
    Inode parentInode{};
    f.seekg(parent,ios::beg);
    f.read((char*)&parentInode,sizeof(Inode));
    int inodeAddr;
    DirItem itemList[32];
    for (int i : parentInode.dirBlock) {
      
        if (i==-1){
            continue;
        }
        f.seekg(i,ios::beg);
        f.read((char*)itemList,sizeof(itemList));
        for (auto & j : itemList) {
            if (strcmp(j.name,name.c_str())==0){
                Inode tmp{};
                f.seekg(j.inodeAddr,ios::beg);
                f.read((char*)&tmp,sizeof(Inode));
                if (!tmp.isFile){
                    //找到了需要删除的目录
                    inodeAddr=j.inodeAddr;
                    cout<<"The inode address of the directory is "<<inodeAddr<<endl;
                    //将父目录的item信息更新
                    strcpy(j.name,"");
                    j.inodeAddr=0;
                }
            }
        }
        f.seekp(i,ios::beg);
        f.write((char*)itemList,sizeof(itemList));
    }
    if (parentInode.indirBlock!=-1){
        int blocks[256];
        f.seekg(parentInode.indirBlock,ios::beg);
        f.read((char*)blocks,sizeof(blocks));
        for (int block : blocks) {
            if ((block-BLOCK_ADD)%BLOCK_SIZE==0){
                f.seekg(block,ios::beg);
                f.read((char*)itemList,sizeof(itemList));
                for (auto & j : itemList) {
                    if (strcmp(j.name,name.c_str())==0){
                        Inode tmp{};
                        f.seekg(j.inodeAddr,ios::beg);
                        f.read((char*)&tmp,sizeof(Inode));
                        if (!tmp.isFile){
                            inodeAddr=j.inodeAddr;
                            cout<<"The Indirinode address of the directory is "<<inodeAddr<<endl;
                             //将父目录的item信息更新
                            strcpy(j.name,"");
                            j.inodeAddr=0;

                        }
                    }
                }
                f.seekp(block,ios::beg);
                f.write((char*)itemList,sizeof(itemList));
            }
        }

    }
    Inode dirInode{};
    f.seekg(inodeAddr,ios::beg);
    f.read((char*)&dirInode,sizeof(Inode));
    //查找目标目录的内容
    /*
    cout<<"When deleteDir"<<endl;
    cout<<"Inode add "<<inodeAddr<<endl;
    cout<<"direct blocks: ";
    for(int i=0;i<10;i++){
        cout<<dirInode.dirBlock[i]<<"  ";
    }
    cout<<endl;
    cout<<"indirct block "<<dirInode.indirBlock<<endl;
    cout<<"cnt: "<<dirInode.cnt<<endl;
    */
    DirItem itemList2[32];
    for (int j=0;j<10;j++) {
        int i=dirInode.dirBlock[j];
        if (i==-1){
            continue;
        }
        f.seekg(i,ios::beg);
        f.read((char*)itemList,sizeof(itemList));
        for (auto & j : itemList) {
            if(j.inodeAddr!=-1){
                Inode tmp{};
                f.seekg(j.inodeAddr,ios::beg);
                f.read((char*)&tmp,sizeof(Inode));
                if(tmp.isFile){
                    //删除文件，调用删除文件函数
                    deleteF(j.inodeAddr);
                }  
                else{
                    //递归调用删除目录函数，删除当前子目录
                    if(strcmp(j.name, ".") == 0){
                       continue;
                    }
                    else if(strcmp(j.name, "..") == 0){
                        continue;
                    }
                    else{
                        deleteDir(inodeAddr,j.name);
                    }
                    }
                        
                }
        }
        bfree(i);
        cout<<"Block at "<<i<<" has been released"<<endl;
    }
    if (dirInode.indirBlock!=-1){
        int blocks[256];
        f.seekg(dirInode.indirBlock,ios::beg);
        f.read((char*)blocks,sizeof(blocks));
        for (int block : blocks) {
            if ((block-BLOCK_ADD)%BLOCK_SIZE==0){
                if(block>0){
                    f.seekg(block,ios::beg);
                    f.read((char*)itemList,sizeof(itemList));
                    for (auto & j : itemList) {
                        if(j.inodeAddr>0){
                            Inode tmp{};
                            f.seekg(j.inodeAddr,ios::beg);
                            f.read((char*)&tmp,sizeof(Inode));
                            if(tmp.isFile){
                                //删除文件，调用删除文件函数
                                deleteF(j.inodeAddr);
                            }  
                            else{
                                //递归调用删除目录函数，删除当前子目录
                                if(strcmp(j.name, ".") == 0){
                                    continue;
                                }
                                else if(strcmp(j.name, "..") == 0){
                                    continue;
                                }
                                else{
                                    deleteDir(inodeAddr,j.name);
                                }
                            }
                            
                        }
                    }
                bfree(block);
                cout<<"INBlock at "<<block<<" has been released"<<endl;      
                }
                
            }
        
        }
    }
    ifree(inodeAddr);
    cout<<"The inode at "<<inodeAddr<<" has been released"<<endl;
    cout<<"The directory has been deleted"<< endl;
    return true;
}