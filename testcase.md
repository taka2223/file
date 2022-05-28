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
   /a1/b1
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
