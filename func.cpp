//
// Created by 94348 on 2022/5/20.
//
#include "func.h"

bool mkroot() {
    Inode root{};
    DirItem itemList[32] = {0};//一个block可以储存32个DirItem
    int inodeAddr = ialloc();
    cout << "inodeAddr: " << inodeAddr << endl;
    int blockAddr = balloc();
    cout << "blockAddr:" << blockAddr << endl;
    if (inodeAddr == -1 || blockAddr == -1) {
        cout << "Allocation error" << endl;
        return false;
    }
    strcpy(itemList[0].name, ".");
    itemList[0].inodeAddr = inodeAddr;
    f.seekp(blockAddr, ios::beg);
    f.write((char *) itemList, sizeof(itemList));

    root.isFile= false;
    root.id = 0;
    time(&root.ctime);
    time(&root.atime);
    time(&root.mtime);
    root.cnt = 1;
    root.isFile= false;
    root.dirBlock[0] = blockAddr;
    root.indirBlock = -1;
    for (int i = 1; i < 10; ++i) {
        root.dirBlock[i] = -1;
    }
    root.size = BLOCK_SIZE;
    f.seekp(inodeAddr, ios::beg);
    f.write((char *) &root, sizeof(root));
    return true;
}

int ialloc() {
    if (superBlock.freeInodeNum <= 0) {
        cout << "No free Inodes" << endl;
        return -1;
    } else {
        int i;
        for (i = 0; i < INODE_NUM; ++i) {
            if (inodeBitMap[i] == 0)break;//空闲inode
        }
        superBlock.freeInodeNum--;
        f.seekp(SUPER_BLOCK_ADD, ios::beg);
        f.write((char *) &superBlock, sizeof(superBlock));

        inodeBitMap[i] = (char) 1;
        f.seekp(INODE_BITMAP_ADD, ios::beg);
        f.write((char *) inodeBitMap.data(), sizeof(char) * inodeBitMap.size());
        return INODE_BLOCK_ADD + i * INODE_SIZE;
    }
}

bool ifree(int addr) {
    if ((addr - INODE_BLOCK_ADD) % INODE_SIZE != 0) {
        cout << "The address is not the beginning of an inode" << endl;
        return false;
    }
    if (superBlock.freeInodeNum == INODE_NUM) {
        cout << "No Inodes need to be free" << endl;
        return false;
    }
    int i = (addr - INODE_BLOCK_ADD) / INODE_SIZE;
    if (inodeBitMap[i] == 0) {
        cout << "The inode " << i << " is not used";
        return false;
    }

    Inode pad = {0};
    f.seekp(addr, ios::beg);
    f.write((char *) &pad, sizeof(pad));

    superBlock.freeInodeNum++;
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *) &superBlock, sizeof(superBlock));

    inodeBitMap[i] = (char) 0;
    f.seekp(INODE_BITMAP_ADD, ios::beg);
    f.write((char *) inodeBitMap.data(), sizeof(char) * inodeBitMap.size());
    return true;
}

int balloc() {
    if (superBlock.freeBlockNum <= 0) {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    int top = (superBlock.freeBlockNum - 1) % BLOCKS_PER_GROUP;
    int blockAddr;
    if (superBlock.free[top] == -1) {
        cout << "No Free Blocks" << endl;
        return -1;
    }
    if (top == 0) {
        //栈底，更新freeAddr
        blockAddr = superBlock.free[top];
        superBlock.freeAddr = superBlock.free[top];
        f.seekg(superBlock.freeAddr, ios::beg);
        f.read(reinterpret_cast<char *>(superBlock.free), sizeof(superBlock.free));
    } else {
        blockAddr = superBlock.free[top];
    }
    superBlock.freeBlockNum--;
    //更新superblock
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *) &superBlock, sizeof(superBlock));
    //更新bitmap
    int i = (blockAddr - BLOCK_ADD) / BLOCK_SIZE;
    cout << "Block" << i << " is allocated" << endl;
    blockBitMap[i] = (char) 1;
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *) blockBitMap.data(), blockBitMap.size() * sizeof(char));
    return blockAddr;
}

bool bfree(int addr) {
    if ((addr - BLOCK_ADD) % BLOCK_SIZE != 0) {
        cout << "The address is not the beginning of a block" << endl;
        return false;
    }
    if (superBlock.freeBlockNum == BLOCK_NUM) {
        cout << "No blocks need to be free" << endl;
        return false;
    }
    int i = (addr - BLOCK_ADD) / BLOCK_SIZE;
    if ((int) blockBitMap[i] == 0) {
        cout << "The block is not used" << endl;
        return false;
    }

    vector<char> pad(BLOCK_SIZE, 0);
    f.seekp(addr, ios::beg);
    f.write((char *) pad.data(), pad.size() * sizeof(char));
    int top = (superBlock.freeBlockNum - 1) % BLOCKS_PER_GROUP;
    if (top == BLOCKS_PER_GROUP - 1) {
        //将空闲块堆栈内容写入新释放的块
        f.seekp(addr, ios::beg);
        f.write((char *) &superBlock.free, sizeof(superBlock.free));
        //更新空闲块堆栈
        for (int j = 1; j < BLOCKS_PER_GROUP; ++j) {
            superBlock.free[j] = -1;
        }
    } else {
        top++;
        superBlock.free[top] = addr;
    }
    superBlock.freeBlockNum++;
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *) &superBlock, sizeof(superBlock));

    blockBitMap[i] = (char) 0;
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *) blockBitMap.data(), sizeof(char) * blockBitMap.size());
    return true;
}

void printInfo() {
    cout << "Number of Free Inode: " << superBlock.freeInodeNum << "/" << INODE_NUM << endl;
    cout << "Number of Free Block: " << superBlock.freeBlockNum << "/" << BLOCK_NUM << endl;
    for (int i = 0; i < blockBitMap.size(); ++i) {
        if (i % BLOCKS_PER_GROUP == 0)cout << endl;
        if (blockBitMap[i] == 1)cout << "*";
        else cout << "-";
    }
}

bool init() {
    superBlock.freeBlockNum = BLOCK_NUM;
    superBlock.freeInodeNum = INODE_NUM;
    superBlock.freeAddr = BLOCK_ADD;
    for (int i = (BLOCK_NUM / BLOCKS_PER_GROUP) - 1; i >= 0; --i) {
        cout << i << endl;
        if (i == (BLOCK_NUM / BLOCKS_PER_GROUP) - 1) {
            //最后一组
            superBlock.free[0] = -1;
        } else {
            superBlock.free[0] = BLOCK_ADD + (i + 1) * BLOCK_SIZE * BLOCKS_PER_GROUP;
        }
        for (int j = 1; j < BLOCKS_PER_GROUP; ++j) {
            superBlock.free[j] = BLOCK_ADD + ((i + 1) * BLOCKS_PER_GROUP - j) * BLOCK_SIZE;
        }
        f.seekp(BLOCK_ADD + i * BLOCK_SIZE * BLOCKS_PER_GROUP, ios::beg);
        f.write((char *) &superBlock.free, sizeof(superBlock.free));
        cout << "good?" << f.good() << endl;
    }
    f.seekp(SUPER_BLOCK_ADD, ios::beg);
    f.write((char *) &superBlock, sizeof(superBlock));

    f.seekp(INODE_BITMAP_ADD, ios::beg);
    f.write((char *) inodeBitMap.data(), inodeBitMap.size() * sizeof(char));
    f.seekp(BLOCK_BITMAP_ADD, ios::beg);
    f.write((char *) blockBitMap.data(), blockBitMap.size() * sizeof(char));

    return true;
}

bool mkdir(int parent, const string &name) {
    if (name.length() > 28) {
        cout << "Exceed the maximum length of file name" << endl;
        return false;
    }
    if (repeat(parent,name, false)){
        return false;
    }
    int pos1=-1,pos2=-1;
    Inode parentInode{};
    f.seekg(parent,ios::beg);
    f.read((char*)&parentInode,sizeof(Inode));
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
        Inode child{};
        DirItem items[32] = {0};//一个block可以储存32个DirItem
        int inodeAddr = ialloc();
        cout << "inodeAddr: " << inodeAddr << endl;
        int blockAddr = balloc();
        cout << "blockAddr:" << blockAddr << endl;
        if (inodeAddr == -1 || blockAddr == -1) {
            cout << "Allocation error" << endl;
            return false;
        }
        strcpy(items[0].name,".");
        strcpy(items[1].name,"..");
        items[0].inodeAddr = inodeAddr;
        items[1].inodeAddr = parent;

        child.id = (inodeAddr-INODE_BLOCK_ADD)/INODE_SIZE;
        time(&child.ctime);
        time(&child.atime);
        time(&child.mtime);
        child.cnt=2;
        f.seekp(blockAddr,ios::beg);
        f.write((char*)items,sizeof(items));
        child.dirBlock[0]=blockAddr;
        for (int k = 1; k <10 ; ++k) {
            child.dirBlock[k]=-1;
        }
        f.seekp(inodeAddr,ios::beg);
        f.write((char*)&child,sizeof(Inode));
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

    }

    parentInode.cnt++;
    f.seekp(parent,ios::beg);
    f.write((char*)&parentInode,sizeof(parentInode));
    return true;
}

bool repeat(int parent, const string &name, bool isFile) {
    Inode parentInode{};
    f.seekg(parent,ios::beg);
    f.read((char*)&parentInode,sizeof(Inode));
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
                if (tmp.isFile==isFile){
                    cout<<"The file or directory is existed"<<endl;
                    return true;
                }
            }
        }
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
                        if (tmp.isFile==isFile){
                            cout<<"The file or directory is existed"<<endl;
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool createDir(string name) {
    return false;
}

void cd(int parent,const string& name) {//当前目录(parent)下的name目录
    Inode parentInode{};
    f.seekg(parent,ios::beg);
    f.read((char*)&parentInode,sizeof(Inode));
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
                    if (strcmp(j.name,".")==0){
                        return;
                    } else if (strcmp(j.name,"..")==0){
                        int k;
                        for (k = strlen(curName.c_str()); k >=0 ; k--) {
                            if (curName[k]=='/')break;
                        }
                        curName = curName.substr(0,k);
                        return;
                    } else
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
                            if (strcmp(j.name,".")==0){
                                return;
                            } else if (strcmp(j.name,"..")==0){
                                int k;
                                for (k = strlen(curName.c_str()); k >=0 ; k--) {
                                    if (curName[k]=='/')break;
                                }
                                curName = curName.substr(0,k);
                                return;
                            } else
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
    }

void changeDir(string name) {
    for (int i = 0; i <strlen(name.c_str()) ; ++i) {
        if (name[i]=='/'){
            string tmp = name.substr(0,i);
            cd(curAddr,tmp);
            name = name.substr(i+1);
            i=0;
        }
    }
    cd(curAddr,name);
}
