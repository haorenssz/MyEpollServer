#ifndef SERVER_TCPSERVER_H
#define SERVER_TCPSERVER_H

#include "TcpSocket.h"
#include "User_Config.h"
#include "ThreadPool.h"

#include <sys/epoll.h>
#include <netinet/ip.h>
#include <memory>
#include <vector>



class TcpServer {
public:
    explicit TcpServer(int maxWaiter = 5);
    
    //bind()+lisent()
    bool listen(int _port = 8001);

    // 处理新的连接函数
    void AddConnection();

    // 处理已经存在的连接的函数
    void existConnection(int sockfd);

    // 文件描述符设为非阻塞
    int setnonblocking(int fd);

    // 在这里进入事件的主循环，可以自行设置时间
    bool startService(int timeout = -1);


    // 每隔一段时间写一次日志
    static void writeLog(int n);
    // 定时器，间隔一定时间记录一次日志


protected:
    // 服务器信息
    struct sockaddr_in serv_addr; 
    // 指向Socket的指针
    std::shared_ptr<TcpSocket> m_tcpSocket;
    // epoll事件队列
    epoll_event m_epollEvents[MAX_EVENTS];  
    // epoll的fd
    int m_epfd;   

    int m_listen_sockfd;
    //线程池 初始为6
    std::shared_ptr<ThreadPool> threadPool;  
};


#endif 
