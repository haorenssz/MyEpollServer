#include "TcpServer.h"

#include <string.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <ctime>
#include <fcntl.h>
#include <zconf.h>

TcpServer::TcpServer(int maxWaiter) {
    m_tcpSocket = std::make_shared<TcpSocket>(maxWaiter);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1024);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    m_listen_sockfd = m_tcpSocket->getSockFD();
    m_epfd = epoll_create1(0);

    
    threadPool = std::make_shared<ThreadPool>(THREADPOOL_SIZE);

    signal(SIGALRM, writeLog);

}



void TcpServer::AddConnection() {
    // 建立新连接在主线程
    int connfd = accept(m_listen_sockfd, NULL, NULL);
    if (connfd < 0) {
        throw std::runtime_error("accept new connection error\n");
    }

    std::cout << "a new client comes\n";

    epoll_event ev;
    bzero(&ev, sizeof(ev));


    //设置为ET触发
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = connfd;

    // 注册新的连接事件
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
        throw std::runtime_error("register event error\n");
    }
    setnonblocking(connfd);
}


int TcpServer::setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

bool TcpServer::listen(int _port) {
    return m_tcpSocket->bindPort(_port) && m_tcpSocket->listenOn();
}


void TcpServer::existConnection(int fd) {
    std::cout << "deal existed connection\n";
    
    // 处理已经存在客户端的请求在子线程处理
    threadPool->enqueue([this, fd]() {  
        
        if (this->m_epollEvents[fd].events & EPOLLIN) { 

            char buf[MAX_BUFFER];
            memset(buf, 0, MAX_BUFFER);
            int rc = recv(m_epollEvents[fd].data.fd, buf, MAX_BUFFER, 0);

            std::cout << "rc: " << rc << std::endl;

            if (rc <= 0) 
            {
                if(errno == EAGAIN )
                {
                    std::cout<<"read later\n";

                }
                throw std::runtime_error("recv() error \n");
            }
            //else if(TrigMode==1)
            //ET触发模式
            else 
            {

                std::cout << "receive client message\n";

                time_t rawtime;
                struct tm *timeinfo;
                char buffer[80];
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
                std::string str(buffer);

                std::ofstream stream;
                stream.open(std::string("./") + str);
                stream << buf;
                stream.close();

                const char *msg = "receive your msg\n";
                int ret = send(m_epollEvents[fd].data.fd, msg, strlen(msg), 0);
                if (ret < 0) {
                    throw std::runtime_error("error in send()\n");
                }
            }

        } 
            //LT触发模式
            //else
            /*

            */
        // 客户端主动断开连接
        else if (this->m_epollEvents[fd].events & EPOLLRDHUP) 
        {  
            if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, m_epollEvents[fd].data.fd, NULL) < 0) {
                throw std::runtime_error("delete client error\n");
            }
            std::cout << "a client left\n";
        } 
        // 未知错误
        else 
        {  
            throw std::runtime_error("unknown error");
        }

    });
}


bool TcpServer::startService(int timeout) {
    epoll_event ev;
    bzero(&ev, sizeof(ev));

    ev.data.fd = m_listen_sockfd;
    ev.events = EPOLLIN | EPOLLET;    // 新来的连接
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_listen_sockfd, &ev) < 0) {
        return false;
    }

    // 服务器的监听循环
    while (true) {
        int nfds = epoll_wait(m_epfd, m_epollEvents, MAX_EVENTS, timeout);
        if (nfds < 0) {
            return false;
        }

        for (int i = 0; i < nfds; ++i) {
            if (m_epollEvents[i].data.fd == m_listen_sockfd) {
                // 处理新的连接
                AddConnection();
            } else {
                // 处理已经存在的连接
                existConnection(i);
            }
        }
    }

    return true;
}


void TcpServer::writeLog(int n) {
    const char *buf = "an hour log...\n";

    std::ofstream stream;
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    std::string str(buffer);
    stream.open(std::string("./") + str);
    stream << buf;
    stream.close();

}