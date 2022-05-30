测试应顺序执行

# toolcd与toolchangeDir

测试 对全局变量或局部变量addr/name的改动
假设mkdir正常
创建的目录结构
/下有a1 a2
a1下有b1 b2
命令
mkdir a2
cd ..
mkdir a1
mkdir b1
cd ..
mkdir b2

当前在b2
每次先回到b2
/a1/b2

1. 尝试绝对路径
   正确路径
   /a1/b1cat
   /a1/b1/////
   /a1/b1/../.././a2
   //a1/../ (/)
   /////
   错误路径
   /a3
2. 相对路径
   正确路径
   ../../a2
   ../b1/
   ../b1/.. (a1)
   ../../../..
   错误路径
   c1
   ../../../a3

# createFile

使用上述基础目录树结构（先创建该目录树）
由dir/cat该文件判断

1. 正确指令
   在a1
   createFile a1 20
   createFile f266 266
2. 错误指令
   在a1
   a1 20
   f267 267

# deleteFile

先createFile
在a1
createFile fdel 266
deleteFile fdel
!!! 删除文件的时候有没有对inode做好恢复？dirItem是否没有清空？

# cp

cat target验证

在b1创建文件
createFile fsameNamel 266
createFile fsameNames 5
在a1
createFile fsource 20
createFile fsource2 266

测试

1. cp /a1/fsource /a1/b1/fsameNamel -> n
2. cp /a1/fsource /a1/b1/fsameNamel -> y
3. cp /a1/fsource /a1/b1/fsameNames -> n
4. cp /a1/fsource /a1/b1/fsameNames -> y
5. cp /a1/fsource target (a1)
6. cp /a1/fsource target2
