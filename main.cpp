
#include "func.h"

using namespace std;

bool preparecmd(string& input, string path){
    cout << path << ": ";
    bool res = true;
    getline(cin, input);
    res = !(input == "exit");
    return res;
}

int main() {
    f.open(DISK_NAME, ios::in | ios::out | ios::binary);
    if (f) {
        cout << "Open the Disk" << endl;
        f.seekg(SUPER_BLOCK_ADD, ios::beg);
        f.read((char *) &superBlock, sizeof(superBlock));

        f.seekg(INODE_BITMAP_ADD, ios::beg);
        f.read((char *) inodeBitMap.data(), inodeBitMap.size() * sizeof(char));
        f.seekg(BLOCK_BITMAP_ADD, ios::beg);
        f.read((char *) blockBitMap.data(), blockBitMap.size() * sizeof(char));
        string input;
//        while (input!="exit"){
//        cin>>input;
//        if (input=="allocate"){
//            for (int i = 0; i <20 ; ++i) {
//                balloc();
//            }
//        } else if (input=="free"){
//            for (int i = 1; i <21 ; ++i) {
//                cout<<bfree(BLOCK_ADD+i*BLOCK_SIZE)<<endl;
//            }
//        } else if (input=="root"){
//            Inode test{};
//            DirItem tmp[32]={0};
//            f.seekg(23552,ios::beg);
//            f.read((char*)&test,sizeof(Inode));
//            cout<<f.good()<<endl;
//            f.seekg(155648,ios::beg);
//            cout<<f.good()<<endl;
//            f.read((char*)tmp,sizeof(tmp));
//            cout<<test.id<<endl;
//            cout<<test.ctime<<endl;
//            cout<<tmp[0].name<<endl;
//        } else if (input=="mkdir"){
//            string name;
//            cin>>name;
//            mkdir(curAddr,name);
//            Inode test{};
//            DirItem tmp[32]={0};
//            f.seekg(curAddr,ios::beg);
//            f.read((char*)&test,sizeof(Inode));
//            cout<<f.good()<<endl;
//            f.seekg(test.dirBlock[0],ios::beg);
//            cout<<f.good()<<endl;
//            f.read((char*)tmp,sizeof(tmp));
//            cout<<test.id<<endl;
//            cout<<test.ctime<<endl;
//            cout<<tmp[0].name<<endl;
//            cout<<tmp[1].name<<endl;
//            cout<<tmp[2].name<<endl;
//        } else if (input=="changeDir"){
//            string name;
//            cin>>name;
//            changeDir(name);
//            Inode test{};
//            DirItem tmp[32]={0};
//            f.seekg(curAddr,ios::beg);
//            f.read((char*)&test,sizeof(Inode));
//            cout<<f.good()<<endl;
//            f.seekg(test.dirBlock[0],ios::beg);
//            cout<<f.good()<<endl;
//            f.read((char*)tmp,sizeof(tmp));
//            cout<<test.id<<endl;
//            cout<<test.ctime<<endl;
//            cout<<tmp[0].name<<endl;
//            cout<<tmp[1].name<<endl;
//            cout<<tmp[2].name<<endl;
//            cout<<tmp[3].name<<endl;
//        }
//        else if (input=="cdl"){//用于测试toolcd和toolchangeDir local
//            int tmpAddr = curAddr;
//            string tmpName = curName;
//
//            string name;
//            cin >> name;
//            toolchangeDir(name, tmpAddr, tmpName);
//
//            cout << "myfileSys# " << tmpName << ">" << endl;
//            Inode test{};
//            DirItem tmp[32]={0};
//            f.seekg(tmpAddr,ios::beg);
//            f.read((char*)&test,sizeof(Inode));
//            cout<<f.good()<<endl;
//            f.seekg(test.dirBlock[0],ios::beg);
//            cout<<f.good()<<endl;
//            f.read((char*)tmp,sizeof(tmp));
//            cout<<test.id<<endl;
//            cout<<test.ctime<<endl;
//            cout<<tmp[0].name<<endl;
//            cout<<tmp[1].name<<endl;
//            cout<<tmp[2].name<<endl;
//        }
//        else if (input=="cdg"){//用于测试toolcd和toolchangeDir global
//
//            string name;
//            cin >> name;
//            toolchangeDir(name, curAddr, curName);
//
//            cout << "myfileSys# " << curName << ">" << endl;
//            Inode test{};
//            DirItem tmp[32]={0};
//            f.seekg(curAddr,ios::beg);
//            f.read((char*)&test,sizeof(Inode));
//            cout<<f.good()<<endl;
//            f.seekg(test.dirBlock[0],ios::beg);
//            cout<<f.good()<<endl;
//            f.read((char*)tmp,sizeof(tmp));
//            cout<<test.id<<endl;
//            cout<<test.ctime<<endl;
//            cout<<tmp[0].name<<endl;
//            cout<<tmp[1].name<<endl;
//            cout<<tmp[2].name<<endl;
//        }
//        else if (input=="cp"){
//            string path1, path2;
//            cin >> path1 >> path2;
//            bool good = false;
//            good = copy(path1, path2);
//            // copy的实现不会改动当前目录。如果要改动可用判断+cd
//            // debug info
//            // 可使用cat验证
//            cout << "debug info" << endl;
//            if (good){
//                cat(path1);
//                cat(path2);
//            }
//        }
//        else if (input=="cat"){
//            string path;
//            cin >> path;
//            cat(path);
//        }
//        else if (input=="dir"){
//            //同行输入
//            //cin getlin可用作为之后的优化参考
//            string path;
//            getline(cin, path);
//            // 分词确定参数数量合法
//            vector<string> args;
//            args = split(path, ' ');
//            if (args.size() > 1){
//                cout << "Invalid #args" << endl;
//            }
//            else{
//                ls(path);
//            }
//        }
//        else if (input=="createFile"){
//            string name;
//            int size;
//            cin>>name>>size;
//            createFile(curAddr,name,size);
//        }
//        else if(input=="deleteDir"){
//            string name;
//            cin>>name;
//            deleteDir(curAddr,name);
//        }
//        else if(input=="deleteFile"){
//            string name;
//            cin>>name;
//            deleteFile(curAddr,name);
//        }
//
//        }//end while
        // while (preparecmd(input, curName) && strcmp(input.c_str(), "exit") != 0) {
        while (preparecmd(input, curName)) {
            if (strcmp(input.c_str(), "") == 0){
                continue;
            }
            vector<string> args;
            args = split(input, ' ');
            //cout << "1";
            if (strcmp("createFile", args[0].c_str()) == 0) {
                if (args.size() != 3) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                vector<string> tmppath;
                string fileName;
                bool relative = true;
                tmppath = split(args[1], '/');
                if (tmppath.empty())
                {
                    cout << "Invalid path/fileName" << endl;
                    continue;
                }
                fileName = tmppath.back();
                if (fileName.length() > 28)
                {
                    cout << "Exceed the maximum length of file name" << endl;
                    continue;
                }
                tmppath.pop_back();
                relative = args[1].at(0) != '/';
                if (!toolchangeDir(tmppath, relative))
                {
                    cout << "Invalid path/filename" << endl;
                    continue;
                }
                createFile(curAddr,fileName,atoi(args[2].c_str()));
            } else if (strcmp("deleteFile", args[0].c_str()) == 0) {
                if (args.size() != 2) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                vector<string> tmppath;
                string fileName;
                bool relative;
                tmppath = split(args[1], '/');
                if (tmppath.empty())
                {
                    cout << "Invalid path/fileName" << endl;
                    continue;
                }
                fileName = tmppath.back();
                tmppath.pop_back();
                relative = args[1].at(0) != '/';
                if (!toolchangeDir(tmppath, relative))
                {
                    cout << "Invalid path/filename" << endl;
                    continue;
                }
                deleteFile(curAddr,fileName);
            } else if (strcmp("createDir", args[0].c_str()) == 0 || strcmp("mkdir", args[0].c_str()) == 0) {
                if (args.size() != 2) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                vector<string> tmppath;
                string fileName;
                bool relative;
                tmppath = split(args[1], '/');
                if (tmppath.empty())
                {
                    cout << "Invalid path/fileName" << endl;
                    continue;
                }
                fileName = tmppath.back();
                tmppath.pop_back();
                relative = args[1].at(0) != '/';
                if (!toolchangeDir(tmppath, relative))
                {
                    cout << "Invalid path/filename" << endl;
                    continue;
                }
                mkdir(curAddr,fileName);
            } else if (strcmp("deleteDir", args[0].c_str()) == 0) {
                if (args.size() != 2) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                vector<string> tmppath;
                string fileName;
                bool relative;
                tmppath = split(args[1], '/');
                if (tmppath.empty())
                {
                    cout << "Invalid path/fileName" << endl;
                    continue;
                }
                fileName = tmppath.back();
                tmppath.pop_back();
                relative = args[1].at(0) != '/';
                if (!toolchangeDir(tmppath, relative))
                {
                    cout << "Invalid path/filename" << endl;
                    continue;
                }
                deleteDir(curAddr,fileName);
            } else if (strcmp("changeDir", args[0].c_str()) == 0 || strcmp("cd", args[0].c_str()) == 0) {
                if (args.size() != 2) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                toolchangeDir(args[1]);
            } else if (strcmp("dir", args[0].c_str()) == 0) {
                if (args.size() != 1) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                ls();
            } else if (strcmp("cp", args[0].c_str()) == 0) {
                if (args.size() != 3) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                copy(args[1],args[2]);
            } else if (strcmp("sum", args[0].c_str()) == 0) {
                if (args.size() != 1) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                printInfo();
            } else if (strcmp("cat", args[0].c_str()) == 0) {
                if (args.size() != 2) {
                    cout << "Invalid input" << endl;
                    continue;
                }
                cat(args[1]);
            }
            //cout << "2";
            // cout<<curName<<": ";
            // getline(cin, input);
        }//end while
    } else {
        cout << "Create the disk" << endl;
        ofstream out(DISK_NAME, ios::binary);
        out.close();
        f.open(DISK_NAME, ios::in | ios::out | ios::binary);
        if (init() && mkroot()) {
            cout << "create success" << endl;
        } else
            cout << "Fail" << endl;
    }
    f.close();
    return 0;
}
