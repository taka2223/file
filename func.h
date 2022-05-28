//
// Created by 94348 on 2022/5/17.
//

#ifndef FILESTREAM_FUNC_H
#define FILESTREAM_FUNC_H

#include "global.h"
#include <cstring>

bool init();

void printInfo();

//block的分配和回收采用分组链接法
//返回分配到的block的地址
int balloc();

//回收成功则为true
bool bfree(int addr);

//inode用顺序分配和回收
//返回分配的inode的地址
int ialloc();
bool ifree(int addr);


bool mkroot();
//检查命名重复
bool repeat(int parent,const string& name,bool isFile);
//创建目录，parent:父目录inode地址，name：新建目录名
bool mkdir(int parent,const string& name);
//支持嵌套（通过cd函数）
bool createDir(string name);

void cd(int parent,const string& name);

void changeDir(string name);//支持嵌套
// deleteDir()

// createFile()
// deleteFile()
// copy() 复制file1到file2，如果file2存在则询问是否继续
bool copy(const string& path1, const string& path2);
// cat()(展示文件内容)
bool cat(const string& path);
//ls （展示所有文件）
bool ls(const string& path);
// utils
// 逐层定位目录的工具cd，行为逻辑同cd
// 添加:无法进行该次定位返回false, 通过引用修改输入的addr和name
bool toolcd(int parent, const string& name, int &l_curAddr = curAddr, string& l_curName = curName);
// void mywrite(int faddr, const char * buffer, int len);
// void myread(int faddr, const char * buffer, int len)
#endif //FILESTREAM_FUNC_H
