#include "client.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>




TFTPClient::TFTPClient(const string &serverIP){

    //创建udp套接字
    this->cfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (cfd < 0) {
        ERR_LOG("sock error");
        return;
    }
    this->server_addr = {.sin_family = AF_INET,
                        .sin_port = htons(PORT),
                        .sin_addr = {.s_addr = inet_addr(serverIP.c_str())}};
                        //string.c_str()转换为c风格的字符串

}

TFTPClient::~TFTPClient(){
    //如果套接字没被关闭,则关闭
    if (cfd > 0) {
        close(cfd);
    }
}


void TFTPClient::run(){

    while (1) {
        //展示菜单
        showMenu();

        char choice;    //菜单选项
        cin >> choice;  //输入选项
        waitForInput(); //吸收垃圾字符

        switch (choice) {
        case '1':
            doDownload();
        break;
        case '2':
            doUpload();
        break;
        case '3':
        return;
        default:
            cout << "输入有误,重新输入" << endl;
        break;
        }

        clearScreen();  //清屏

    }

}
//下载
int TFTPClient::doDownload(){

    //封装读写请求包
    //操作码|下载文件名|'\0'|"octet"/"netascii"|'\0'
    string filename;    //要下载的文件名
    cout << "请输入要下载的文件的文件名" <<endl;
    //cin >> filename;    //这种只能输入没有空格的字符串,其实和get_line一样,一般文件都没有空格
    getline(cin, filename);
    
    //封装下载请求
    char buf[BUFFER_SIZE] = "";
    //这里涉及网络存储(大端/小端)
    int buf_size = sprintf(buf, "%c%c%s%c%s%c", 0, 1, filename.c_str(), 0, "octet",0);
    //但是这里好像又没有解决filename带空格的问题?
    //向服务器端发送下载请求
    if(sendto(cfd, buf, buf_size, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        ERR_LOG("sendto error");
        return -1;
    }

    //等待服务器应答(有文件:发 无文件:退出)
    //循环接收服务器发来的数据包
    size_t recv_len;    //传输完成/传输失败的依据
    unsigned short num = 1; //这个是用来标记包的块编号
    socklen_t addrlen = sizeof(server_addr);    //send传入的地址

    //定义有关文件操作的变量
    int flag = 0;   //标识文件是否被打开
    int fd = -1;    //文件描述符
    
    while (1) {
        //清空
        bzero(buf, BUFFER_SIZE);

        recv_len = recvfrom(cfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addrlen);
        if (recv_len < 0) {
            ERR_LOG("recvfrom error");
            return -1;
        }

        //判断是否为数据包
        if (buf[1] != 3) {
            //非数据包
            if (buf[1] == 5) {
                //错误
                cout << "____error" << buf+4 << "____" << endl;
            }
            if (flag == 1) {
                close(fd);
            }
            break;
        }

        //数据包,读取数据
        //因为我们下载的逻辑是:服务器上的文件拷贝到本地,所以是往本地文件写内容:所以需要打开文件操作
        if (0 == flag) {
            fd = open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0664);
            if (fd < 0) {
                ERR_LOG("loacl open file error");
                return -1;
            }
            flag = 1;

        }
        //我客户端请求文件,文件中有很多文件,这些复制是有顺序的吗,不然可能会导致多个文件争抢fd
        //判断数据块编号
        if (htons(num) == *(unsigned short*)(buf+2)) {
            //表示传过来的是第一个数据包,写入文件
            //现在还不是num++的时候,要先回复ACK===其实不然,看下面ACK注释
            num++;  //数据块编号自增
#ifdef DEBUG
            cout << "fd = " << fd << "recv_len = " << recv_len << endl;
#endif
            if(write(fd, buf+4, recv_len-1) < 0){
                ERR_LOG("write error");
                close(fd);
                break;
            }
        }
        //成功读取数据块,返回ACK
        //组装ACK包发送给服务器端
        //这里有个小细节,这时候我们的buf是从服务器端拿过来的,所以buf的低2-4个字节天然就是数据块编号,不用再重新赋值,很巧妙

        buf[1] = 4;
        if(sendto(cfd, buf, 4, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
            ERR_LOG("sendto error");
            close(fd);
            break;
        }

        //为什么最后才判断?因为即使recv_len<BUFFER_SIZE,此时也已经下载成功了...
        if (recv_len < BUFFER_SIZE) {
            //该数据块是最后一块了
            cout << "Doload success" << endl;
            close(fd);
            break;
        }
        
    }
    return 0;
}
    
//上传
int TFTPClient::doUpload(){
    
    return 0;
}

//清屏
void TFTPClient::clearScreen(){

    cout << "请输入任意字符进行清屏";
    while (getchar() != '\n');


}

//等待输入函数(吸收垃圾字符函数)
void TFTPClient::waitForInput(){
    while (getchar() != '\n');
}

//展示菜单函数
void TFTPClient::showMenu(){
    system("clear");
    cout << "=============基于UDP的TFTP文件传输===============" << endl;
    cout << "=================1、下载=======================" << endl;
    cout << "=================2、上传=======================" << endl;
    cout << "=================3、退出=======================" << endl;
    cout << "==============================================" << endl;

}