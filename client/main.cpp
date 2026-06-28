#include "client.h"
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>



int main(int argc, const char *argv[]){

    //对输入的服务器ip地址进行判断
    if (argc < 2) {
        cout << "请输入ip地址" << endl;
        return -1;
    }

    //实例化对象
    try {
        TFTPClient client(argv[1]);
        //执行客户端p
        client.run();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return -1;
    }
    
    
    return 0;

}