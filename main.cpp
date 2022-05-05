#include <iostream>
#include "func.h"
int main(int, char **)
{
    // SuperBlock superBlock;
    // f.open(DISK_NAME, ios::binary);
    // if (!f)
    // {
    //     cout << "Create the disk" << endl;
    //     f.open(DISK_NAME, ios::binary);
    //     if (f)
    //     {
    //         if (superBlock.init())
    //         {
    //             cout << "create success" << endl;
    //             f << superBlock;
    //         }
    //         else
    //             cout << "Failed to create disk" << endl;
    //     }
    //     f.close();
    // }
    // else
    // {
    //     f.open(DISK_NAME, ios::binary | ios::app);
    //     cout << "Disk has existed" << endl;
    //     f >> superBlock;
    //     f.seekg(superBlock.s_InodeBitmap_StartAddr, ios::beg);
    //     f >> inodeBitMap;
    //     f.seekg(superBlock.s_BlockBitmap_StartAddr, ios::beg);
    //     f >> blockBitMap;

    //     for (size_t i = 0; i < 200; i++)
    //     {
    //         balloc(superBlock);
    //     }
    // }
    // superBlock.printInfo();

    // BlockInfo(superBlock);
    fstream f;
    f.open("test.txt", ios::binary | ios::in);
    int a, b, c, pos = 0;
    if (!f)
    {
        a = 20, b = 30, c = 40;
        f.open("test.txt", ios::binary | ios::out);
        f.seekp(pos, ios::beg);
        f.write((char *)&a, sizeof(int));
        f.seekp(pos + sizeof(int), ios::beg);
        f.write((char *)&b, sizeof(int));
        f.seekp(pos + 2 * sizeof(int), ios::beg);
        f.write((char *)&c, sizeof(int));
    }
    else
    {
        f.open("test.txt", ios::binary | ios::in);
        f.seekg(pos, ios::beg);
        f.read((char *)&a, sizeof(int));
        f.seekg(pos + sizeof(int), ios::beg);
        f.read((char *)&b, sizeof(int));
        f.seekg(pos + 2 * sizeof(int), ios::beg);
        f.read((char *)&c, sizeof(int));
        cout << a << ' ' << b << ' ' << c << endl;
        a++;
        b++;
        c++;
        f.open("test.txt", ios::binary | ios::out);
        f.seekp(pos, ios::beg);
        f.write((char *)&a, sizeof(int));
        f.seekp(pos + sizeof(int), ios::beg);
        f.write((char *)&b, sizeof(int));
        f.seekp(pos + 2 * sizeof(int), ios::beg);
        f.write((char *)&c, sizeof(int));
    }

    f.close();
    cout << "close" << endl;
    return 0;
}
