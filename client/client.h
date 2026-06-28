#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <system_error>

#define DEBUG

//用于打印错误信息的宏
#define ERR_LOG(msg)    do{\
    perror(msg);\
    cout << __LINE__ << " " << __func__ << " " << __FILE__ << endl;\
}while(0);

using namespace std;


//封装一个客户端类

class TFTPClient{

private:
    static const int PORT = 69;         //服务器端口号
    static const int BUFFER_SIZE = 516; //使用数据包,可以通过内容换成其他报文格式
                                        //操作码2B,块编号2B,数据0~512B
    int cfd;
    struct sockaddr_in server_addr;     //服务器地址信息结构体

    //该客户端提供的私有的成员函数
    int doDownload();   //下载
    int doUpload();     //上传
    void clearScreen();   //清屏
    void waitForInput(); //等待输入函数
    void showMenu();      //展示菜单函数

public:
    //构造函数
    TFTPClient(const string &serverIP);
    //析构函数
    ~TFTPClient();
    //客户端执行函数
    void run();
    
};



















#endif