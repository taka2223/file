
#include "func.h"
using namespace std;

int main() {
    f.open(DISK_NAME,ios::in|ios::out|ios::binary);
    if (f){
        cout<<"Open the Disk"<<endl;
        f.seekg(SUPER_BLOCK_ADD,ios::beg);
        f.read((char*)&superBlock,sizeof(superBlock));

        f.seekg(INODE_BITMAP_ADD,ios::beg);
        f.read((char*)inodeBitMap.data(),inodeBitMap.size()*sizeof(char));
        f.seekg(BLOCK_BITMAP_ADD,ios::beg);
        f.read((char*)blockBitMap.data(),blockBitMap.size()*sizeof(char));
        printInfo();
        string input;
        while (input!="exit"){
        cin>>input;
        if (input=="allocate"){
            for (int i = 0; i <20 ; ++i) {
                balloc();
            }
        } else if (input=="free"){
            for (int i = 1; i <21 ; ++i) {
                cout<<bfree(BLOCK_ADD+i*BLOCK_SIZE)<<endl;
            }
        } else if (input=="root"){
            Inode test{};
            DirItem tmp[32]={0};
            f.seekg(23552,ios::beg);
            f.read((char*)&test,sizeof(Inode));
            cout<<f.good()<<endl;
            f.seekg(155648,ios::beg);
            cout<<f.good()<<endl;
            f.read((char*)tmp,sizeof(tmp));
            cout<<test.id<<endl;
            cout<<test.ctime<<endl;
            cout<<tmp[0].name<<endl;
        } else if (input=="mkdir"){
            string name;
            cin>>name;
            mkdir(curAddr,name);
            Inode test{};
            DirItem tmp[32]={0};
            f.seekg(curAddr,ios::beg);
            f.read((char*)&test,sizeof(Inode));
            cout<<f.good()<<endl;
            f.seekg(test.dirBlock[0],ios::beg);
            cout<<f.good()<<endl;
            f.read((char*)tmp,sizeof(tmp));
            cout<<test.id<<endl;
            cout<<test.ctime<<endl;
            cout<<tmp[0].name<<endl;
            cout<<tmp[1].name<<endl;
            cout<<tmp[2].name<<endl;
        } else if (input=="changeDir"){
            string name;
            cin>>name;
            changeDir(name);
            Inode test{};
            DirItem tmp[32]={0};
            f.seekg(curAddr,ios::beg);
            f.read((char*)&test,sizeof(Inode));
            cout<<f.good()<<endl;
            f.seekg(test.dirBlock[0],ios::beg);
            cout<<f.good()<<endl;
            f.read((char*)tmp,sizeof(tmp));
            cout<<test.id<<endl;
            cout<<test.ctime<<endl;
            cout<<tmp[0].name<<endl;
            cout<<tmp[1].name<<endl;
            cout<<tmp[2].name<<endl;
            cout<<tmp[3].name<<endl;
        }
        else if (input=="cdl"){//用于测试toolcd和toolchangeDir local
            int tmpAddr = curAddr;
            string tmpName = curName;

            string name;
            cin >> name;
            toolchangeDir(name, tmpAddr, tmpName);

            cout << "myfileSys# " << tmpName << ">" << endl;
            Inode test{};
            DirItem tmp[32]={0};
            f.seekg(tmpAddr,ios::beg);
            f.read((char*)&test,sizeof(Inode));
            cout<<f.good()<<endl;
            f.seekg(test.dirBlock[0],ios::beg);
            cout<<f.good()<<endl;
            f.read((char*)tmp,sizeof(tmp));
            cout<<test.id<<endl;
            cout<<test.ctime<<endl;
            cout<<tmp[0].name<<endl;
            cout<<tmp[1].name<<endl;
            cout<<tmp[2].name<<endl;
        }
        else if (input=="cdg"){//用于测试toolcd和toolchangeDir global

            string name;
            cin >> name;
            toolchangeDir(name, curAddr, curName);

            cout << "myfileSys# " << curName << ">" << endl;
            Inode test{};
            DirItem tmp[32]={0};
            f.seekg(curAddr,ios::beg);
            f.read((char*)&test,sizeof(Inode));
            cout<<f.good()<<endl;
            f.seekg(test.dirBlock[0],ios::beg);
            cout<<f.good()<<endl;
            f.read((char*)tmp,sizeof(tmp));
            cout<<test.id<<endl;
            cout<<test.ctime<<endl;
            cout<<tmp[0].name<<endl;
            cout<<tmp[1].name<<endl;
            cout<<tmp[2].name<<endl;
        }
        else if (input=="cp"){
            string path1, path2;
            cin >> path1 >> path2;
            bool good = false;
            good = copy(path1, path2);
            // copy的实现不会改动当前目录。如果要改动可用判断+cd
            // debug info
            // 可使用cat验证
            cout << "debug info" << endl;
            if (good){
                cat(path1);
                cat(path2);
            }
        }
        else if (input=="cat"){
            string path;
            cin >> path;
            cat(path);
        }
        else if (input=="dir"){
            //同行输入
            //cin getlin可用作为之后的优化参考
            string path;
            getline(cin, path);
            // 分词确定参数数量合法
            vector<string> args;
            args = split(path, ' ');
            if (args.size() > 1){
                cout << "Invalid #args" << endl;
            }
            else{
                ls(path);
            }
        }

        
        }//end while
        
    } else{
        cout<<"Create the disk"<<endl;
        ofstream out(DISK_NAME,ios::binary);
        out.close();
        f.open(DISK_NAME,ios::in|ios::out|ios::binary);
        if (init()&&mkroot()){
            cout<<"create success"<<endl;
        } else
            cout<<"Fail"<<endl;
    }
    printInfo();
    f.close();

    return 0;
}
